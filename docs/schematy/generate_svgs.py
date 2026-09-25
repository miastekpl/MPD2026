#!/usr/bin/env python3
"""Generator grafik SVG dokumentacji MPD2026.

Uruchomienie:  python docs/schematy/generate_svgs.py
Wyniki (w tym samym katalogu):
  schemat_polaczen.svg        - schemat elektryczny wszystkich połączeń (piny z src/config.h)
  ekran_roboczy.svg           - makieta ekranu roboczego modułu 7"
  panel_A_kabinowy.svg        - propozycja A: układ kabinowy 10+1 (jak STiM, poziomy)
  panel_B_pas_pod_ekranem.svg - propozycja B: klawisze w rzędzie pod ekranem
  panel_C_pionowy.svg         - propozycja C: pionowy panel (jak na zdjęciu STiM)
  panel_D_estop.svg           - propozycja D: duży grzybek STOP + blok klawiszy
"""
import os
from xml.sax.saxutils import escape

OUT = os.path.dirname(os.path.abspath(__file__))
FONT = "Segoe UI, Arial, Helvetica, sans-serif"


class Svg:
    def __init__(self, w, h, bg="#ffffff"):
        self.w, self.h = w, h
        self.items = [f'<rect x="0" y="0" width="{w}" height="{h}" fill="{bg}"/>']

    def add(self, s):
        self.items.append(s)

    def rect(self, x, y, w, h, fill="none", stroke="none", sw=1, rx=0, extra=""):
        self.add(f'<rect x="{x}" y="{y}" width="{w}" height="{h}" rx="{rx}" fill="{fill}" '
                 f'stroke="{stroke}" stroke-width="{sw}" {extra}/>')

    def text(self, x, y, s, size=14, fill="#111", anchor="start", weight="normal", extra=""):
        ff = "" if "font-family" in extra else f'font-family="{FONT}" '
        self.add(f'<text x="{x}" y="{y}" {ff}font-size="{size}" fill="{fill}" '
                 f'text-anchor="{anchor}" font-weight="{weight}" {extra}>{escape(s)}</text>')

    def line(self, x1, y1, x2, y2, stroke="#333", sw=1.5, dash=None):
        d = f' stroke-dasharray="{dash}"' if dash else ""
        self.add(f'<line x1="{x1}" y1="{y1}" x2="{x2}" y2="{y2}" stroke="{stroke}" stroke-width="{sw}"{d}/>')

    def circle(self, x, y, r, fill="#000", stroke="none", sw=1):
        self.add(f'<circle cx="{x}" cy="{y}" r="{r}" fill="{fill}" stroke="{stroke}" stroke-width="{sw}"/>')

    def polygon(self, pts, fill, stroke="none", sw=1, opacity=1):
        p = " ".join(f"{a},{b}" for a, b in pts)
        self.add(f'<polygon points="{p}" fill="{fill}" stroke="{stroke}" stroke-width="{sw}" opacity="{opacity}"/>')

    def group_open(self, transform):
        self.add(f'<g transform="{transform}">')

    def group_close(self):
        self.add("</g>")

    def save(self, name):
        body = "\n".join(self.items)
        svg = (f'<svg xmlns="http://www.w3.org/2000/svg" width="{self.w}" height="{self.h}" '
               f'viewBox="0 0 {self.w} {self.h}">\n{body}\n</svg>\n')
        with open(os.path.join(OUT, name), "w", encoding="utf-8") as f:
            f.write(svg)
        print("zapisano", name)


