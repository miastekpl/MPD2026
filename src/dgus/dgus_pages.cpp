#include "config.h"

#if HAS_DGUS_LINK

// ============================================================
// MPD2026 - dane dla ekranu DWIN DGUS: blok statusu roboczego, kolumny
// wzorcow S1-S10 i generyczne pola ekranow serwisowych.
//
// Etykiety, tla i przyciski sa statyczne (narysowane w DGUS Designer wedlug
// tabeli w docs/ARCHITEKTURA_TERMINAL.md); tutaj wypelniamy TYLKO wartosci.
// ============================================================

#include "dgus_pages.h"
#include "dgus_link.h"
#include "menu.h"
#include "patterns.h"
#include "pattern_layout.h"
#include "pattern_buttons.h"
#include "encoder_distance.h"
#include "statistics.h"
#include "guns.h"
#include "report_logger.h"
#include "paint_consumption.h"
#include "painting_engine.h"
#include "gps_handler.h"
#include "gps_track.h"
#include "temp_sensor.h"
#include "web_server.h"
#include <cstdio>

// ============ Ekran roboczy ============

void dgusBuildHomeStatus(DgusHomeStatus& o) {
    STATE_LOCK();
    o.state = (int)g_state.machineState;
    o.mode  = (int)g_state.machineMode;
    o.night = g_state.nightMode;
    o.reversed = g_state.patternReversed;
    STATE_UNLOCK();
    o.patGroup = (int)patternButtons.getGroup();

    const PatternDef& pat = patternMgr.getCurrent();
    strlcpy(o.patternCode, pat.code, sizeof(o.patternCode));
    strlcpy(o.patternName, pat.name, sizeof(o.patternName));
    o.gapStart = paintEngine.isGapStart();
    o.speedKmh = encoderDist.getSpeedKmh();
    o.distanceM = stats.getSessionDistance();
    o.areaM2 = stats.getSessionArea();
    o.elapsedS = stats.getSessionTimeSec();
    o.patDistM = paintEngine.getPatternDistance();
    o.overspeed = paintEngine.isOverspeed();
    o.lowSpeed = paintEngine.isLowSpeed();
    o.autoPaused = paintEngine.isAutoPaused();
    o.semiComplete = paintEngine.isSemiLineComplete();
    o.semiSegment = paintEngine.getSemiSegmentNum();
    for (int i = 0; i < NUM_GUNS; i++) {
        GunPatternCfg cfg = patternMgr.getGunConfig((GunID)i);
        bool on = guns.getState(i);
        o.gunState[i] = on ? 1 : (cfg.mode != GUN_OFF ? 2 : 0);
    }
    o.paintPct = (int)(paintConsumption.getTankCapacity() > 0
                        ? (paintConsumption.getCurrentLevel() * 100.0f / paintConsumption.getTankCapacity())
                        : 0);
    if (o.paintPct < 0) o.paintPct = 0;
    if (o.paintPct > 100) o.paintPct = 100;
    o.gpsSat = gpsHandler.getSatellites();
    o.gpsFix = gpsHandler.hasFix();
    o.patternPending = paintEngine.isPatternChangePending();
    if (o.patternPending) {
        strlcpy(o.pendingCode, patternMgr.getPattern(paintEngine.getPendingPattern()).code, sizeof(o.pendingCode));
    } else {
        o.pendingCode[0] = 0;
    }
    o.linkOk = true;   // wypelniane tylko gdy faktycznie wysylamy (patrz dgus_link.cpp)
    o.sdReady = reportLogger.isReady();

    // Anomalia pistoletow (odczyt pod lockiem - modyfikowane z Core 1, patrz main.cpp).
    // Fix analogiczny do web_server.cpp::getStateJson(): trylock, bezpieczny default = brak anomalii.
    if (STATE_TRYLOCK(200)) {
        o.gunAnomalyDetected = gunAnomaly.detected;
        for (int i = 0; i < NUM_GUNS; i++) o.gunAnomaly[i] = gunAnomaly.alert[i];
        STATE_UNLOCK();
    }

    // Enkoder
    o.encCalibrated = encoderDist.isCalibrated();
    o.encPpm = encoderDist.getPulsesPerMeter();

    // GPS (pelny zestaw)
    o.gpsSpeedKmh = gpsHandler.getGpsSpeed();
    o.gpsLat = gpsHandler.getLat();
    o.gpsLng = gpsHandler.getLng();
    o.gpsHdop = (float)gpsHandler.getHdop();
    o.gpxRecording = gpsTrack.isRecording();
    o.gpxPoints = gpsTrack.getPointCount();
    o.gpxOverflow = gpsTrack.isOverflowed();

    // Czujnik temperatury DS18B20 (opcjonalny - moze nie byc zamontowany)
    o.tempAvailable = tempSensor.isAvailable();
    o.tempC = tempSensor.getTemperature();

    // Diagnostyka systemu - przydatna w terenie bez laptopa/panelu WWW
    o.freeHeapKB = (int)(ESP.getFreeHeap() / 1024);
    o.uptimeMin = (uint32_t)(millis() / 60000UL);
    o.wwwClients = webServer.getConnectedClients();
}

