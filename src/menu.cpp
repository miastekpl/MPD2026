// ============================================================
// TrassarV3 - System menu - rdzen
// Inicjalizacja, dyspozycja zdarzen, renderowanie
// Handlery per ekran: menu_handlers.cpp
// ============================================================

#include "menu.h"
#if HAS_SMALL_TFT
#include "display_manager.h"
#endif
#include "dgus/dgus_link.h"
#include "patterns.h"
#include "encoder_distance.h"
#include "painting_engine.h"
#include "statistics.h"
#include "guns.h"
#include "button_handler.h"
#include "joystick.h"
#include "report_logger.h"
#include "buzzer.h"
#include "gps_track.h"
#include "paint_consumption.h"
#include <esp_task_wdt.h>
#include <cmath>

MenuSystem menu;

// ============ Inicjalizacja ============

void MenuSystem::begin() {
    STATE_LOCK();
    g_state.currentScreen = SCREEN_HOME;
    g_state.menuIndex = 0;
    g_state.displayNeedsUpdate = true;
    g_state.forceFullRedraw = true;
    STATE_UNLOCK();
}

// ============ Przejście między ekranami ============

void MenuSystem::goToScreen(ScreenID screen) {
    ScreenID prevScreen;
    STATE_LOCK();
    prevScreen = g_state.currentScreen;
    STATE_UNLOCK();

    // Upewnij sie ze pistolety sa wylaczone przy wyjsciu z czyszczenia
    if (prevScreen == SCREEN_NOZZLE_CLEAN) {
        guns.allOff();
    }
    // Zwolnij staly ton alarmu dystansu przy wyjsciu - inaczej zostalby wlaczony na zawsze
    // (petla "reassert" w update() dziala tylko, gdy ekran jest aktywny)
    if (prevScreen == SCREEN_DISTANCE_METER && buzzer.isHolding()) {
        buzzer.releaseTone();
    }

    STATE_LOCK();
    g_state.currentScreen = screen;
    g_state.menuIndex = 0;
    g_state.displayNeedsUpdate = true;
    g_state.forceFullRedraw = true;
    STATE_UNLOCK();

    lastScreenChangeMs = millis();
    // Blokuj osie joysticka dopoki nie wroci do centrum —
    // zapobiega szumowi ADC2 (WiFi) generujacemu falszywe zdarzenia na nowym ekranie
#if HAS_JOYSTICK
    joystick.requireCenter();
#endif
}

void MenuSystem::setDistMeterTarget(float meters) {
    if (isnan(meters) || isinf(meters)) return;
    if (meters < 0) meters = 0;
    if (meters > 9999) meters = 9999;
    distMeterTarget = roundf(meters * 10.0f) / 10.0f;   // zaokraglenie do 0.1 m
    // Nowy cel - zdejmij zatrzask i wygas ewentualny staly ton, zeby alarm liczyl sie od nowa
    if (distMeterReached) {
        distMeterReached = false;
        if (buzzer.isHolding()) buzzer.releaseTone();
    }
}

// ============ Dyspozycja zdarzeń ============

void MenuSystem::handleEvent(ButtonEvent event) {
    if (event == EVT_NONE) return;

    STATE_LOCK();
    ScreenID screen = g_state.currentScreen;
    STATE_UNLOCK();

    switch (screen) {
        case SCREEN_HOME:           handleHomeScreen(event);      break;
        case SCREEN_PAINTING:       handlePaintingScreen(event);  break;
        case SCREEN_SERVICE_MENU:   handleServiceMenu(event);     break;
        case SCREEN_CALIBRATION:    handleCalibration(event);     break;
        case SCREEN_DISTANCE_METER: handleDistanceMeter(event);   break;
        case SCREEN_REPORTS:        handleReports(event);         break;
        case SCREEN_NOZZLE_CLEAN:   handleNozzleClean(event);     break;
        case SCREEN_SETUP:          handleSetup(event);           break;
        case SCREEN_SESSION_RESET:  handleSessionReset(event);    break;
        case SCREEN_COUNTER_RESET:  handleCounterReset(event);    break;
        case SCREEN_SUMMARY:        handleSummary(event);         break;
        case SCREEN_LIFETIME_STATS: handleLifetimeStats(event);   break;
        case SCREEN_CUSTOM_PATTERN: handleCustomPattern(event);   break;
        case SCREEN_STATS_EXPORT:   handleStatsExport(event);     break;
        case SCREEN_FACTORY_RESET:  handleFactoryReset(event);    break;
        case SCREEN_TANKOWANIE:     handleTankowanie(event);      break;
        case SCREEN_POST:           handlePost(event);            break;
    }
}

