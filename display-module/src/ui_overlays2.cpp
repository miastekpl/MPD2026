// ============================================================
// Nakładki funkcjonalne: statystyki, ustawienia, kalibracja,
// farba/zbiornik, edytor wzorca własnego.
// ============================================================

#include "ui_common.h"
#include "link.h"
#include "settings_store.h"
#include "app_config.h"
#include <stdio.h>
#include <math.h>

static float clampf(float v, float lo, float hi) { return v < lo ? lo : (v > hi ? hi : v); }

static void setLbl(lv_obj_t* l, const char* t) {
    if (l && strcmp(lv_label_get_text(l), t) != 0) lv_label_set_text(l, t);
}

// Stepper: [-] wartość [+]. user_data przycisków = id*2 (minus) / id*2+1 (plus)
static lv_obj_t* makeStepper(lv_obj_t* parent, int x, int y, int id, lv_event_cb_t cb, int valW) {
    uiBtn(parent, "-", x, y, 60, 52, C_BTN, cb, (void*)(intptr_t)(id * 2), FONT_L);
    lv_obj_t* v = uiLabel(parent, "", x + 60, y + 10, FONT_L, C_YELLOW);
    lv_obj_set_width(v, valW);
    lv_obj_set_style_text_align(v, LV_TEXT_ALIGN_CENTER, 0);
    uiBtn(parent, "+", x + 60 + valW, y, 60, 52, C_BTN, cb, (void*)(intptr_t)(id * 2 + 1), FONT_L);
    return v;
}

static bool requireOnline() {
    if (!g_online) { uiToast("brak lacznosci ze sterownikiem", true); return false; }
    return true;
}

// ------------------------------------------------------------
// Statystyki
// ------------------------------------------------------------
static lv_obj_t* s_statLeft = nullptr;
static lv_obj_t* s_statRight = nullptr;

static void statsUpdate() {
    if (!s_statLeft) return;
    StatsData d;
    if (!linkGetStats(d)) {
        setLbl(s_statLeft, "Pobieranie statystyk...");
        setLbl(s_statRight, "");
        return;
    }
    char a[512], b[512];
    snprintf(a, sizeof(a),
        "#FFD400 SESJA (etap)#\n"
        "Dystans:      %.1f m\n"
        "Powierzchnia: %.2f m2\n"
        "Czas:         %lu:%02lu:%02lu\n\n"
        "#FFD400 LACZNIE (od poczatku)#\n"
        "Dystans:      %.1f m\n"
        "Powierzchnia: %.2f m2\n"
        "Czas:         %lu:%02lu:%02lu\n\n"
        "Karta SD: %s   raportow: %d",
        d.sessDistance, d.sessArea,
        (unsigned long)(d.sessTimeSec / 3600), (unsigned long)((d.sessTimeSec / 60) % 60), (unsigned long)(d.sessTimeSec % 60),
        d.lifeDistance, d.lifeArea,
        (unsigned long)(d.lifeTimeSec / 3600), (unsigned long)((d.lifeTimeSec / 60) % 60), (unsigned long)(d.lifeTimeSec % 60),
        d.sdReady ? "OK" : "brak", d.reportCount);
    int n = snprintf(b, sizeof(b), "#FFD400 PISTOLETY#  (dystans / strzaly)\n");
    for (int g = 0; g < NGUNS && n < (int)sizeof(b) - 40; g++) {
        n += snprintf(b + n, sizeof(b) - n, "P%d:  %.1f m  /  %lu\n", g + 1, d.gunDist[g], (unsigned long)d.gunShots[g]);
    }
    snprintf(b + n, sizeof(b) - n,
        "\n#FFD400 FARBA#\nPoziom: %.1f L z %.0f L\nZuzyte (sesja): %.1f L\nTankowan: %d (%.0f L)",
        d.paintLevelL, d.paintTankL, d.paintUsedL, d.refuelCount, d.totalRefueledL);
    setLbl(s_statLeft, a);
    setLbl(s_statRight, b);
}

static void statsClosed() { linkWantStats(false); s_statLeft = nullptr; s_statRight = nullptr; }

