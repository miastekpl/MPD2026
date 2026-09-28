// ============================================================
// SERWIS: reset etapu, reset licznikow, eksport statystyk, factory reset,
// czyszczenie dysz. Odpowiednik ekranow serwisowych sterownika (SCREEN_
// SESSION_RESET/COUNTER_RESET/STATS_EXPORT/FACTORY_RESET/NOZZLE_CLEAN) —
// dotad dostepnych tylko z malego ekranu ILI9341/ekranu DGUS, ktorych
// w tej architekturze nie ma; ten modul jest teraz jedynym "panelem
// fizycznym" operatora, wiec te funkcje musza byc tu dostepne.
// ============================================================

#include "ui_common.h"
#include "link.h"
#include <stdio.h>
#include <math.h>

static bool requireOnline() {
    if (!g_online) { uiToast("brak lacznosci ze sterownikiem", true); return false; }
    return true;
}

static void setLbl(lv_obj_t* l, const char* t) {
    if (l && strcmp(lv_label_get_text(l), t) != 0) lv_label_set_text(l, t);
}

// Kody zdarzen menu sterownika (src/button_handler.h) — stabilny kontrakt,
// ten sam uzywany przez ekran DGUS (src/dgus/dgus_map.h KEY_*).
constexpr int EVT_START_SHORT_CODE  = 1;   // pomiar dystansu: start/pauza
constexpr int EVT_STOP_SHORT_CODE   = 3;   // pomiar dystansu: zeruj
constexpr int EVT_STOP_LONG_CODE    = 4;   // wyjscie z ekranu serwisowego
constexpr int EVT_SELECT_SHORT_CODE = 5;   // nastepny wzorzec
constexpr int EVT_SELECT_LONG_CODE  = 6;   // poprzedni wzorzec

constexpr uint32_t CONFIRM_WINDOW_MS = 3000;   // czas na potwierdzenie drugim dotknieciem

// ------------------------------------------------------------
// Pomocnicze: przycisk "dotknij dwa razy, aby potwierdzic"
// (te same 3 przypadki - malo, zeby robic z tego osobny widget)
// ------------------------------------------------------------
struct ConfirmState {
    lv_obj_t* btn = nullptr;
    const char* label = nullptr;
    uint32_t armedUntil = 0;
};
static ConfirmState s_cSession, s_cCounter, s_cFactory;

static void confirmTick(ConfirmState& s) {
    if (s.armedUntil && millis() > s.armedUntil && s.btn) {
        s.armedUntil = 0;
        uiBtnSetText(s.btn, s.label);
        uiBtnSetColor(s.btn, C_BTN);
    }
}

// Zwraca true, gdy to bylo DRUGIE (potwierdzajace) dotkniecie
static bool confirmTap(ConfirmState& s) {
    uint32_t now = millis();
    if (s.armedUntil && now < s.armedUntil) {
        s.armedUntil = 0;
        uiBtnSetText(s.btn, s.label);
        uiBtnSetColor(s.btn, C_BTN);
        return true;
    }
    s.armedUntil = now + CONFIRM_WINDOW_MS;
    uiBtnSetText(s.btn, "NA PEWNO?");
    uiBtnSetColor(s.btn, C_ORANGE);
    return false;
}

// ------------------------------------------------------------
// Reset etapu / reset licznikow / factory reset / eksport
// ------------------------------------------------------------
static void onSessionReset(lv_event_t*) {
    if (!requireOnline()) return;
    if (confirmTap(s_cSession)) {
        uiSend("action=session_reset&value=1");
        uiToast("Etap zresetowany", false);
    }
}

static void onCounterReset(lv_event_t*) {
    if (!requireOnline()) return;
    if (confirmTap(s_cCounter)) {
        uiSend("action=counter_reset&value=1");
        uiToast("Liczniki zresetowane (kalibracja zachowana)", false);
    }
}

static void onFactoryReset(lv_event_t*) {
    if (!requireOnline()) return;
    if (confirmTap(s_cFactory)) {
        uiSend("action=factory_reset&value=1");
        uiToast("FACTORY RESET — sterownik zaraz sie zrestartuje", true);
    }
}

static void onStatsExport(lv_event_t*) {
    if (!requireOnline()) return;
    uiSend("action=stats_export");
    uiToast("Eksportowanie na kartę SD...", false);
}

