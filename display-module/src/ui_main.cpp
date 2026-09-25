// ============================================================
// Główny ekran roboczy (styl kabinowy): prędkość 7-seg, droga w
// perspektywie, szybkie wzorce po bokach, tryby i START/STOP na dole.
// ============================================================

#include "ui_common.h"
#include "link.h"
#include "settings_store.h"
#include "app_config.h"
#include <stdio.h>
#include <math.h>

Status g_st;
bool   g_stValid = false;
bool   g_online = false;

// ---------- układ poziomy (piksele); układ pionowy ma stałe w buildMainPortrait() ----------
static const int TOP_H       = 56;
static const int COL_W       = 138;
static const int SLOT_H      = 62;
static const int SLOT_STEP   = 66;
static const int SLOT_Y0     = 62;
static const int CX0         = 150;              // lewa krawędź strefy środkowej
static const int CW          = 500;              // szerokość strefy środkowej
static const int ROAD_Y      = 176;
static const int ROAD_H      = 176;
static const int PILL_Y      = 356;
static const int BAR_Y       = 394;
static const int BAR_H       = 80;

// ---------- widgety ----------
static lv_obj_t* s_lblLink;
static lv_obj_t* s_lblGps;
static lv_obj_t* s_barPaint;
static lv_obj_t* s_lblPaint;
static lv_obj_t* s_tabAxis;
static lv_obj_t* s_tabEdge;

static lv_obj_t* s_slotBtn[10];
static lv_obj_t* s_slotGlyph[10];
static lv_obj_t* s_slotLbl[10];
static lv_obj_t* s_slotSpec[10];   // podpis liczbowy pod kodem (układ pionowy)
static lv_obj_t* s_bigGlyph;       // duży rysunek aktualnego wzorca (układ pionowy)
static lv_obj_t* s_lblSpec;        // zapis liczbowy aktualnego wzorca (układ pionowy)
static bool      s_slotSel[10];

static lv_obj_t* s_speed;
static lv_obj_t* s_lblUnit;
static lv_obj_t* s_lblPattern;
static lv_obj_t* s_lblMode;
static lv_obj_t* s_lblDist;
static lv_obj_t* s_lblArea;
static lv_obj_t* s_lblTime;

static lv_obj_t* s_road;
static lv_obj_t* s_lblPending;
static lv_obj_t* s_btnReverse;
static lv_obj_t* s_banner;
static lv_obj_t* s_lblBanner;
static lv_obj_t* s_pill[NGUNS];

static lv_obj_t* s_btnMode[3];
static lv_obj_t* s_btnStart;
static lv_obj_t* s_btnStop;
static lv_obj_t* s_btnGap;

static lv_obj_t* s_cover;
static lv_obj_t* s_lblCoverTitle;
static lv_obj_t* s_lblCoverReason;

static uint32_t  s_lastResultSeq = 0;

// ---------- pomocnicze ----------
static void setLbl(lv_obj_t* l, const char* t) {
    if (strcmp(lv_label_get_text(l), t) != 0) lv_label_set_text(l, t);
}

static void setTxtColor(lv_obj_t* l, lv_color_t c) {
    if (lv_obj_get_style_text_color(l, LV_PART_MAIN).full != c.full)
        lv_obj_set_style_text_color(l, c, 0);
}

static void setVisible(lv_obj_t* o, bool vis) {
    bool hidden = lv_obj_has_flag(o, LV_OBJ_FLAG_HIDDEN);
    if (vis && hidden) lv_obj_clear_flag(o, LV_OBJ_FLAG_HIDDEN);
    else if (!vis && !hidden) lv_obj_add_flag(o, LV_OBJ_FLAG_HIDDEN);
}

static void fmtTime(uint32_t sec, char* out, size_t n) {
    uint32_t h = sec / 3600, m = (sec / 60) % 60, s = sec % 60;
    if (h > 0) snprintf(out, n, "%lu:%02lu:%02lu", (unsigned long)h, (unsigned long)m, (unsigned long)s);
    else       snprintf(out, n, "%02lu:%02lu", (unsigned long)m, (unsigned long)s);
}

static bool needOnline() {
    if (!g_online) {
        uiToast("brak lacznosci ze sterownikiem", true);
        return false;
    }
    return true;
}

// ---------- strony wzorców: OŚ / KRAWĘDŹ ----------
static int      s_localGroup = 0;      // rezerwa dla sterownika bez pola patGroup
static int      s_grpOverride = -1;    // wybór z ekranu do czasu potwierdzenia przez sterownik
static uint32_t s_grpOverrideMs = 0;

static int currentGroup(const Status& st) {
    if (s_grpOverride >= 0 && millis() - s_grpOverrideMs < 1500) return s_grpOverride;
    s_grpOverride = -1;
    if (st.hasPatGroup) return st.patGroup;
    s_localGroup = patternGroupOf(st.patternIdx, s_localGroup);
    return s_localGroup;
}

static void onTab(lv_event_t* e) {
    int g = (int)(intptr_t)lv_event_get_user_data(e);
    if (!needOnline()) return;
    s_grpOverride = g;
    s_grpOverrideMs = millis();
    s_localGroup = g;
    uiSendAction("set_pattern_group", g);
}