# ---------------------------------------------------------------------------
# 1. Schemat elektryczny
# ---------------------------------------------------------------------------
def wiring():
    W, H = 2100, 1400
    s = Svg(W, H, "#ffffff")
    s.text(40, 46, "MPD2026 — schemat połączeń elektronicznych", 30, "#0b1f4a", weight="bold")
    s.text(40, 74, "Sterownik Trassar (ESP32-S3 N16R8, firmware 2.52.0) + moduł wyświetlacza 7\" (Sunton ESP32-8048S070C)."
                   " Numery = GPIO ESP32-S3 (zgodnie z src/config.h).", 15, "#444")

    # --- zasilanie ---
    s.rect(40, 100, 200, 60, "#fff3cd", "#b8860b", 2, 6)
    s.text(140, 126, "Akumulator / instalacja", 14, anchor="middle", weight="bold")
    s.text(140, 146, "maszyny 12 V / 24 V DC", 14, anchor="middle")
    s.line(240, 130, 300, 130, "#b8860b", 3)
    s.rect(300, 100, 200, 60, "#fff3cd", "#b8860b", 2, 6)
    s.text(400, 126, "Przetwornica DC-DC", 14, anchor="middle", weight="bold")
    s.text(400, 146, "12/24 V -> 5 V, min. 3 A", 14, anchor="middle")
    s.line(500, 130, 560, 130, "#c1121f", 3)
    s.rect(560, 100, 170, 60, "#fde2e4", "#c1121f", 2, 6)
    s.text(645, 126, "J1 TS13CP03", 14, anchor="middle", weight="bold")
    s.text(645, 146, "+5 V / GND / GND", 13, anchor="middle")
    # szyna +5V
    s.line(730, 130, 1980, 130, "#c1121f", 3)
    s.text(750, 122, "+5 V (PTC 1,5 A + TVS)", 13, "#c1121f", weight="bold")
    # szyna 5V do modułu 7"
    s.line(1980, 130, 1980, 1105, "#c1121f", 3)
    s.text(1968, 640, "osobne odgałęzienie 5 V (PTC >= 1 A)", 13, "#c1121f", "middle", extra='transform="rotate(-90 1968 640)"')
    s.text(1900, 122, "+5 V", 13, "#c1121f", weight="bold")

    # --- ESP32 ---
    EX, EY, EW, EH = 800, 190, 460, 940
    s.rect(EX, EY, EW, EH, "#e8f0fe", "#0b1f4a", 3, 10)
    s.text(EX + EW / 2, EY + 32, "ESP32-S3 N16R8", 20, "#0b1f4a", "middle", "bold")
    s.text(EX + EW / 2, EY + 54, "DevKitC-1 — sterownik Trassar", 14, "#0b1f4a", "middle")
    # 5V do ESP
    s.line(EX + EW / 2, 130, EX + EW / 2, EY, "#c1121f", 3)
    s.text(EX + EW / 2 + 8, 170, "5 V (VBUS)", 12, "#c1121f")
    s.text(EX + EW / 2, EY + EH - 22, "3V3 (regulator) -> TFT, SD, GPS, MCP23017, joystick, enkoder    GND: masa wspólna", 12, "#333", "middle")

    pitch = 28

    def pin_left(y, gpio, label):
        s.line(EX - 6, y, EX, y, "#0b1f4a", 2)
        s.circle(EX, y, 4, "#0b1f4a")
        s.text(EX + 12, y + 5, f"GPIO {gpio}", 13, "#0b1f4a", weight="bold")
        s.text(EX + 88, y + 5, label, 12, "#555")

    def pin_right(y, gpio, label):
        s.circle(EX + EW, y, 4, "#0b1f4a")
        s.text(EX + EW - 12, y + 5, f"GPIO {gpio}", 13, "#0b1f4a", "end", "bold")
        s.text(EX + EW - 88, y + 5, label, 12, "#555", "end")

    def module(x, y, w, h, title, sub=None, fill="#f1f8e9", stroke="#33691e"):
        s.rect(x, y, w, h, fill, stroke, 2, 8)
        s.text(x + w / 2, y + 22, title, 15, "#111", "middle", "bold")
        if sub:
            s.text(x + w / 2, y + 40, sub, 12, "#444", "middle")

    def wire(x1, y, x2, color="#333", label=None):
        s.line(x1, y, x2, y, color, 1.8)
        if label:
            s.text((x1 + x2) / 2, y - 4, label, 11, "#666", "middle")

    # ---------- lewa strona ----------
    LX, LW = 120, 380
    y = EY + 90
    # TFT + SD (SPI)
    tft_rows = [("10", "CS", "TFT CS"), ("9", "DC", "TFT DC"), ("14", "RST", "TFT RST"),
                ("11", "MOSI", "SPI MOSI (TFT+SD+Touch)"), ("12", "SCK", "SPI SCK"),
                ("13", "MISO", "SPI MISO"), ("21", "BL", "podświetlenie PWM"),
                ("15", "T_CS", "Touch CS / DS18B20"), ("16", "SD_CS", "karta SD CS")]
    top = y - 20
    module(LX, top, LW, len(tft_rows) * pitch + 50, "ILI9341 2,8\" TFT + karta SD",
           "SPI 27 MHz, zasilanie 3V3, kabel ekranowany <= 20 cm")
    for i, (g, port, lab) in enumerate(tft_rows):
        yy = y + 30 + i * pitch
        s.text(LX + LW - 10, yy + 5, port, 12, "#111", "end", "bold")
        wire(LX + LW, yy, EX, "#6a1b9a")
        pin_left(yy, g, lab)
    y = y + 30 + len(tft_rows) * pitch + 40

    # I2C
    top = y - 20
    module(LX, top, LW, 2 * pitch + 78, "Magistrala I2C", "DS1307 (0x68, 5 V) + MCP23017 (0x20, 3V3)",
           "#e3f2fd", "#0d47a1")
    for i, (g, port, lab) in enumerate([("17", "SDA", "I2C SDA"), ("18", "SCL", "I2C SCL")]):
        yy = y + 30 + i * pitch
        s.text(LX + LW - 10, yy + 5, port, 12, "#111", "end", "bold")
        wire(LX + LW, yy, EX, "#0d47a1")
        pin_left(yy, g, lab)
    y = y + 30 + 2 * pitch + 44

    # GPS
    top = y - 20
    module(LX, top, LW, 2 * pitch + 50, "GPS GY-NEO6MV2 (NEO-6M)", "UART2 9600 baud, zasilanie 3V3",
           "#fff8e1", "#e65100")
    for i, (g, port, lab) in enumerate([("47", "TX", "RX <- GPS TX"), ("48", "RX", "TX -> GPS RX")]):
        yy = y + 30 + i * pitch
        s.text(LX + LW - 10, yy + 5, port, 12, "#111", "end", "bold")
        wire(LX + LW, yy, EX, "#e65100")
        pin_left(yy, g, lab)
    y = y + 30 + 2 * pitch + 24

    # Przyciski
    top = y - 20
    module(LX, top, LW, 3 * pitch + 78, "Przyciski panelu BS-33B",
           "styk -> GND, pull-up wewn.; równolegle: J4 pilot, J5 pedał", "#fce4ec", "#880e4f")
    for i, (g, port, lab) in enumerate([("38", "START", "START"), ("39", "STOP", "STOP (+ISR awaryjny)"),
                                        ("40", "SELEKTOR", "SELEKTOR")]):
        yy = y + 30 + i * pitch
        s.text(LX + LW - 10, yy + 5, port, 12, "#111", "end", "bold")
        wire(LX + LW, yy, EX, "#880e4f")
        pin_left(yy, g, lab)
    y = y + 30 + 3 * pitch + 44

    # Joystick
    top = y - 20
    module(LX, top, LW, 3 * pitch + 50, "Joystick KY-023", "zasilanie 3V3, kabel ekranowany",
           "#fce4ec", "#880e4f")
    for i, (g, port, lab) in enumerate([("19", "VRx", "ADC2 oś X"), ("20", "VRy", "ADC2 oś Y"),
                                        ("46", "SW", "przycisk (STRAP!)")]):
        yy = y + 30 + i * pitch
        s.text(LX + LW - 10, yy + 5, port, 12, "#111", "end", "bold")
        wire(LX + LW, yy, EX, "#880e4f")
        pin_left(yy, g, lab)
    y_left_end = y + 30 + 3 * pitch + 20

    # ---------- prawa strona ----------
    RX, RW = 1400, 300
    y = EY + 90
    top = y - 20
    relay_rows = [("41", "IN1", "P1 oś lewy 12 cm"), ("42", "IN2", "P2 oś środek 12 cm"),
                  ("1", "IN3", "P3 oś prawy 12 cm"), ("2", "IN4", "P4 oś szeroki 24 cm"),
                  ("3", "IN5", "P5 krawędź 12 cm"), ("4", "IN6", "P6 krawędź 24 cm")]
    module(RX, top, RW, 6 * pitch + 140, "Moduł przekaźników 6 kanałów",
           "5 V, opto-izolacja, wejścia HIGH = ON", "#e8f5e9", "#1b5e20")
    for i, (g, port, lab) in enumerate(relay_rows):
        yy = y + 30 + i * pitch
        s.text(RX + 10, yy + 5, port, 12, "#111", "start", "bold")
        wire(EX + EW, yy, RX, "#1b5e20")
        pin_right(yy, g, lab)
        # wyjście NO -> J2 -> zawór
        s.text(RX + RW - 10, yy + 5, f"NO{i + 1}", 12, "#111", "end", "bold")
        wire(RX + RW, yy, 1770, "#e65100")
        s.rect(1770, yy - 11, 90, 22, "#fff3e0", "#e65100", 1.5, 4)
        s.text(1815, yy + 4, f"J2/{i + 1} -> P{i + 1}", 11, "#111", "middle")
        wire(1860, yy, 1900, "#e65100")
        s.rect(1900, yy - 11, 70, 22, "#ffe0b2", "#e65100", 1.5, 4)
        s.text(1935, yy + 4, f"zawór P{i + 1}", 11, "#111", "middle")
    yb = y + 30 + 6 * pitch + 6
    s.text(RX + RW / 2, yb + 12, "COM przekaźników (J2/7, J2/8) <- +12/24 V instalacji maszyny", 11, "#b71c1c", "middle", "bold")
    s.text(RX + RW / 2, yb + 28, "masa zaworów = minus instalacji (NIE do GND ESP32)", 11, "#b71c1c", "middle")
    y = yb + 120

    # Enkoder
    top = y - 20
    module(RX, top, RW, 3 * pitch + 78, "Enkoder KY-040 (złącze J3)", "skrętka CLK+DT, 100 nF do GND, <= 2 m",
           "#e0f7fa", "#006064")
    for i, (g, port, lab) in enumerate([("5", "CLK", "CLK (A), ISR"), ("6", "DT", "DT (B), ISR"),
                                        ("7", "SW", "SW = GAP (start od przerwy)")]):
        yy = y + 30 + i * pitch
        s.text(RX + 10, yy + 5, port, 12, "#111", "start", "bold")
        wire(EX + EW, yy, RX, "#006064")
        pin_right(yy, g, lab)
    s.text(RX + RW / 2, y + 30 + 3 * pitch + 14, "GPIO 7 równolegle: J4/4 (pilot GAP)", 11, "#006064", "middle")
    y = y + 30 + 3 * pitch + 60

    # Buzzer
    top = y - 20
    module(RX, top, RW, pitch + 66, "Buzzer pasywny 5 V", "LEDC PWM 100 Hz - 5 kHz", "#f3e5f5", "#4a148c")
    yy = y + 30
    s.text(RX + 10, yy + 5, "(+)", 12, "#111", "start", "bold")
    wire(EX + EW, yy, RX, "#4a148c")
    pin_right(yy, "8", "buzzer (PWM)")
    y = y + 30 + pitch + 50

    # WiFi do modułu 7"
    top = y - 20
    s.rect(RX, top, RW, 120, "#ede7f6", "#311b92", 2.5, 10)
    s.text(RX + RW / 2, top + 30, "Moduł wyświetlacza 7\"", 16, "#111", "middle", "bold")
    s.text(RX + RW / 2, top + 52, "Sunton ESP32-8048S070C", 13, "#333", "middle")
    s.text(RX + RW / 2, top + 74, "800x480 RGB + dotyk GT911", 13, "#333", "middle")
    s.text(RX + RW / 2, top + 98, "zasilanie: 5 V (bez łącza sygnałowego)", 12, "#c1121f", "middle")
    ywifi = EY + EH - 120
    s.add(f'<path d="M {EX + EW} {ywifi} C {EX + EW + 120} {ywifi}, {RX - 120} {top + 60}, {RX} {top + 60}" '
          f'fill="none" stroke="#311b92" stroke-width="3" stroke-dasharray="10 6"/>')
    s.circle(EX + EW, ywifi, 4, "#311b92")
    s.text(EX + EW - 12, ywifi + 5, "WiFi AP 'TrassarV3' (kanał 6)", 12, "#311b92", "end", "bold")
    s.text(1250, ywifi - 12, "WiFi 2,4 GHz: WebSocket :81 (status), HTTP :80 (polecenia)", 12, "#311b92", "middle")

    # ---------- klawisze soft-key (MCP23017) ----------
    ky = max(y_left_end, EY + EH) + 30
    s.rect(120, ky, 1100, 250, "#fffde7", "#f57f17", 2, 10)
    s.text(140, ky + 28, "Przyciski wzorców przy ekranie 7\" — MCP23017 (I2C 0x20, SDA=GPIO 17, SCL=GPIO 18): układ soft-key 10 + GRUPA",
           15, "#111", weight="bold")
    keys = [("S1", "GPA0", "P-1a | P-6"), ("S2", "GPA1", "P-1b | P-7a"), ("S3", "GPA2", "P-1c | P-7b"),
            ("S4", "GPA3", "P-1d | P-7c"), ("S5", "GPA4", "P-1e | P-7d"), ("S6", "GPA5", "P-2a | WŁASNY"),
            ("S7", "GPA6", "P-2b | -"), ("S8", "GPA7", "P-3a | -"), ("S9", "GPB0", "P-3b | -"),
            ("S10", "GPB1", "P-4 | -"), ("GRUPA", "GPB2", "OŚ <-> KRAWĘDŹ")]
    bx = 140
    for i, (k, pin, lab) in enumerate(keys):
        x = bx + i * 96
        s.rect(x, ky + 56, 86, 40, "#fff", "#f57f17", 1.5, 6)
        s.text(x + 43, ky + 80, k, 14, "#111", "middle", "bold")
        s.line(x + 43, ky + 96, x + 43, ky + 130, "#f57f17", 2)
        s.text(x + 43, ky + 148, pin, 12, "#111", "middle", "bold")
        s.text(x + 43, ky + 166, lab, 10, "#555", "middle")
    s.line(bx + 43, ky + 190, bx + 43 + 10 * 96, ky + 190, "#000", 3)
    s.text(bx, ky + 212, "GND (wspólna masa przycisków); pull-up włączane programowo; układ klasyczny 15 przycisków: patrz SCHEMAT_PODLACZEN.md 4.3",
           12, "#333")
    s.text(bx, ky + 232, "Podpis 'A | B' = wzorzec w grupie OŚ | grupie KRAWĘDŹ.  Nieużywane w soft-key: GPB3-GPB6, GPB7.", 12, "#333")

    # legenda
    ly = ky + 270
    s.text(120, ly, "Legenda:", 14, weight="bold")
    for i, (c, t) in enumerate([("#c1121f", "zasilanie 5 V"), ("#6a1b9a", "SPI"), ("#0d47a1", "I2C"),
                                ("#e65100", "UART GPS / wyjścia mocy do zaworów"), ("#1b5e20", "sygnały do przekaźników"),
                                ("#311b92", "WiFi (dashed)")]):
        s.line(200 + i * 250, ly - 5, 240 + i * 250, ly - 5, c, 3)
        s.text(246 + i * 250, ly, t, 12, "#333")
    s.text(120, ly + 30, "Uwagi: GPIO 26-37 zajęte (Flash + Octal PSRAM), GPIO 46 to pin rozruchowy (nie wciskać joysticka przy starcie).",
           12, "#b71c1c")
    s.text(120, ly + 50, "Złącza maszynowe: J1 zasilanie 5 V, J2 zawory P1-P6, J3 enkoder, J4 pilot (START/SEL/STOP/GAP), J5 pedał (START+STOP).", 12, "#333")
    s.save("schemat_polaczen.svg")