// ============ Kolumny S1..S10 ============

void dgusBuildSoftkeys(DgusSoftkeys& o) {
    STATE_LOCK();
    PatternID cur = g_state.currentPattern;
    STATE_UNLOCK();
    uint8_t group = patternButtons.getGroup();

    for (int slot = 0; slot < 10; slot++) {
        int pat = softKeyPattern(group, slot);
        if (pat < 0) {
            o.patternIdx[slot] = -1;
            o.selected[slot] = false;
            o.code[slot][0] = 0;
            continue;
        }
        o.patternIdx[slot] = (int8_t)pat;
        o.selected[slot] = (pat == (int)cur);
        const char* code = (pat == PatternManager::PREDEFINED_PAT_COUNT) ? "WLASNY"
                                                                          : patternMgr.getPattern((PatternID)pat).code;
        strlcpy(o.code[slot], code, sizeof(o.code[slot]));
    }
    o.customValid = patternMgr.isCustomValid();
}

// ============ Ekrany serwisowe ============

namespace {

void setRow(DgusPageData& d, int i, const char* text) {
    if (i < 0 || i >= dgusmap::DGUS_ROW_COUNT) return;
    strlcpy(d.row[i], text, sizeof(d.row[i]));
}

void setRowF(DgusPageData& d, int i, const char* fmt, ...) __attribute__((format(printf, 3, 4)));
void setRowF(DgusPageData& d, int i, const char* fmt, ...) {
    char buf[sizeof(d.row[0])];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    setRow(d, i, buf);
}

const char* modeName(int m) {
    switch (m) {
        case 0: return "AUTO";
        case 1: return "SEMI-AUTO";
        case 2: return "RECZNY";
        case 3: return "DEMO";
        default: return "?";
    }
}

void hms(char* out, size_t n, unsigned long sec) {
    snprintf(out, n, "%02lu:%02lu:%02lu", sec / 3600UL, (sec / 60UL) % 60UL, sec % 60UL);
}

}  // namespace

void MenuSystem::infoBegin(const char* title, const char* message) {
    strlcpy(infoTitle, title ? title : "", sizeof(infoTitle));
    strlcpy(infoMsg, message ? message : "", sizeof(infoMsg));
    infoCount = 0;
}

void MenuSystem::infoRow(const char* label, const char* value) {
    if (infoCount >= INFO_ROWS) return;
    strlcpy(infoLabel[infoCount], label ? label : "", sizeof(infoLabel[0]));
    strlcpy(infoValue[infoCount], value ? value : "", sizeof(infoValue[0]));
    infoCount++;
}