static void onNozzleOpen(lv_event_t*);
static void onDistMeterOpen(lv_event_t*);

static void serviceUpdate() {
    confirmTick(s_cSession);
    confirmTick(s_cCounter);
    confirmTick(s_cFactory);
}

static void serviceClosed() {
    s_cSession.btn = s_cCounter.btn = s_cFactory.btn = nullptr;
    s_cSession.armedUntil = s_cCounter.armedUntil = s_cFactory.armedUntil = 0;
}

void uiOpenService() {
    lv_obj_t* ov = uiOverlay("SERWIS", serviceUpdate);
    uiSetOverlayCloseHandler(serviceClosed);

    s_cSession.label = "RESET ETAPU";
    s_cCounter.label = "RESET LICZNIKOW";
    s_cFactory.label = "FACTORY RESET";

#if UI_PORTRAIT
    int y = OV_TOP + 8;
    uiBtn(ov, LV_SYMBOL_TINT " CZYSZCZENIE DYSZ", 8, y, OV_W - 16, 64, C_BTN, onNozzleOpen, nullptr, FONT_M);
    y += 76;
    uiBtn(ov, LV_SYMBOL_REFRESH " POMIAR DYSTANSU", 8, y, OV_W - 16, 64, C_BTN, onDistMeterOpen, nullptr, FONT_M);
    y += 76;
    s_cSession.btn = uiBtn(ov, s_cSession.label, 8, y, OV_W - 16, 64, C_BTN, onSessionReset, nullptr, FONT_M);
    y += 76;
    s_cCounter.btn = uiBtn(ov, s_cCounter.label, 8, y, OV_W - 16, 64, C_BTN, onCounterReset, nullptr, FONT_M);
    y += 76;
    uiBtn(ov, LV_SYMBOL_SD_CARD " EKSPORT STATYSTYK", 8, y, OV_W - 16, 64, C_BTN, onStatsExport, nullptr, FONT_M);
    y += 76;
    s_cFactory.btn = uiBtn(ov, s_cFactory.label, 8, y, OV_W - 16, 64, C_RED, onFactoryReset, nullptr, FONT_M);
    y += 76;
    lv_obj_t* warn = uiLabel(ov, "Reset licznikow i factory reset wymagaja podwojnego dotkniecia w ciagu 3 s.",
                              8, y, FONT_S, C_DIM);
    lv_obj_set_width(warn, OV_W - 16);
#else
    // 2 kolumny x 3 wiersze - 6 pozycji nie miescilyby sie w jednej kolumnie na 480px wysokosci
    int colW = (OV_W - 48) / 2;
    int x0 = 16, x1 = 32 + colW;
    int y0 = 70, y1 = 146, y2 = 222;
    uiBtn(ov, LV_SYMBOL_TINT " CZYSZCZENIE DYSZ", x0, y0, colW, 64, C_BTN, onNozzleOpen, nullptr, FONT_M);
    uiBtn(ov, LV_SYMBOL_REFRESH " POMIAR DYSTANSU", x1, y0, colW, 64, C_BTN, onDistMeterOpen, nullptr, FONT_M);
    s_cSession.btn = uiBtn(ov, s_cSession.label, x0, y1, colW, 64, C_BTN, onSessionReset, nullptr, FONT_M);
    s_cCounter.btn = uiBtn(ov, s_cCounter.label, x1, y1, colW, 64, C_BTN, onCounterReset, nullptr, FONT_M);
    uiBtn(ov, LV_SYMBOL_SD_CARD " EKSPORT STATYSTYK", x0, y2, colW, 64, C_BTN, onStatsExport, nullptr, FONT_M);
    s_cFactory.btn = uiBtn(ov, s_cFactory.label, x1, y2, colW, 64, C_RED, onFactoryReset, nullptr, FONT_M);
    lv_obj_t* warn = uiLabel(ov, "Reset licznikow i factory reset wymagaja podwojnego dotkniecia w ciagu 3 s.",
                              16, y2 + 76, FONT_S, C_DIM);
    lv_obj_set_width(warn, OV_W - 32);
#endif
}

// ------------------------------------------------------------
// Czyszczenie dysz — przegladanie wzorca + "martwy czlowiek" (przytrzymaj)
// ------------------------------------------------------------
static lv_obj_t* s_nzInfo;
static lv_obj_t* s_nzHold;
static bool      s_nzHoldOn = false;