# ---------------------------------------------------------------------------
# 2. Makieta ekranu roboczego (800x480, wspólna dla paneli)
# ---------------------------------------------------------------------------
def ui_mock(s, x, y, scale, group=0):
    s.group_open(f"translate({x},{y}) scale({scale})")
    s.rect(0, 0, 800, 480, "#07142e", "#000", 0)
    # pasek górny
    s.text(12, 34, "POŁĄCZONO", 20, "#35f27a", weight="bold")
    s.text(212, 34, "GPS 8 sat", 20, "#35f27a", weight="bold")
    s.rect(350, 12, 110, 30, "#0f2557", rx=6)
    s.rect(350, 12, 82, 30, "#1fa34a", rx=6)
    s.text(405, 33, "FARBA 72%", 14, "#fff", "middle")
    s.rect(470, 6, 78, 46, "#2f86ff", "#ffd400", 5, 10)
    s.text(509, 36, "OŚ", 20, "#fff", "middle", "bold")
    s.rect(552, 6, 96, 46, "#1f52d6", rx=10)
    s.text(600, 34, "KRAWĘDŹ", 15, "#fff", "middle", "bold")
    s.rect(654, 6, 138, 46, "#1f52d6", rx=10)
    s.text(723, 36, "MENU", 20, "#fff", "middle", "bold")
    left = ["P-1a", "P-1b", "P-1c", "P-1d", "P-1e"]
    right = ["P-2a", "P-2b", "P-3a", "P-3b", "P-4"]
    if group == 1:
        left = ["P-6", "P-7a", "P-7b", "P-7c", "P-7d"]
        right = ["WŁASNY", "", "", "", ""]
    sel = "P-3a" if group == 0 else "P-7b"
    for col, names, x0 in ((0, left, 6), (1, right, 656)):
        for i, n in enumerate(names):
            yy = 62 + i * 66
            if not n:
                s.rect(x0, yy, 138, 62, "#27314d", "#3b4a73", 2, 10)
                continue
            is_sel = (n == sel)
            s.rect(x0, yy, 138, 62, "#2f86ff" if is_sel else "#1f52d6", "#ffd400" if is_sel else "#5b7fe0",
                   5 if is_sel else 2, 10)
            s.rect(x0 + 6, yy + 7, 40, 48, "#1b1b1f", rx=3)
            s.rect(x0 + 24, yy + 10, 4, 42, "#ffd400")
            s.text(x0 + 132, yy + 41, n, 26 if len(n) < 6 else 16, "#fff", "end", "bold")
    # prędkość
    s.text(156, 134, "12.5", 84, "#fff", weight="bold", extra='font-family="Consolas, monospace"')
    s.text(364, 130, "km/h", 28, "#8fa3cc")
    s.text(650, 104, sel, 44, "#ffd400", "end", "bold")
    s.text(650, 130, "AUTO  MALOWANIE", 20, "#35f27a", "end")
    s.text(156, 164, "DYST 1234.5 m    POW 148.1 m2    CZAS 12:34", 20, "#fff")
    # droga
    s.rect(150, 176, 500, 176, "#143a7a", rx=6)
    s.polygon([(230, 350), (570, 350), (450, 180), (350, 180)], "#1b1b1f")
    for i, (yy, hh) in enumerate([(320, 22), (270, 16), (230, 11), (205, 7)]):
        w = 10 - i * 2
        s.polygon([(400 - w, yy + hh), (400 + w, yy + hh), (400 + w - 1, yy), (400 - w + 1, yy)],
                  "#ffd400" if i else "#35f27a")
    s.rect(240, 346, 320, 4, "#fff")
    # pistolety
    for g in range(6):
        on = g in ((0, 2) if group == 0 else (5,))
        s.rect(150 + g * 84, 356, 78, 32, "#35f27a" if on else "#0f2557", "#ffd400" if g in (0, 2) else "#0f2557", 3, 16)
        s.text(150 + g * 84 + 39, 379, f"P{g + 1}", 18, "#03210f" if on else "#fff", "middle")
    # pasek dolny
    for i, (t, c) in enumerate([("AUTO", "#2f86ff"), ("SEMI", "#1f52d6"), ("RĘCZNY", "#1f52d6")]):
        s.rect(6 + i * 108, 394, 104, 80, c, "#ffd400" if i == 0 else "#5b7fe0", 5 if i == 0 else 2, 10)
        s.text(58 + i * 108, 442, t, 20, "#fff", "middle", "bold")
    s.rect(336, 394, 214, 80, "#1fa34a", rx=10)
    s.text(443, 445, "▶ START", 28, "#fff", "middle", "bold")
    s.rect(556, 394, 132, 80, "#d62828", rx=10)
    s.text(622, 445, "■ STOP", 28, "#fff", "middle", "bold")
    s.rect(694, 394, 100, 80, "#1f52d6", rx=10)
    s.text(744, 428, "START OD", 15, "#fff", "middle")
    s.text(744, 448, "PRZERWY", 15, "#fff", "middle")
    s.group_close()


