#!/usr/bin/env python3
"""Wizualizacja obudowy komputera MPD2026 z pionowym ekranem 7".

Wzorzec: zdjęcie referencyjne (sprzęt STiM): jasna obudowa, ciemnogranatowe klawisze w kształcie strzałek
po bokach ekranu (przy etykietach wzorców), dolna grupa klawiszy funkcyjnych.

Wymiary są ORIENTACYJNE (do weryfikacji na prototypie i rzeczywistych modułach).
Uruchomienie: python docs/schematy/generate_enclosure.py  (wywoływane też przez generate_svgs.py)
Wynik: obudowa_pionowa_os.svg (grupa OŚ) i obudowa_pionowa_krawedz.svg (grupa KRAWĘDŹ) — ten sam sprzęt,
różni się tylko zawartością ekranu i wskazaniem przycisku GRUPA.
"""
import os
import sys
import textwrap

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from generate_svgs import Svg, ui_mock_portrait  # noqa: E402

# ---- wymiary (mm) ----
HW, HH, HD = 172, 250, 62              # obudowa: szerokość, wysokość, głębokość
WIN_W, WIN_H = 94, 160                 # okno w płycie czołowej (widoczna czarna ramka modułu)
WIN_X, WIN_Y = (HW - WIN_W) / 2, 15
ACT_W, ACT_H = 85.9, 154.2             # obszar aktywny wyświetlacza 7" (obrócony do pionu)
ACT_X, ACT_Y = (HW - ACT_W) / 2, WIN_Y + (WIN_H - ACT_H) / 2
MOD_W, MOD_H = 100, 165                # moduł DWIN DGUS 7" (obrys płytki, orientacyjnie, pionowo)
KEY_W, KEY_H = 25, 20.4                # klawisz strzałkowy
KEY_PITCH = 116 * ACT_H / 800          # = odstęp etykiet wzorców na ekranie (22,4 mm)
KEY_Y0 = ACT_Y + 106 * ACT_H / 800
LKEY_X, RKEY_X = 8, HW - 8 - KEY_W

DEFS = """<defs>
<linearGradient id="gHousing" x1="0" y1="0" x2="1" y2="1">
  <stop offset="0" stop-color="#fbfbfc"/><stop offset="0.55" stop-color="#e3e6ea"/><stop offset="1" stop-color="#c5cad1"/>
</linearGradient>
<linearGradient id="gSide" x1="0" y1="0" x2="1" y2="0">
  <stop offset="0" stop-color="#cfd4da"/><stop offset="1" stop-color="#9aa1aa"/>
</linearGradient>
<linearGradient id="gTop" x1="0" y1="1" x2="0" y2="0">
  <stop offset="0" stop-color="#e9ecef"/><stop offset="1" stop-color="#f8f9fa"/>
</linearGradient>
<linearGradient id="gKey" x1="0" y1="0" x2="0" y2="1">
  <stop offset="0" stop-color="#34509f"/><stop offset="1" stop-color="#14265a"/>
</linearGradient>
<linearGradient id="gStart" x1="0" y1="0" x2="0" y2="1">
  <stop offset="0" stop-color="#33c463"/><stop offset="1" stop-color="#127a36"/>
</linearGradient>
<linearGradient id="gStop" x1="0" y1="0" x2="0" y2="1">
  <stop offset="0" stop-color="#ea4b4e"/><stop offset="1" stop-color="#9b1418"/>
</linearGradient>
<linearGradient id="gGlass" x1="0" y1="0" x2="1" y2="1">
  <stop offset="0" stop-color="#0c1116"/><stop offset="1" stop-color="#1c2531"/>
</linearGradient>
<pattern id="hatch" width="6" height="6" patternUnits="userSpaceOnUse" patternTransform="rotate(45)">
  <line x1="0" y1="0" x2="0" y2="6" stroke="#adb5bd" stroke-width="1"/>
</pattern>
</defs>"""


