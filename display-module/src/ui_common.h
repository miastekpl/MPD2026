#pragma once
// ============================================================
// Wspólne definicje UI: paleta, pomocnicze konstruktory, widgety
// ============================================================

#include <lvgl.h>
#include "model.h"

// ---- Paleta (wysoki kontrast, czytelna w słońcu) ----
#define C_BG       lv_color_hex(0x07142E)
#define C_PANEL    lv_color_hex(0x0F2557)
#define C_BTN      lv_color_hex(0x1F52D6)
#define C_BTN_SEL  lv_color_hex(0x2F86FF)
#define C_BTN_DIS  lv_color_hex(0x27314D)
#define C_TEXT     lv_color_hex(0xFFFFFF)
#define C_DIM      lv_color_hex(0x8FA3CC)
#define C_YELLOW   lv_color_hex(0xFFD400)
#define C_GREEN    lv_color_hex(0x1FA34A)
#define C_GREENB   lv_color_hex(0x35F27A)
#define C_RED      lv_color_hex(0xD62828)
#define C_ORANGE   lv_color_hex(0xF77F00)
#define C_ROAD     lv_color_hex(0x1B1B1F)
#define C_SKY      lv_color_hex(0x143A7A)

#define FONT_S   (&lv_font_montserrat_16)
#define FONT_M   (&lv_font_montserrat_20)
#define FONT_L   (&lv_font_montserrat_28)
#define FONT_XL  (&lv_font_montserrat_40)

// ---- Konstruktory pomocnicze ----
lv_obj_t* uiLabel(lv_obj_t* parent, const char* text, lv_coord_t x, lv_coord_t y,
                  const lv_font_t* font, lv_color_t color);
lv_obj_t* uiBtn(lv_obj_t* parent, const char* text, lv_coord_t x, lv_coord_t y,
                lv_coord_t w, lv_coord_t h, lv_color_t bg, lv_event_cb_t cb,
                void* user, const lv_font_t* font);
void      uiBtnSetText(lv_obj_t* btn, const char* text);
void      uiBtnSetColor(lv_obj_t* btn, lv_color_t bg);
void      uiBtnEnable(lv_obj_t* btn, bool enabled);

// Polecenie do sterownika (POST /api/control)
void      uiSend(const char* formBody, bool urgent = false);
void      uiSendAction(const char* action, int value);

// Komunikat chwilowy (toast)
void      uiToast(const char* msg, bool error);

// ---- Nakładki pełnoekranowe ----
typedef void (*OverlayUpdateFn)();
lv_obj_t* uiOverlay(const char* title, OverlayUpdateFn updateFn);   // zwraca kontener 800x480
void      uiCloseOverlay();
bool      uiOverlayOpen();
// Wywoływane raz przy zamknięciu bieżącej nakładki (ustawić PO uiOverlay())
void      uiSetOverlayCloseHandler(void (*fn)());

// Zdefiniowane w main.cpp
void      applyBrightness(uint8_t value);
void      uiStartMainTimer();

// Ostatni status (odświeżany przez timer głównego ekranu)
extern Status g_st;
extern bool   g_stValid;
extern bool   g_online;

// ---- Widgety ----
lv_obj_t* uiSevenSeg(lv_obj_t* parent, lv_coord_t x, lv_coord_t y, lv_coord_t w, lv_coord_t h);
void      uiSevenSegSet(lv_obj_t* o, const char* txt, lv_color_t col);

lv_obj_t* uiRoadView(lv_obj_t* parent, lv_coord_t x, lv_coord_t y, lv_coord_t w, lv_coord_t h);
void      uiRoadSet(lv_obj_t* o, const Status& st, float patDistNow);

lv_obj_t* uiGlyph(lv_obj_t* parent, lv_coord_t x, lv_coord_t y, lv_coord_t w, lv_coord_t h);
void      uiGlyphSet(lv_obj_t* o, int patIdx, bool reversed, const Status& st);

// ---- Ekrany / nakładki ----
void uiBuildMain();
void uiOpenPicker(bool assignMode, int favSlot);
void uiOpenMenu();
void uiOpenStats();
void uiOpenSettings();
void uiOpenCalibration();
void uiOpenPaint();
void uiOpenWifi();
void uiOpenCustomEditor();
void uiOpenInfo();
