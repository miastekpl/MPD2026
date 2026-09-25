// ============================================================
// MPD2026 - Moduł wyświetlacza 7" dla sterownika Trassar
// Sunton ESP32-8048S070C (ESP32-S3, 800x480 RGB, GT911)
// ============================================================

#include <Arduino.h>
#include <lvgl.h>
#include <esp_heap_caps.h>

#include "lgfx_sunton7.h"
#include "app_config.h"
#include "settings_store.h"
#include "link.h"
#include "ui_common.h"

DisplaySettings g_settings;

static LGFX gfx;

static const uint32_t BUF_LINES = 48;
static lv_disp_draw_buf_t s_drawBuf;
static lv_disp_drv_t      s_dispDrv;
static lv_indev_drv_t     s_indevDrv;

void applyBrightness(uint8_t value) {
    gfx.setBrightness(value);
}

static void flushCb(lv_disp_drv_t* drv, const lv_area_t* a, lv_color_t* c) {
    uint32_t w = a->x2 - a->x1 + 1;
    uint32_t h = a->y2 - a->y1 + 1;
    gfx.startWrite();
    gfx.setAddrWindow(a->x1, a->y1, w, h);
    gfx.writePixels((lgfx::rgb565_t*)&c->full, w * h);
    gfx.endWrite();
    lv_disp_flush_ready(drv);
}

static void touchCb(lv_indev_drv_t*, lv_indev_data_t* d) {
    uint16_t x, y;
    if (gfx.getTouch(&x, &y)) {
        d->state = LV_INDEV_STATE_PR;
        d->point.x = x;
        d->point.y = y;
    } else {
        d->state = LV_INDEV_STATE_REL;
    }
}

void setup() {
    Serial.begin(115200);
    Serial.println("\n[MPD2026] Modul wyswietlacza " DISPLAY_FW_VERSION);

    g_settings.load();

    gfx.init();
#if UI_PORTRAIT
    gfx.setRotation(UI_ROTATION);     // obrót panelu 800x480 do układu pionowego 480x800
#endif
    gfx.setBrightness(g_settings.brightness);
    gfx.fillScreen(0x000000);
    Serial.printf("[MPD2026] Ekran: %dx%d (%s)\n", (int)gfx.width(), (int)gfx.height(),
                  UI_PORTRAIT ? "pionowo" : "poziomo");

    lv_init();

    size_t bufBytes = (size_t)SCR_W * BUF_LINES * sizeof(lv_color_t);
    void* buf = heap_caps_malloc(bufBytes, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    if (!buf) buf = heap_caps_malloc(bufBytes, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (!buf) {
        Serial.println("[MPD2026] BLAD: brak pamieci na bufor LVGL");
        for (;;) delay(1000);
    }
    lv_disp_draw_buf_init(&s_drawBuf, buf, nullptr, SCR_W * BUF_LINES);

    lv_disp_drv_init(&s_dispDrv);
    s_dispDrv.hor_res = SCR_W;
    s_dispDrv.ver_res = SCR_H;
    s_dispDrv.flush_cb = flushCb;
    s_dispDrv.draw_buf = &s_drawBuf;
    lv_disp_drv_register(&s_dispDrv);

    lv_indev_drv_init(&s_indevDrv);
    s_indevDrv.type = LV_INDEV_TYPE_POINTER;
    s_indevDrv.read_cb = touchCb;
    s_indevDrv.long_press_time = 900;   // przypisanie ulubionego wzorca: przytrzymaj 0.9 s
    lv_indev_drv_register(&s_indevDrv);

    uiBuildMain();
    uiStartMainTimer();
    linkBegin();

    Serial.printf("[MPD2026] Start OK. Wolny RAM: %u, PSRAM: %u\n",
                  (unsigned)heap_caps_get_free_size(MALLOC_CAP_INTERNAL),
                  (unsigned)heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
}

void loop() {
    lv_timer_handler();
    delay(5);
}