def arrow_key(s, x, y, w, h, tip_right, label, fs):
    t = 0.30 * h
    if tip_right:
        pts = [(x, y), (x + w - t, y), (x + w, y + h / 2), (x + w - t, y + h), (x, y + h)]
    else:
        pts = [(x + w, y), (x + t, y), (x, y + h / 2), (x + t, y + h), (x + w, y + h)]
    s.add('<polygon points="' + " ".join(f"{a:.1f},{b:.1f}" for a, b in pts) +
          '" fill="url(#gKey)" stroke="#0a1330" stroke-width="2.5" stroke-linejoin="round"/>')
    hx1, hx2 = (x + 4, x + w - t - 2) if tip_right else (x + t + 2, x + w - 4)
    s.line(hx1, y + 3.5, hx2, y + 3.5, "#7f97e0", 1.5)
    cx = x + (w - t / 2) / 2 if tip_right else x + t / 2 + (w - t / 2) / 2
    s.text(cx, y + h / 2 + fs * 0.36, label, fs, "#fff", "middle", "bold")


def front(s, ox, oy, k, detail=True, group=0):
    """Widok z przodu; k = px na mm. group: 0 = OŚ JEZDNI, 1 = KRAWĘDŹ (zmienia zawartość ekranu i wskaźnik GRUPY)."""
    def X(v): return ox + v * k
    def Y(v): return oy + v * k

    # cień + korpus
    s.rect(X(2.2), Y(2.8), HW * k, HH * k, "#00000026", rx=9 * k)
    s.rect(X(0), Y(0), HW * k, HH * k, "url(#gHousing)", "#7b838c", 2, 9 * k)
    s.rect(X(2), Y(2), (HW - 4) * k, (HH - 4) * k, "none", "#ffffffb0", 1.2, 7.5 * k)

    # górna listwa: logo + otwory buzzera
    fs = max(9, 4.2 * k)
    s.text(X(9), Y(9.6), "TRASSAR", fs, "#14265a", weight="bold")
    s.text(X(HW - 9), Y(9.6), "MPD2026", fs * 0.85, "#495057", "end")
    for i in range(6):
        s.circle(X(HW / 2 - 7.5 + i * 3), Y(6.2), max(0.9, 0.45 * k), "#6c757d")

    # okno ekranu
    s.rect(X(WIN_X - 1.2), Y(WIN_Y - 1.2), (WIN_W + 2.4) * k, (WIN_H + 2.4) * k, "#8b939c", "none", 0, 3.6 * k)
    s.rect(X(WIN_X), Y(WIN_Y), WIN_W * k, WIN_H * k, "url(#gGlass)", "#000", 1.5, 2.6 * k)
    sx, sy = ACT_W * k / 480, ACT_H * k / 800
    s.group_open(f"translate({X(ACT_X):.2f},{Y(ACT_Y):.2f}) scale({sx:.5f},{sy:.5f})")
    ui_mock_portrait(s, 0, 0, 1.0, group)
    s.group_close()
    # odblask na szybie
    s.add(f'<polygon points="{X(WIN_X):.1f},{Y(WIN_Y):.1f} {X(WIN_X + 34):.1f},{Y(WIN_Y):.1f} '
          f'{X(WIN_X):.1f},{Y(WIN_Y + 60):.1f}" fill="#ffffff" opacity="0.06"/>')

    # klawisze strzałkowe S1-S10 na wysokości etykiet (kod wzorca aktywnej grupy widać już na ekranie obok)
    kfs = max(8, 3.6 * k)
    for i in range(5):
        yy = KEY_Y0 + i * KEY_PITCH
        arrow_key(s, X(LKEY_X), Y(yy), KEY_W * k, KEY_H * k, True, f"S{i + 1}", kfs)
        arrow_key(s, X(RKEY_X), Y(yy), KEY_W * k, KEY_H * k, False, f"S{i + 6}", kfs)

    # dolna grupa: GRUPA / SELEKTOR / GAP (GRUPA pokazuje aktualnie aktywną stronę wzorców)
    by = 184
    grp_col = "#2f86ff" if group == 0 else "#f57f17"
    grp_txt = "GRUPA: OŚ" if group == 0 else "GRUPA: KRAWĘDŹ"
    for i, (name, sub) in enumerate([(grp_txt, "OŚ / KRAWĘDŹ"), ("SELEKTOR", "odwróć"), ("GAP", "od przerwy")]):
        bx = 10 + i * 54
        fillc = "url(#gKey)" if i != 0 else grp_col
        s.rect(X(bx), Y(by), 44 * k, 14 * k, fillc, "#0a1330", 2, 2.4 * k)
        s.line(X(bx + 3), Y(by + 1.8), X(bx + 41), Y(by + 1.8), "#7f97e0", 1.2)
        s.text(X(bx + 22), Y(by + 8.6), name, max(7, 3.1 * k) if i == 0 else max(8, 3.5 * k), "#fff", "middle", "bold")
        if detail:
            s.text(X(bx + 22), Y(by + 18), sub, max(6.5, 2.6 * k), "#495057", "middle")

    # START / STOP
    s.rect(X(10), Y(204), 92 * k, 34 * k, "url(#gStart)", "#0b4d22", 2.5, 4 * k)
    s.text(X(56), Y(204 + 21.5), "▶ START", max(11, 8 * k), "#fff", "middle", "bold")
    s.rect(X(108), Y(204), 54 * k, 34 * k, "url(#gStop)", "#5e0c0f", 2.5, 4 * k)
    s.text(X(135), Y(204 + 21.5), "■ STOP", max(10, 6.6 * k), "#fff", "middle", "bold")