// ---------- callbacki przycisków ----------
static void onSlotClick(lv_event_t* e) {
    int s = (int)(intptr_t)lv_event_get_user_data(e);
    int pat = softKeyPattern(currentGroup(g_st), s);
    if (pat < 0) return;
    if (!needOnline()) return;
    if (pat == PAT_CUSTOM_IDX && !g_st.customValid) {
        uiToast("wzorzec wlasny nie jest zapisany", true);
        return;
    }
    uiSendAction("set_pattern", pat);
}

static void onMode(lv_event_t* e) {
    int m = (int)(intptr_t)lv_event_get_user_data(e);
    if (!needOnline()) return;
    uiSendAction("set_mode", m);
}

static void onStart(lv_event_t*) {
    if (!needOnline()) return;
    if (g_st.state == MS_PAINTING) {
        if (g_st.mode == MM_SEMI && g_st.semiLineComplete) uiSend("action=semi_next_line");
        else uiSend("action=pause");
    } else {
        uiSend("action=start");
    }
}

static void onStop(lv_event_t*) {
    uiSend("action=stop", true);
}

static void onGap(lv_event_t*) {
    if (!needOnline()) return;
    uiSend("action=start_from_gap");
}

static void onReverse(lv_event_t*) {
    if (!needOnline()) return;
    uiSend("action=toggle_reverse");
}

static void onMenu(lv_event_t*) { uiOpenMenu(); }
static void onCoverWifi(lv_event_t*) { uiOpenWifi(); }

// ---------- budowa ekranu ----------
#if UI_PORTRAIT
static const lv_font_t* const SLOT_FONT = FONT_M;
#else
static const lv_font_t* const SLOT_FONT = FONT_L;
#endif