void uiOpenStats() {
    lv_obj_t* ov = uiOverlay("STATYSTYKI", statsUpdate);
    uiSetOverlayCloseHandler(statsClosed);
    linkWantStats(true);
#if UI_PORTRAIT
    s_statLeft = uiLabel(ov, "", 12, OV_TOP, FONT_S, C_TEXT);
    s_statRight = uiLabel(ov, "", 12, OV_TOP + 290, FONT_S, C_TEXT);
#else
    s_statLeft = uiLabel(ov, "", 16, 68, FONT_M, C_TEXT);
    s_statRight = uiLabel(ov, "", 420, 68, FONT_M, C_TEXT);
#endif
    lv_label_set_recolor(s_statLeft, true);
    lv_label_set_recolor(s_statRight, true);
    lv_obj_set_style_text_line_space(s_statLeft, 4, 0);
    lv_obj_set_style_text_line_space(s_statRight, 4, 0);
    statsUpdate();
}

// ------------------------------------------------------------
// Ustawienia
// ------------------------------------------------------------
static lv_obj_t *s_setMax, *s_setMin, *s_setBri, *s_setSwitch, *s_setResume, *s_setLayout;
static float    s_vMax = 15, s_vMin = 0;
static uint32_t s_setTouchMs = 0;

static void onSetStep(lv_event_t* e) {
    int u = (int)(intptr_t)lv_event_get_user_data(e);
    int id = u / 2;
    int dir = (u % 2) ? +1 : -1;
    char b[64];
    if (id == 0) {
        if (!requireOnline()) return;
        s_vMax = clampf(roundf(s_vMax + dir), 5.0f, 30.0f);
        s_setTouchMs = millis();
        snprintf(b, sizeof(b), "action=set_max_speed&value=%.1f", s_vMax);
        uiSend(b);
    } else if (id == 1) {
        if (!requireOnline()) return;
        s_vMin = clampf(roundf((s_vMin + dir * 0.5f) * 2.0f) / 2.0f, 0.0f, 10.0f);
        s_setTouchMs = millis();
        snprintf(b, sizeof(b), "action=set_min_speed&value=%.1f", s_vMin);
        uiSend(b);
    } else if (id == 2) {
        int v = (int)g_settings.brightness + dir * 25;
        if (v < 20) v = 20;
        if (v > 255) v = 255;
        g_settings.brightness = (uint8_t)v;
        applyBrightness(g_settings.brightness);
    }
}

static void onSetSwitch(lv_event_t*) {
    if (!requireOnline()) return;
    uiSendAction("set_switch_mode", g_st.smartSwitch ? 1 : 0);
}

static void onSetResume(lv_event_t*) {
    if (!requireOnline()) return;
    uiSendAction("set_auto_resume", g_st.autoResume ? 0 : 1);
}

static void onSetLayout(lv_event_t*) {
    if (!requireOnline()) return;
    uiSendAction("set_pattern_layout", g_st.patBtnLayout == 1 ? 0 : 1);
}

static void settingsUpdate() {
    if (!s_setMax) return;
    if (millis() - s_setTouchMs > 2500) { s_vMax = g_st.maxSpeed; s_vMin = g_st.minSpeed; }
    char b[48];
    snprintf(b, sizeof(b), "%.0f", s_vMax);
    setLbl(s_setMax, b);
    snprintf(b, sizeof(b), "%.1f", s_vMin);
    setLbl(s_setMin, b);
    snprintf(b, sizeof(b), "%d%%", (int)g_settings.brightness * 100 / 255);
    setLbl(s_setBri, b);
    uiBtnSetText(s_setSwitch, g_st.smartSwitch ? "SMART (dokoncz cykl)" : "INSTANT (od razu)");
    uiBtnSetColor(s_setSwitch, g_st.smartSwitch ? C_GREEN : C_ORANGE);
    uiBtnSetText(s_setResume, g_st.autoResume ? "WLACZONE" : "WYLACZONE");
    uiBtnSetColor(s_setResume, g_st.autoResume ? C_GREEN : C_BTN_DIS);
    uiBtnSetText(s_setLayout, g_st.patBtnLayout == 1 ? "SOFT-KEY 10 + GRUPA" : "KLASYCZNE 15");
    uiBtnSetColor(s_setLayout, g_st.patBtnLayout == 1 ? C_GREEN : C_BTN);
}