def dim_h(s, x1, x2, y, label, size=13):
    s.line(x1, y, x2, y, "#c1121f", 1.4)
    for x in (x1, x2):
        s.line(x, y - 6, x, y + 6, "#c1121f", 1.4)
    s.text((x1 + x2) / 2, y - 6, label, size, "#c1121f", "middle", "bold")


def dim_v(s, x, y1, y2, label, size=13):
    s.line(x, y1, x, y2, "#c1121f", 1.4)
    for y in (y1, y2):
        s.line(x - 6, y, x + 6, y, "#c1121f", 1.4)
    s.add(f'<text x="{x + 6}" y="{(y1 + y2) / 2}" font-family="Segoe UI, Arial, sans-serif" font-size="{size}" '
          f'fill="#c1121f" font-weight="bold" text-anchor="middle" transform="rotate(-90 {x + 6} {(y1 + y2) / 2})">'
          f'{label}</text>')


def badge(s, x, y, n):
    s.circle(x, y, 11, "#c1121f", "#fff", 1.5)
    s.text(x, y + 5, str(n), 13, "#fff", "middle", "bold")


def side_section(s, ox, oy, k):
    def X(v): return ox + v * k
    def Y(v): return oy + v * k
    s.rect(X(0), Y(0), HD * k, HH * k, "#f1f3f5", "#495057", 2.5)
    # ściany obudowy 3 mm
    for (a, b, c, d) in [(0, 0, HD, 3), (0, HH - 3, HD, 3), (HD - 3, 0, 3, HH)]:
        s.rect(X(a), Y(b), c * k, d * k, "#adb5bd", "#495057", 1)
    # płyta czołowa 3 mm (z oknem), szyba
    s.rect(X(0), Y(0), 3 * k, HH * k, "#ced4da", "#495057", 1)
    s.rect(X(0), Y(WIN_Y), 3 * k, WIN_H * k, "#8ecae6", "#023e8a", 1)
    # moduł 7"
    s.rect(X(3), Y(12.5), 13 * k, MOD_H * k, "#343a40", "#000", 1.5)
    s.text(X(9.5), Y(95), "Wyświetlacz DGUS (13 mm)", 11, "#fff", "middle", extra=f'transform="rotate(-90 {X(9.5)} {Y(95)})"')
    # PCB klawiszy dolnej grupy (klawisze S1-S10 leżą z boków modułu — poza płaszczyzną przekroju)
    s.rect(X(3), Y(184), 5 * k, 56 * k, "#2b8a3e", "#1b5e20", 1)
    s.text(X(5.5), Y(236), "PCB", 9, "#fff", "middle")
    # płyta sterownika za modułem
    s.rect(X(20), Y(70), 14 * k, 92 * k, "#1971c2", "#0b3d91", 1.5)
    s.text(X(27), Y(116), "płyta sterownika", 10, "#fff", "middle", extra=f'transform="rotate(-90 {X(27)} {Y(116)})"')
    # moduł przekaźników + zasilacz
    s.rect(X(20), Y(186), 17 * k, 54 * k, "#e8590c", "#7a2e00", 1.5)
    s.text(X(28.5), Y(215), "przekaźniki + zasilacz", 10, "#fff", "middle", extra=f'transform="rotate(-90 {X(28.5)} {Y(215)})"')
    # VESA
    s.rect(X(HD), Y(88), 8 * k, 12 * k, "url(#hatch)", "#495057", 1.5)
    s.rect(X(HD), Y(150), 8 * k, 12 * k, "url(#hatch)", "#495057", 1.5)
    s.text(X(HD + 10), Y(128), "VESA 75", 11, "#495057", extra=f'transform="rotate(90 {X(HD + 10)} {Y(128)})"')
    # złącza dolne
    for i in range(3):
        cx = 12 + i * 20
        s.rect(X(cx), Y(HH), 14 * k, 12 * k, "#868e96", "#343a40", 1.5, 3)
    # uszczelka + odpowietrznik
    s.circle(X(HD - 1.5), Y(30), 5, "#f59f00", "#7a4b00", 1.5)
    # slot microSD (bok)
    s.rect(X(HD - 3), Y(196), 3 * k, 3 * k, "#212529")
    s.text(X(HD - 4), Y(196 - 3), "microSD", 10, "#212529", "end")
    # wymiary
    dim_h(s, X(0), X(HD), Y(HH) + 58, f"{HD} mm", 12)
    dim_v(s, X(-2.6), Y(0), Y(HH), f"{HH} mm", 12)
    s.text(X(HD / 2), Y(HH) + 50, "złącza J1–J6 (spód)", 10.5, "#495057", "middle")