bool MenuSystem::fillDgusPage(DgusPageData& d) {
    STATE_LOCK();
    ScreenID screen = g_state.currentScreen;
    int menuIdx = g_state.menuIndex;
    STATE_UNLOCK();

    if (screen == SCREEN_HOME || screen == SCREEN_PAINTING) return false;

    switch (screen) {

        case SCREEN_SERVICE_MENU: {
            d.selected = menuIdx;
            break;
        }

        case SCREEN_CALIBRATION: {
            bool cal = encoderDist.isCalibrating();
            d.flagA = cal;
            setRow(d, 0, cal ? "W TOKU" : "nieaktywna");
            setRowF(d, 1, "%.0f", encoderDist.getCalibrationPulses());
            setRowF(d, 2, "%.1f", encoderDist.getPulsesPerMeter());
            setRow(d, 3, encoderDist.isCalibrated() ? "TAK" : "NIE");
            d.num1 = (int32_t)lroundf(encoderDist.getCalibrationPulses() * 10.0f);
            break;
        }

        case SCREEN_DISTANCE_METER: {
            d.flagA = distMeasuring;
            setRowF(d, 0, "%.2f m", distMeterValue);
            setRow(d, 1, distMeasuring ? "TRWA" : "zatrzymany");
            d.num1 = (int32_t)lroundf(distMeterValue * 100.0f);
            break;
        }

        case SCREEN_REPORTS: {
            static int      s_lastScreen = -1;
            static uint32_t s_readMs = 0;
            static int      s_count = 0;
            static char     s_last[64] = {};
            uint32_t now = millis();
            if (s_lastScreen != (int)SCREEN_REPORTS || now - s_readMs > 15000) {
                s_count = reportLogger.getReportCount();
                s_last[0] = 0;
                reportLogger.getLastReport(s_last, sizeof(s_last));
                s_readMs = now;
            }
            s_lastScreen = (int)SCREEN_REPORTS;
            setRow(d, 0, reportLogger.isReady() ? "OK" : "BRAK");
            setRowF(d, 1, "%d", s_count);
            setRow(d, 2, s_last[0] ? s_last : "-");
            break;
        }

        case SCREEN_NOZZLE_CLEAN: {
            const PatternDef& pat = patternMgr.getPattern((PatternID)nozzlePatternIdx);
            setRowF(d, 0, "%s %s", pat.code, pat.name);
            char active[24] = "";
            int pos = 0;
            for (int i = 0; i < NUM_GUNS; i++) {
                if (pat.guns[i].mode != GUN_OFF) {
                    pos += snprintf(active + pos, sizeof(active) - pos, "%sP%d", pos ? " " : "", i + 1);
                }
            }
            setRow(d, 1, active[0] ? active : "-");
            char open[24] = "";
            pos = 0;
            for (int i = 0; i < NUM_GUNS; i++) {
                if (guns.getState(i)) pos += snprintf(open + pos, sizeof(open) - pos, "%sP%d", pos ? " " : "", i + 1);
            }
            setRow(d, 2, open[0] ? open : "-");
            d.num1 = nozzlePatternIdx;
            break;
        }

        case SCREEN_SETUP: {
            setRow(d, 0, modeName(setupMode));
            setRow(d, 1, setupSmart ? "SMART" : "INSTANT");
            setRow(d, 2, setupGapStart ? "OD PRZERWY" : "NORMALNY");
            d.selected = setupCursor;
            break;
        }

        case SCREEN_SESSION_RESET: {
            setRowF(d, 0, "%.1f m", stats.getSessionDistance());
            setRowF(d, 1, "%.2f m2", stats.getSessionArea());
            char t[16]; hms(t, sizeof(t), stats.getSessionTimeSec());
            setRow(d, 2, t);
            break;
        }

        case SCREEN_COUNTER_RESET: {
            setRowF(d, 0, "%.1f m", stats.getLifetimeDistance());
            setRowF(d, 1, "%.2f m2", stats.getLifetimeArea());
            char t[16]; hms(t, sizeof(t), stats.getLifetimePaintTimeSec());
            setRow(d, 2, t);
            break;
        }

        case SCREEN_SUMMARY: {
            setRow(d, 0, summaryPatCode);
            setRowF(d, 1, "%.1f m", summaryDist);
            setRowF(d, 2, "%.2f m2", summaryArea);
            char t[16]; hms(t, sizeof(t), summaryTime);
            setRow(d, 3, t);
            setRowF(d, 4, "%.1f km/h", summaryAvgSpeed);
            if (summaryHasGps) setRowF(d, 5, "%.5f, %.5f", summaryLat, summaryLon);
            break;
        }

        case SCREEN_LIFETIME_STATS: {
            setRowF(d, 0, "%.1f m", stats.getLifetimeDistance());
            setRowF(d, 1, "%.2f m2", stats.getLifetimeArea());
            char t[16]; hms(t, sizeof(t), stats.getLifetimePaintTimeSec());
            setRow(d, 2, t);
            char m[16]; hms(m, sizeof(m), stats.getMTHSeconds());
            setRow(d, 3, m);
            for (int i = 0; i < NUM_GUNS && i < 4; i++) {
                setRowF(d, 4 + i, "%lu", (unsigned long)stats.getGunShotCount(i));
            }
            break;
        }

        case SCREEN_CUSTOM_PATTERN: {
            static const char* MODES[] = {"WYLACZONY", "CIAGLY", "PRZERYWANY"};
            int gm = custCfg.gunModes[custGunIdx];
            if (gm < 0 || gm > 2) gm = 0;
            setRowF(d, 0, "P%d", custGunIdx + 1);
            setRow(d, 1, MODES[gm]);
            setRowF(d, 2, "%.1f m", custCfg.lineLen[custGunIdx]);
            setRowF(d, 3, "%.1f m", custCfg.gapLen[custGunIdx]);
            d.selected = custCursor;
            break;
        }

        case SCREEN_STATS_EXPORT: {
            d.flagA = exportDone;
            d.flagB = exportSuccess;
            setRow(d, 0, reportLogger.isReady() ? "OK" : "BRAK");
            setRow(d, 1, !exportDone ? "gotowy" : (exportSuccess ? "ZAPISANO" : "BLAD"));
            break;
        }

        case SCREEN_FACTORY_RESET:
            break;

        case SCREEN_TANKOWANIE: {
            d.flagA = tankRefuelDone;
            setRowF(d, 0, "%.0f L", tankRefuelAmount);
            setRowF(d, 1, "%.1f L", paintConsumption.getCurrentLevel());
            setRowF(d, 2, "%.0f L", paintConsumption.getTankCapacity());
            d.num1 = (int32_t)lroundf(tankRefuelAmount * 10.0f);
            d.num2 = (int32_t)lroundf(paintConsumption.getCurrentLevel() * 10.0f);
            break;
        }

        case SCREEN_POST: {
            // Tytul ("START TRASSAR") jest statyczny w DGUS Designer; komunikat idzie do VP_MSG,
            // a wszystkie 8 wierszy diagnostyki miesci sie w wierszach 0-7.
            strlcpy(d.message, infoMsg, sizeof(d.message));
            for (int i = 0; i < infoCount && i < dgusmap::DGUS_ROW_COUNT; i++) {
                setRowF(d, i, "%s: %s", infoLabel[i], infoValue[i]);
            }
            break;
        }

        default:
            break;
    }
    return true;
}

#endif  // HAS_DGUS_LINK