static void settingsClosed() {
    g_settings.save();
    s_setMax = s_setMin = s_setBri = s_setSwitch = s_setResume = s_setLayout = nullptr;
}

void uiOpenSettings() {
    lv_obj_t* ov = uiOverlay("USTAWIENIA", settingsUpdate);
    uiSetOverlayCloseHandler(settingsClosed);
    s_vMax = g_st.maxSpeed;
    s_vMin = g_st.minSpeed;
    s_setTouchMs = 0;

#if UI_PORTRAIT
    // Pion: etykieta nad kontrolką, wiersze co 100 px
    const int rowStep = 100;
    int y = OV_TOP;
    uiLabel(ov, "Maks. predkosc malowania [km/h]", 12, y, FONT_M, C_TEXT);
    s_setMax = makeStepper(ov, 110, y + 30, 0, onSetStep, 150);
    y += rowStep;
    uiLabel(ov, "Min. predkosc malowania [km/h]", 12, y, FONT_M, C_TEXT);
    s_setMin = makeStepper(ov, 110, y + 30, 1, onSetStep, 150);
    y += rowStep;
    uiLabel(ov, "Zmiana wzorca w trakcie pracy", 12, y, FONT_M, C_TEXT);
    s_setSwitch = uiBtn(ov, "", 12, y + 30, 456, 52, C_GREEN, onSetSwitch, nullptr, FONT_M);
    y += rowStep;
    uiLabel(ov, "Auto-wznowienie po postoju", 12, y, FONT_M, C_TEXT);
    s_setResume = uiBtn(ov, "", 12, y + 30, 456, 52, C_GREEN, onSetResume, nullptr, FONT_M);
    y += rowStep;
    uiLabel(ov, "Przyciski wzorcow (sterownik)", 12, y, FONT_M, C_TEXT);
    s_setLayout = uiBtn(ov, "", 12, y + 30, 456, 52, C_BTN, onSetLayout, nullptr, FONT_M);
    y += rowStep;
    uiLabel(ov, "Jasnosc ekranu", 12, y, FONT_M, C_TEXT);
    s_setBri = makeStepper(ov, 110, y + 30, 2, onSetStep, 150);
#else
    int y = 68;
    uiLabel(ov, "Maks. predkosc malowania [km/h]", 16, y + 14, FONT_M, C_TEXT);
    s_setMax = makeStepper(ov, 470, y, 0, onSetStep, 130);
    y += 66;
    uiLabel(ov, "Min. predkosc malowania [km/h]", 16, y + 14, FONT_M, C_TEXT);
    s_setMin = makeStepper(ov, 470, y, 1, onSetStep, 130);
    y += 66;
    uiLabel(ov, "Zmiana wzorca w trakcie pracy", 16, y + 14, FONT_M, C_TEXT);
    s_setSwitch = uiBtn(ov, "", 470, y, 320, 52, C_GREEN, onSetSwitch, nullptr, FONT_M);
    y += 66;
    uiLabel(ov, "Auto-wznowienie po postoju", 16, y + 14, FONT_M, C_TEXT);
    s_setResume = uiBtn(ov, "", 470, y, 320, 52, C_GREEN, onSetResume, nullptr, FONT_M);
    y += 66;
    uiLabel(ov, "Przyciski wzorcow (sterownik)", 16, y + 14, FONT_M, C_TEXT);
    s_setLayout = uiBtn(ov, "", 470, y, 320, 52, C_BTN, onSetLayout, nullptr, FONT_M);
    y += 66;
    uiLabel(ov, "Jasnosc ekranu", 16, y + 14, FONT_M, C_TEXT);
    s_setBri = makeStepper(ov, 470, y, 2, onSetStep, 130);
#endif
    settingsUpdate();
}

