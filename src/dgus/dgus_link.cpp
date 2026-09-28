#include "sys_log.h"
// ============================================================
// MPD2026 - lacze sterownika z ekranem DWIN DGUS (UART)
// ============================================================

#include "dgus_link.h"

DgusLink dgusLink;

#if HAS_DGUS_LINK

#include <cmath>
#include <driver/uart.h>
#include <esp_idf_version.h>
#include "dgus_protocol.h"
#include "dgus_map.h"
#include "dgus_pages.h"
#include "control_api.h"
#include "menu.h"
#include "painting_engine.h"
#include "patterns.h"
#include "pattern_layout.h"
#include "pattern_buttons.h"
#include "buzzer.h"
#include "event_log.h"
#include "storage.h"
#include "estop.h"

using namespace dgusmap;

namespace {

constexpr uart_port_t DGUS_UART = UART_NUM_1;
constexpr int  RX_BUF = 2048;
constexpr int  TX_BUF = 2048;
constexpr int  RX_BUDGET = 512;              // maks. bajtow przetwarzanych w jednym update()

constexpr uint32_t LINK_LOSS_MS       = 1200;   // brak ramek dluzej niz to = utrata lacza
constexpr uint32_t PING_PERIOD_MS     = 700;    // odpytanie VP_PING gdy nic innego nie przyszlo
constexpr uint32_t HOME_PERIOD_MS     = 250;    // odswiezanie ekranu roboczego
constexpr uint32_t SERVICE_PERIOD_MS  = 300;    // odswiezanie ekranu serwisowego
constexpr uint32_t ESTOP_LINK_PERIOD_MS = 200;  // odswiezanie statusu E-STOP - niezalezne od strony, szybsze niz SERVICE
constexpr uint32_t ALARM_BEEP_MS      = 3000;
constexpr uint32_t NOZZLE_HOLD_MAX_MS = 8000;   // twardy limit "martwego czlowieka" - patrz dgus_map.h

dgus::Parser g_parser;
uint8_t      g_frame[dgus::MAX_FRAME];

void sendFrameLen(size_t n) {
    if (n) uart_write_bytes(DGUS_UART, (const char*)g_frame, n);
}

void sendWord(uint16_t vp, uint16_t value) {
    sendFrameLen(dgus::encodeWriteWord(vp, value, g_frame, sizeof(g_frame)));
}

void sendWords(uint16_t vp, const uint16_t* words, size_t count) {
    sendFrameLen(dgus::encodeWrite(vp, words, count, g_frame, sizeof(g_frame)));
}

void sendText(uint16_t vp, const char* text, size_t fixedWords) {
    sendFrameLen(dgus::encodeWriteText(vp, text, fixedWords, g_frame, sizeof(g_frame)));
}

// Liczba 32-bit (2 slowa, big-endian) - format uzywany przez dystans/powierzchnie/czas/GPS itd.
void sendI32(uint16_t vp, int32_t value) {
    uint16_t w[2] = {(uint16_t)((uint32_t)value >> 16), (uint16_t)((uint32_t)value & 0xFFFF)};
    sendWords(vp, w, 2);
}

void sendPageSwitch(uint16_t page) {
    sendFrameLen(dgus::encodeSwitchPage(page, g_frame, sizeof(g_frame)));
}

// Argumenty polecenia z pojedynczej pary (akcja, wartosc calkowita) - ekran DGUS
// nie przesyla formularzy, tylko stale kody zdarzen zamieniane tutaj na akcje API.
class DgusArgs : public ControlArgs {
public:
    explicit DgusArgs(int value) : val(value) {}
    bool has(const char* key) const override { return strcmp(key, "value") == 0; }
    String get(const char* key) const override {
        return strcmp(key, "value") == 0 ? String(val) : String();
    }
private:
    int val;
};

}  // namespace