static void onNzPrev(lv_event_t*) {
    if (!requireOnline()) return;
    uiSendAction("send_event", EVT_SELECT_LONG_CODE);
}
static void onNzNext(lv_event_t*) {
    if (!requireOnline()) return;
    uiSendAction("send_event", EVT_SELECT_SHORT_CODE);
}

static void onNzHold(lv_event_t* e) {
    lv_event_code_t c = lv_event_get_code(e);
    if (c == LV_EVENT_PRESSED) {
        if (!requireOnline()) return;
        s_nzHoldOn = true;
        uiSend("action=nozzle_hold_on", true);
    } else if (c == LV_EVENT_RELEASED || c == LV_EVENT_PRESS_LOST) {
        if (s_nzHoldOn) {
            s_nzHoldOn = false;
            uiSend("action=nozzle_hold_off", true);
        }
    }
}

static void nozzleUpdate() {
    if (!s_nzInfo) return;
    char b[80];
    int idx = g_st.nozzlePatternIdx;
    if (idx < 0 || idx >= NPAT_PREDEF) idx = 0;
    const PatternInfo& p = PATTERNS[idx];
    char guns[32] = "";
    int n = 0;
    for (int i = 0; i < NGUNS; i++) {
        if (p.guns[i].mode != GM_OFF) {
            n += snprintf(guns + n, sizeof(guns) - n, "%sP%d", n ? "," : "", i + 1);
        }
    }
    snprintf(b, sizeof(b), "%s — %s\nAktywne pistolety: %s", p.code, p.name, n ? guns : "brak");
    if (strcmp(lv_label_get_text(s_nzInfo), b) != 0) lv_label_set_text(s_nzInfo, b);
    if (s_nzHold) uiBtnEnable(s_nzHold, g_online);
}

static void nozzleClosed() {
    if (s_nzHoldOn) { s_nzHoldOn = false; uiSend("action=nozzle_hold_off", true); }
    uiSendAction("send_event", EVT_STOP_LONG_CODE);   // wroc do SERVICE_MENU, guns.allOff()
    s_nzInfo = nullptr;
    s_nzHold = nullptr;
}

static void onNozzleOpen(lv_event_t*) {
    if (!requireOnline()) return;
    uiSend("action=set_screen&value=6");   // 6 = SCREEN_NOZZLE_CLEAN (src/config.h)
    lv_obj_t* ov = uiOverlay("CZYSZCZENIE DYSZ", nozzleUpdate);
    uiSetOverlayCloseHandler(nozzleClosed);

#if UI_PORTRAIT
    lv_obj_t* h = uiLabel(ov, "Wybierz wzorzec strzalkami, potem PRZYTRZYMAJ przycisk ponizej, "
                              "aby otworzyc dysze. Puszczenie zamyka je natychmiast.",
                          12, OV_TOP, FONT_M, C_DIM);
    lv_obj_set_width(h, OV_W - 24);
    lv_obj_set_style_text_line_space(h, 8, 0);
    uiBtn(ov, "<", 12, OV_TOP + 90, 90, 80, C_BTN, onNzPrev, nullptr, FONT_L);
    uiBtn(ov, ">", OV_W - 102, OV_TOP + 90, 90, 80, C_BTN, onNzNext, nullptr, FONT_L);
    s_nzInfo = uiLabel(ov, "", 110, OV_TOP + 100, FONT_M, C_YELLOW);
    lv_obj_set_width(s_nzInfo, OV_W - 220);
    lv_obj_set_style_text_line_space(s_nzInfo, 6, 0);
    s_nzHold = uiBtn(ov, "PRZYTRZYMAJ, ABY OTWORZYC DYSZE", 12, OV_H - 160, OV_W - 24, 130,
                     C_ORANGE, nullptr, nullptr, FONT_L);
#else
    lv_obj_t* h = uiLabel(ov, "Wybierz wzorzec strzalkami, potem PRZYTRZYMAJ przycisk ponizej, "
                              "aby otworzyc dysze. Puszczenie zamyka je natychmiast.",
                          16, 72, FONT_M, C_DIM);
    lv_obj_set_style_text_line_space(h, 8, 0);
    uiBtn(ov, "<", 16, 150, 90, 90, C_BTN, onNzPrev, nullptr, FONT_L);
    uiBtn(ov, ">", 694, 150, 90, 90, C_BTN, onNzNext, nullptr, FONT_L);
    s_nzInfo = uiLabel(ov, "", 120, 165, FONT_M, C_YELLOW);
    lv_obj_set_width(s_nzInfo, 560);
    lv_obj_set_style_text_line_space(s_nzInfo, 6, 0);
    s_nzHold = uiBtn(ov, "PRZYTRZYMAJ, ABY OTWORZYC DYSZE", 16, 320, OV_W - 32, 130,
                     C_ORANGE, nullptr, nullptr, FONT_L);
#endif
    lv_obj_add_event_cb(s_nzHold, onNzHold, LV_EVENT_PRESSED, nullptr);
    lv_obj_add_event_cb(s_nzHold, onNzHold, LV_EVENT_RELEASED, nullptr);
    lv_obj_add_event_cb(s_nzHold, onNzHold, LV_EVENT_PRESS_LOST, nullptr);
    nozzleUpdate();
}