// ------------------------------------------------------------
// Kalibracja enkodera
// ------------------------------------------------------------
static lv_obj_t *s_calInfo, *s_calStart, *s_calFinish;

static void onCalStart(lv_event_t*) {
    if (!requireOnline()) return;
    if (g_st.state == MS_PAINTING || g_st.state == MS_PAUSED) {
        uiToast("najpierw zatrzymaj malowanie", true);
        return;
    }
    uiSend("action=cal_start");
}

static void onCalFinish(lv_event_t*) {
    if (!requireOnline()) return;
    uiSend("action=cal_finish");
}

static void calUpdate() {
    if (!s_calInfo) return;
    char b[160];
    if (g_st.calibrating)
        snprintf(b, sizeof(b), "KALIBRACJA TRWA - przejedz dokladnie 10 m\nZliczone impulsy: %.0f", g_st.calPulses);
    else
        snprintf(b, sizeof(b), "%s\nAktualnie: %.2f impulsow / metr",
                 g_st.calibrated ? "Enkoder skalibrowany" : "ENKODER NIESKALIBROWANY", g_st.ppm);
    setLbl(s_calInfo, b);
    uiBtnEnable(s_calStart, g_online && !g_st.calibrating);
    uiBtnEnable(s_calFinish, g_online && g_st.calibrating);
}

static void calClosed() { s_calInfo = s_calStart = s_calFinish = nullptr; }

void uiOpenCalibration() {
    lv_obj_t* ov = uiOverlay("KALIBRACJA ENKODERA", calUpdate);
    uiSetOverlayCloseHandler(calClosed);
#if UI_PORTRAIT
    lv_obj_t* h = uiLabel(ov,
        "1. Ustaw maszyne na poczatku odcinka 10 m.\n"
        "2. Nacisnij START KALIBRACJI.\n"
        "3. Przejedz dokladnie 10 m.\n"
        "4. Nacisnij KONIEC KALIBRACJI.",
        12, OV_TOP, FONT_M, C_DIM);
    lv_obj_set_width(h, OV_W - 24);
    lv_obj_set_style_text_line_space(h, 8, 0);
    s_calInfo = uiLabel(ov, "", 12, OV_TOP + 190, FONT_M, C_YELLOW);
    lv_obj_set_width(s_calInfo, OV_W - 24);
    lv_obj_set_style_text_line_space(s_calInfo, 8, 0);
    s_calStart = uiBtn(ov, "START KALIBRACJI", 12, OV_H - 280, OV_W - 24, 120, C_GREEN, onCalStart, nullptr, FONT_L);
    s_calFinish = uiBtn(ov, "KONIEC KALIBRACJI", 12, OV_H - 145, OV_W - 24, 120, C_ORANGE, onCalFinish, nullptr, FONT_L);
#else
    lv_obj_t* h = uiLabel(ov,
        "1. Ustaw maszyne na poczatku odcinka 10 m.\n"
        "2. Nacisnij START KALIBRACJI.\n"
        "3. Przejedz dokladnie 10 m.\n"
        "4. Nacisnij KONIEC KALIBRACJI.",
        16, 72, FONT_M, C_DIM);
    lv_obj_set_style_text_line_space(h, 8, 0);
    s_calInfo = uiLabel(ov, "", 16, 220, FONT_L, C_YELLOW);
    lv_obj_set_style_text_line_space(s_calInfo, 8, 0);
    s_calStart = uiBtn(ov, "START KALIBRACJI", 16, 340, 360, 110, C_GREEN, onCalStart, nullptr, FONT_L);
    s_calFinish = uiBtn(ov, "KONIEC KALIBRACJI", 424, 340, 360, 110, C_ORANGE, onCalFinish, nullptr, FONT_L);
#endif
    calUpdate();
}

// ------------------------------------------------------------
// Farba / zbiornik
// ------------------------------------------------------------
static lv_obj_t *s_paintInfo, *s_paintTank;
static float    s_vTank = 100;
static uint32_t s_paintTouchMs = 0;