def right_side_exterior(s, ox, oy, k):
    """Widok zewnętrzny prawej ściany bocznej — montaż grzybka E-STOP."""
    def X(v): return ox + v * k
    def Y(v): return oy + v * k
    s.rect(X(0), Y(0), HD * k, HH * k, "url(#gSide)", "#495057", 2.5, 6)
    s.text(X(HD / 2), Y(0) - 10, "PRZÓD", 10.5, "#495057", "middle")
    s.text(X(HD / 2), Y(HH) + 16, "TYŁ", 10.5, "#495057", "middle")

    # grzybek E-STOP: kolnierz + kapelusz, wzorowany na realnym przycisku grzybkowym
    bx, by = HD / 2, 90
    s.circle(X(bx), Y(by), 13 * k, "#495057", "#212529", 2)
    s.circle(X(bx), Y(by), 11 * k, "#adb5bd", "#212529", 1.5)
    s.circle(X(bx), Y(by), 8.6 * k, "url(#gStop)", "#5e0c0f", 2)
    s.text(X(bx), Y(by) + 4.6 * k, "STOP", max(8, 3.1 * k), "#fff", "middle", "bold")
    badge(s, X(bx) + 13 * k + 14, Y(by) - 13 * k - 6, 11)
    s.line(X(bx) + 13 * k, Y(by), X(bx) + 13 * k + 34, Y(by), "#c1121f", 1.4)
    s.text(X(bx) + 13 * k + 38, Y(by) + 4, "grzybek E-STOP", 12, "#0b1f4a", weight="bold")
    s.text(X(bx) + 13 * k + 38, Y(by) + 20, "styk NC, w torze zasilania", 10.5, "#495057")
    s.text(X(bx) + 13 * k + 38, Y(by) + 34, "przekaźników (ARCHITEKTURA", 10.5, "#495057")
    s.text(X(bx) + 13 * k + 38, Y(by) + 48, "TERMINAL.md, rozdz. 7)", 10.5, "#495057")

    dim_h(s, X(0), X(HD), Y(HH) + 40, f"{HD} mm", 12)
    dim_v(s, X(-16), Y(0), Y(HH), f"{HH} mm", 12)
    dim_v(s, X(HD) + 18, Y(0), Y(by), f"{by} mm", 11)


