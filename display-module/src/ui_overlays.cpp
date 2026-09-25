// ============================================================
// Nakładki: wybór wzorca, menu główne, hasło WiFi, informacje
// ============================================================

#include "ui_common.h"
#include "link.h"
#include "settings_store.h"
#include "app_config.h"
#include <stdio.h>
#include <ctype.h>
#include <esp_heap_caps.h>

// ------------------------------------------------------------
// Wybór wzorca — wszystkie 16 na jednej liście (poza stronami OŚ/KRAWĘDŹ)
// ------------------------------------------------------------
static void onPickCard(lv_event_t* e) {
    int pat = (int)(intptr_t)lv_event_get_user_data(e);
    if (!g_online) { uiToast("brak lacznosci ze sterownikiem", true); return; }
    if (pat == PAT_CUSTOM_IDX && !g_st.customValid) {
        uiToast("wzorzec wlasny nie jest zapisany", true);
        return;
    }
    uiSendAction("set_pattern", pat);
    uiCloseOverlay();
}

void uiOpenPicker() {
    lv_obj_t* ov = uiOverlay("WSZYSTKIE WZORCE", nullptr);

#if UI_PORTRAIT
    // 3 kolumny x 6 rzędów
    const int cols = 3, cardW = 150, cardH = 104, stepX = 156, stepY = 108, x0 = 8, y0 = OV_TOP;
    const int gx = 6, gy = 8, gw = 42, gh = 88, tx = 54, nameW = 88;
    const lv_font_t* codeFont = FONT_M;
#else
    // 4 kolumny x 4 rzędy
    const int cols = 4, cardW = 190, cardH = 98, stepX = 196, stepY = 104, x0 = 8, y0 = OV_TOP;
    const int gx = 8, gy = 8, gw = 56, gh = 78, tx = 74, nameW = 108;
    const lv_font_t* codeFont = FONT_L;
#endif
    for (int i = 0; i < NPAT; i++) {
        int col = i % cols, row = i / cols;
        int x = x0 + col * stepX;
        int y = y0 + row * stepY;
        bool sel = (i == g_st.patternIdx);
        bool custom = (i == PAT_CUSTOM_IDX);
        lv_obj_t* b = uiBtn(ov, "", x, y, cardW, cardH, sel ? C_BTN_SEL : C_BTN, onPickCard,
                            (void*)(intptr_t)i, FONT_L);
        if (sel) {
            lv_obj_set_style_border_width(b, 5, 0);
            lv_obj_set_style_border_color(b, C_YELLOW, 0);
            lv_obj_set_style_border_opa(b, LV_OPA_COVER, 0);
        }
        lv_obj_t* g = uiGlyph(b, gx, gy, gw, gh);
        uiGlyphSet(g, i, false, g_st);

        lv_obj_t* code = lv_obj_get_child(b, 0);
        lv_label_set_text(code, custom ? "WLASNY" : PATTERNS[i].code);
        lv_obj_set_style_text_font(code, custom ? FONT_S : codeFont, 0);
        lv_obj_align(code, LV_ALIGN_TOP_LEFT, tx, 10);

        lv_obj_t* nm = lv_label_create(b);
        lv_label_set_long_mode(nm, LV_LABEL_LONG_WRAP);
        lv_obj_set_width(nm, nameW);
        lv_obj_set_style_text_font(nm, FONT_S, 0);
        lv_obj_set_style_text_color(nm, custom && !g_st.customValid ? C_DIM : C_TEXT, 0);
        lv_label_set_text(nm, custom ? (g_st.customValid ? "Wzorzec wlasny" : "(nie zapisany)")
                                     : PATTERNS[i].name);
        lv_obj_align(nm, LV_ALIGN_TOP_LEFT, tx, 44);
    }
}

// ------------------------------------------------------------
// Menu główne
// ------------------------------------------------------------
static void onMenuItem(lv_event_t* e) {
    int id = (int)(intptr_t)lv_event_get_user_data(e);
    switch (id) {
        case 0: uiOpenStats();        break;
        case 1: uiOpenCustomEditor(); break;
        case 2: uiOpenCalibration();  break;
        case 3: uiOpenPaint();        break;
        case 4: uiOpenSettings();     break;
        case 5: uiOpenWifi();         break;
        case 6: uiOpenInfo();         break;
        case 7: uiOpenPicker();       break;
    }
}