static void onPaintStep(lv_event_t* e) {
    int u = (int)(intptr_t)lv_event_get_user_data(e);
    int dir = (u % 2) ? +1 : -1;
    if (!requireOnline()) return;
    s_vTank = clampf(roundf(s_vTank + dir * 10.0f), 1.0f, 1000.0f);
    s_paintTouchMs = millis();
    char b[64];
    snprintf(b, sizeof(b), "action=set_tank_capacity&value=%.0f", s_vTank);
    uiSend(b);
}

static void onRefuel(lv_event_t* e) {
    int v = (int)(intptr_t)lv_event_get_user_data(e);
    if (!requireOnline()) return;
    if (v <= 0) {   // zbiornik pełny
        StatsData d;
        if (!linkGetStats(d)) { uiToast("brak danych o zbiorniku", true); return; }
        float diff = d.paintTankL - d.paintLevelL;
        if (diff < 1.0f) { uiToast("zbiornik juz pelny", false); return; }
        v = (int)ceilf(diff);
    }
    uiSendAction("refuel", v);
    uiToast("dolano farbe", false);
}

static void paintUpdate() {
    if (!s_paintInfo) return;
    StatsData d;
    if (!linkGetStats(d)) { setLbl(s_paintInfo, "Pobieranie danych..."); return; }
    if (millis() - s_paintTouchMs > 2500) s_vTank = d.paintTankL;
    char b[200];
    snprintf(b, sizeof(b),
        "Poziom w zbiorniku: %.1f L (%d%%)\nZuzyte w tej sesji: %.1f L\nTankowan lacznie: %d (%.0f L)",
        d.paintLevelL, g_st.paintLevelPct, d.paintUsedL, d.refuelCount, d.totalRefueledL);
    setLbl(s_paintInfo, b);
    snprintf(b, sizeof(b), "%.0f L", s_vTank);
    setLbl(s_paintTank, b);
}

static void paintClosed() { linkWantStats(false); s_paintInfo = s_paintTank = nullptr; }

void uiOpenPaint() {
    lv_obj_t* ov = uiOverlay("FARBA / ZBIORNIK", paintUpdate);
    uiSetOverlayCloseHandler(paintClosed);
    linkWantStats(true);
    s_vTank = 100;
    s_paintTouchMs = 0;
#if UI_PORTRAIT
    s_paintInfo = uiLabel(ov, "", 12, OV_TOP, FONT_M, C_TEXT);
    lv_obj_set_width(s_paintInfo, OV_W - 24);
    lv_obj_set_style_text_line_space(s_paintInfo, 8, 0);

    uiLabel(ov, "Pojemnosc zbiornika", 12, OV_TOP + 150, FONT_M, C_TEXT);
    s_paintTank = makeStepper(ov, 100, OV_TOP + 182, 0, onPaintStep, 160);

    uiLabel(ov, "Tankowanie - dolej farbe:", 12, OV_TOP + 270, FONT_M, C_DIM);
    static const int amounts[5] = {10, 25, 50, 100, 0};
    static const char* names[5] = {"+10 L", "+25 L", "+50 L", "+100 L", "DO PELNA"};
    for (int i = 0; i < 5; i++) {
        uiBtn(ov, names[i], 12 + (i % 3) * 156, OV_TOP + 306 + (i / 3) * 112, 148, 100,
              i == 4 ? C_GREEN : C_BTN, onRefuel, (void*)(intptr_t)amounts[i], FONT_L);
    }
#else
    s_paintInfo = uiLabel(ov, "", 16, 72, FONT_M, C_TEXT);
    lv_obj_set_style_text_line_space(s_paintInfo, 8, 0);

    uiLabel(ov, "Pojemnosc zbiornika", 16, 210, FONT_M, C_TEXT);
    s_paintTank = makeStepper(ov, 400, 196, 0, onPaintStep, 160);

    uiLabel(ov, "Tankowanie - dolej farbe:", 16, 280, FONT_M, C_DIM);
    static const int amounts[5] = {10, 25, 50, 100, 0};
    static const char* names[5] = {"+10 L", "+25 L", "+50 L", "+100 L", "DO PELNA"};
    for (int i = 0; i < 5; i++) {
        uiBtn(ov, names[i], 16 + i * 156, 320, 148, 100, i == 4 ? C_GREEN : C_BTN, onRefuel,
              (void*)(intptr_t)amounts[i], FONT_L);
    }
#endif
    paintUpdate();
}