void DgusLink::begin() {
    lossPolicy = storage.loadTermLossPolicy() == 1 ? 1 : 0;

    uart_config_t cfg;
    memset(&cfg, 0, sizeof(cfg));
    cfg.baud_rate = (int)dgus::BAUD;
    cfg.data_bits = UART_DATA_8_BITS;
    cfg.parity = UART_PARITY_DISABLE;
    cfg.stop_bits = UART_STOP_BITS_1;
    cfg.flow_ctrl = UART_HW_FLOWCTRL_DISABLE;
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
    cfg.source_clk = UART_SCLK_DEFAULT;
#else
    cfg.source_clk = UART_SCLK_APB;
#endif

    if (uart_driver_install(DGUS_UART, RX_BUF, TX_BUF, 0, nullptr, 0) != ESP_OK ||
        uart_param_config(DGUS_UART, &cfg) != ESP_OK ||
        uart_set_pin(DGUS_UART, PIN_DGUS_TX, PIN_DGUS_RX, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE) != ESP_OK) {
        DBG_PRINTLN("[DGUS] BLAD: inicjalizacja UART1 nie powiodla sie — ekran wylaczony");
        return;
    }
    started = true;
    g_parser.reset();
    DBG_PRINTF("[DGUS] Lacze: UART1 TX=%d RX=%d %u 8N1, polityka utraty: %s\n",
               PIN_DGUS_TX, PIN_DGUS_RX, (unsigned)dgus::BAUD,
               lossPolicy ? "AUTO-PAUZA" : "KONTYNUUJ+ALARM");
}

void DgusLink::setLossPolicy(uint8_t p) {
    lossPolicy = (p == 1) ? 1 : 0;
    storage.saveTermLossPolicy(lossPolicy);
    eventLog.logf("DGUS", "Polityka utraty ekranu: %s", lossPolicy ? "auto-pauza" : "kontynuuj+alarm");
}

uint32_t DgusLink::framesOk() const  { return g_parser.okFrames(); }
uint32_t DgusLink::framesBad() const { return g_parser.badFrames() + g_parser.overflows(); }

bool DgusLink::isHoldActive() const {
    if (!started || !linkUp || !nozzleHoldOn) return false;
    return (uint32_t)(millis() - nozzleHoldSetMs) < NOZZLE_HOLD_MAX_MS;
}

void DgusLink::dispatchAction(const char* action, int value) {
    DgusArgs args(value);
    ControlResult r = executeControl(String(action), args);
    if (!r.ok) buzzer.play(BUZ_ERROR);   // jedyna informacja zwrotna na ekranie DGUS przy bledzie
}

void DgusLink::handleTouchEvent(uint16_t code, uint32_t now) {
    if (code >= KEY_START_SHORT && code <= KEY_GAP_START) {
        // Dokladnie ta sama sciezka co przycisk fizyczny
        menu.handleEvent((ButtonEvent)code);
        return;
    }
    if (code >= KEY_GOTO_CALIBRATION && code <= KEY_GOTO_FACTORY_RESET) {
        static const ScreenID target[] = {
            SCREEN_CALIBRATION, SCREEN_DISTANCE_METER, SCREEN_REPORTS, SCREEN_NOZZLE_CLEAN,
            SCREEN_LIFETIME_STATS, SCREEN_CUSTOM_PATTERN, SCREEN_STATS_EXPORT,
            SCREEN_SESSION_RESET, SCREEN_COUNTER_RESET, SCREEN_TANKOWANIE, SCREEN_FACTORY_RESET
        };
        dispatchAction("set_screen", (int)target[code - KEY_GOTO_CALIBRATION]);
        return;
    }
    if (code >= KEY_SOFTKEY_BASE && code < KEY_SOFTKEY_BASE + 10) {
        int slot = (int)(code - KEY_SOFTKEY_BASE);
        int pat = softKeyPattern(patternButtons.getGroup(), slot);
        if (pat >= 0) dispatchAction("set_pattern", pat);
        return;
    }
    if (code == KEY_GRUPA) {
        dispatchAction("set_pattern_group", patternButtons.getGroup() == 0 ? 1 : 0);
        return;
    }
    if (code == KEY_NOZZLE_HOLD_ON) {
        nozzleHoldOn = true;
        nozzleHoldSetMs = now;
        return;
    }
    if (code == KEY_NOZZLE_HOLD_OFF) {
        nozzleHoldOn = false;
        return;
    }
#if HAS_ESTOP
    if (code == KEY_ESTOP_ACK) {
        estop.acknowledge();  // bez efektu, dopoki petla E-STOP pozostaje otwarta - patrz estop.cpp
        return;
    }
#endif
    DBG_PRINTF("[DGUS] Nieznany kod zdarzenia: %u\n", (unsigned)code);
}

