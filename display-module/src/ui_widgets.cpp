// ============================================================
// Widgety rysowane własnym kodem: 7-segmentowy odczyt prędkości,
// wizualizacja drogi w perspektywie, miniatura wzorca.
// ============================================================

#include "ui_common.h"
#include <math.h>
#include <stdlib.h>

// ------------------------------------------------------------
// Pomocnicze rysowanie
// ------------------------------------------------------------
static void fillRect(lv_draw_ctx_t* ctx, int x, int y, int w, int h, lv_color_t c,
                     lv_opa_t opa = LV_OPA_COVER, int radius = 0) {
    if (w <= 0 || h <= 0) return;
    lv_draw_rect_dsc_t d;
    lv_draw_rect_dsc_init(&d);
    d.bg_color = c;
    d.bg_opa = opa;
    d.radius = radius;
    d.border_width = 0;
    lv_area_t a;
    a.x1 = x; a.y1 = y; a.x2 = x + w - 1; a.y2 = y + h - 1;
    lv_draw_rect(ctx, &d, &a);
}

// Czworokąt wypukły (trapez) o poziomych krawędziach y1 (dolna) i y2 (górna)
static void quad(lv_draw_ctx_t* ctx, lv_color_t c, lv_opa_t opa,
                 float xl1, float xr1, float y1, float xl2, float xr2, float y2) {
    if (fabsf(y1 - y2) < 1.0f) return;
    lv_draw_rect_dsc_t d;
    lv_draw_rect_dsc_init(&d);
    d.bg_color = c;
    d.bg_opa = opa;
    d.border_width = 0;
    d.radius = 0;
    lv_point_t p[4];
    p[0].x = (lv_coord_t)xl1; p[0].y = (lv_coord_t)y1;
    p[1].x = (lv_coord_t)xr1; p[1].y = (lv_coord_t)y1;
    p[2].x = (lv_coord_t)xr2; p[2].y = (lv_coord_t)y2;
    p[3].x = (lv_coord_t)xl2; p[3].y = (lv_coord_t)y2;
    lv_draw_polygon(ctx, &d, p, 4);
}

// ------------------------------------------------------------
// 7-segmentowy odczyt
// ------------------------------------------------------------
struct SevenSegData {
    char txt[12];
    lv_color_t col;
};

// bity: a b c d e f g
static const uint8_t SEG_MAP[10] = {
    0b0111111, // 0
    0b0000110, // 1
    0b1011011, // 2
    0b1001111, // 3
    0b1100110, // 4
    0b1101101, // 5
    0b1111101, // 6
    0b0000111, // 7
    0b1111111, // 8
    0b1101111  // 9
};

static void drawDigit(lv_draw_ctx_t* ctx, int x, int y, int w, int h, uint8_t mask,
                      lv_color_t on, lv_color_t off) {
    int t = w * 17 / 100;
    if (t < 3) t = 3;
    int hh = h / 2;
    int vh = hh - t;
    auto seg = [&](int bit, int sx, int sy, int sw, int sh) {
        fillRect(ctx, sx, sy, sw, sh, (mask & (1 << bit)) ? on : off, LV_OPA_COVER, t / 3);
    };
    seg(0, x + t, y,                 w - 2 * t, t);                 // a
    seg(1, x + w - t, y + t / 2,     t, vh);                        // b
    seg(2, x + w - t, y + hh + t / 2, t, vh);                       // c
    seg(3, x + t, y + h - t,         w - 2 * t, t);                 // d
    seg(4, x, y + hh + t / 2,        t, vh);                        // e
    seg(5, x, y + t / 2,             t, vh);                        // f
    seg(6, x + t, y + hh - t / 2,    w - 2 * t, t);                 // g
}