def bottom_view(s, ox, oy, k):
    def X(v): return ox + v * k
    def Y(v): return oy + v * k
    s.rect(X(0), Y(0), HW * k, HD * k, "url(#gSide)", "#495057", 2.5, 5)
    s.text(X(HW / 2), Y(-3), "panel złączy (spód obudowy) — 6 x złącze okrągłe", 13, "#0b1f4a", "middle", "bold")
    names = [("J1", "5 V", "TS13CP03"), ("J2", "ZAWORY", "TS17CP10"), ("J3", "ENKODER", "TS13CP05"),
             ("J4", "PILOT", "TS13PS06"), ("J5", "PEDAŁ", "TS21CP04"), ("J6", "ŁĄCZE*", "M12 4-pin")]
    for i, (j, nm, typ) in enumerate(names):
        cx = 20 + i * 26.5
        s.circle(X(cx), Y(HD / 2 - 6), 11 * k, "#495057", "#212529", 2)
        s.circle(X(cx), Y(HD / 2 - 6), 8.4 * k, "#adb5bd", "#212529", 1.5)
        for a in range(6):
            s.circle(X(cx) + 3.4 * k * (1 if a % 2 else -1), Y(HD / 2 - 6) + (a - 2.5) * 1.4 * k, 1.5, "#212529")
        s.text(X(cx), Y(HD / 2 + 12.5), j, 15, "#0b1f4a", "middle", "bold")
        s.text(X(cx), Y(HD / 2 + 18), nm, 10.5, "#212529", "middle")
        s.text(X(cx), Y(HD / 2 + 23), typ, 8.5, "#495057", "middle")
    s.text(X(HW / 2), Y(HD + 7), "* J6 = wyprowadzenie UART ekranu DGUS, gdyby był montowany osobno (opcja); w obudowie zintegrowanej łącze jest wewnętrzne.",
           11, "#b71c1c", "middle")