def screen_mock():
    s = Svg(880, 560, "#eef1f6")
    s.text(40, 40, "Ekran roboczy modułu 7\" — makieta (800x480)", 22, "#0b1f4a", weight="bold")
    s.text(40, 62, "Strona OŚ: S1-S5 lewa kolumna, S6-S10 prawa kolumna; zakładki OŚ / KRAWĘDŹ w górnym pasku", 14, "#444")
    ui_mock(s, 40, 80, 1.0, 0)
    s.save("ekran_roboczy.svg")


# ---------------------------------------------------------------------------
# 3. Propozycje wizualizacji panelu
# ---------------------------------------------------------------------------
def key(s, x, y, w, h, label, sub=None, fill="#2a3a6a", txt="#fff", stroke="#8fa3cc", rx=10):
    s.rect(x, y, w, h, fill, stroke, 2, rx)
    s.text(x + w / 2, y + h / 2 + (2 if sub else 6), label, 15, txt, "middle", "bold")
    if sub:
        s.text(x + w / 2, y + h / 2 + 18, sub, 10, txt, "middle")


def panel_frame(s, x, y, w, h, title):
    s.rect(x, y, w, h, "#e9ecef", "#495057", 4, 26)
    s.rect(x + 10, y + 10, w - 20, h - 20, "#dee2e6", "#adb5bd", 1.5, 20)
    s.text(x + 28, y + 36, title, 16, "#212529", weight="bold")