static void sevenSegDraw(lv_event_t* e) {
    lv_obj_t* o = lv_event_get_target(e);
    SevenSegData* d = (SevenSegData*)lv_obj_get_user_data(o);
    if (!d) return;
    lv_draw_ctx_t* ctx = lv_event_get_draw_ctx(e);
    lv_area_t a;
    lv_obj_get_coords(o, &a);
    int W = a.x2 - a.x1 + 1;
    int H = a.y2 - a.y1 + 1;

    int n = strlen(d->txt);
    if (n == 0) return;
    int dots = 0;
    for (int i = 0; i < n; i++) if (d->txt[i] == '.') dots++;
    int digits = n - dots;
    if (digits <= 0) return;

    // szerokość cyfry: cyfry + kropki (0.35 cyfry) mieszczą się w W
    float unit = (float)W / (digits + dots * 0.35f);
    int dw = (int)(unit * 0.86f);        // odstęp między cyframi
    int cell = (int)unit;
    int dotCell = (int)(unit * 0.35f);
    int x = a.x1;
    lv_color_t off = lv_color_hex(0x16264D);

    for (int i = 0; i < n; i++) {
        char c = d->txt[i];
        if (c == '.') {
            int s = dw * 17 / 100 + 2;
            fillRect(ctx, x + (dotCell - s) / 2, a.y1 + H - s, s, s, d->col, LV_OPA_COVER, s / 3);
            x += dotCell;
            continue;
        }
        uint8_t mask = 0;
        if (c >= '0' && c <= '9') mask = SEG_MAP[c - '0'];
        else if (c == '-') mask = 0b1000000;
        drawDigit(ctx, x, a.y1, dw, H, mask, d->col, off);
        x += cell;
    }
}

static void sevenSegDelete(lv_event_t* e) {
    lv_obj_t* o = lv_event_get_target(e);
    SevenSegData* d = (SevenSegData*)lv_obj_get_user_data(o);
    if (d) free(d);
}