void DgusLink::sendPing(uint32_t now) {
    if (now - lastPingMs < PING_PERIOD_MS) return;
    lastPingMs = now;
    sendFrameLen(dgus::encodeReadRequest(VP_PING, 1, g_frame, sizeof(g_frame)));
}

void DgusLink::sendEstopStatus(uint32_t now) {
    // Wysylane NIEZALEZNIE od aktywnej strony (w odroznieniu od sendHomeStatus/sendServicePage,
    // ktore dziela ten sam adresowy "blok 3" per-strona) - alarm STOP-u awaryjnego musi byc
    // widoczny bez wzgledu na to, w jakim menu jest akurat operator.
#if HAS_ESTOP
    if (now - lastEstopMs < ESTOP_LINK_PERIOD_MS) return;
    lastEstopMs = now;
    sendWord(VP_ESTOP_TRIGGERED, estop.isTriggered() ? 1 : 0);
    sendWord(VP_ESTOP_AWAIT_ACK, estop.isAwaitingAck() ? 1 : 0);
#else
    (void)now;
#endif
}

void DgusLink::sendHomeStatus() {
    DgusHomeStatus s;
    dgusBuildHomeStatus(s);

    sendWord(VP_STATE, (uint16_t)s.state);
    sendWord(VP_MODE, (uint16_t)s.mode);
    sendText(VP_PATTERN_CODE, s.patternCode, 6);
    sendText(VP_PATTERN_NAME, s.patternName, 16);
    sendWord(VP_REVERSED, s.reversed ? 1 : 0);
    sendWord(VP_GAP_START, s.gapStart ? 1 : 0);
    sendWord(VP_SPEED_X10, (uint16_t)(int16_t)lroundf(s.speedKmh * 10.0f));
    sendI32(VP_DISTANCE_DM, lroundf(s.distanceM * 10.0f));
    sendI32(VP_AREA_CM2, lroundf(s.areaM2 * 100.0f));
    sendI32(VP_ELAPSED_S, (int32_t)s.elapsedS);
    sendI32(VP_PATDIST_DM, lroundf(s.patDistM * 10.0f));
    sendWord(VP_OVERSPEED, s.overspeed ? 1 : 0);
    sendWord(VP_LOWSPEED, s.lowSpeed ? 1 : 0);
    sendWord(VP_AUTO_PAUSED, s.autoPaused ? 1 : 0);
    sendWord(VP_SEMI_COMPLETE, s.semiComplete ? 1 : 0);
    sendWord(VP_SEMI_SEGMENT, (uint16_t)s.semiSegment);
    {
        uint16_t w[NUM_GUNS];
        for (int i = 0; i < NUM_GUNS; i++) w[i] = s.gunState[i];
        sendWords(VP_GUN_BASE, w, NUM_GUNS);
    }
    sendWord(VP_PAINT_PCT, (uint16_t)s.paintPct);
    sendWord(VP_GPS_SAT, (uint16_t)s.gpsSat);
    sendWord(VP_GPS_FIX, s.gpsFix ? 1 : 0);
    sendWord(VP_PAT_PENDING, s.patternPending ? 1 : 0);
    sendText(VP_PENDING_CODE, s.pendingCode, 6);
    sendWord(VP_PAT_GROUP, (uint16_t)s.patGroup);
    sendWord(VP_NIGHT, s.night ? 1 : 0);
    sendWord(VP_LINK_OK, 1);   // ramka dociera, wiec z definicji "tak" w tej samej chwili
    sendWord(VP_SD_READY, s.sdReady ? 1 : 0);

    // Pistolety - anomalia
    sendWord(VP_GUN_ANOMALY, s.gunAnomalyDetected ? 1 : 0);
    {
        uint16_t w[NUM_GUNS];
        for (int i = 0; i < NUM_GUNS; i++) w[i] = s.gunAnomaly[i] ? 1 : 0;
        sendWords(VP_GUN_ANOMALY_BASE, w, NUM_GUNS);
    }

    // Enkoder
    sendWord(VP_ENC_CALIBRATED, s.encCalibrated ? 1 : 0);
    sendWord(VP_ENC_PPM_X10, (uint16_t)(int16_t)lroundf(s.encPpm * 10.0f));

    // GPS (pelny zestaw)
    sendWord(VP_GPS_SPEED_X10, (uint16_t)(int16_t)lroundf(s.gpsSpeedKmh * 10.0f));
    sendI32(VP_GPS_LAT_X1E6, (int32_t)lround(s.gpsLat * 1000000.0));
    sendI32(VP_GPS_LNG_X1E6, (int32_t)lround(s.gpsLng * 1000000.0));
    sendWord(VP_GPS_HDOP_X10, (uint16_t)(int16_t)lroundf(s.gpsHdop * 10.0f));
    sendWord(VP_GPX_RECORDING, s.gpxRecording ? 1 : 0);
    sendI32(VP_GPX_POINTS, (int32_t)s.gpxPoints);
    sendWord(VP_GPX_OVERFLOW, s.gpxOverflow ? 1 : 0);

    // Czujnik temperatury
    sendWord(VP_TEMP_AVAILABLE, s.tempAvailable ? 1 : 0);
    sendWord(VP_TEMP_X10, (uint16_t)(int16_t)lroundf(s.tempC * 10.0f));

    // Diagnostyka systemu
    sendWord(VP_FREE_HEAP_KB, (uint16_t)s.freeHeapKB);
    sendWord(VP_UPTIME_MIN, (uint16_t)s.uptimeMin);
    sendWord(VP_WWW_CLIENTS, (uint16_t)s.wwwClients);
}