def screen_bezel(s, x, y, w, h):
    s.rect(x - 8, y - 8, w + 16, h + 16, "#111", "#000", 2, 8)


def panel_A():
    W, H = 1100, 760
    s = Svg(W, H, "#f8f9fa")
    s.text(30, 38, "Propozycja A — układ kabinowy 10 + 1 (poziomy, jak STiM)", 22, "#0b1f4a", weight="bold")
    s.text(30, 62, "Ekran 7\" w środku, S1-S5 po lewej i S6-S10 po prawej (etykiety obok na ekranie), GRUPA w narożniku, START/STOP na dole.",
           14, "#444")
    px, py, pw, ph = 40, 90, 1020, 640
    panel_frame(s, px, py, pw, ph, "TRASSAR  MPD2026")
    sx, sy = px + 190, py + 80
    sw, sh = 640, 384      # 800x480 * 0.8
    screen_bezel(s, sx, sy, sw, sh)
    ui_mock(s, sx, sy, 0.8, 0)
    # klawisze boczne
    for i in range(5):
        yy = sy + 4 + i * 76
        key(s, px + 60, yy, 110, 62, f"S{i + 1}", "◄")
        key(s, px + pw - 170, yy, 110, 62, f"S{i + 6}", "►")
    key(s, px + pw - 170, py + 22, 110, 44, "GRUPA", "OŚ ⇄ KRAWĘDŹ", "#7a4b00", "#fff", "#ffd400")
    # dolny rząd
    by = sy + sh + 40
    key(s, px + 60, by, 110, 66, "SELEKTOR", None)
    key(s, px + 190, by, 110, 66, "GAP", "od przerwy")
    s.rect(px + 320, by, 200, 66, "#1fa34a", "#0b6b2e", 3, 12)
    s.text(px + 420, by + 42, "START", 24, "#fff", "middle", "bold")
    s.rect(px + 540, by, 200, 66, "#d62828", "#7a0f0f", 3, 12)
    s.text(px + 640, by + 42, "STOP", 24, "#fff", "middle", "bold")
    s.circle(px + 870, by + 33, 30, "#343a40", "#adb5bd", 3)
    s.text(px + 870, by + 39, "JOY", 14, "#fff", "middle", "bold")
    s.text(px + 870, by + 82, "joystick KY-023 (opcjonalny)", 11, "#495057", "middle")
    s.text(px + 30, py + ph - 24, "Przyciski fizyczne: 10 (S1-S10) + GRUPA + START + STOP + SELEKTOR + GAP = 15 (było 19).  "
                                  "Fizyczne START/STOP/SELEKTOR/GAP zostają (rękawice, deszcz, tryb RĘCZNY).", 12, "#495057")
    s.save("panel_A_kabinowy.svg")