static lv_obj_t* makeSlot(lv_obj_t* parent, int s, int x, int y, int w, int h,
                          int gx, int gy, int gw, int gh) {
    lv_obj_t* b = uiBtn(parent, "", x, y, w, h, C_BTN, onSlotClick,
                        (void*)(intptr_t)s, SLOT_FONT);
    lv_obj_t* lbl = lv_obj_get_child(b, 0);
#if UI_PORTRAIT
    lv_obj_align(lbl, LV_ALIGN_TOP_RIGHT, -4, 6);
    lv_obj_t* sp = lv_label_create(b);
    lv_label_set_text(sp, "");
    lv_obj_set_width(sp, 62);
    lv_obj_set_style_text_font(sp, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(sp, C_TEXT, 0);
    lv_obj_set_style_text_align(sp, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_align(sp, LV_ALIGN_TOP_RIGHT, -4, 34);
    s_slotSpec[s] = sp;
#else
    lv_obj_align(lbl, LV_ALIGN_RIGHT_MID, -6, 0);
    s_slotSpec[s] = nullptr;
#endif
    s_slotLbl[s] = lbl;
    s_slotGlyph[s] = uiGlyph(b, gx, gy, gw, gh);
    s_slotBtn[s] = b;
    return b;
}

// Etykieta o stałej szerokości, wyrównana do środka
static lv_obj_t* centerLabel(lv_obj_t* parent, const char* text, int x, int y, int w,
                             const lv_font_t* font, lv_color_t color) {
    lv_obj_t* l = uiLabel(parent, text, x, y, font, color);
    lv_obj_set_width(l, w);
    lv_obj_set_style_text_align(l, LV_TEXT_ALIGN_CENTER, 0);
    return l;
}

#if UI_PORTRAIT
// ============================================================
// Układ PIONOWY 480x800: kolumny wzorców przy krawędziach ekranu
// (fizyczne klawisze S1-S5 i S6-S10 leżą obok etykiet na tej samej wysokości)
// ============================================================
static void buildMainPortrait() {
    const int SW = SCR_W, SH = SCR_H;
    const int COLW = 108, SLOTH = 106, STEP = 116, Y0 = 106;
    const int MX = 118, MW = 244;                    // strefa środkowa
    lv_obj_t* scr = lv_scr_act();
    lv_obj_set_style_bg_color(scr, C_BG, 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);
    lv_obj_clear_flag(scr, LV_OBJ_FLAG_SCROLLABLE);

    // --- pasek górny ---
    s_lblLink  = uiLabel(scr, LV_SYMBOL_WIFI " ---", 8, 14, FONT_S, C_DIM);
    s_lblGps   = uiLabel(scr, LV_SYMBOL_GPS " ---", 122, 14, FONT_S, C_DIM);
    s_barPaint = lv_bar_create(scr);
    lv_obj_set_pos(s_barPaint, 244, 10);
    lv_obj_set_size(s_barPaint, 92, 28);
    lv_bar_set_range(s_barPaint, 0, 100);
    lv_obj_set_style_bg_color(s_barPaint, C_PANEL, LV_PART_MAIN);
    lv_obj_set_style_bg_color(s_barPaint, C_GREEN, LV_PART_INDICATOR);
    lv_obj_set_style_radius(s_barPaint, 6, LV_PART_MAIN);
    lv_obj_set_style_radius(s_barPaint, 6, LV_PART_INDICATOR);
    s_lblPaint = lv_label_create(s_barPaint);
    lv_label_set_text(s_lblPaint, "FARBA --");
    lv_obj_set_style_text_font(s_lblPaint, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(s_lblPaint, C_TEXT, 0);
    lv_obj_center(s_lblPaint);
    uiBtn(scr, LV_SYMBOL_LIST " MENU", SW - 4 - 130, 4, 130, 42, C_BTN, onMenu, nullptr, FONT_M);

    // --- zakładki grup (dotyk; ta sama grupa co fizyczny przycisk GRUPA) ---
    s_tabAxis = uiBtn(scr, "OS JEZDNI", 6, 54, 228, 42, C_BTN, onTab, (void*)(intptr_t)0, FONT_M);
    s_tabEdge = uiBtn(scr, "KRAWEDZ", 246, 54, 228, 42, C_BTN, onTab, (void*)(intptr_t)1, FONT_M);

    // --- kolumny wzorców ---
    for (int i = 0; i < 5; i++)
        makeSlot(scr, i, 4, Y0 + i * STEP, COLW, SLOTH, 6, 8, 34, SLOTH - 16);
    for (int j = 0; j < 5; j++)
        makeSlot(scr, 5 + j, SW - 4 - COLW, Y0 + j * STEP, COLW, SLOTH, 6, 8, 34, SLOTH - 16);

    // --- środek: wzorzec, prędkość, liczniki ---
    // duży rysunek aktualnego wzorca (w skali) obok kodu i zapisu liczbowego
    s_bigGlyph = uiGlyph(scr, MX + 2, 102, 66, 80);
    s_lblPattern = uiLabel(scr, "---", MX + 76, 100, FONT_XL, C_YELLOW);
    lv_obj_set_width(s_lblPattern, MW - 78);
    s_lblSpec = uiLabel(scr, "", MX + 76, 148, &lv_font_montserrat_12, C_TEXT);
    lv_obj_set_width(s_lblSpec, MW - 78);
    lv_obj_set_style_text_line_space(s_lblSpec, 2, 0);
    s_lblMode = centerLabel(scr, "---", MX, 184, MW, FONT_S, C_TEXT);
    lv_label_set_long_mode(s_lblMode, LV_LABEL_LONG_DOT);
    lv_obj_set_height(s_lblMode, 22);
    s_speed = uiSevenSeg(scr, MX + (MW - 172) / 2, 208, 172, 60);
    s_lblUnit = centerLabel(scr, "km/h", MX, 270, MW, FONT_M, C_DIM);
    s_lblDist = centerLabel(scr, "", MX, 298, MW, FONT_S, C_TEXT);
    s_lblArea = centerLabel(scr, "", MX, 318, MW, FONT_S, C_TEXT);
    s_lblTime = centerLabel(scr, "", MX, 338, MW, FONT_S, C_TEXT);
    lv_label_set_recolor(s_lblDist, true);
    lv_label_set_recolor(s_lblArea, true);
    lv_label_set_recolor(s_lblTime, true);

    // --- droga ---
    const int RY = 364, RH = 240;
    s_road = uiRoadView(scr, MX, RY, MW, RH);
    s_btnReverse = uiBtn(s_road, LV_SYMBOL_LOOP " ODWROC", 6, 4, 130, 34, C_BTN, onReverse, nullptr, FONT_S);
    s_lblPending = uiLabel(s_road, "", 6, 42, FONT_S, C_YELLOW);
    lv_obj_set_width(s_lblPending, MW - 12);
    s_banner = lv_obj_create(s_road);
    lv_obj_remove_style_all(s_banner);
    lv_obj_set_size(s_banner, MW - 16, 58);
    lv_obj_set_pos(s_banner, 8, 84);
    lv_obj_set_style_bg_color(s_banner, C_RED, 0);
    lv_obj_set_style_bg_opa(s_banner, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(s_banner, 8, 0);
    lv_obj_clear_flag(s_banner, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
    s_lblBanner = lv_label_create(s_banner);
    lv_label_set_text(s_lblBanner, "");
    lv_label_set_long_mode(s_lblBanner, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(s_lblBanner, MW - 32);
    lv_obj_set_style_text_align(s_lblBanner, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_font(s_lblBanner, FONT_S, 0);
    lv_obj_set_style_text_color(s_lblBanner, C_TEXT, 0);
    lv_obj_center(s_lblBanner);
    lv_obj_add_flag(s_banner, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(s_btnReverse, LV_OBJ_FLAG_HIDDEN);

    // --- kapsuły pistoletów ---
    for (int g = 0; g < NGUNS; g++) {
        lv_obj_t* p = lv_obj_create(scr);
        lv_obj_remove_style_all(p);
        lv_obj_set_pos(p, MX + 2 + g * 41, RY + RH + 6);
        lv_obj_set_size(p, 38, 28);
        lv_obj_set_style_bg_color(p, C_PANEL, 0);
        lv_obj_set_style_bg_opa(p, LV_OPA_COVER, 0);
        lv_obj_set_style_radius(p, 14, 0);
        lv_obj_set_style_border_width(p, 2, 0);
        lv_obj_set_style_border_color(p, C_PANEL, 0);
        lv_obj_clear_flag(p, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
        char t[8];
        snprintf(t, sizeof(t), "P%d", g + 1);
        lv_obj_t* l = lv_label_create(p);
        lv_label_set_text(l, t);
        lv_obj_set_style_text_font(l, &lv_font_montserrat_12, 0);
        lv_obj_set_style_text_color(l, C_DIM, 0);
        lv_obj_center(l);
        s_pill[g] = p;
    }

    // --- pasek dolny: tryby, START OD PRZERWY, START, STOP ---
    static const char* modeTxt[3] = {"AUTO", "SEMI", "RECZNY"};
    for (int m = 0; m < 3; m++) {
        s_btnMode[m] = uiBtn(scr, modeTxt[m], 4 + m * 96, 694, 92, 44, C_BTN,
                             onMode, (void*)(intptr_t)m, FONT_S);
    }
    s_btnGap = uiBtn(scr, "START OD PRZERWY", 292, 694, 184, 44, C_BTN, onGap, nullptr, FONT_S);
    s_btnStart = uiBtn(scr, LV_SYMBOL_PLAY " START", 4, 746, 296, 50, C_GREEN, onStart, nullptr, FONT_L);
    s_btnStop = uiBtn(scr, LV_SYMBOL_STOP " STOP", 306, 746, 170, 50, C_RED, onStop, nullptr, FONT_L);

    // --- nakładka braku łączności ---
    s_cover = lv_obj_create(scr);
    lv_obj_remove_style_all(s_cover);
    lv_obj_set_pos(s_cover, 0, 50);
    lv_obj_set_size(s_cover, SW, SH - 50);
    lv_obj_set_style_bg_color(s_cover, C_BG, 0);
    lv_obj_set_style_bg_opa(s_cover, LV_OPA_90, 0);
    lv_obj_clear_flag(s_cover, LV_OBJ_FLAG_SCROLLABLE);
    s_lblCoverTitle = centerLabel(s_cover, "BRAK LACZNOSCI ZE STEROWNIKIEM", 12, 150, SW - 24, FONT_L, C_RED);
    lv_label_set_long_mode(s_lblCoverTitle, LV_LABEL_LONG_WRAP);
    s_lblCoverReason = centerLabel(s_cover, "", 12, 290, SW - 24, FONT_M, C_TEXT);
    lv_label_set_long_mode(s_lblCoverReason, LV_LABEL_LONG_WRAP);
    lv_obj_t* warn = centerLabel(s_cover, "UZYJ FIZYCZNEGO PRZYCISKU STOP NA STEROWNIKU", 12, 400,
                                 SW - 24, FONT_M, C_ORANGE);
    lv_label_set_long_mode(warn, LV_LABEL_LONG_WRAP);
    uiBtn(s_cover, LV_SYMBOL_WIFI " USTAW POLACZENIE WiFi", 40, 520, SW - 80, 64, C_BTN,
          onCoverWifi, nullptr, FONT_M);

    s_lastResultSeq = linkResultSeq();
}
#endif  // UI_PORTRAIT

// ============================================================
// Układ POZIOMY 800x480
// ============================================================
#if !UI_PORTRAIT
static void buildMainLandscape() {
    lv_obj_t* scr = lv_scr_act();
    lv_obj_set_style_bg_color(scr, C_BG, 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);
    lv_obj_clear_flag(scr, LV_OBJ_FLAG_SCROLLABLE);

    // --- pasek górny ---
    s_lblLink  = uiLabel(scr, LV_SYMBOL_WIFI " ---", 10, 14, FONT_M, C_DIM);
    s_lblGps   = uiLabel(scr, LV_SYMBOL_GPS " ---", 210, 14, FONT_M, C_DIM);
    s_barPaint = lv_bar_create(scr);
    lv_obj_set_pos(s_barPaint, 350, 12);
    lv_obj_set_size(s_barPaint, 110, 30);
    lv_bar_set_range(s_barPaint, 0, 100);
    lv_obj_set_style_bg_color(s_barPaint, C_PANEL, LV_PART_MAIN);
    lv_obj_set_style_bg_color(s_barPaint, C_GREEN, LV_PART_INDICATOR);
    lv_obj_set_style_radius(s_barPaint, 6, LV_PART_MAIN);
    lv_obj_set_style_radius(s_barPaint, 6, LV_PART_INDICATOR);
    s_lblPaint = lv_label_create(s_barPaint);
    lv_label_set_text(s_lblPaint, "FARBA --");
    lv_obj_set_style_text_font(s_lblPaint, FONT_S, 0);
    lv_obj_set_style_text_color(s_lblPaint, C_TEXT, 0);
    lv_obj_center(s_lblPaint);
    s_tabAxis = uiBtn(scr, "OS", 470, 6, 78, 46, C_BTN, onTab, (void*)(intptr_t)0, FONT_M);
    s_tabEdge = uiBtn(scr, "KRAWEDZ", 552, 6, 96, 46, C_BTN, onTab, (void*)(intptr_t)1, FONT_S);
    uiBtn(scr, LV_SYMBOL_LIST " MENU", 654, 6, 138, 46, C_BTN, onMenu, nullptr, FONT_M);

    // --- kolumny szybkich wzorców ---
    for (int i = 0; i < 5; i++)
        makeSlot(scr, i, 6, SLOT_Y0 + i * SLOT_STEP, COL_W, SLOT_H, 6, 6, 40, 48);
    for (int j = 0; j < 5; j++)
        makeSlot(scr, 5 + j, 800 - 6 - COL_W, SLOT_Y0 + j * SLOT_STEP, COL_W, SLOT_H, 6, 6, 40, 48);

    // --- prędkość i wzorzec ---
    s_speed = uiSevenSeg(scr, CX0 + 6, 62, 200, 76);
    s_lblUnit = uiLabel(scr, "km/h", CX0 + 214, 100, FONT_L, C_DIM);
    s_lblPattern = uiLabel(scr, "---", CX0 + 300, 58, FONT_XL, C_YELLOW);
    lv_obj_set_width(s_lblPattern, 200);
    lv_obj_set_style_text_align(s_lblPattern, LV_TEXT_ALIGN_RIGHT, 0);
    s_lblMode = uiLabel(scr, "---", CX0 + 200, 108, FONT_M, C_TEXT);
    lv_obj_set_width(s_lblMode, 300);
    lv_obj_set_style_text_align(s_lblMode, LV_TEXT_ALIGN_RIGHT, 0);

    s_lblDist = uiLabel(scr, "", CX0 + 6, 142, FONT_M, C_TEXT);
    s_lblArea = uiLabel(scr, "", CX0 + 176, 142, FONT_M, C_TEXT);
    s_lblTime = uiLabel(scr, "", CX0 + 380, 142, FONT_M, C_TEXT);
    lv_label_set_recolor(s_lblDist, true);
    lv_label_set_recolor(s_lblArea, true);
    lv_label_set_recolor(s_lblTime, true);

    // --- droga ---
    s_road = uiRoadView(scr, CX0, ROAD_Y, CW, ROAD_H);
    s_lblPending = uiLabel(s_road, "", 10, 6, FONT_M, C_YELLOW);
    s_btnReverse = uiBtn(s_road, LV_SYMBOL_LOOP " ODWROC", CW - 138, 6, 130, 38, C_BTN,
                         onReverse, nullptr, FONT_S);
    s_banner = lv_obj_create(s_road);
    lv_obj_remove_style_all(s_banner);
    lv_obj_set_size(s_banner, CW - 60, 44);
    lv_obj_set_pos(s_banner, 30, 62);
    lv_obj_set_style_bg_color(s_banner, C_RED, 0);
    lv_obj_set_style_bg_opa(s_banner, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(s_banner, 8, 0);
    lv_obj_clear_flag(s_banner, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
    s_lblBanner = lv_label_create(s_banner);
    lv_label_set_text(s_lblBanner, "");
    lv_obj_set_style_text_font(s_lblBanner, FONT_L, 0);
    lv_obj_set_style_text_color(s_lblBanner, C_TEXT, 0);
    lv_obj_center(s_lblBanner);
    lv_obj_add_flag(s_banner, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(s_btnReverse, LV_OBJ_FLAG_HIDDEN);

    // --- kapsuły pistoletów ---
    for (int g = 0; g < NGUNS; g++) {
        lv_obj_t* p = lv_obj_create(scr);
        lv_obj_remove_style_all(p);
        lv_obj_set_pos(p, CX0 + g * 84, PILL_Y);
        lv_obj_set_size(p, 78, 32);
        lv_obj_set_style_bg_color(p, C_PANEL, 0);
        lv_obj_set_style_bg_opa(p, LV_OPA_COVER, 0);
        lv_obj_set_style_radius(p, 16, 0);
        lv_obj_set_style_border_width(p, 3, 0);
        lv_obj_set_style_border_color(p, C_PANEL, 0);
        lv_obj_clear_flag(p, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
        char t[8];
        snprintf(t, sizeof(t), "P%d", g + 1);
        lv_obj_t* l = lv_label_create(p);
        lv_label_set_text(l, t);
        lv_obj_set_style_text_font(l, FONT_M, 0);
        lv_obj_set_style_text_color(l, C_DIM, 0);
        lv_obj_center(l);
        s_pill[g] = p;
    }

    // --- pasek dolny ---
    static const char* modeTxt[3] = {"AUTO", "SEMI", "RECZNY"};
    for (int m = 0; m < 3; m++) {
        s_btnMode[m] = uiBtn(scr, modeTxt[m], 6 + m * 108, BAR_Y, 104, BAR_H, C_BTN,
                             onMode, (void*)(intptr_t)m, FONT_M);
    }
    s_btnStart = uiBtn(scr, LV_SYMBOL_PLAY " START", 336, BAR_Y, 214, BAR_H, C_GREEN,
                       onStart, nullptr, FONT_L);
    s_btnStop = uiBtn(scr, LV_SYMBOL_STOP " STOP", 556, BAR_Y, 132, BAR_H, C_RED,
                      onStop, nullptr, FONT_L);
    s_btnGap = uiBtn(scr, "START OD\nPRZERWY", 694, BAR_Y, 100, BAR_H, C_BTN,
                     onGap, nullptr, FONT_S);

    // --- nakładka braku łączności ---
    s_cover = lv_obj_create(scr);
    lv_obj_remove_style_all(s_cover);
    lv_obj_set_pos(s_cover, 0, TOP_H);
    lv_obj_set_size(s_cover, 800, 480 - TOP_H);
    lv_obj_set_style_bg_color(s_cover, C_BG, 0);
    lv_obj_set_style_bg_opa(s_cover, LV_OPA_90, 0);
    lv_obj_clear_flag(s_cover, LV_OBJ_FLAG_SCROLLABLE);
    s_lblCoverTitle = uiLabel(s_cover, "BRAK LACZNOSCI ZE STEROWNIKIEM", 0, 90, FONT_XL, C_RED);
    lv_obj_set_width(s_lblCoverTitle, 800);
    lv_obj_set_style_text_align(s_lblCoverTitle, LV_TEXT_ALIGN_CENTER, 0);
    s_lblCoverReason = uiLabel(s_cover, "", 0, 160, FONT_L, C_TEXT);
    lv_obj_set_width(s_lblCoverReason, 800);
    lv_obj_set_style_text_align(s_lblCoverReason, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_t* warn = uiLabel(s_cover, "UZYJ FIZYCZNEGO PRZYCISKU STOP NA STEROWNIKU", 0, 250,
                             FONT_L, C_ORANGE);
    lv_obj_set_width(warn, 800);
    lv_obj_set_style_text_align(warn, LV_TEXT_ALIGN_CENTER, 0);
    uiBtn(s_cover, LV_SYMBOL_WIFI " USTAW POLACZENIE WiFi", 230, 310, 340, 64, C_BTN,
          onCoverWifi, nullptr, FONT_M);

    s_lastResultSeq = linkResultSeq();
}
#endif  // !UI_PORTRAIT

void uiBuildMain() {
#if UI_PORTRAIT
    buildMainPortrait();
#else
    buildMainLandscape();
#endif
}

// ---------- odświeżanie ----------
static void updateTopBar(LinkState ls, const Status& st, bool have) {
    switch (ls) {
        case LS_ONLINE:
            setLbl(s_lblLink, LV_SYMBOL_WIFI " POLACZONO");
            setTxtColor(s_lblLink, C_GREENB);
            break;
        case LS_NO_DATA:
            setLbl(s_lblLink, LV_SYMBOL_WIFI " BRAK DANYCH");
            setTxtColor(s_lblLink, C_ORANGE);
            break;
        case LS_WIFI_CONNECTING:
            setLbl(s_lblLink, LV_SYMBOL_WIFI " LACZENIE...");
            setTxtColor(s_lblLink, C_ORANGE);
            break;
        default:
            setLbl(s_lblLink, LV_SYMBOL_WIFI " BRAK HASLA");
            setTxtColor(s_lblLink, C_RED);
            break;
    }

    char buf[48];
    if (have && ls == LS_ONLINE) {
        if (st.gpsFix) {
            snprintf(buf, sizeof(buf), LV_SYMBOL_GPS " %d sat", st.gpsSat);
            setTxtColor(s_lblGps, C_GREENB);
        } else {
            snprintf(buf, sizeof(buf), LV_SYMBOL_GPS " brak (%d)", st.gpsSat);
            setTxtColor(s_lblGps, C_ORANGE);
        }
        setLbl(s_lblGps, buf);

        int pct = st.paintLevelPct;
        if (pct < 0) pct = 0;
        if (pct > 100) pct = 100;
        if (lv_bar_get_value(s_barPaint) != pct) lv_bar_set_value(s_barPaint, pct, LV_ANIM_OFF);
        lv_color_t bc = pct < 5 ? C_RED : (pct < 15 ? C_ORANGE : C_GREEN);
        if (lv_obj_get_style_bg_color(s_barPaint, LV_PART_INDICATOR).full != bc.full)
            lv_obj_set_style_bg_color(s_barPaint, bc, LV_PART_INDICATOR);
        snprintf(buf, sizeof(buf), "FARBA %d%%", pct);
        setLbl(s_lblPaint, buf);
    } else {
        setLbl(s_lblGps, LV_SYMBOL_GPS " ---");
        setTxtColor(s_lblGps, C_DIM);
    }
}

static void setTab(lv_obj_t* tab, bool sel) {
    uiBtnSetColor(tab, sel ? C_BTN_SEL : C_BTN);
    lv_obj_set_style_border_width(tab, sel ? 5 : 2, 0);
    lv_obj_set_style_border_color(tab, sel ? C_YELLOW : lv_color_lighten(C_BTN, 90), 0);
}

static void updateSlots(const Status& st) {
    int grp = currentGroup(st);
    setTab(s_tabAxis, grp == 0);
    setTab(s_tabEdge, grp == 1);

    for (int s = 0; s < 10; s++) {
        int pat = softKeyPattern(grp, s);
        bool used = (pat >= 0);
        lv_obj_t* btn = s_slotBtn[s];

        if (!used) {
            setLbl(s_slotLbl[s], "");
            if (s_slotSpec[s]) setLbl(s_slotSpec[s], "");
            setVisible(s_slotGlyph[s], false);
            uiBtnSetColor(btn, C_BTN_DIS);
            lv_obj_set_style_border_width(btn, 2, 0);
            s_slotSel[s] = false;
            continue;
        }
        setVisible(s_slotGlyph[s], true);
        const char* code = (pat == PAT_CUSTOM_IDX) ? "WLASNY" : PATTERNS[pat].code;
        setLbl(s_slotLbl[s], code);
        lv_obj_set_style_text_font(s_slotLbl[s], pat == PAT_CUSTOM_IDX ? FONT_S : SLOT_FONT, 0);
        uiGlyphSet(s_slotGlyph[s], pat, false, st);
        if (s_slotSpec[s]) {
            GunCfg cfg[NGUNS];
            for (int g = 0; g < NGUNS; g++) cfg[g] = patternGun(pat, false, g, st);
            char m[48], w[24], txt[80];
            patternSpecText(cfg, m, sizeof(m), w, sizeof(w));
            if (pat == PAT_CUSTOM_IDX && !st.customValid) snprintf(txt, sizeof(txt), "nie zapisany");
            else snprintf(txt, sizeof(txt), "%s\n%s", m, w);
            setLbl(s_slotSpec[s], txt);
        }

        bool sel = (pat == st.patternIdx);
        s_slotSel[s] = sel;
        uiBtnSetColor(btn, sel ? C_BTN_SEL : C_BTN);
        lv_obj_set_style_border_width(btn, sel ? 5 : 2, 0);
        lv_obj_set_style_border_color(btn, sel ? C_YELLOW : lv_color_lighten(C_BTN, 90), 0);
    }
}

static void updateBanner(const Status& st, bool blink) {
    const char* txt = nullptr;
    lv_color_t col = C_RED;
    if (st.overspeed) { txt = "ZA SZYBKO!"; col = C_RED; }
    else if (st.anomalyDetected) {
        static char b[40];
        int first = -1;
        for (int g = 0; g < NGUNS; g++) if (st.gunAnomaly[g]) { first = g; break; }
        if (first >= 0) snprintf(b, sizeof(b), "ANOMALIA PISTOLETU P%d", first + 1);
        else snprintf(b, sizeof(b), "ANOMALIA PISTOLETU");
        txt = b; col = C_RED;
    }
    else if (st.autoPaused) { txt = "AUTO-PAUZA (postoj)"; col = C_ORANGE; }
    else if (st.lowSpeed && st.state == MS_PAINTING) { txt = "ZA WOLNO"; col = C_ORANGE; }
    else if (st.mode == MM_SEMI && st.state == MS_PAINTING && st.semiLineComplete) {
        txt = "LINIA GOTOWA - NASTEPNA LINIA"; col = C_BTN;
    }
    else if (st.mode == MM_MANUAL && st.state == MS_PAINTING) {
        txt = "RECZNY: TRZYMAJ FIZYCZNY START"; col = C_BTN;
    }

    if (!txt) { setVisible(s_banner, false); return; }
    setVisible(s_banner, true);
    setLbl(s_lblBanner, txt);
    lv_obj_center(s_lblBanner);
    if (lv_obj_get_style_bg_color(s_banner, LV_PART_MAIN).full != col.full)
        lv_obj_set_style_bg_color(s_banner, col, 0);
    bool flash = (st.overspeed || st.anomalyDetected) && blink;
    lv_opa_t op = flash ? LV_OPA_40 : LV_OPA_COVER;
    if (lv_obj_get_style_bg_opa(s_banner, LV_PART_MAIN) != op) lv_obj_set_style_bg_opa(s_banner, op, 0);
}

static void updateBottom(const Status& st, bool online) {
    bool idle = (st.state == MS_IDLE || st.state == MS_STOPPED);

    for (int m = 0; m < 3; m++) {
        uiBtnEnable(s_btnMode[m], online && idle);
        bool sel = ((int)st.mode == m);
        uiBtnSetColor(s_btnMode[m], sel ? C_BTN_SEL : C_BTN);
        lv_obj_set_style_border_width(s_btnMode[m], sel ? 5 : 2, 0);
        lv_obj_set_style_border_color(s_btnMode[m], sel ? C_YELLOW : lv_color_lighten(C_BTN, 90), 0);
    }

    const char* t;
    lv_color_t c;
    if (st.state == MS_PAINTING) {
        if (st.mode == MM_SEMI && st.semiLineComplete) { t = LV_SYMBOL_NEXT " NASTEPNA LINIA"; c = C_BTN_SEL; }
        else { t = LV_SYMBOL_PAUSE " PAUZA"; c = C_ORANGE; }
    } else if (st.state == MS_PAUSED) {
        t = LV_SYMBOL_PLAY " WZNOW"; c = C_GREEN;
    } else {
        t = LV_SYMBOL_PLAY " START"; c = C_GREEN;
    }
    uiBtnSetText(s_btnStart, t);
    uiBtnSetColor(s_btnStart, c);
    uiBtnEnable(s_btnStart, online);
    uiBtnEnable(s_btnGap, online && idle);
}

static void updateGuns(const Status& st) {
    for (int g = 0; g < NGUNS; g++) {
        GunCfg c = effectiveGun(st, g);
        bool used = (c.mode != GM_OFF);
        bool on = st.guns[g];
        bool an = st.gunAnomaly[g];
        lv_obj_t* p = s_pill[g];
        lv_color_t bg = on ? C_GREENB : (used ? C_PANEL : lv_color_hex(0x0C1A3A));
        if (lv_obj_get_style_bg_color(p, LV_PART_MAIN).full != bg.full) lv_obj_set_style_bg_color(p, bg, 0);
        lv_color_t bd = an ? C_RED : (used ? C_YELLOW : lv_color_hex(0x0C1A3A));
        if (!used && !on) bd = lv_color_hex(0x0C1A3A);
        if (lv_obj_get_style_border_color(p, LV_PART_MAIN).full != bd.full) lv_obj_set_style_border_color(p, bd, 0);
        lv_obj_t* l = lv_obj_get_child(p, 0);
        setTxtColor(l, on ? lv_color_hex(0x03210F) : (used ? C_TEXT : lv_color_hex(0x3B4A73)));
    }
}

static void updateCover(LinkState ls, bool show) {
    setVisible(s_cover, show);
    if (!show) return;
    switch (ls) {
        case LS_NO_PASSWORD:
            setLbl(s_lblCoverReason, "Wpisz haslo WiFi sterownika (ostatnie 4 bajty MAC)");
            break;
        case LS_WIFI_CONNECTING:
            setLbl(s_lblCoverReason, "Laczenie z siecia " CTRL_SSID "...");
            break;
        default:
            setLbl(s_lblCoverReason, "Polaczono z WiFi, brak danych ze sterownika");
            break;
    }
}

static void mainTick(lv_timer_t*) {
    Status st;
    uint32_t age = 0;
    bool have = linkGetStatus(st, &age);
    LinkState ls = linkState();
    bool online = have && (ls == LS_ONLINE);
    g_online = online;
    if (have) { g_st = st; g_stValid = true; }

    // Komunikaty o wyniku poleceń (tylko błędy)
    uint32_t sq = linkResultSeq();
    if (sq != s_lastResultSeq) {
        s_lastResultSeq = sq;
        if (!linkResultOk()) {
            char m[64];
            linkResultMsg(m, sizeof(m));
            uiToast(m, true);
        }
    }

    updateTopBar(ls, st, have);
    updateCover(ls, !online);
    if (!have) return;

    // --- prędkość ---
    float spd = st.speed;
    if (spd < 0) spd = 0;
    if (spd > 99.9f) spd = 99.9f;
    char buf[48];
    snprintf(buf, sizeof(buf), "%4.1f", spd);
    lv_color_t sc = st.overspeed ? C_RED : ((st.lowSpeed && st.state == MS_PAINTING) ? C_ORANGE : C_TEXT);
    uiSevenSegSet(s_speed, buf, sc);

    // --- wzorzec / tryb ---
    snprintf(buf, sizeof(buf), "%s%s", st.patternCode, st.reversed ? " <>" : "");
    setLbl(s_lblPattern, buf);
#if UI_PORTRAIT
    uiGlyphSet(s_bigGlyph, st.patternIdx, st.reversed, st);
    {
        GunCfg cfg[NGUNS];
        for (int g = 0; g < NGUNS; g++) cfg[g] = effectiveGun(st, g);
        char m[48], w[24], sp[80];
        patternSpecText(cfg, m, sizeof(m), w, sizeof(w));
        snprintf(sp, sizeof(sp), "%s\n%s", m, w);
        setLbl(s_lblSpec, sp);
    }
#endif
    const char* gapTxt = st.gapStart ? "  (od przerwy)" : "";
    if (st.mode == MM_SEMI && st.state != MS_IDLE && st.state != MS_STOPPED)
        snprintf(buf, sizeof(buf), "%s #%d  %s%s", modeName(st.mode), st.semiSegment, stateName(st.state), gapTxt);
    else
        snprintf(buf, sizeof(buf), "%s  %s%s", modeName(st.mode), stateName(st.state), gapTxt);
    setLbl(s_lblMode, buf);
    setTxtColor(s_lblMode, st.state == MS_PAINTING ? C_GREENB : (st.state == MS_PAUSED ? C_ORANGE : C_TEXT));

    // --- liczniki ---
    snprintf(buf, sizeof(buf), "#8FA3CC DYST# %.1f m", st.distance);
    setLbl(s_lblDist, buf);
    snprintf(buf, sizeof(buf), "#8FA3CC POW# %.2f m2", st.area);
    setLbl(s_lblArea, buf);
    char tb[16];
    fmtTime(st.elapsed, tb, sizeof(tb));
    snprintf(buf, sizeof(buf), "#8FA3CC CZAS# %s", tb);
    setLbl(s_lblTime, buf);

    // --- droga: dystans od startu wzorca (ekstrapolacja między ramkami) ---
    float pd = st.hasPatDist ? st.patDist : st.distance;
    if (st.state == MS_PAINTING && age < 1200) pd += (st.speed / 3.6f) * (age / 1000.0f);
    if (st.state == MS_PAINTING || age < ANIM_PERIOD_MS * 2 + 20) uiRoadSet(s_road, st, pd);

    // --- nakładki na drodze ---
    if (st.patternPending) snprintf(buf, sizeof(buf), "OCZEKUJE: %s (%s)", st.pendingCode, st.smartSwitch ? "po cyklu" : "");
    else buf[0] = 0;
    setLbl(s_lblPending, buf);
    bool hasRev = (st.patternIdx < NPAT_PREDEF) && PATTERNS[st.patternIdx].hasReverse;
    setVisible(s_btnReverse, hasRev);
    if (hasRev) uiBtnEnable(s_btnReverse, online);

    bool blink = ((millis() / 400) & 1) != 0;
    updateBanner(st, blink);

    updateSlots(st);
    updateGuns(st);
    updateBottom(st, online);
}

void uiStartMainTimer() {
    lv_timer_create(mainTick, ANIM_PERIOD_MS, nullptr);
}