void uiOpenMenu() {
    lv_obj_t* ov = uiOverlay("MENU", nullptr);
    static const char* names[8] = {
        LV_SYMBOL_LIST "\nSTATYSTYKI",
        LV_SYMBOL_EDIT "\nWZOR WLASNY",
        LV_SYMBOL_REFRESH "\nKALIBRACJA",
        LV_SYMBOL_TINT "\nFARBA / ZBIORNIK",
        LV_SYMBOL_SETTINGS "\nUSTAWIENIA",
        LV_SYMBOL_WIFI "\nPOLACZENIE WiFi",
        LV_SYMBOL_WARNING "\nINFORMACJE",
        LV_SYMBOL_DIRECTORY "\nWSZYSTKIE WZORCE"
    };
    for (int i = 0; i < 8; i++) {
#if UI_PORTRAIT
        int c = i % 2, r = i / 2;
        uiBtn(ov, names[i], 8 + c * 236, OV_TOP + 8 + r * 160, 228, 150, C_BTN, onMenuItem,
              (void*)(intptr_t)i, FONT_M);
#else
        int c = i % 3, r = i / 3;
        uiBtn(ov, names[i], 16 + c * 260, 76 + r * 132, 252, 120, C_BTN, onMenuItem,
              (void*)(intptr_t)i, FONT_M);
#endif
    }
}

// ------------------------------------------------------------
// Hasło WiFi
// ------------------------------------------------------------
static lv_obj_t* s_wifiTa = nullptr;
static lv_obj_t* s_wifiInfo = nullptr;

static void wifiSave() {
    if (!s_wifiTa) return;
    char buf[24];
    strlcpy(buf, lv_textarea_get_text(s_wifiTa), sizeof(buf));
    for (char* p = buf; *p; p++) *p = (char)toupper((unsigned char)*p);
    if (strlen(buf) < 8) {
        uiToast("haslo musi miec co najmniej 8 znakow", true);
        return;
    }
    linkSetPassword(buf);
    uiToast("zapisano - lacze z siecia", false);
    uiCloseOverlay();
}

static void onWifiSave(lv_event_t*) { wifiSave(); }

static void onKbEvent(lv_event_t* e) {
    lv_event_code_t c = lv_event_get_code(e);
    if (c == LV_EVENT_READY) wifiSave();
    else if (c == LV_EVENT_CANCEL) uiCloseOverlay();
}

static void wifiUpdate() {
    if (!s_wifiInfo) return;
    char buf[80];
    switch (linkState()) {
        case LS_ONLINE:          snprintf(buf, sizeof(buf), "Polaczono (sygnal %d dBm)", linkRssi()); break;
        case LS_NO_DATA:         snprintf(buf, sizeof(buf), "WiFi OK, brak danych ze sterownika"); break;
        case LS_WIFI_CONNECTING: snprintf(buf, sizeof(buf), "Laczenie z " CTRL_SSID "..."); break;
        default:                 snprintf(buf, sizeof(buf), "Brak zapisanego hasla"); break;
    }
    if (strcmp(lv_label_get_text(s_wifiInfo), buf) != 0) lv_label_set_text(s_wifiInfo, buf);
}

static void wifiClosed() { s_wifiTa = nullptr; s_wifiInfo = nullptr; }