def panel_B():
    W, H = 1100, 690
    s = Svg(W, H, "#f8f9fa")
    s.text(30, 38, "Propozycja B — klawisze w rzędzie pod ekranem (niska obudowa)", 22, "#0b1f4a", weight="bold")
    s.text(30, 62, "Niski pulpit do montażu przed operatorem. Etykiety wzorców na dolnej krawędzi ekranu nad klawiszami (wymaga wariantu UI).",
           14, "#444")
    px, py, pw, ph = 40, 90, 1020, 570
    panel_frame(s, px, py, pw, ph, "TRASSAR  MPD2026")
    sx, sy = px + 190, py + 56
    sw, sh = 640, 384
    screen_bezel(s, sx, sy, sw, sh)
    ui_mock(s, sx, sy, 0.8, 0)
    # rząd 10 klawiszy pod ekranem
    ky = sy + sh + 24
    kw = 60
    total = 10 * kw + 9 * 6
    kx = sx + (sw - total) / 2
    for i in range(10):
        key(s, kx + i * (kw + 6), ky, kw, 40, f"S{i + 1}", None)
    # boki
    key(s, px + 40, sy, 120, 70, "GRUPA", "OŚ ⇄ KRAWĘDŹ", "#7a4b00", "#fff", "#ffd400")
    key(s, px + 40, sy + 100, 120, 60, "SELEKTOR", None)
    key(s, px + 40, sy + 180, 120, 60, "GAP", "od przerwy")
    s.rect(px + pw - 160, sy, 120, 120, "#1fa34a", "#0b6b2e", 3, 14)
    s.text(px + pw - 100, sy + 68, "START", 24, "#fff", "middle", "bold")
    s.rect(px + pw - 160, sy + 150, 120, 120, "#d62828", "#7a0f0f", 3, 14)
    s.text(px + pw - 100, sy + 218, "STOP", 24, "#fff", "middle", "bold")
    s.text(px + 30, py + ph - 20, "Zaleta: niska, szeroka obudowa. Wada: klawisze poza osią ekranu — etykiety trzeba rysować wzdłuż dolnej krawędzi.",
           12, "#495057")
    s.save("panel_B_pas_pod_ekranem.svg")