// ------------------------------------------------------------
// Edytor wzorca własnego (3 sloty, 6 pistoletów)
// ------------------------------------------------------------
struct GunEdit { uint8_t mode; float line; float gap; };
static GunEdit  s_ed[NGUNS];
static int      s_edSlot = 0;
static bool     s_edLoaded = false;
static uint32_t s_edReqMs = 0;
static lv_obj_t *s_edMode[NGUNS][3];
static lv_obj_t *s_edBox[NGUNS][2];
static lv_obj_t *s_edVal[NGUNS][2];
static lv_obj_t *s_edSlotBtn[NSLOTS];
static lv_obj_t *s_edStatus;

static void edDefaults() {
    for (int g = 0; g < NGUNS; g++) s_ed[g] = {GM_OFF, 4.0f, 8.0f};
}

static void edRefresh() {
    for (int g = 0; g < NGUNS; g++) {
        for (int m = 0; m < 3; m++) {
            bool sel = (s_ed[g].mode == m);
            uiBtnSetColor(s_edMode[g][m], sel ? C_BTN_SEL : C_BTN_DIS);
            lv_obj_set_style_border_width(s_edMode[g][m], sel ? 4 : 2, 0);
            lv_obj_set_style_border_color(s_edMode[g][m], sel ? C_YELLOW : lv_color_lighten(C_BTN_DIS, 60), 0);
        }
        bool dash = (s_ed[g].mode == GM_DASH);
        for (int w = 0; w < 2; w++) {
            if (dash) lv_obj_clear_flag(s_edBox[g][w], LV_OBJ_FLAG_HIDDEN);
            else lv_obj_add_flag(s_edBox[g][w], LV_OBJ_FLAG_HIDDEN);
        }
        char b[16];
        snprintf(b, sizeof(b), "%.1f", s_ed[g].line);
        setLbl(s_edVal[g][0], b);
        snprintf(b, sizeof(b), "%.1f", s_ed[g].gap);
        setLbl(s_edVal[g][1], b);
    }
    for (int s = 0; s < NSLOTS; s++) {
        char b[16];
        snprintf(b, sizeof(b), "SLOT %d%s", s + 1, g_st.slotsValid[s] ? " *" : "");
        uiBtnSetText(s_edSlotBtn[s], b);
        uiBtnSetColor(s_edSlotBtn[s], s == s_edSlot ? C_BTN_SEL : C_BTN);
    }
}

static void edRequestSlot(int slot) {
    s_edSlot = slot;
    s_edLoaded = false;
    s_edReqMs = millis();
    linkRequestSlot(slot);
    setLbl(s_edStatus, "Wczytywanie slotu...");
}

static void onEdSlot(lv_event_t* e) {
    int s = (int)(intptr_t)lv_event_get_user_data(e);
    if (!requireOnline()) return;
    edRequestSlot(s);
    edRefresh();
}

static void onEdMode(lv_event_t* e) {
    int u = (int)(intptr_t)lv_event_get_user_data(e);
    int g = u / 3, m = u % 3;
    s_ed[g].mode = (uint8_t)m;
    edRefresh();
}

// user = id*2 (minus) / id*2+1 (plus), id = g*2 + which (0=kreska, 1=przerwa)
static void onEdStep(lv_event_t* e) {
    int u = (int)(intptr_t)lv_event_get_user_data(e);
    int id = u / 2;
    int dir = (u % 2) ? +1 : -1;
    int g = id / 2, which = id % 2;
    float& v = which == 0 ? s_ed[g].line : s_ed[g].gap;
    v = clampf(roundf((v + dir * 0.5f) * 2.0f) / 2.0f, 0.5f, 50.0f);
    edRefresh();
}