void DgusLink::sendSoftkeys() {
    DgusSoftkeys sk;
    dgusBuildSoftkeys(sk);
    uint16_t idx[10], sel[10];
    for (int i = 0; i < 10; i++) {
        idx[i] = (sk.patternIdx[i] < 0) ? 0xFFFF : (uint16_t)sk.patternIdx[i];
        sel[i] = sk.selected[i] ? 1 : 0;
    }
    sendWords(VP_SLOT_PATIDX_BASE, idx, 10);
    sendWords(VP_SLOT_SEL_BASE, sel, 10);
    for (int i = 0; i < 10; i++) {
        sendText((uint16_t)(VP_SLOT_CODE_BASE + i * 6), sk.code[i], 6);
    }
    sendWord(VP_CUSTOM_VALID, sk.customValid ? 1 : 0);
}

void DgusLink::sendServicePage(int screen) {
    DgusPageData d;
    if (!menu.fillDgusPage(d)) return;
    for (int i = 0; i < dgusmap::DGUS_ROW_COUNT; i++) sendText((uint16_t)(VP_ROW_BASE + i * dgusmap::DGUS_ROW_WORDS), d.row[i], dgusmap::DGUS_ROW_WORDS);
    sendWord(VP_ROW_SELECTED, (uint16_t)(int16_t)d.selected);
    sendText(VP_MSG, d.message, 32);
    sendWord(VP_FLAGS, (uint16_t)((d.flagA ? 1 : 0) | (d.flagB ? 2 : 0)));
    sendI32(VP_NUM1, d.num1);
    sendI32(VP_NUM2, d.num2);
    (void)screen;
}