lv_obj_t* uiSevenSeg(lv_obj_t* parent, lv_coord_t x, lv_coord_t y, lv_coord_t w, lv_coord_t h) {
    lv_obj_t* o = lv_obj_create(parent);
    lv_obj_remove_style_all(o);
    lv_obj_set_pos(o, x, y);
    lv_obj_set_size(o, w, h);
    lv_obj_clear_flag(o, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
    SevenSegData* d = (SevenSegData*)calloc(1, sizeof(SevenSegData));
    d->col = C_TEXT;
    strcpy(d->txt, "0.0");
    lv_obj_set_user_data(o, d);
    lv_obj_add_event_cb(o, sevenSegDraw, LV_EVENT_DRAW_MAIN, nullptr);
    lv_obj_add_event_cb(o, sevenSegDelete, LV_EVENT_DELETE, nullptr);
    return o;
}

void uiSevenSegSet(lv_obj_t* o, const char* txt, lv_color_t col) {
    SevenSegData* d = (SevenSegData*)lv_obj_get_user_data(o);
    if (!d) return;
    if (strcmp(d->txt, txt) == 0 && d->col.full == col.full) return;
    strlcpy(d->txt, txt, sizeof(d->txt));
    d->col = col;
    lv_obj_invalidate(o);
}

// ------------------------------------------------------------
// Widok drogi (perspektywa)
// ------------------------------------------------------------
struct RoadData {
    Status st;
    float  patDist;
};

static const float ROAD_D0     = 3.0f;    // parametr perspektywy [m]
static const float ROAD_DMAX   = 22.0f;   // zasięg widoku do przodu [m]
static const float ROAD_DBACK  = 1.0f;    // widok za pojazdem [m]

static void roadDraw(lv_event_t* e) {
    lv_obj_t* o = lv_event_get_target(e);
    RoadData* rd = (RoadData*)lv_obj_get_user_data(o);
    if (!rd) return;
    lv_draw_ctx_t* ctx = lv_event_get_draw_ctx(e);
    lv_area_t a;
    lv_obj_get_coords(o, &a);
    const Status& st = rd->st;

    float W = a.x2 - a.x1 + 1;
    float H = a.y2 - a.y1 + 1;
    float cx = a.x1 + W / 2.0f;
    float yb = a.y2 - 2;
    float yt = a.y1 + 2;
    float HH = yb - yt;
    float Wn = W * 0.90f;
    float sf = 0.26f;                       // stosunek szerokości daleko/blisko

    auto roadW = [&](float t) { return Wn * (1.0f - (1.0f - sf) * t); };
    auto yOf   = [&](float t) { return yb - t * HH; };
    float dTop = ROAD_DMAX + ROAD_DBACK;
    float den  = dTop / (dTop + ROAD_D0);
    auto tOf   = [&](float d) {
        float dd = d + ROAD_DBACK;
        if (dd < 0) dd = 0;
        float t = (dd / (dd + ROAD_D0)) / den;
        if (t > 1.0f) t = 1.0f;
        return t;
    };

    // jezdnia
    quad(ctx, C_ROAD, LV_OPA_COVER,
         cx - roadW(0) / 2, cx + roadW(0) / 2, yb,
         cx - roadW(1) / 2, cx + roadW(1) / 2, yt);
    // pobocza (jasne linie krawędziowe jezdni)
    {
        float e = 3.0f;
        quad(ctx, lv_color_hex(0x6D7A99), LV_OPA_COVER,
             cx - roadW(0) / 2 - e, cx - roadW(0) / 2, yb,
             cx - roadW(1) / 2 - 1, cx - roadW(1) / 2, yt);
        quad(ctx, lv_color_hex(0x6D7A99), LV_OPA_COVER,
             cx + roadW(0) / 2, cx + roadW(0) / 2 + e, yb,
             cx + roadW(1) / 2, cx + roadW(1) / 2 + 1, yt);
    }

    bool painting = (st.state == MS_PAINTING);

    // pas [d1,d2] (d1<d2, metry względem pojazdu) dla pistoletu g
    auto strip = [&](int g, float d1, float d2, lv_color_t col, lv_opa_t opa) {
        if (d2 <= -ROAD_DBACK || d1 >= ROAD_DMAX) return;
        if (d1 < -ROAD_DBACK) d1 = -ROAD_DBACK;
        if (d2 > ROAD_DMAX) d2 = ROAD_DMAX;
        if (d2 - d1 < 0.001f) return;
        float t1 = tOf(d1), t2 = tOf(d2);
        float y1 = yOf(t1), y2 = yOf(t2);
        float lat = GUN_LATERAL[g];
        float wf  = GUN_WIDTH_FR[g];
        float w1 = roadW(t1), w2 = roadW(t2);
        float c1 = cx + lat * w1, c2 = cx + lat * w2;
        float h1 = wf * w1 / 2, h2 = wf * w2 / 2;
        if (h1 < 1.5f) h1 = 1.5f;
        if (h2 < 1.0f) h2 = 1.0f;
        quad(ctx, col, opa, c1 - h1, c1 + h1, y1, c2 - h2, c2 + h2, y2);
    };

    lv_color_t colPainted = C_YELLOW;
    lv_color_t colAhead   = lv_color_hex(0xB89F00);

    for (int g = 0; g < NGUNS; g++) {
        GunCfg c = effectiveGun(st, g);
        if (c.mode == GM_OFF) continue;
        if (c.mode == GM_CONT) {
            strip(g, -ROAD_DBACK, 0.0f, painting ? colPainted : colAhead, painting ? LV_OPA_COVER : LV_OPA_60);
            strip(g, 0.0f, ROAD_DMAX, colAhead, LV_OPA_70);
        } else {
            float cyc = c.line + c.gap;
            if (cyc <= 0.01f) continue;
            float pd = rd->patDist;
            int k0 = (int)floorf((pd - ROAD_DBACK) / cyc);
            int k1 = (int)ceilf((pd + ROAD_DMAX) / cyc);
            if (k0 < 0) k0 = 0;
            for (int k = k0; k <= k1; k++) {
                float p1 = k * cyc;
                float d1 = p1 - pd;
                float d2 = d1 + c.line;
                if (d1 < 0.0f && d2 > 0.0f) {
                    strip(g, d1, 0.0f, painting ? colPainted : colAhead, painting ? LV_OPA_COVER : LV_OPA_60);
                    strip(g, 0.0f, d2, colAhead, LV_OPA_70);
                } else if (d2 <= 0.0f) {
                    strip(g, d1, d2, painting ? colPainted : colAhead, painting ? LV_OPA_COVER : LV_OPA_60);
                } else {
                    strip(g, d1, d2, colAhead, LV_OPA_70);
                }
            }
        }
    }

    // Pistolety aktualnie strzelające (stan rzeczywisty ze sterownika)
    for (int g = 0; g < NGUNS; g++) {
        if (!st.guns[g]) continue;
        strip(g, -0.7f, 0.0f, C_GREENB, LV_OPA_COVER);
    }

    // Linia bieżącej pozycji maszyny
    {
        float t0 = tOf(0.0f);
        float y0 = yOf(t0);
        float ww = roadW(t0);
        quad(ctx, C_TEXT, LV_OPA_COVER, cx - ww / 2, cx + ww / 2, y0 + 1.5f, cx - ww / 2, cx + ww / 2, y0 - 1.5f);
    }
}

static void roadDelete(lv_event_t* e) {
    lv_obj_t* o = lv_event_get_target(e);
    RoadData* d = (RoadData*)lv_obj_get_user_data(o);
    if (d) delete d;
}

lv_obj_t* uiRoadView(lv_obj_t* parent, lv_coord_t x, lv_coord_t y, lv_coord_t w, lv_coord_t h) {
    lv_obj_t* o = lv_obj_create(parent);
    lv_obj_remove_style_all(o);
    lv_obj_set_pos(o, x, y);
    lv_obj_set_size(o, w, h);
    lv_obj_set_style_bg_color(o, C_SKY, 0);
    lv_obj_set_style_bg_opa(o, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(o, 6, 0);
    lv_obj_set_style_clip_corner(o, true, 0);
    lv_obj_clear_flag(o, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
    RoadData* d = new RoadData();
    d->patDist = 0;
    lv_obj_set_user_data(o, d);
    lv_obj_add_event_cb(o, roadDraw, LV_EVENT_DRAW_MAIN_END, nullptr);
    lv_obj_add_event_cb(o, roadDelete, LV_EVENT_DELETE, nullptr);
    return o;
}

void uiRoadSet(lv_obj_t* o, const Status& st, float patDistNow) {
    RoadData* d = (RoadData*)lv_obj_get_user_data(o);
    if (!d) return;
    d->st = st;
    d->patDist = patDistNow;
    lv_obj_invalidate(o);
}

// ------------------------------------------------------------
// Miniatura wzorca (widok z góry, cykl od dołu)
// ------------------------------------------------------------
struct GlyphData {
    int    pat;
    GunCfg cfg[NGUNS];
};

// Rysunek wzorca W SKALI (widok z góry, początek wzorca na dole):
//  - poziomo okno 72 cm; szerokości linii proporcjonalne (24 cm = 2 x 12 cm), rozstaw pistoletów jak w
//    wzorcach P-3 / P-4 (środki co 24 cm);
//  - pionowo dokładnie 2 pełne cykle kreska+przerwa najdłuższego pistoletu przerywanego, kreski i przerwy
//    w proporcji długości wzorca;
//  - wzorce krawędziowe: po prawej szare pobocze, linia 25 cm od krawędzi jezdni (nominalnie).
static void glyphDraw(lv_event_t* e) {
    lv_obj_t* o = lv_event_get_target(e);
    GlyphData* d = (GlyphData*)lv_obj_get_user_data(o);
    if (!d) return;
    lv_draw_ctx_t* ctx = lv_event_get_draw_ctx(e);
    lv_area_t a;
    lv_obj_get_coords(o, &a);
    const int W = a.x2 - a.x1 + 1;
    const int H = a.y2 - a.y1 + 1;

    fillRect(ctx, a.x1, a.y1, W, H, C_ROAD, LV_OPA_COVER, 3);

    bool any = false, edgeOnly = true;
    float maxCycle = 0.0f;
    for (int g = 0; g < NGUNS; g++) {
        const GunCfg& c = d->cfg[g];
        if (c.mode == GM_OFF) continue;
        any = true;
        if (g < 4) edgeOnly = false;
        if (c.mode == GM_DASH && c.line > 0.0f) {
            float cyc = c.line + c.gap;
            if (cyc > maxCycle) maxCycle = cyc;
        }
    }
    if (!any) return;

    const float WIN_CM = 72.0f;
    const float pxPerCm = W / WIN_CM;
    int base = (int)floorf(12.0f * pxPerCm + 0.5f);          // szerokość linii 12 cm w pikselach
    if (base < 2) base = 2;
    float cx = a.x1 + W / 2.0f;
    if (edgeOnly) cx -= 8.0f * pxPerCm;                      // linia trochę w lewo od środka okna

    if (edgeOnly) {
        // pobocze: za krawędzią jezdni (25 cm od środka linii)
        int ex = (int)(cx + EDGE_LINE_OFFSET_CM * pxPerCm);
        if (ex < a.x2) fillRect(ctx, ex, a.y1 + 1, a.x2 - ex, H - 2, lv_color_hex(0x59606e), LV_OPA_COVER, 0);
    }

    const float L = (maxCycle > 0.0f) ? 2.0f * maxCycle : 0.0f;
    const float pxPerM = (L > 0.0f) ? (H - 4) / L : 0.0f;

    for (int g = 0; g < NGUNS; g++) {
        const GunCfg& c = d->cfg[g];
        if (c.mode == GM_OFF) continue;
        int lw = base * GUN_WIDTH_CM[g] / 12;                // 24 cm = dokładnie 2 x 12 cm
        float centerCm = edgeOnly ? 0.0f : (float)GUN_CENTER_CM[g];
        int lx = (int)floorf(cx + centerCm * pxPerCm - lw / 2.0f + 0.5f);
        if (c.mode == GM_CONT) {
            fillRect(ctx, lx, a.y1 + 2, lw, H - 4, C_YELLOW);
        } else if (c.line > 0.0f && c.gap >= 0.0f && L > 0.0f) {
            float cyc = c.line + c.gap;
            for (float p = 0.0f; p < L - 0.001f; p += cyc) {
                float p2 = p + c.line;
                if (p2 > L) p2 = L;
                int yb = a.y2 - 2 - (int)floorf(p * pxPerM + 0.5f);
                int yt = a.y2 - 2 - (int)floorf(p2 * pxPerM + 0.5f);
                if (yt < a.y1 + 2) yt = a.y1 + 2;
                if (yb - yt < 1) continue;
                fillRect(ctx, lx, yt, lw, yb - yt, C_YELLOW);
            }
        }
    }
}

static void glyphDelete(lv_event_t* e) {
    lv_obj_t* o = lv_event_get_target(e);
    GlyphData* d = (GlyphData*)lv_obj_get_user_data(o);
    if (d) delete d;
}

lv_obj_t* uiGlyph(lv_obj_t* parent, lv_coord_t x, lv_coord_t y, lv_coord_t w, lv_coord_t h) {
    lv_obj_t* o = lv_obj_create(parent);
    lv_obj_remove_style_all(o);
    lv_obj_set_pos(o, x, y);
    lv_obj_set_size(o, w, h);
    lv_obj_clear_flag(o, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(o, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(o, LV_OBJ_FLAG_EVENT_BUBBLE);
    GlyphData* d = new GlyphData();
    d->pat = -1;
    for (int i = 0; i < NGUNS; i++) d->cfg[i] = {GM_OFF, 0, 0};
    lv_obj_set_user_data(o, d);
    lv_obj_add_event_cb(o, glyphDraw, LV_EVENT_DRAW_MAIN, nullptr);
    lv_obj_add_event_cb(o, glyphDelete, LV_EVENT_DELETE, nullptr);
    return o;
}

void uiGlyphSet(lv_obj_t* o, int patIdx, bool reversed, const Status& st) {
    GlyphData* d = (GlyphData*)lv_obj_get_user_data(o);
    if (!d) return;
    GunCfg tmp[NGUNS];
    for (int g = 0; g < NGUNS; g++) tmp[g] = patternGun(patIdx, reversed, g, st);
    bool same = (d->pat == patIdx);
    for (int g = 0; g < NGUNS && same; g++) {
        if (d->cfg[g].mode != tmp[g].mode || d->cfg[g].line != tmp[g].line ||
            d->cfg[g].gap != tmp[g].gap) same = false;
    }
    if (same) return;
    d->pat = patIdx;
    for (int g = 0; g < NGUNS; g++) d->cfg[g] = tmp[g];
    lv_obj_invalidate(o);
}