def rear_layout(s, ox, oy, k):
    """Widok od tyłu, pokrywa zdjęta (lustrzany względem przodu) — orientacyjne rozmieszczenie."""
    def X(v): return ox + (HW - v) * k          # lustro: patrzymy od tyłu
    def R(v, y, w, h, fill, stroke, label, fs=11, lab_col="#fff", sw=1.5):
        s.rect(X(v + w), oy + y * k, w * k, h * k, fill, stroke, sw, 3)
        if label:
            s.text(X(v + w / 2), oy + (y + h / 2) * k + fs * 0.35, label, fs, lab_col, "middle", "bold")
    s.rect(ox, oy, HW * k, HH * k, "#f8f9fa", "#495057", 3, 9 * k)
    s.rect(ox + 3 * k, oy + 3 * k, (HW - 6) * k, (HH - 6) * k, "none", "#adb5bd", 1.2, 6 * k)
    # ekran DGUS (schowany za PCB, ma wlasna plyte sterujaca - czarna skrzynka)
    R((HW - MOD_W) / 2, 12.5, MOD_W, MOD_H, "#343a40", "#000", "")
    s.text(ox + HW / 2 * k, oy + 19 * k, "wyświetlacz DGUS, orient. 100 x 165", 11, "#fff", "middle")
    # PCB klawiszy S1-S10
    R(LKEY_X - 1, 34, KEY_W + 3, 122, "#2b8a3e", "#1b5e20", "PCB S1–S5", 10)
    R(RKEY_X - 2, 34, KEY_W + 3, 122, "#2b8a3e", "#1b5e20", "PCB S6–S10", 10)
    # płyta sterownika
    R(40, 46, 92, 84, "#1971c2", "#0b3d91", "", 12)
    s.text(X(86), oy + 60 * k, "PŁYTA STEROWNIKA", 13, "#fff", "middle", "bold")
    for (dx, dy, w, h, lb) in [(44, 66, 56, 26, "ESP32-S3 DevKitC-1"),
                               (44, 96, 24, 18, "MCP23017"), (72, 96, 16, 18, "RTC"), (92, 96, 30, 18, "GPS NEO-6M")]:
        R(dx, dy, w, h, "#74c0fc", "#1864ab", lb, 8.5, "#0b1f4a", 1)
    R(104, 66, 28, 26, "#ffd8a8", "#e8590c", "dzielnik", 8, "#7a2e00", 1)
    R(14, 140, 22, 16, "#e9ecef", "#495057", "BUZZER", 7.5, "#212529", 1)
    # dolny pas: przekaźniki + zasilanie + START/STOP PCB
    R(10, 184, 98, 56, "#e8590c", "#7a2e00", "MODUŁ 6 PRZEKAŹNIKÓW", 11)
    R(112, 184, 50, 56, "#f08c00", "#7a4b00", "", 11)
    s.text(X(137), oy + 200 * k, "ZASILANIE", 11, "#fff", "middle", "bold")
    s.text(X(137), oy + 213 * k, "bezpiecznik, TVS,", 9, "#fff", "middle")
    s.text(X(137), oy + 224 * k, "przetwornica 5 V", 9, "#fff", "middle")
    # przewody
    s.line(X(86), oy + 130 * k, X(86), oy + 184 * k, "#212529", 2.5, "6 4")
    s.line(X(50), oy + 130 * k, X(50), oy + 156 * k, "#2b8a3e", 2.5, "6 4")
    s.line(X(130), oy + 130 * k, X(130), oy + 156 * k, "#2b8a3e", 2.5, "6 4")
    s.line(X(137), oy + 240 * k, X(137), oy + 250 * k, "#212529", 3)
    s.text(ox, oy + HH * k + 24, "linie przerywane: wiązki wewnętrzne (sygnały, I2C, UART ekranu — 2 przewody + dzielnik, zasilanie 5 V)", 10.5, "#495057")
    s.text(ox, oy + HH * k + 42, "brak drugiego ESP32 — ekran DGUS ma własną elektronikę; przekaźniki i zasilacz można wynieść do osobnej skrzynki.", 10.5, "#495057")