def panel_C():
    W, H = 760, 1120
    s = Svg(W, H, "#f8f9fa")
    s.text(30, 38, "Propozycja C — panel pionowy (jak na zdjęciu STiM)", 22, "#0b1f4a", weight="bold")
    s.text(30, 62, "Ekran obrócony do pionu (480x800). Wymaga trybu portretowego UI (LVGL) — do wykonania.", 14, "#444")
    px, py, pw, ph = 40, 90, 680, 980
    panel_frame(s, px, py, pw, ph, "TRASSAR  MPD2026")
    # ekran pionowy 480x800 -> 0.6
    sw, sh = 288, 480
    sx, sy = px + (pw - sw) / 2, py + 70
    screen_bezel(s, sx, sy, sw, sh)
    s.rect(sx, sy, sw, sh, "#07142e")
    s.text(sx + sw / 2, sy + 30, "12.5 km/h", 30, "#fff", "middle", "bold", 'font-family="Consolas, monospace"')
    s.text(sx + sw / 2, sy + 58, "P-3a  AUTO  MALOWANIE", 13, "#35f27a", "middle")
    s.polygon([(sx + 40, sy + 470), (sx + sw - 40, sy + 470), (sx + sw - 90, sy + 200), (sx + 90, sy + 200)], "#1b1b1f")
    for i, (yy, hh, w) in enumerate([(430, 34, 8), (370, 24, 6), (320, 16, 5), (280, 11, 4), (250, 7, 3)]):
        s.polygon([(sx + sw / 2 - w, yy + hh), (sx + sw / 2 + w, yy + hh), (sx + sw / 2 + w - 1, yy), (sx + sw / 2 - w + 1, yy)],
                  "#ffd400")
    s.text(sx + sw / 2, sy + 98, "OŚ   |   KRAWĘDŹ", 14, "#8fa3cc", "middle", "bold")
    for i in range(5):
        yy = sy + 6 + i * 96
        key(s, px + 40, yy, 92, 76, f"S{i + 1}", "◄")
        key(s, px + pw - 132, yy, 92, 76, f"S{i + 6}", "►")
    # dolny blok
    by = sy + sh + 50
    key(s, px + 40, by, 130, 70, "GRUPA", "OŚ ⇄ KRAWĘDŹ", "#7a4b00", "#fff", "#ffd400")
    key(s, px + 190, by, 130, 70, "SELEKTOR", None)
    key(s, px + 340, by, 130, 70, "GAP", "od przerwy")
    s.rect(px + 60, by + 100, 270, 110, "#1fa34a", "#0b6b2e", 3, 16)
    s.text(px + 195, by + 168, "START", 30, "#fff", "middle", "bold")
    s.rect(px + 350, by + 100, 270, 110, "#d62828", "#7a0f0f", 3, 16)
    s.text(px + 485, by + 168, "STOP", 30, "#fff", "middle", "bold")
    s.text(px + 30, py + ph - 20, "Zaleta: kształt znany operatorom STiM, wąska obudowa. Wada: wymaga przeróbki UI na tryb pionowy.", 12, "#495057")
    s.save("panel_C_pionowy.svg")