// ============ Renderowanie + logika ciagla ============

void MenuSystem::update() {
    // Odczytaj stan ekranu pod lockiem (Core 0 moze czytac rownoczesnie)
    STATE_LOCK();
    ScreenID curScreen = g_state.currentScreen;
    STATE_UNLOCK();

    // --- Logika ciagla: pomiar dystansu + alarm przy zadanym celu ---
    if (curScreen == SCREEN_DISTANCE_METER && distMeasuring) {
        float current = encoderDist.getDistanceMeters();
        float delta = current - distMeterLast;
        distMeterLast = current;
        if (delta > 0) distMeterValue += delta;

        uint32_t now = millis();
        if (distMeterTarget > 0) {
            if (!distMeterReached && distMeterValue >= distMeterTarget) {
                // Cel osiagniety/przekroczony - zatrzask, staly ton (dopoki ZERUJ)
                distMeterReached = true;
                buzzer.holdTone(1500);
                distMeterHoldMs = now;
            } else if (distMeterReached) {
                // Odswiezaj staly ton periodycznie - odzyskuje buzzer, gdyby cos innego
                // (np. BUZ_ERROR z innego modulu) chwilowo go przejelo.
                if (now - distMeterHoldMs >= DIST_METER_HOLD_REASSERT_MS) {
                    buzzer.holdTone(1500);
                    distMeterHoldMs = now;
                }
            } else {
                float remaining = distMeterTarget - distMeterValue;
                if (remaining <= DIST_METER_WARN_MARGIN_M &&
                    now - distMeterWarnBeepMs >= DIST_METER_WARN_BEEP_MS) {
                    buzzer.play(BUZ_DIST_WARN);
                    distMeterWarnBeepMs = now;
                }
            }
        }
        STATE_LOCK();
        g_state.displayNeedsUpdate = true;
        STATE_UNLOCK();
    }

    // --- Logika ciagla: czyszczenie dysz ---
    if (curScreen == SCREEN_NOZZLE_CLEAN) {
        // Fizyczny START, przycisk "martwego czlowieka" na ekranie DGUS (wygasa przy utracie
        // lacza) albo z panelu WWW/Sunton (wwwNozzleHoldOn — twardy limit NOZZLE_HOLD_MAX_MS
        // na wypadek zgubienia sygnalu "puszczono", patrz control_api.cpp).
        bool wwwHeld;
        STATE_LOCK();
        wwwHeld = g_state.wwwNozzleHoldOn &&
                  (uint32_t)(millis() - g_state.wwwNozzleHoldSetMs) < NOZZLE_HOLD_MAX_MS;
        STATE_UNLOCK();
        bool held = buttons.isStartHeld() || dgusLink.isHoldActive() || wwwHeld;
        const PatternDef& pat = patternMgr.getPattern((PatternID)nozzlePatternIdx);
        for (int i = 0; i < NUM_GUNS; i++) {
            bool active = (pat.guns[i].mode != GUN_OFF);
            guns.setGun((GunID)i, held && active);
        }
        // Wymusz odswiezanie zeby pokazac stan pistoletow
        g_state.displayNeedsUpdate = true;
    }

#if !HAS_SMALL_TFT
    // Wariant bez ILI9341: ekrany opisuje dgus_pages.cpp, wysyla je dgus_link.cpp
    return;
#else
    // --- Renderowanie ---
    STATE_LOCK();
    bool needsUpdate = g_state.displayNeedsUpdate;
    ScreenID preScreen = g_state.currentScreen;
    STATE_UNLOCK();
    if (!needsUpdate) return;

    // Fix #24: Pobierz dane z SD PRZED zablokowaniem mutexu SPI.
    // getReportCount() i getLastReport() uzywaja SD_LOCK() wewnetrznie —
    // wczesniej byly wolane WEWNATRZ mutexu SD (w switch/case SCREEN_REPORTS),
    // co powodowalo deadlock (nie-rekursywny mutex) i 4s blokade → WDT reset!
    int cachedReportCount = 0;
    char cachedLastReport[128] = {};
    if (preScreen == SCREEN_REPORTS) {
        cachedReportCount = reportLogger.getReportCount();
        reportLogger.getLastReport(cachedLastReport, sizeof(cachedLastReport));
    }

    // SPI wspoldzielone: TFT i SD na tej samej magistrali HSPI.
    // Probuj zablokowac SPI z retry — jesli SD jest zajete (Core 0 pisze GPX/raport),
    // ponow probe do TFT_SD_MUTEX_RETRIES razy, potem pomin klatke.
    if (g_sdMutex) {
        bool acquired = false;
        for (int attempt = 0; attempt <= TFT_SD_MUTEX_RETRIES; attempt++) {
            if (xSemaphoreTake(g_sdMutex, pdMS_TO_TICKS(TFT_SD_MUTEX_TIMEOUT_MS)) == pdTRUE) {
                acquired = true;
                break;
            }
        }
        if (!acquired) return;  // SD zajete zbyt dlugo, sprobuj w nastepnej klatce
    }
    // Deselect SD przed operacjami TFT
    digitalWrite(PIN_SD_CS, HIGH);

    STATE_LOCK();
    bool fullRedraw = g_state.forceFullRedraw;
    g_state.displayNeedsUpdate = false;
    g_state.forceFullRedraw = false;
    curScreen = g_state.currentScreen;
    STATE_UNLOCK();

    // Pelne czyszczenie tylko przy zmianie ekranu (eliminacja migania)
    if (fullRedraw) {
        display.clear();
    }

    // Fix #24: WDT reset przed renderowaniem — clear() + mutex wait mogly zuzyc
    // znaczna czesc budgetu WDT. Reset tutaj daje pelne 5s na rendering ekranu.
    esp_task_wdt_reset();

    switch (curScreen) {

        // ---- Ekran glowny ----
        case SCREEN_HOME: {
            const PatternDef& pat = patternMgr.getCurrent();
            STATE_LOCK();
            bool reversed = g_state.patternReversed;
            STATE_UNLOCK();
            display.drawHomeScreen(
                pat.code,
                pat.name,
                encoderDist.getSpeedKmh(),
                stats.getSessionArea(),
                pat.guns,
                reversed,
                pat.hasReverse
            );
            break;
        }

        // ---- Ekran malowania ----
        case SCREEN_PAINTING: {
            const PatternDef& pat = patternMgr.getCurrent();
            STATE_LOCK();
            MachineState mState = g_state.machineState;
            bool reversed = g_state.patternReversed;
            STATE_UNLOCK();
            bool gunStates[6];
            for (int i = 0; i < NUM_GUNS; i++) {
                gunStates[i] = guns.getState(i);
            }
            display.drawPaintingScreen(
                mState,
                pat.code,
                encoderDist.getSpeedKmh(),
                stats.getSessionArea(),
                pat.guns,
                gunStates,
                reversed,
                paintEngine.isGapStart(),
                paintEngine.isOverspeed(),
                paintEngine.isLowSpeed(),
                stats.getSessionTimeSec(),
                stats.getSessionDistance(),
                paintEngine.getPatternDistance(),
                paintEngine.isWaitingForMovement()
            );
            // Ikona SD warning na ekranie malowania
            if (!reportLogger.isReady()) {
                display.drawSdWarningIcon();
            }
            // Ikona GPS overflow na ekranie malowania
            if (gpsTrack.isOverflowed()) {
                display.drawGpsOverflowIcon();
            }
            break;
        }

        // ---- Menu serwisowe ----
        case SCREEN_SERVICE_MENU: {
            STATE_LOCK();
            int menuIdx = g_state.menuIndex;
            STATE_UNLOCK();
            display.drawServiceMenu(menuIdx);
            break;
        }

        // ---- Kalibracja ----
        case SCREEN_CALIBRATION:
            display.drawCalibrationScreen(
                encoderDist.isCalibrating(),
                encoderDist.getCalibrationPulses(),
                encoderDist.getPulsesPerMeter(),
                encoderDist.isCalibrated()
            );
            break;

        // ---- Pomiar dystansu ----
        case SCREEN_DISTANCE_METER:
            display.drawDistanceMeter(distMeterValue, distMeasuring);
            break;

        // ---- Raporty ----
        // Fix #24: Dane raportow pobrane PRZED mutex SD (patrz wyzej) —
        // zapobiega deadlockowi nie-rekursywnego mutexu
        case SCREEN_REPORTS: {
            display.drawReportsScreen(
                reportLogger.isReady(),
                cachedReportCount,
                cachedLastReport
            );
            break;
        }

        // ---- Czyszczenie dysz ----
        case SCREEN_NOZZLE_CLEAN: {
            const PatternDef& pat = patternMgr.getPattern((PatternID)nozzlePatternIdx);
            bool gunStates[6];
            for (int i = 0; i < NUM_GUNS; i++) {
                gunStates[i] = guns.getState(i);
            }
            display.drawNozzleClean(pat.code, pat.name, pat.guns, gunStates);
            break;
        }

        // ---- Ekran przygotowania (SETUP) ----
        case SCREEN_SETUP:
            display.drawSetupScreen(setupCursor, (MachineMode)setupMode,
                                    setupSmart, setupGapStart);
            break;

        // ---- Reset etapu ----
        case SCREEN_SESSION_RESET:
            display.drawSessionResetScreen(
                stats.getSessionDistance(),
                stats.getSessionArea(),
                stats.getSessionTimeSec()
            );
            break;

        // ---- Reset wszystkich licznikow ----
        case SCREEN_COUNTER_RESET: {
            uint32_t gunShots[NUM_GUNS];
            for (int i = 0; i < NUM_GUNS; i++) {
                gunShots[i] = stats.getGunShotCount(i);
            }
            display.drawCounterResetScreen(
                stats.getLifetimeDistance(),
                stats.getLifetimeArea(),
                stats.getLifetimePaintTimeSec(),
                gunShots
            );
            break;
        }

        // ---- Podsumowanie etapu ----
        case SCREEN_SUMMARY:
            display.drawSummaryScreen(
                summaryPatCode, summaryDist, summaryArea,
                summaryTime, summaryAvgSpeed,
                summaryHasGps, summaryLat, summaryLon
            );
            break;

        // ---- Statystyki lifetime ----
        case SCREEN_LIFETIME_STATS: {
            uint32_t gunShots[NUM_GUNS];
            for (int i = 0; i < NUM_GUNS; i++) {
                gunShots[i] = stats.getGunShotCount(i);
            }
            display.drawLifetimeStatsScreen(
                stats.getLifetimeDistance(),
                stats.getLifetimeArea(),
                stats.getLifetimePaintTimeSec(),
                gunShots,
                stats.getMTHSeconds()
            );
            break;
        }

        // ---- Edycja wzorca wlasnego ----
        case SCREEN_CUSTOM_PATTERN:
            display.drawCustomPatternScreen(custCursor, custGunIdx, custCfg);
            break;

        // ---- Eksport statystyk ----
        case SCREEN_STATS_EXPORT:
            display.drawStatsExportScreen(!exportDone, exportSuccess);
            break;

        // ---- Factory reset NVS ----
        case SCREEN_FACTORY_RESET:
            display.drawFactoryResetScreen();
            break;

        // ---- Tankowanie farby ----
        case SCREEN_TANKOWANIE:
            display.drawTankowanieScreen(tankRefuelAmount, tankRefuelDone);
            break;

        // ---- POST (diagnostyka) ----
        case SCREEN_POST:
            // POST jest obslugiwany w setup(), ten case zapobiega warningowi
            break;
    }

    // Fix #24: WDT reset po renderowaniu — dlugie ekrany (Painting, Stats)
    // moga trwac setki ms przez wiele operacji SPI na TFT
    esp_task_wdt_reset();

    // Zwolnij mutex SPI po renderowaniu TFT
    SD_UNLOCK();
#endif  // HAS_SMALL_TFT
}