// ------------------------------------------------------------
// Pomiar dystansu z alarmem: miga na zolto w strefie ostrzegawczej, zielony
// ekran + duzy napis STOP po osiagnieciu celu. Staly ton buzzera generuje
// sterownik (src/menu.cpp) - ten ekran tylko pokazuje stan.
// ------------------------------------------------------------
static float clampf3(float v, float lo, float hi) { return v < lo ? lo : (v > hi ? hi : v); }

static lv_obj_t* s_dmOverlay;
static lv_obj_t* s_dmValue;
static lv_obj_t* s_dmTargetLbl;
static lv_obj_t* s_dmStop;
static lv_obj_t* s_dmStartBtn;
static float     s_dmTarget = 50.0f;
static uint32_t  s_dmTouchMs = 0;

static void onDmStart(lv_event_t*) {
    if (!requireOnline()) return;
    uiSendAction("send_event", EVT_START_SHORT_CODE);
}
static void onDmZero(lv_event_t*) {
    if (!requireOnline()) return;
    uiSendAction("send_event", EVT_STOP_SHORT_CODE);
}
static void onDmTargetStep(lv_event_t* e) {
    if (!requireOnline()) return;
    int dir = (int)(intptr_t)lv_event_get_user_data(e);
    s_dmTarget = clampf3(roundf((s_dmTarget + dir * 0.1f) * 10.0f) / 10.0f, 0.0f, 9999.0f);
    s_dmTouchMs = millis();
    char b[48];
    snprintf(b, sizeof(b), "action=set_dist_target&value=%d", (int)roundf(s_dmTarget * 10.0f));
    uiSend(b);
}

