// ============================================================
// Podstawy UI: konstruktory, nakładki pełnoekranowe, komunikaty
// ============================================================

#include "ui_common.h"
#include "link.h"
#include <stdio.h>

// ------------------------------------------------------------
// Konstruktory
// ------------------------------------------------------------
lv_obj_t* uiLabel(lv_obj_t* parent, const char* text, lv_coord_t x, lv_coord_t y,
                  const lv_font_t* font, lv_color_t color) {
    lv_obj_t* l = lv_label_create(parent);
    lv_label_set_text(l, text);
    lv_obj_set_pos(l, x, y);
    lv_obj_set_style_text_font(l, font, 0);
    lv_obj_set_style_text_color(l, color, 0);
    return l;
}

lv_obj_t* uiBtn(lv_obj_t* parent, const char* text, lv_coord_t x, lv_coord_t y,
                lv_coord_t w, lv_coord_t h, lv_color_t bg, lv_event_cb_t cb,
                void* user, const lv_font_t* font) {
    lv_obj_t* b = lv_btn_create(parent);
    lv_obj_set_pos(b, x, y);
    lv_obj_set_size(b, w, h);
    lv_obj_set_style_bg_color(b, bg, 0);
    lv_obj_set_style_bg_color(b, lv_color_lighten(bg, 70), LV_STATE_PRESSED);
    lv_obj_set_style_bg_color(b, C_BTN_DIS, LV_STATE_DISABLED);
    lv_obj_set_style_bg_grad_dir(b, LV_GRAD_DIR_NONE, 0);
    lv_obj_set_style_radius(b, 10, 0);
    lv_obj_set_style_shadow_width(b, 0, 0);
    lv_obj_set_style_border_width(b, 2, 0);
    lv_obj_set_style_border_color(b, lv_color_lighten(bg, 90), 0);
    lv_obj_set_style_border_opa(b, LV_OPA_60, 0);
    lv_obj_set_style_text_font(b, font, 0);
    lv_obj_set_style_text_color(b, C_TEXT, 0);
    lv_obj_set_style_text_color(b, C_DIM, LV_STATE_DISABLED);
    lv_obj_t* l = lv_label_create(b);
    lv_label_set_text(l, text);
    lv_obj_set_style_text_align(l, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_center(l);
    if (cb) lv_obj_add_event_cb(b, cb, LV_EVENT_CLICKED, user);
    return b;
}

void uiBtnSetText(lv_obj_t* btn, const char* text) {
    lv_obj_t* l = lv_obj_get_child(btn, 0);
    if (!l) return;
    if (strcmp(lv_label_get_text(l), text) != 0) {
        lv_label_set_text(l, text);
        lv_obj_center(l);
    }
}

void uiBtnSetColor(lv_obj_t* btn, lv_color_t bg) {
    lv_color_t cur = lv_obj_get_style_bg_color(btn, LV_PART_MAIN);
    if (cur.full == bg.full) return;
    lv_obj_set_style_bg_color(btn, bg, 0);
    lv_obj_set_style_bg_color(btn, lv_color_lighten(bg, 70), LV_STATE_PRESSED);
    lv_obj_set_style_border_color(btn, lv_color_lighten(bg, 90), 0);
}

void uiBtnEnable(lv_obj_t* btn, bool enabled) {
    bool isDis = lv_obj_has_state(btn, LV_STATE_DISABLED);
    if (enabled && isDis) lv_obj_clear_state(btn, LV_STATE_DISABLED);
    else if (!enabled && !isDis) lv_obj_add_state(btn, LV_STATE_DISABLED);
}

// ------------------------------------------------------------
// Polecenia
// ------------------------------------------------------------
void uiSend(const char* formBody, bool urgent) {
    if (!linkSend(formBody, urgent)) uiToast("kolejka polecen pelna", true);
}

void uiSendAction(const char* action, int value) {
    char b[64];
    snprintf(b, sizeof(b), "action=%s&value=%d", action, value);
    uiSend(b);
}

// ------------------------------------------------------------
// Komunikat chwilowy
// ------------------------------------------------------------
static lv_obj_t*   s_toast = nullptr;
static lv_timer_t* s_toastTimer = nullptr;

static void toastHide(lv_timer_t* t) {
    if (s_toast) { lv_obj_del(s_toast); s_toast = nullptr; }
    s_toastTimer = nullptr;   // timer jednorazowy usuwa się sam
}

void uiToast(const char* msg, bool error) {
    if (s_toastTimer) { lv_timer_del(s_toastTimer); s_toastTimer = nullptr; }
    if (s_toast) { lv_obj_del(s_toast); s_toast = nullptr; }

    s_toast = lv_obj_create(lv_layer_top());
    lv_obj_remove_style_all(s_toast);
    lv_obj_set_style_bg_color(s_toast, error ? C_RED : C_GREEN, 0);
    lv_obj_set_style_bg_opa(s_toast, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(s_toast, 10, 0);
    lv_obj_set_style_pad_hor(s_toast, 24, 0);
    lv_obj_set_style_pad_ver(s_toast, 12, 0);
    lv_obj_set_size(s_toast, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_clear_flag(s_toast, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_t* l = lv_label_create(s_toast);
    lv_label_set_text(l, msg);
    lv_obj_set_style_text_font(l, FONT_M, 0);
    lv_obj_set_style_text_color(l, C_TEXT, 0);
    lv_obj_align(s_toast, LV_ALIGN_TOP_MID, 0, 66);

    s_toastTimer = lv_timer_create(toastHide, 3200, nullptr);
    lv_timer_set_repeat_count(s_toastTimer, 1);
}

// ------------------------------------------------------------
// Nakładki pełnoekranowe (z zawsze dostępnym przyciskiem STOP)
// ------------------------------------------------------------
static lv_obj_t*        s_overlay = nullptr;
static lv_timer_t*      s_ovTimer = nullptr;
static OverlayUpdateFn  s_ovFn = nullptr;
static lv_obj_t*        s_topStop = nullptr;

static void stopClicked(lv_event_t*) {
    linkSend("action=stop", true);
}

static void ovTimerCb(lv_timer_t*) {
    if (s_ovFn) s_ovFn();
}

static void closeClicked(lv_event_t*) {
    uiCloseOverlay();
}

static void (*s_closeHandler)() = nullptr;

void uiSetOverlayCloseHandler(void (*fn)()) { s_closeHandler = fn; }

bool uiOverlayOpen() { return s_overlay != nullptr; }

void uiCloseOverlay() {
    if (s_closeHandler) { void (*h)() = s_closeHandler; s_closeHandler = nullptr; h(); }
    if (s_ovTimer) { lv_timer_del(s_ovTimer); s_ovTimer = nullptr; }
    s_ovFn = nullptr;
    if (s_overlay) { lv_obj_del_async(s_overlay); s_overlay = nullptr; }
    if (s_topStop) lv_obj_add_flag(s_topStop, LV_OBJ_FLAG_HIDDEN);
}

lv_obj_t* uiOverlay(const char* title, OverlayUpdateFn updateFn) {
    uiCloseOverlay();

    if (!s_topStop) {
        s_topStop = uiBtn(lv_layer_top(), LV_SYMBOL_STOP " STOP", 8, 6, 130, 48, C_RED,
                          stopClicked, nullptr, FONT_M);
    }
    lv_obj_clear_flag(s_topStop, LV_OBJ_FLAG_HIDDEN);
    lv_obj_move_foreground(s_topStop);

    s_overlay = lv_obj_create(lv_scr_act());
    lv_obj_remove_style_all(s_overlay);
    lv_obj_set_size(s_overlay, 800, 480);
    lv_obj_set_pos(s_overlay, 0, 0);
    lv_obj_set_style_bg_color(s_overlay, C_BG, 0);
    lv_obj_set_style_bg_opa(s_overlay, LV_OPA_COVER, 0);
    lv_obj_clear_flag(s_overlay, LV_OBJ_FLAG_SCROLLABLE);

    uiLabel(s_overlay, title, 156, 12, FONT_L, C_YELLOW);
    uiBtn(s_overlay, "ZAMKNIJ", 650, 6, 142, 48, C_BTN, closeClicked, nullptr, FONT_M);

    s_ovFn = updateFn;
    if (updateFn) s_ovTimer = lv_timer_create(ovTimerCb, 250, nullptr);
    return s_overlay;
}