def enclosure_vertical(group=0):
    grp_name = "OŚ JEZDNI" if group == 0 else "KRAWĘDŹ"
    W, H = 1900, 1700
    s = Svg(W, H, "#f8f9fa")
    s.add(DEFS)
    s.text(40, 46, f"Obudowa komputera MPD2026 — wersja pionowa, grupa {grp_name} (wzorzec: sprzęt STiM)", 24, "#0b1f4a", weight="bold")
    s.text(40, 74, "Jasna obudowa, granatowe klawisze-strzałki przy etykietach wzorców, grupa funkcyjna pod ekranem. "
                   "Wymiary orientacyjne (~172 x 250 x 62 mm) — do weryfikacji na prototypie i rzeczywistych modułach.", 14, "#444")

    # baner: prostota podłączenia (ekran DGUS = jeden kabel UART, bez RS-485, bez drugiego ESP32)
    s.rect(1420, 26, 440, 64, "#e6f7ee", "#1b5e20", 2, 10)
    s.text(1640, 48, "PROSTE PODŁĄCZENIE EKRANU", 13, "#1b5e20", "middle", "bold")
    s.text(1640, 66, "4 przewody (5 V, GND, TX, RX) — bez RS-485,", 11.5, "#215732", "middle")
    s.text(1640, 82, "bez drugiego ESP32. Konfiguracja: DGUS Designer.", 11.5, "#215732", "middle")

    # A: widok z przodu (3 px/mm)
    k = 3
    ax, ay = 110, 140
    s.text(ax, ay - 16, f"WIDOK Z PRZODU — grupa {grp_name}", 15, "#0b1f4a", weight="bold")
    front(s, ax, ay, k, group=group)
    dim_h(s, ax, ax + HW * k, ay + HH * k + 34, f"{HW} mm")
    dim_v(s, ax - 26, ay, ay + HH * k, f"{HH} mm")
    dim_h(s, ax + ACT_X * k, ax + (ACT_X + ACT_W) * k, ay + 12.2 * k, "86 mm", 11)
    dim_v(s, ax + (WIN_X + WIN_W) * k + 14, ay + ACT_Y * k, ay + (ACT_Y + ACT_H) * k, "154 mm", 11)
    # numery elementów
    badge(s, ax + (WIN_X + WIN_W / 2) * k, ay + 175 * k + 0, 1)
    badge(s, ax + (LKEY_X + KEY_W / 2) * k, ay + (KEY_Y0 - 2.5) * k, 2)
    badge(s, ax + 32 * k, ay + 181.5 * k, 3)
    badge(s, ax + 56 * k, ay + 246.5 * k, 4)
    badge(s, ax + 135 * k, ay + 246.5 * k, 5)
    badge(s, ax + (HW / 2 + 16) * k, ay + 6.2 * k, 8)

    # B: przekrój boczny
    bx, by = 760, 140
    s.text(bx, by - 16, "PRZEKRÓJ BOCZNY (przód po lewej)", 15, "#0b1f4a", weight="bold")
    side_section(s, bx, by, k)
    badge(s, bx + (HD + 10) * k * 0 + HD * k - 8, by + 196 * k, 7)
    badge(s, bx + (HD + 5) * k, by + 128 * k - 40, 9)
    badge(s, bx + (HD - 1.5) * k, by + 30 * k - 24, 10)

    # C: widok 3/4 (oblique)
    cx0, cy0 = 1120, 190
    kk = 1.9
    dx, dy = HD * kk * 0.42, -HD * kk * 0.30
    s.text(cx0, cy0 - 40, "WIDOK 3/4", 15, "#0b1f4a", weight="bold")
    fx, fy, fw, fh = cx0, cy0 + 60, HW * kk, HH * kk
    s.add(f'<polygon points="{fx + fw:.1f},{fy:.1f} {fx + fw + dx:.1f},{fy + dy:.1f} {fx + fw + dx:.1f},{fy + fh + dy:.1f} '
          f'{fx + fw:.1f},{fy + fh:.1f}" fill="url(#gSide)" stroke="#6c757d" stroke-width="2"/>')
    s.add(f'<polygon points="{fx:.1f},{fy:.1f} {fx + dx:.1f},{fy + dy:.1f} {fx + fw + dx:.1f},{fy + dy:.1f} {fx + fw:.1f},{fy:.1f}" '
          f'fill="url(#gTop)" stroke="#6c757d" stroke-width="2"/>')
    front(s, fx, fy, kk, detail=False, group=group)
    s.text(fx + fw + dx + 8, fy + fh / 2, "daszek przeciwsłoneczny (opcja) — nad ekranem", 11, "#495057",
           extra=f'transform="rotate(90 {fx + fw + dx + 8} {fy + fh / 2})"')

    # D: widok od dołu
    dx0, dy0 = 1120, 890
    s.text(dx0, dy0 - 40, "WIDOK OD DOŁU — panel złączy", 15, "#0b1f4a", weight="bold")
    bottom_view(s, dx0, dy0 + 6, 3)
    badge(s, dx0 - 20, dy0 + 6 + 30 * 3, 6)

    # D2: prawa ściana boczna — grzybek E-STOP
    gx0, gy0 = 1560, 190
    s.text(gx0, gy0 - 16, "WIDOK Z PRAWEJ STRONY — grzybek E-STOP", 15, "#0b1f4a", weight="bold")
    right_side_exterior(s, gx0, gy0, 2.4)

    # E: wnętrze od tyłu
    ex, ey = 110, 1060
    s.text(ex, ey - 16, "WNĘTRZE — widok od tyłu, pokrywa zdjęta (lustrzanie)", 15, "#0b1f4a", weight="bold")
    rear_layout(s, ex, ey, 2.2)

    # F: tabela wycięć
    tx, ty = 1120, 1160
    s.rect(tx - 20, ty - 34, 740, 545, "#fff", "#adb5bd", 1.5, 10)
    s.text(tx, ty - 8, "Elementy i wycięcia (numery z widoków)", 16, "#0b1f4a", weight="bold")
    rows = [
        ("1", "okno ekranu", "94 x 160 mm, naroża R3; szyba 3 mm; widoczny obszar 86 x 154 mm"),
        ("2", "klawisze S1–S10", "10 x 25 x 20,4 mm; skok 22,4 mm = odstęp etykiet wzorców na ekranie"),
        ("3", "GRUPA, SELEKTOR, GAP", "3 x 44 x 14 mm; GRUPA przełącza OŚ ⇄ KRAWĘDŹ"),
        ("4", "START", "92 x 34 mm, zielony; RĘCZNY: trzymaj = strzelaj"),
        ("5", "STOP", "54 x 34 mm, czerwony; przerwanie awaryjne GPIO 39"),
        ("6", "panel złączy J1–J6", "6 otworów wg katalogu złączy TS (Ø do weryfikacji), uszczelki IP65"),
        ("7", "slot microSD", "14 x 3 mm z gumową klapką, po prawej stronie (SD osobnym modułem)"),
        ("8", "otwory buzzera", "6 x Ø1,5 mm w górnej listwie"),
        ("9", "mocowanie tylne", "VESA 75: 4 x M5 (uchwyt/ramię do kabiny maszyny)"),
        ("10", "odpowietrznik", "M12 (membrana Gore), boczna ścianka — wyrównanie ciśnienia/kondensat"),
        ("11", "grzybek E-STOP", "Ø22 mm otwór montażowy, prawa ściana boczna, ~90 mm od góry; styk NC, IP65"),
    ]
    for i, (n, a, b) in enumerate(rows):
        yy = ty + 26 + i * 30
        badge(s, tx + 8, yy - 4, n)
        s.text(tx + 30, yy, a, 13, "#0b1f4a", weight="bold")
        s.text(tx + 230, yy, b, 12.5, "#212529")
    yy = ty + 26 + len(rows) * 30 + 12
    notes = [
        "Materiał: aluminium 3 mm (lakier proszkowy RAL 9003/7035) lub druk 3D ASA/PETG na prototyp; klawisze: silikonowe strzałki RAL 5013 na mikroprzełącznikach.",
        "Klawisze S1–S10, GRUPA i START/STOP → PCB klawiszy → MCP23017 (I2C) / GPIO sterownika. Ekran DGUS nie ma dostępu do klawiszy — tylko dotyk.",
        "Dotyk pojemnościowy przez szybę wymaga sprawdzenia (grubość/klejenie optyczne) na prototypie.",
        "Grzybek awaryjnego STOP: zamontowany w prawej ścianie bocznej obudowy (widok D2). Styk NC, wpięty w tor"
        " zasilania modułu przekaźników (tnie prąd sprzętowo) — nie jest to zwykły przycisk równoległy do STOP/pilota."
        " Status pętli na GPIO 21 — patrz ARCHITEKTURA_TERMINAL.md, rozdz. 7.",
    ]
    line = 0
    for t in notes:
        for j, part in enumerate(textwrap.wrap(t, 100)):
            s.text(tx + (0 if j == 0 else 12), yy + line * 19, ("• " if j == 0 else "") + part, 11.5, "#495057")
            line += 1

    s.save(f"obudowa_pionowa_{'os' if group == 0 else 'krawedz'}.svg")


if __name__ == "__main__":
    enclosure_vertical(0)
    enclosure_vertical(1)