static void dmUpdate() {
    if (!s_dmValue) return;
    if (millis() - s_dmTouchMs > 2500) s_dmTarget = g_st.distMeterTarget;

    char b[24];
    snprintf(b, sizeof(b), "%.1f m", g_st.distMeterValue);
    setLbl(s_dmValue, b);
    snprintf(b, sizeof(b), "%.1f m", s_dmTarget);
    setLbl(s_dmTargetLbl, b);
    uiBtnSetText(s_dmStartBtn, g_st.distMeasuring ? "PAUZA" : "START");

    bool blink = ((millis() / 300) & 1) != 0;
    if (g_st.distMeterReached) {
        lv_obj_set_style_bg_color(s_dmOverlay, C_GREEN, 0);
        lv_obj_clear_flag(s_dmStop, LV_OBJ_FLAG_HIDDEN);
    } else if (g_st.distMeterWarning) {
        lv_obj_set_style_bg_color(s_dmOverlay, blink ? C_YELLOW : C_BG, 0);
        lv_obj_add_flag(s_dmStop, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_set_style_bg_color(s_dmOverlay, C_BG, 0);
        lv_obj_add_flag(s_dmStop, LV_OBJ_FLAG_HIDDEN);
    }
}

static void dmClosed() {
    uiSendAction("send_event", EVT_STOP_LONG_CODE);   // wyjscie z ekranu, zwalnia staly ton po stronie sterownika
    s_dmOverlay = s_dmValue = s_dmTargetLbl = s_dmStop = s_dmStartBtn = nullptr;
}

static void onDistMeterOpen(lv_event_t*) {
    if (!requireOnline()) return;
    uiSend("action=set_screen&value=4");   // 4 = SCREEN_DISTANCE_METER (src/config.h)
    s_dmTarget = g_st.distMeterTarget;
    s_dmOverlay = uiOverlay("POMIAR DYSTANSU", dmUpdate);
    uiSetOverlayCloseHandler(dmClosed);

#if UI_PORTRAIT
    s_dmValue = uiLabel(s_dmOverlay, "0.0 m", 0, OV_TOP + 40, FONT_XL, C_TEXT);
    lv_obj_set_width(s_dmValue, OV_W);
    lv_obj_set_style_text_align(s_dmValue, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_t* dmCelLbl = uiLabel(s_dmOverlay, "Cel (krok 0,1 m):", 0, OV_TOP + 110, FONT_M, C_DIM);
    lv_obj_set_width(dmCelLbl, OV_W);
    lv_obj_set_style_text_align(dmCelLbl, LV_TEXT_ALIGN_CENTER, 0);
    uiBtn(s_dmOverlay, "-0,1", 12, OV_TOP + 150, 100, 70, C_BTN, onDmTargetStep, (void*)(intptr_t)-1, FONT_L);
    s_dmTargetLbl = uiLabel(s_dmOverlay, "50.0 m", 0, OV_TOP + 170, FONT_L, C_YELLOW);
    lv_obj_set_width(s_dmTargetLbl, OV_W);
    lv_obj_set_style_text_align(s_dmTargetLbl, LV_TEXT_ALIGN_CENTER, 0);
    uiBtn(s_dmOverlay, "+0,1", OV_W - 112, OV_TOP + 150, 100, 70, C_BTN, onDmTargetStep, (void*)(intptr_t)+1, FONT_L);
    s_dmStop = uiLabel(s_dmOverlay, "STOP", 0, OV_TOP + 280, FONT_XL, C_TEXT);
    lv_obj_set_width(s_dmStop, OV_W);
    lv_obj_set_style_text_align(s_dmStop, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_add_flag(s_dmStop, LV_OBJ_FLAG_HIDDEN);
    s_dmStartBtn = uiBtn(s_dmOverlay, "START", 12, OV_H - 160, OV_W - 24, 70, C_GREEN, onDmStart, nullptr, FONT_L);
    uiBtn(s_dmOverlay, "ZERUJ", 12, OV_H - 80, OV_W - 24, 60, C_BTN, onDmZero, nullptr, FONT_M);
#else
    s_dmValue = uiLabel(s_dmOverlay, "0.0 m", 0, 90, FONT_XL, C_TEXT);
    lv_obj_set_width(s_dmValue, OV_W);
    lv_obj_set_style_text_align(s_dmValue, LV_TEXT_ALIGN_CENTER, 0);
    uiLabel(s_dmOverlay, "Cel (krok 0,1 m):", 280, 190, FONT_M, C_DIM);
    uiBtn(s_dmOverlay, "-0,1", 220, 220, 100, 70, C_BTN, onDmTargetStep, (void*)(intptr_t)-1, FONT_L);
    s_dmTargetLbl = uiLabel(s_dmOverlay, "50.0 m", 330, 236, FONT_L, C_YELLOW);
    lv_obj_set_width(s_dmTargetLbl, 140);
    lv_obj_set_style_text_align(s_dmTargetLbl, LV_TEXT_ALIGN_CENTER, 0);
    uiBtn(s_dmOverlay, "+0,1", 480, 220, 100, 70, C_BTN, onDmTargetStep, (void*)(intptr_t)+1, FONT_L);
    s_dmStop = uiLabel(s_dmOverlay, "STOP", 0, 200, FONT_XL, C_TEXT);
    lv_obj_set_width(s_dmStop, OV_W);
    lv_obj_set_style_text_align(s_dmStop, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_add_flag(s_dmStop, LV_OBJ_FLAG_HIDDEN);
    s_dmStartBtn = uiBtn(s_dmOverlay, "START", 220, 340, 180, 80, C_GREEN, onDmStart, nullptr, FONT_L);
    uiBtn(s_dmOverlay, "ZERUJ", 420, 340, 160, 80, C_BTN, onDmZero, nullptr, FONT_L);
#endif
    dmUpdate();
}