void uiOpenWifi() {
    lv_obj_t* ov = uiOverlay("POLACZENIE WiFi", wifiUpdate);
    uiSetOverlayCloseHandler(wifiClosed);

#if UI_PORTRAIT
    lv_obj_t* hint = uiLabel(ov, "Siec: " CTRL_SSID "  Haslo: 8 znakow HEX (ekran startowy sterownika)", 12, OV_TOP,
                             FONT_S, C_DIM);
    lv_obj_set_width(hint, OV_W - 24);
    const int taX = 8, taY = OV_TOP + 50, taW = OV_W - 16;
    const int btnX = 8, btnY = OV_TOP + 118, btnW = OV_W - 16;
    const int infoX = 12, infoY = OV_TOP + 188;
    const int kbY = OV_H - 320, kbH = 320;
#else
    uiLabel(ov, "Siec: " CTRL_SSID "   Haslo: 8 znakow HEX (ekran startowy sterownika)", 16, 64,
            FONT_S, C_DIM);
    const int taX = 16, taY = 92, taW = 520;
    const int btnX = 548, btnY = 92, btnW = 236;
    const int infoX = 16, infoY = 160;
    const int kbY = 230, kbH = 250;
#endif
    s_wifiTa = lv_textarea_create(ov);
    lv_obj_set_pos(s_wifiTa, taX, taY);
    lv_obj_set_size(s_wifiTa, taW, 60);
    lv_textarea_set_one_line(s_wifiTa, true);
    lv_textarea_set_max_length(s_wifiTa, 20);
    lv_textarea_set_text(s_wifiTa, g_settings.wifiPass);
    lv_textarea_set_placeholder_text(s_wifiTa, "haslo WiFi");
    lv_obj_set_style_text_font(s_wifiTa, FONT_L, 0);

    uiBtn(ov, "ZAPISZ I POLACZ", btnX, btnY, btnW, 60, C_GREEN, onWifiSave, nullptr, FONT_M);
    s_wifiInfo = uiLabel(ov, "", infoX, infoY, FONT_M, C_YELLOW);
    lv_obj_set_width(s_wifiInfo, OV_W - 24);

    lv_obj_t* kb = lv_keyboard_create(ov);
    lv_obj_set_size(kb, OV_W, kbH);
    lv_obj_set_pos(kb, 0, kbY);
    lv_keyboard_set_mode(kb, LV_KEYBOARD_MODE_TEXT_UPPER);
    lv_keyboard_set_textarea(kb, s_wifiTa);
    lv_obj_add_event_cb(kb, onKbEvent, LV_EVENT_ALL, nullptr);
    wifiUpdate();
}

// ------------------------------------------------------------
// Informacje
// ------------------------------------------------------------
static lv_obj_t* s_infoLbl = nullptr;

static void infoUpdate() {
    if (!s_infoLbl) return;
    char buf[960];
    const Status& s = g_st;
    snprintf(buf, sizeof(buf),
        "#8FA3CC Modul wyswietlacza:# v" DISPLAY_FW_VERSION "   wolny RAM %u KB   PSRAM %u KB\n"
        "#8FA3CC Sterownik:# v%s   czas pracy %lu s   klienci WiFi %d\n"
        "#8FA3CC Lacze:# %s   sygnal %d dBm\n"
        "#8FA3CC GPS:# %s   satelity %d   HDOP %.1f   predkosc GPS %.1f km/h   trasa GPX: %s\n"
        "#8FA3CC Enkoder:# %s   %.1f imp/m\n"
        "#8FA3CC Progi predkosci:# min %.1f  max %.1f km/h\n"
        "#8FA3CC Farba:# %.1f L (%d%%)\n"
        "#8FA3CC Przyciski wzorcow:# %s   strona: %s",
        (unsigned)(heap_caps_get_free_size(MALLOC_CAP_INTERNAL) / 1024),
        (unsigned)(heap_caps_get_free_size(MALLOC_CAP_SPIRAM) / 1024),
        s.firmware, (unsigned long)s.uptime, s.clients,
        g_online ? "polaczono" : "BRAK", linkRssi(),
        s.gpsFix ? "fix" : "brak fix", s.gpsSat, s.gpsHdop, s.gpsSpeed, s.gpxRec ? "zapis" : "wylaczona",
        s.calibrated ? "skalibrowany" : "NIESKALIBROWANY", s.ppm,
        s.minSpeed, s.maxSpeed,
        s.paintLevelL, s.paintLevelPct,
        s.patBtnLayout == 1 ? "soft-key 10+GRUPA" : "klasyczne 15",
        s.patGroup == 1 ? "KRAWEDZ" : "OS");
    lv_label_set_text(s_infoLbl, buf);
}

static void infoClosed() { s_infoLbl = nullptr; }

void uiOpenInfo() {
    lv_obj_t* ov = uiOverlay("INFORMACJE", infoUpdate);
    uiSetOverlayCloseHandler(infoClosed);
#if UI_PORTRAIT
    s_infoLbl = uiLabel(ov, "", 12, OV_TOP + 6, FONT_S, C_TEXT);
    lv_obj_set_width(s_infoLbl, OV_W - 24);
    lv_obj_set_style_text_line_space(s_infoLbl, 8, 0);
#else
    s_infoLbl = uiLabel(ov, "", 16, 72, FONT_M, C_TEXT);
    lv_obj_set_width(s_infoLbl, 770);
    lv_obj_set_style_text_line_space(s_infoLbl, 12, 0);
#endif
    lv_label_set_recolor(s_infoLbl, true);
    lv_label_set_long_mode(s_infoLbl, LV_LABEL_LONG_WRAP);
    infoUpdate();
}