static void edSave(bool useAfter) {
    if (!requireOnline()) return;
    if (!s_edLoaded) { uiToast("slot jeszcze nie wczytany", true); return; }
    bool any = false;
    for (int g = 0; g < NGUNS; g++) if (s_ed[g].mode != GM_OFF) any = true;
    if (!any) { uiToast("wlacz przynajmniej jeden pistolet", true); return; }

    char body[200];
    int n = snprintf(body, sizeof(body), "action=save_custom_pattern&slot=%d", s_edSlot);
    for (int g = 0; g < NGUNS && n < (int)sizeof(body) - 32; g++) {
        n += snprintf(body + n, sizeof(body) - n, "&g%d=%d&ln%d=%.1f&gp%d=%.1f",
                      g, (int)s_ed[g].mode, g, s_ed[g].line, g, s_ed[g].gap);
    }
    uiSend(body);
    if (useAfter) uiSendAction("set_pattern", PAT_CUSTOM_IDX);
    uiToast(useAfter ? "zapisano i wybrano wzorzec wlasny" : "zapisano wzorzec wlasny", false);
}

static void onEdSave(lv_event_t*)    { edSave(false); }
static void onEdSaveUse(lv_event_t*) { edSave(true); }

static void edUpdate() {
    if (!s_edStatus) return;
    if (!s_edLoaded) {
        SlotCfg c;
        if (linkGetSlot(s_edSlot, c)) {
            for (int g = 0; g < NGUNS; g++) {
                s_ed[g].mode = c.guns[g].mode;
                s_ed[g].line = clampf(c.guns[g].line > 0 ? c.guns[g].line : 4.0f, 0.5f, 50.0f);
                s_ed[g].gap  = clampf(c.guns[g].gap  > 0 ? c.guns[g].gap  : 8.0f, 0.5f, 50.0f);
            }
            if (!c.valid) edDefaults();
            s_edLoaded = true;
            setLbl(s_edStatus, c.valid ? "Slot wczytany" : "Slot pusty - ustaw wzorzec");
            edRefresh();
        } else if (millis() - s_edReqMs > 3500) {
            linkRequestSlot(s_edSlot);
            s_edReqMs = millis();
            setLbl(s_edStatus, "Brak odpowiedzi sterownika...");
        }
    }
    edRefresh();
}

static void edClosed() { s_edStatus = nullptr; }