def panel_D():
    W, H = 1100, 760
    s = Svg(W, H, "#f8f9fa")
    s.text(30, 38, "Propozycja D — duży grzybek awaryjnego STOP + blok klawiszy 2x5", 22, "#0b1f4a", weight="bold")
    s.text(30, 62, "Fizyczny STOP jako czerwony grzybek (styk NO równolegle do przycisku STOP, GPIO 39). Klawisze wzorców w bloku po prawej.",
           14, "#444")
    px, py, pw, ph = 40, 90, 1020, 640
    panel_frame(s, px, py, pw, ph, "TRASSAR  MPD2026")
    sx, sy = px + 190, py + 60
    sw, sh = 560, 336
    screen_bezel(s, sx, sy, sw, sh)
    ui_mock(s, sx, sy, 0.7, 1)
    # grzybek
    cx, cy = px + 100, py + 180
    s.circle(cx, cy, 70, "#7a0f0f", "#450a0a", 4)
    s.circle(cx, cy, 58, "#d62828", "#a01010", 3)
    s.text(cx, cy + 8, "STOP", 26, "#fff", "middle", "bold")
    s.text(cx, cy + 96, "grzybek awaryjny", 12, "#495057", "middle")
    # blok 2x5 po prawej
    for i in range(5):
        key(s, px + pw - 220, sy + 4 + i * 67, 94, 56, f"S{i + 1}", None)
        key(s, px + pw - 116, sy + 4 + i * 67, 94, 56, f"S{i + 6}", None)
    key(s, px + 40, py + 320, 130, 62, "GRUPA", "OŚ ⇄ KRAWĘDŹ", "#7a4b00", "#fff", "#ffd400")
    by = sy + sh + 40
    key(s, px + 40, by, 110, 66, "SELEKTOR", None)
    key(s, px + 170, by, 110, 66, "GAP", "od przerwy")
    s.rect(px + 330, by, 330, 66, "#1fa34a", "#0b6b2e", 3, 12)
    s.text(px + 495, by + 42, "START", 26, "#fff", "middle", "bold")
    s.text(px + 30, py + ph - 20, "Zaleta: najlepsza widoczność i dostępność STOP, klawisze wzorców zgrupowane. "
                                  "Wada: większa obudowa, więcej otworów (grzybek).", 12, "#495057")
    s.save("panel_D_estop.svg")


if __name__ == "__main__":
    wiring()
    screen_mock()
    panel_A()
    panel_B()
    panel_C()
    panel_D()