void DgusLink::onLinkChange(bool up, uint32_t now) {
    linkUp = up;
    if (up) {
        everUp = true;
        lastPageSent = -1;   // wymus przeslanie pelnego stanu strony po odzyskaniu lacza
        if (lossAlarm) {
            lossAlarm = false;
            lossPauseDone = false;
            eventLog.log("DGUS", "Ekran ponownie polaczony");
        } else {
            eventLog.log("DGUS", "Ekran polaczony");
        }
        DBG_PRINTLN("[DGUS] Lacze: POLACZONE");
    } else {
        nozzleHoldOn = false;   // utrata lacza = puszczony przycisk (martwy czlowiek)
        DBG_PRINTLN("[DGUS] Lacze: UTRACONE");
        if (everUp) {
            lossAlarm = true;
            lastAlarmBeepMs = 0;
            eventLog.log("DGUS", "UTRATA LACZA Z EKRANEM");
        }
    }
}

void DgusLink::applyLossPolicy(uint32_t now) {
    if (!lossAlarm) return;
    STATE_LOCK();
    MachineState ms = g_state.machineState;
    STATE_UNLOCK();
    if (ms != STATE_PAINTING && ms != STATE_PAUSED) return;

    if (lossPolicy == 1 && ms == STATE_PAINTING && !lossPauseDone) {
        lossPauseDone = true;
        paintEngine.pause();
        eventLog.log("DGUS", "Utrata ekranu — automatyczna pauza");
    }
    if (now - lastAlarmBeepMs >= ALARM_BEEP_MS) {
        lastAlarmBeepMs = now;
        buzzer.play(BUZ_ERROR);
    }
}

void DgusLink::update() {
    if (!started) return;
    uint32_t now = millis();

    // --- RX ---
    uint8_t rx[64];
    int budget = RX_BUDGET;
    while (budget > 0) {
        int want = budget < (int)sizeof(rx) ? budget : (int)sizeof(rx);
        int n = uart_read_bytes(DGUS_UART, rx, want, 0);
        if (n <= 0) break;
        budget -= n;
        for (int i = 0; i < n; i++) {
            if (!g_parser.feed(rx[i])) continue;
            lastRxMs = now;
            if (g_parser.cmd() == dgus::CMD_READ && g_parser.vp() == VP_TOUCH_EVENT && g_parser.wordCount() >= 1) {
                uint16_t code = g_parser.word(0);
                uint8_t next = (uint8_t)((evHead + 1) % EVQ);
                if (next != evTail) { evq[evHead] = code; evHead = next; }
            }
            // odpowiedzi na VP_PING i inne odczyty: sam fakt odebrania juz potwierdza zywe lacze
        }
    }

    // --- kolejka zdarzen dotyku (przetwarzana poza petla odbioru, jak przy przyciskach) ---
    while (evTail != evHead) {
        uint16_t code = evq[evTail];
        evTail = (uint8_t)((evTail + 1) % EVQ);
        handleTouchEvent(code, now);
    }

    // --- twardy limit martwego czlowieka (na wypadek zgubienia ramki zwolnienia) ---
    if (nozzleHoldOn && (uint32_t)(now - nozzleHoldSetMs) >= NOZZLE_HOLD_MAX_MS) {
        nozzleHoldOn = false;
    }

    // --- stan lacza ---
    bool up = (lastRxMs != 0) && ((uint32_t)(now - lastRxMs) < LINK_LOSS_MS);
    if (up != linkUp) onLinkChange(up, now);
    applyLossPolicy(now);

    // --- TX ---
    sendPing(now);
    sendEstopStatus(now);

    STATE_LOCK();
    int screen = (int)g_state.currentScreen;
    STATE_UNLOCK();

    if (screen != lastPageSent) {
        sendPageSwitch((uint16_t)screen);
        lastPageSent = screen;
    }

    if (screen == SCREEN_HOME || screen == SCREEN_PAINTING) {
        if (now - lastHomeMs >= HOME_PERIOD_MS) {
            lastHomeMs = now;
            sendHomeStatus();
            sendSoftkeys();
        }
    } else {
        if (now - lastServiceMs >= SERVICE_PERIOD_MS) {
            lastServiceMs = now;
            sendServicePage(screen);
        }
    }
}

#endif  // HAS_DGUS_LINK