void uiOpenCustomEditor() {
    lv_obj_t* ov = uiOverlay("WZOR WLASNY", edUpdate);
    uiSetOverlayCloseHandler(edClosed);
    edDefaults();
    static const char* modeNames[3] = {"WYL", "CIAGLY", "PRZERYW."};

#if UI_PORTRAIT
    // Pion: 3 przyciski slotów, potem 6 pistoletów po 2 wiersze (tryb / kreska + przerwa)
    for (int s = 0; s < NSLOTS; s++) {
        s_edSlotBtn[s] = uiBtn(ov, "", 12 + s * 154, OV_TOP, 148, 40, C_BTN, onEdSlot,
                               (void*)(intptr_t)s, FONT_S);
    }
    for (int g = 0; g < NGUNS; g++) {
        int y = OV_TOP + 52 + g * 92;
        char t[8];
        snprintf(t, sizeof(t), "P%d", g + 1);
        uiLabel(ov, t, 12, y + 10, FONT_M, C_TEXT);
        for (int m = 0; m < 3; m++) {
            s_edMode[g][m] = uiBtn(ov, modeNames[m], 52 + m * 138, y, 132, 44, C_BTN_DIS, onEdMode,
                                   (void*)(intptr_t)(g * 3 + m), FONT_S);
        }
        for (int w = 0; w < 2; w++) {
            lv_obj_t* box = lv_obj_create(ov);
            lv_obj_remove_style_all(box);
            lv_obj_set_pos(box, 12 + w * 234, y + 46);
            lv_obj_set_size(box, 222, 40);
            lv_obj_clear_flag(box, LV_OBJ_FLAG_SCROLLABLE);
            int id = g * 2 + w;
            uiLabel(box, w == 0 ? "kreska" : "przerwa", 0, 12, FONT_S, C_DIM);
            uiBtn(box, "-", 62, 0, 44, 40, C_BTN, onEdStep, (void*)(intptr_t)(id * 2), FONT_M);
            lv_obj_t* v = uiLabel(box, "", 106, 8, FONT_M, C_YELLOW);
            lv_obj_set_width(v, 70);
            lv_obj_set_style_text_align(v, LV_TEXT_ALIGN_CENTER, 0);
            uiBtn(box, "+", 176, 0, 44, 40, C_BTN, onEdStep, (void*)(intptr_t)(id * 2 + 1), FONT_M);
            s_edBox[g][w] = box;
            s_edVal[g][w] = v;
        }
    }
    uiBtn(ov, "ZAPISZ", 12, OV_H - 86, 200, 56, C_GREEN, onEdSave, nullptr, FONT_M);
    uiBtn(ov, "ZAPISZ I UZYJ", 222, OV_H - 86, 246, 56, C_GREEN, onEdSaveUse, nullptr, FONT_M);
    s_edStatus = uiLabel(ov, "", 12, OV_H - 24, FONT_S, C_YELLOW);
    lv_obj_set_width(s_edStatus, OV_W - 24);
#else
    for (int s = 0; s < NSLOTS; s++) {
        s_edSlotBtn[s] = uiBtn(ov, "", 16 + s * 112, 60, 106, 34, C_BTN, onEdSlot,
                               (void*)(intptr_t)s, FONT_S);
    }
    s_edStatus = uiLabel(ov, "", 590, 424, FONT_S, C_YELLOW);
    lv_obj_set_width(s_edStatus, 200);
    uiLabel(ov, "KRESKA [m]", 450, 70, FONT_S, C_DIM);
    uiLabel(ov, "PRZERWA [m]", 645, 70, FONT_S, C_DIM);

    for (int g = 0; g < NGUNS; g++) {
        int y = 100 + g * 50;
        char t[8];
        snprintf(t, sizeof(t), "P%d", g + 1);
        uiLabel(ov, t, 16, y + 10, FONT_L, C_TEXT);
        for (int m = 0; m < 3; m++) {
            s_edMode[g][m] = uiBtn(ov, modeNames[m], 70 + m * 108, y, 104, 48, C_BTN_DIS, onEdMode,
                                   (void*)(intptr_t)(g * 3 + m), FONT_S);
        }
        for (int w = 0; w < 2; w++) {
            lv_obj_t* box = lv_obj_create(ov);
            lv_obj_remove_style_all(box);
            lv_obj_set_pos(box, 400 + w * 200, y);
            lv_obj_set_size(box, 190, 48);
            lv_obj_clear_flag(box, LV_OBJ_FLAG_SCROLLABLE);
            int id = g * 2 + w;
            uiBtn(box, "-", 0, 0, 52, 48, C_BTN, onEdStep, (void*)(intptr_t)(id * 2), FONT_L);
            lv_obj_t* v = uiLabel(box, "", 52, 8, FONT_L, C_YELLOW);
            lv_obj_set_width(v, 86);
            lv_obj_set_style_text_align(v, LV_TEXT_ALIGN_CENTER, 0);
            uiBtn(box, "+", 138, 0, 52, 48, C_BTN, onEdStep, (void*)(intptr_t)(id * 2 + 1), FONT_L);
            s_edBox[g][w] = box;
            s_edVal[g][w] = v;
        }
    }
    uiBtn(ov, "ZAPISZ", 16, 410, 250, 62, C_GREEN, onEdSave, nullptr, FONT_M);
    uiBtn(ov, "ZAPISZ I UZYJ", 276, 410, 300, 62, C_GREEN, onEdSaveUse, nullptr, FONT_M);
#endif

    edRequestSlot(g_st.activeSlot >= 0 && g_st.activeSlot < NSLOTS ? g_st.activeSlot : 0);
    edRefresh();
}
