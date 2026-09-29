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
    W, H = 2100, 1560
    s = Svg(W, H, "#ffffff")
    s.text(40, 46, "MPD2026 — schemat połączeń elektronicznych", 30, "#0b1f4a", weight="bold")
    s.text(40, 74, "Sterownik Trassar (ESP32-S3 N16R8, firmware 2.53.0) + moduł wyświetlacza 7\" (Sunton ESP32-8048S070C)."
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
    module(LX, top, LW, len(tft_rows) * pitch + 50, "ILI9341 2,8\" (ekran serwisowy) + karta SD",
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
# 1b. Schemat elektryczny — wariant DOCELOWY (ekran DWIN DGUS, bez ILI9341/joysticka)
# ---------------------------------------------------------------------------
def wiring_dgus():
    W, H = 2100, 1900
    s = Svg(W, H, "#ffffff")
    s.text(40, 46, "MPD2026 — schemat połączeń, ALTERNATYWA (ekran DWIN DGUS, wymaga DGUS Designer)", 30, "#0b1f4a", weight="bold")
    s.text(40, 74, "Sterownik Trassar (ESP32-S3 N16R8, firmware 2.53.0, środowisko esp32s3_terminal) + wyświetlacz"
                   " inteligentny DWIN DGUS (np. DMG10600T070_09WTC, 7\" 1024x600) na łączu UART, bez ILI9341."
                   " Joystick zostaje (nawigacja menu). NIE ZALECANE jako punkt startowy — wymaga ręcznej pracy"
                   " w DGUS Designer (patrz docs/EKRAN_DGUS.md). Wariant docelowy: schemat_polaczen_docelowy.svg.", 15, "#444")
    s.text(40, 94, "Kod skompilowany i sprawdzony; POŁĄCZENIA nie były jeszcze zweryfikowane na fizycznym sprzęcie — sprawdź to na prototypie.",
           13, "#b71c1c", weight="bold")

    # --- zasilanie ---
    s.rect(40, 116, 200, 60, "#fff3cd", "#b8860b", 2, 6)
    s.text(140, 142, "Akumulator / instalacja", 14, anchor="middle", weight="bold")
    s.text(140, 162, "maszyny 12 V / 24 V DC", 14, anchor="middle")
    s.line(240, 146, 300, 146, "#b8860b", 3)
    s.rect(300, 116, 200, 60, "#fff3cd", "#b8860b", 2, 6)
    s.text(400, 142, "Przetwornica DC-DC", 14, anchor="middle", weight="bold")
    s.text(400, 162, "12/24 V -> 5 V, min. 3 A", 14, anchor="middle")
    s.line(500, 146, 560, 146, "#c1121f", 3)
    s.rect(560, 116, 170, 60, "#fde2e4", "#c1121f", 2, 6)
    s.text(645, 142, "J1 TS13CP03", 14, anchor="middle", weight="bold")
    s.text(645, 162, "+5 V / GND / GND", 13, anchor="middle")
    s.line(730, 146, 1980, 146, "#c1121f", 3)
    s.text(750, 138, "+5 V (PTC 1,5 A + TVS)", 13, "#c1121f", weight="bold")
    s.line(1980, 146, 1980, 700, "#c1121f", 3)
    s.text(1900, 138, "+5 V", 13, "#c1121f", weight="bold")

    EX, EY, EW, EH = 800, 200, 460, 860
    s.rect(EX, EY, EW, EH, "#e8f0fe", "#0b1f4a", 3, 10)
    s.text(EX + EW / 2, EY + 32, "ESP32-S3 N16R8", 20, "#0b1f4a", "middle", "bold")
    s.text(EX + EW / 2, EY + 54, "DevKitC-1 — sterownik Trassar (esp32s3_terminal)", 13, "#0b1f4a", "middle")
    s.line(EX + EW / 2, 146, EX + EW / 2, EY, "#c1121f", 3)
    s.text(EX + EW / 2 + 8, 180, "5 V (VBUS)", 12, "#c1121f")
    s.text(EX + EW / 2, EY + EH - 18, "3V3: SD, GPS, MCP23017, enkoder, poziomowanie UART    GND: masa wspólna", 12, "#333", "middle")

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
    LX, LW = 120, 420
    y = EY + 90

    # Ekran DGUS (UART, z dzielnikiem na linii RX)
    top = y - 20
    module(LX, top, LW, 2 * pitch + 130, "Wyświetlacz DGUS (np. DMG10600T070_09WTC)",
           "UART 115200 8N1, 5 V; TX ekranu -> dzielnik -> RX ESP32", "#ede7f6", "#311b92")
    yy0 = y + 30
    s.text(LX + LW - 10, yy0 + 5, "RX", 12, "#111", "end", "bold")
    wire(LX + LW, yy0, EX, "#311b92")
    pin_left(yy0, "9", "UART1 TX -> RX ekranu")
    yy1 = y + 30 + pitch
    s.text(LX + LW - 10, yy1 + 5, "TX", 12, "#111", "end", "bold")
    pin_left(yy1, "10", "UART1 RX <- (przez dzielnik!)")
    # dzielnik napiecia na linii TX ekranu -> RX ESP32
    dvx, dvy = LX + LW - 210, yy1 + 44
    s.rect(dvx, dvy, 200, 66, "#fff3e0", "#e65100", 2, 8)
    s.text(dvx + 100, dvy + 20, "Dzielnik 5V -> 3,3V", 12, "#111", "middle", weight="bold")
    s.text(dvx + 100, dvy + 38, "R1=10 k (do sygnalu), R2=20 k (do GND)", 11, "#333", "middle")
    s.text(dvx + 100, dvy + 54, "albo gotowy level-shifter I2C/UART", 11, "#333", "middle")
    s.line(dvx + 200, yy1, LX + LW, yy1, "#e65100", 1.8, "4 3")
    s.line(dvx, yy1, EX, yy1, "#e65100", 1.8, "4 3")
    y = dvy + 66 + 30

    # Karta SD - osobny modul
    top = y - 20
    module(LX, top, LW, 4 * pitch + 50, "Moduł microSD (osobny, SPI)", "te same piny co dawniej ekran ILI9341",
           "#e3f2fd", "#0d47a1")
    sd_rows = [("11", "MOSI", "SPI MOSI"), ("12", "SCK", "SPI SCK"), ("13", "MISO", "SPI MISO"), ("16", "CS", "SD CS")]
    for i, (g, port, lab) in enumerate(sd_rows):
        yy = y + 30 + i * pitch
        s.text(LX + LW - 10, yy + 5, port, 12, "#111", "end", "bold")
        wire(LX + LW, yy, EX, "#0d47a1")
        pin_left(yy, g, lab)
    y = y + 30 + 4 * pitch + 34

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
    y_left_end = y + 30 + 2 * pitch + 20

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

    top = y - 20
    module(RX, top, RW, 2 * pitch + 90, "Przyciski START / STOP (+opcjonalny SELEKTOR)",
           "styk -> GND, pull-up wewn.; równolegle: J4 pilot, J5 pedał", "#fce4ec", "#880e4f")
    for i, (g, port, lab) in enumerate([("38", "START", "START"), ("39", "STOP", "STOP (+ISR awaryjny)")]):
        yy = y + 30 + i * pitch
        s.text(RX + 10, yy + 5, port, 12, "#111", "start", "bold")
        wire(EX + EW, yy, RX, "#880e4f")
        pin_right(yy, g, lab)
    s.text(RX + RW / 2, y + 30 + 2 * pitch + 34, "GPIO 40 (SELEKTOR) jedyny wolny pin w tym wariancie (bez SELEKTORA)",
           11, "#1b5e20", "middle", weight="bold")
    y = y + 30 + 2 * pitch + 74

    top = y - 20
    module(RX, top, RW, pitch + 66, "Buzzer pasywny 5 V", "LEDC PWM 100 Hz - 5 kHz", "#f3e5f5", "#4a148c")
    yy = y + 30
    s.text(RX + 10, yy + 5, "(+)", 12, "#111", "start", "bold")
    wire(EX + EW, yy, RX, "#4a148c")
    pin_right(yy, "8", "buzzer (PWM)")
    y = y + 30 + pitch + 50

    top = y - 20
    module(RX, top, RW, pitch + 66, "Czujnik temperatury DS18B20 (opcjonalny)",
           "OneWire, rezystor podciagajacy 4,7 k do 3V3", "#ede7f6", "#4527a0")
    yy = y + 30
    s.text(RX + 10, yy + 5, "DATA", 12, "#111", "start", "bold")
    wire(EX + EW, yy, RX, "#4527a0")
    pin_right(yy, "15", "OneWire (jesli zamontowany)")
    y = y + 30 + pitch + 50

    # Joystick KY-023 - nawigacja menu, rownolegle do dotyku ekranu (ta sama sciezka
    # menu.handleEvent() co kody dotyku 1-7 - patrz ARCHITEKTURA_TERMINAL.md rozdz. 6)
    top = y - 20
    module(RX, top, RW, 3 * pitch + 90, "Joystick KY-023 (nawigacja menu)",
           "analogowy X/Y + przycisk SW, rownolegle do dotyku ekranu", "#e1f5fe", "#01579b")
    joy_rows = [("19", "VRx", "os pozioma (ADC2)"), ("20", "VRy", "os pionowa (ADC2)"),
                ("46", "SW", "przycisk (strap pin - patrz joystick.cpp)")]
    for i, (g, port, lab) in enumerate(joy_rows):
        yy = y + 30 + i * pitch
        s.text(RX + 10, yy + 5, port, 12, "#111", "start", "bold")
        wire(EX + EW, yy, RX, "#01579b")
        pin_right(yy, g, lab)
    s.text(RX + RW / 2, y + 30 + 3 * pitch + 14,
           "GPIO 19/20 = ADC2 (mozliwe zaklocenia WiFi - kod usrednia probki + histereza)",
           10, "#01579b", "middle")
    y = y + 30 + 3 * pitch + 54

    # STOP awaryjny (E-STOP) - ciecie zasilania jest SPRZETOWE (grzybek w torze zasilania
    # przekaznikow), GPIO 21 to WYLACZNIE odczyt statusu petli dla firmware/UI/logow
    top = y - 20
    module(RX, top, RW, pitch + 90, "STOP awaryjny — grzybek E-STOP (styk NC)",
           "CIĘCIE ZASILANIA: grzybek W SERII z +12/24V do COM przekaźników", "#ffebee", "#b71c1c")
    yy = y + 30
    s.text(RX + 10, yy + 5, "STATUS", 12, "#111", "start", "bold")
    wire(EX + EW, yy, RX, "#b71c1c")
    pin_right(yy, "21", "petla NC -> GND (INPUT_PULLUP, fail-safe)")
    s.text(RX + RW / 2, yy + pitch + 14,
           "Odcięcie mocy NIE zależy od firmware — ten pin to TYLKO status (UI/log/alarm)",
           10, "#b71c1c", "middle", weight="bold")
    y_right_end = y + 30 + pitch + 50

    # ---------- klawisze soft-key (MCP23017) ----------
    ky = max(y_left_end, y_right_end) + 40
    s.rect(120, ky, 1100, 250, "#fffde7", "#f57f17", 2, 10)
    s.text(140, ky + 28, "Przyciski wzorców — MCP23017 (I2C 0x20, SDA=GPIO 17, SCL=GPIO 18): układ soft-key 10 + GRUPA",
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
    s.text(bx, ky + 212, "GND wspólna; SELEKTOR (odwracanie P-3a/b) opcjonalny — patrz obudowa_pionowa_os.svg / obudowa_pionowa_krawedz.svg",
           12, "#333")
    s.text(bx, ky + 232, "Podpis 'A | B' = wzorzec w grupie OŚ | grupie KRAWĘDŹ. Ta sama logika co w wariancie przejściowym.", 12, "#333")

    ly = ky + 270
    s.text(120, ly, "Legenda:", 14, weight="bold")
    for i, (c, t) in enumerate([("#c1121f", "zasilanie 5 V"), ("#311b92", "UART ekranu DGUS"), ("#0d47a1", "SPI / I2C"),
                                ("#e65100", "UART GPS / wyjścia mocy do zaworów"), ("#1b5e20", "sygnały do przekaźników"),
                                ("#4527a0", "DS18B20 (OneWire)"), ("#01579b", "joystick (ADC + SW)"),
                                ("#b71c1c", "status E-STOP")]):
        col, row = i % 3, i // 3
        s.line(200 + col * 320, ly - 5 + row * 22, 240 + col * 320, ly - 5 + row * 22, c, 3)
        s.text(246 + col * 320, ly + row * 22, t, 12, "#333")
    ly2 = ly + 2 * 22 + 30
    s.text(120, ly2, "GPIO 26-37 zajęte (Flash + Octal PSRAM). GPIO 15 zajęte przez czujnik temperatury DS18B20"
                      " (nawet jeśli niezamontowany — pin zarezerwowany w firmware).", 12, "#b71c1c")
    s.text(120, ly2 + 20, "Joystick (19/20/46) i status E-STOP (21) zajmują niemal wszystkie pozostałe piny:"
                          " 27 zajęte / 1 wolny — GPIO 14 (+ GPIO 40, jeśli pominiesz SELEKTOR).", 12, "#b71c1c")
    s.text(120, ly2 + 44, "Złącza maszynowe: J1 zasilanie 5 V, J2 zawory P1-P6, J3 enkoder, J4 pilot, J5 pedał, J6 (opcja) wyprowadzenie UART ekranu.", 12, "#333")
    s.save("schemat_polaczen_dgus.svg")


def wiring_headless():
    """Schemat połączeń wariantu DOCELOWEGO: sterownik "headless" (bez żadnego ekranu
    podłączonego bezpośrednio) + moduł wyświetlacza Sunton 7" wyłącznie po WiFi.
    Środowisko esp32s3 (domyślne), HAS_SMALL_TFT=0 HAS_JOYSTICK=0 HAS_DGUS_LINK=0 HAS_ESTOP=1."""
    W, H = 2100, 1750
    s = Svg(W, H, "#ffffff")
    s.text(40, 46, "MPD2026 — schemat połączeń, wariant DOCELOWY (sterownik headless + moduł Sunton po WiFi)",
           28, "#0b1f4a", weight="bold")
    s.text(40, 74, "Sterownik Trassar (ESP32-S3 N16R8, firmware 2.53.0, środowisko esp32s3_terminal) + moduł"
                   " wyświetlacza Sunton 7\" (ESP32-8048S070C) jako JEDYNY interfejs operatora — połączony"
                   " WYŁĄCZNIE przez WiFi (bez żadnych przewodów danych). Numery = GPIO ESP32-S3 (src/config.h).",
           15, "#444")
    s.text(40, 94, "Kod skompilowany i sprawdzony; POŁĄCZENIA nie były jeszcze zweryfikowane na fizycznym sprzęcie — sprawdź to na prototypie.",
           13, "#b71c1c", weight="bold")

    # --- zasilanie ---
    s.rect(40, 116, 200, 60, "#fff3cd", "#b8860b", 2, 6)
    s.text(140, 142, "Akumulator / instalacja", 14, anchor="middle", weight="bold")
    s.text(140, 162, "maszyny 12 V / 24 V DC", 14, anchor="middle")
    s.line(240, 146, 300, 146, "#b8860b", 3)
    s.rect(300, 116, 200, 60, "#fff3cd", "#b8860b", 2, 6)
    s.text(400, 142, "Przetwornica DC-DC", 14, anchor="middle", weight="bold")
    s.text(400, 162, "12/24 V -> 5 V, min. 3 A", 14, anchor="middle")
    s.line(500, 146, 560, 146, "#c1121f", 3)
    s.rect(560, 116, 170, 60, "#fde2e4", "#c1121f", 2, 6)
    s.text(645, 142, "J1 TS13CP03", 14, anchor="middle", weight="bold")
    s.text(645, 162, "+5 V / GND / GND", 13, anchor="middle")
    s.line(730, 146, 1980, 146, "#c1121f", 3)
    s.text(750, 138, "+5 V (PTC 1,5 A + TVS)", 13, "#c1121f", weight="bold")
    s.line(1980, 146, 1980, 260, "#c1121f", 3)
    s.line(1980, 260, 1550, 260, "#c1121f", 3)
    s.text(1560, 252, "+5 V do modułu Sunton (osobny odczep — patrz uwaga)", 12, "#c1121f")

    EX, EY, EW, EH = 800, 200, 460, 680
    s.rect(EX, EY, EW, EH, "#e8f0fe", "#0b1f4a", 3, 10)
    s.text(EX + EW / 2, EY + 32, "ESP32-S3 N16R8", 20, "#0b1f4a", "middle", "bold")
    s.text(EX + EW / 2, EY + 54, "DevKitC-1 — sterownik Trassar (esp32s3_terminal), \"headless\"", 13, "#0b1f4a", "middle")
    s.line(EX + EW / 2, 146, EX + EW / 2, EY, "#c1121f", 3)
    s.text(EX + EW / 2 + 8, 180, "5 V (VBUS)", 12, "#c1121f")
    s.text(EX + EW / 2, EY + EH - 18, "3V3: SD, GPS, MCP23017, enkoder    GND: masa wspólna", 12, "#333", "middle")

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
    LX, LW = 120, 420
    y = EY + 40

    # Modul Sunton 7" - JEDYNY ekran, BEZ zadnych przewodow danych (wylacznie WiFi)
    top = y - 20
    s.rect(LX, top, LW, pitch + 96, "#e1f5fe", "#01579b", 2, 8)
    s.text(LX + LW / 2, top + 22, "Moduł wyświetlacza Sunton 7\" (ESP32-8048S070C)", 14, "#111", "middle", "bold")
    s.text(LX + LW / 2, top + 40, "JEDYNY interfejs operatora — display-module/", 12, "#01579b", "middle", weight="bold")
    s.text(LX + LW / 2, top + 58, "BEZ przewodów danych do sterownika — tylko WiFi", 11, "#444", "middle")
    s.text(LX + LW / 2, top + 76, "Zasilanie: osobne 5 V (USB-C), min. 2 A, wspólna masa", 11, "#444", "middle")
    s.text(LX + LW / 2, top + 94, "z instalacją maszyny (patrz uwaga o wspólnym GND niżej)", 11, "#444", "middle")
    box_bottom = top + pitch + 96
    wifi_y = box_bottom + 24
    s.line(LX + LW / 2, box_bottom, LX + LW / 2, wifi_y, "#01579b", 2, "6 4")
    s.line(LX + LW / 2, wifi_y, EX - 6, wifi_y, "#01579b", 2, "6 4")
    s.circle(EX - 6, wifi_y, 4, "#01579b")
    s.text(LX + LW / 2 + 10, wifi_y - 6, "WiFi: WebSocket :81 (status) + HTTP :80 (POST /api/control)", 11, "#01579b")
    y = wifi_y + 40

    # Karta SD - osobny modul
    top = y - 20
    module(LX, top, LW, 4 * pitch + 50, "Moduł microSD (osobny, SPI)", "raporty, GPX, kopia ustawień",
           "#e3f2fd", "#0d47a1")
    sd_rows = [("11", "MOSI", "SPI MOSI"), ("12", "SCK", "SPI SCK"), ("13", "MISO", "SPI MISO"), ("16", "CS", "SD CS")]
    for i, (g, port, lab) in enumerate(sd_rows):
        yy = y + 30 + i * pitch
        s.text(LX + LW - 10, yy + 5, port, 12, "#111", "end", "bold")
        wire(LX + LW, yy, EX, "#0d47a1")
        pin_left(yy, g, lab)
    y = y + 30 + 4 * pitch + 34

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
    y_left_end = y + 30 + 2 * pitch + 20

    # ---------- prawa strona ----------
    RX, RW = 1400, 300
    y = EY + 40
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

    top = y - 20
    module(RX, top, RW, 2 * pitch + 90, "Przyciski START / STOP (+opcjonalny SELEKTOR)",
           "styk -> GND, pull-up wewn.; równolegle: J4 pilot, J5 pedał", "#fce4ec", "#880e4f")
    for i, (g, port, lab) in enumerate([("38", "START", "START"), ("39", "STOP", "STOP (+ISR awaryjny)")]):
        yy = y + 30 + i * pitch
        s.text(RX + 10, yy + 5, port, 12, "#111", "start", "bold")
        wire(EX + EW, yy, RX, "#880e4f")
        pin_right(yy, g, lab)
    s.text(RX + RW / 2, y + 30 + 2 * pitch + 34, "GPIO 40 (SELEKTOR) opcjonalny — patrz uwaga o wolnych pinach niżej",
           11, "#1b5e20", "middle", weight="bold")
    y = y + 30 + 2 * pitch + 74

    top = y - 20
    module(RX, top, RW, pitch + 66, "Buzzer pasywny 5 V", "LEDC PWM 100 Hz - 5 kHz", "#f3e5f5", "#4a148c")
    yy = y + 30
    s.text(RX + 10, yy + 5, "(+)", 12, "#111", "start", "bold")
    wire(EX + EW, yy, RX, "#4a148c")
    pin_right(yy, "8", "buzzer (PWM)")
    y = y + 30 + pitch + 50

    top = y - 20
    module(RX, top, RW, pitch + 66, "Czujnik temperatury DS18B20 (opcjonalny)",
           "OneWire, rezystor podciagajacy 4,7 k do 3V3", "#ede7f6", "#4527a0")
    yy = y + 30
    s.text(RX + 10, yy + 5, "DATA", 12, "#111", "start", "bold")
    wire(EX + EW, yy, RX, "#4527a0")
    pin_right(yy, "15", "OneWire (jesli zamontowany)")
    y = y + 30 + pitch + 50

    # STOP awaryjny (E-STOP) - ciecie zasilania jest SPRZETOWE (grzybek w torze zasilania
    # przekaznikow), GPIO 21 to WYLACZNIE odczyt statusu petli dla firmware/UI/logow
    top = y - 20
    module(RX, top, RW, pitch + 90, "STOP awaryjny — grzybek E-STOP (styk NC)",
           "CIĘCIE ZASILANIA: grzybek W SERII z +12/24V do COM przekaźników", "#ffebee", "#b71c1c")
    yy = y + 30
    s.text(RX + 10, yy + 5, "STATUS", 12, "#111", "start", "bold")
    wire(EX + EW, yy, RX, "#b71c1c")
    pin_right(yy, "21", "petla NC -> GND (INPUT_PULLUP, fail-safe)")
    s.text(RX + RW / 2, yy + pitch + 14,
           "Odcięcie mocy NIE zależy od firmware — ten pin to TYLKO status (UI/log/alarm)",
           10, "#b71c1c", "middle", weight="bold")
    y_right_end = y + 30 + pitch + 50

    # ---------- klawisze soft-key (MCP23017) ----------
    ky = max(y_left_end, y_right_end) + 40
    s.rect(120, ky, 1100, 250, "#fffde7", "#f57f17", 2, 10)
    s.text(140, ky + 28, "Przyciski wzorców — MCP23017 (I2C 0x20, SDA=GPIO 17, SCL=GPIO 18): układ soft-key 10 + GRUPA",
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
    s.text(bx, ky + 212, "GND wspólna; SELEKTOR (odwracanie P-3a/b) opcjonalny — patrz obudowa_pionowa_os.svg / obudowa_pionowa_krawedz.svg",
           12, "#333")
    s.text(bx, ky + 232, "Podpis 'A | B' = wzorzec w grupie OŚ | grupie KRAWĘDŹ.", 12, "#333")

    ly = ky + 270
    s.text(120, ly, "Legenda:", 14, weight="bold")
    for i, (c, t) in enumerate([("#c1121f", "zasilanie 5 V"), ("#01579b", "WiFi (bez przewodu)"), ("#0d47a1", "SPI / I2C"),
                                ("#e65100", "UART GPS / wyjścia mocy do zaworów"), ("#1b5e20", "sygnały do przekaźników"),
                                ("#4527a0", "DS18B20 (OneWire)"), ("#b71c1c", "status E-STOP")]):
        col, row = i % 3, i // 3
        s.line(200 + col * 320, ly - 5 + row * 22, 240 + col * 320, ly - 5 + row * 22, c, 3)
        s.text(246 + col * 320, ly + row * 22, t, 12, "#333")
    ly2 = ly + 2 * 22 + 30
    s.text(120, ly2, "GPIO 26-37 zajęte (Flash + Octal PSRAM). GPIO 15 zajęte przez czujnik temperatury DS18B20"
                      " (nawet jeśli niezamontowany — pin zarezerwowany w firmware).", 12, "#b71c1c")
    s.text(120, ly2 + 20, "Bez ekranu bezpośredniego i joysticka (domyślnie wyłączony) WOLNE pozostają: GPIO 9, 10,"
                          " 14, 19, 20, 46 (+ 40, jeśli pominiesz SELEKTOR) — 23 zajęte / 6 wolnych.", 12, "#1b5e20", weight="bold")
    s.text(120, ly2 + 44, "Joystick KY-023 (opcja, GPIO 19/20/46) można podłączyć jak w wariancie z ekranem DGUS —"
                          " ustaw HAS_JOYSTICK=1 w platformio.ini. Zajmie 3 z 6 wolnych pinów.", 12, "#333")
    s.text(120, ly2 + 68, "Złącza maszynowe: J1 zasilanie 5 V, J2 zawory P1-P6, J3 enkoder, J4 pilot, J5 pedał.", 12, "#333")
    s.save("schemat_polaczen_docelowy.svg")


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


# --- wzorce (kopia display-module/src/model.cpp) i rysunek w skali (kopia ui_widgets.cpp: glyphDraw) ---
OFF, CONT = (0, 0, 0), (1, 0, 0)


def DASH(l, g):
    return (2, l, g)


PATS = {
    "P-1a": [OFF, DASH(4, 8), OFF, OFF, OFF, OFF], "P-1b": [OFF, DASH(2, 4), OFF, OFF, OFF, OFF],
    "P-1c": [OFF, DASH(2, 2), OFF, OFF, OFF, OFF], "P-1d": [OFF, DASH(1, 1), OFF, OFF, OFF, OFF],
    "P-1e": [OFF, OFF, OFF, DASH(1, 1), OFF, OFF], "P-2a": [OFF, CONT, OFF, OFF, OFF, OFF],
    "P-2b": [OFF, OFF, OFF, CONT, OFF, OFF], "P-3a": [CONT, OFF, DASH(4, 2), OFF, OFF, OFF],
    "P-3b": [CONT, OFF, DASH(1, 1), OFF, OFF, OFF], "P-4": [CONT, OFF, CONT, OFF, OFF, OFF],
    "P-6": [OFF, OFF, OFF, OFF, DASH(4, 2), OFF], "P-7a": [OFF, OFF, OFF, OFF, OFF, DASH(1, 1)],
    "P-7b": [OFF, OFF, OFF, OFF, OFF, CONT], "P-7c": [OFF, OFF, OFF, OFF, DASH(1, 1), OFF],
    "P-7d": [OFF, OFF, OFF, OFF, CONT, OFF],
    "WŁASNY": [CONT, OFF, OFF, OFF, DASH(3, 2), OFF],
}
GUN_W_CM = [12, 12, 12, 24, 12, 24]
GUN_CENTER_CM = [-12, 0, 12, 0, 30, 30]


def fmtg(v):
    return ("%g" % v)


def spec_text(cfg):
    modes, widths = [], []
    for g, (m, l, gp) in enumerate(cfg):
        if m == 0:
            continue
        modes.append("ciagla" if m == 1 else f"{fmtg(l)}/{fmtg(gp)} m")
        widths.append(str(GUN_W_CM[g]))
    if not modes:
        return "brak", ""
    return " + ".join(modes), "+".join(widths) + " cm"


def glyph(s, x, y, w, h, cfg, rounded=3):
    """Rysunek wzorca w skali: okno 72 cm, 2 pełne cykle, pobocze dla wzorców krawędziowych."""
    s.rect(x, y, w, h, "#1b1b1f", rx=rounded)
    active = [(g, c) for g, c in enumerate(cfg) if c[0] != 0]
    if not active:
        return
    edge_only = all(g >= 4 for g, _ in active)
    max_cycle = max([c[1] + c[2] for _, c in active if c[0] == 2 and c[1] > 0], default=0)
    ppc = w / 72.0
    base = max(2, int(12 * ppc + 0.5))
    cx = x + w / 2.0 - (8 * ppc if edge_only else 0)
    if edge_only:
        ex = int(cx + 25 * ppc)
        if ex < x + w:
            s.rect(ex, y + 1, x + w - ex, h - 2, "#59606e")
    L = 2.0 * max_cycle
    ppm = (h - 4) / L if L > 0 else 0
    for g, (m, l, gp) in active:
        lw = base * GUN_W_CM[g] // 12
        centre = 0 if edge_only else GUN_CENTER_CM[g]
        lx = int(cx + centre * ppc - lw / 2.0 + 0.5)
        if m == 1:
            s.rect(lx, y + 2, lw, h - 4, "#ffd400")
        elif l > 0 and L > 0:
            cyc = l + gp
            p = 0.0
            while p < L - 0.001:
                p2 = min(p + l, L)
                yb = int(y + h - 2 - int(p * ppm + 0.5))
                yt = max(int(y + h - 2 - int(p2 * ppm + 0.5)), y + 2)
                if yb - yt >= 1:
                    s.rect(lx, yt, lw, yb - yt, "#ffd400")
                p += cyc


def ui_mock_portrait(s, x, y, scale, group=0):
    """Makieta ekranu pionowego 480x800: wzorce w kolumnach przy krawędziach (5 + 5), rysunki w skali."""
    s.group_open(f"translate({x},{y}) scale({scale})")
    s.rect(0, 0, 480, 800, "#07142e")
    s.text(10, 32, "POŁĄCZONO", 16, "#35f27a", weight="bold")
    s.text(140, 32, "GPS 8 sat", 16, "#35f27a", weight="bold")
    s.rect(240, 10, 96, 28, "#0f2557", rx=6)
    s.rect(240, 10, 70, 28, "#1fa34a", rx=6)
    s.text(288, 30, "FARBA 72%", 12, "#fff", "middle")
    s.rect(346, 4, 130, 40, "#1f52d6", rx=10)
    s.text(411, 32, "MENU", 18, "#fff", "middle", "bold")
    s.rect(6, 54, 228, 42, "#2f86ff" if group == 0 else "#1f52d6", "#ffd400" if group == 0 else "#5b7fe0",
           5 if group == 0 else 2, 10)
    s.text(120, 83, "OŚ JEZDNI", 20, "#fff", "middle", "bold")
    s.rect(246, 54, 228, 42, "#2f86ff" if group == 1 else "#1f52d6", "#ffd400" if group == 1 else "#5b7fe0",
           5 if group == 1 else 2, 10)
    s.text(360, 83, "KRAWĘDŹ", 20, "#fff", "middle", "bold")
    left = ["P-1a", "P-1b", "P-1c", "P-1d", "P-1e"]
    right = ["P-2a", "P-2b", "P-3a", "P-3b", "P-4"]
    if group == 1:
        left = ["P-6", "P-7a", "P-7b", "P-7c", "P-7d"]
        right = ["WŁASNY", "", "", "", ""]
    sel = "P-3a" if group == 0 else "P-7b"
    for names, x0 in ((left, 4), (right, 368)):
        for i, n in enumerate(names):
            yy = 106 + i * 116
            if not n:
                s.rect(x0, yy, 108, 106, "#27314d", "#3b4a73", 2, 10)
                continue
            is_sel = (n == sel)
            s.rect(x0, yy, 108, 106, "#2f86ff" if is_sel else "#1f52d6", "#ffd400" if is_sel else "#5b7fe0",
                   5 if is_sel else 2, 10)
            glyph(s, x0 + 6, yy + 8, 34, 90, PATS[n])
            s.text(x0 + 104, yy + 26, n, 20 if len(n) < 6 else 13, "#fff", "end", "bold")
            m, wd = spec_text(PATS[n])
            parts = m.split(" + ") if len(m) > 9 else [m]
            lines = []
            if len(parts) == 2:
                lines = [parts[0] + " +", parts[1]]
            else:
                lines = [m]
            lines.append(wd)
            for k, ln in enumerate(lines):
                s.text(x0 + 104, yy + 46 + k * 15, ln, 11, "#e9efff", "end")
    # środek: duży rysunek wzorca + kod + zapis liczbowy + prędkość
    cx = 240
    glyph(s, 120, 102, 66, 80, PATS[sel], 4)
    s.text(196, 138, sel, 40, "#ffd400", weight="bold")
    m, wd = spec_text(PATS[sel])
    s.text(196, 160, m, 12, "#e9efff")
    s.text(196, 176, wd, 12, "#e9efff")
    s.text(cx, 201, "AUTO  MALOWANIE", 15, "#35f27a", "middle")
    s.text(cx, 256, "12.5", 64, "#fff", "middle", "bold", 'font-family="Consolas, monospace"')
    s.text(cx, 290, "km/h", 20, "#8fa3cc", "middle")
    s.text(cx, 312, "DYST 1234.5 m", 15, "#fff", "middle")
    s.text(cx, 332, "POW 148.1 m2", 15, "#fff", "middle")
    s.text(cx, 352, "CZAS 12:34", 15, "#fff", "middle")
    s.rect(118, 364, 244, 240, "#143a7a", rx=6)
    s.polygon([(128, 600), (352, 600), (290, 368), (190, 368)], "#1b1b1f")
    for i, (yy, hh, w) in enumerate([(548, 44, 11), (478, 34, 9), (420, 24, 7), (390, 14, 5)]):
        s.polygon([(cx - w, yy + hh), (cx + w, yy + hh), (cx + w - 1, yy), (cx - w + 1, yy)],
                  "#35f27a" if i == 0 else "#ffd400")
    s.rect(140, 596, 200, 4, "#fff")
    for g in range(6):
        on = g in ((0, 2) if group == 0 else (5,))
        s.rect(120 + g * 41, 610, 38, 28, "#35f27a" if on else "#0f2557", "#ffd400" if g in (0, 2) else "#0f2557", 2, 14)
        s.text(120 + g * 41 + 19, 630, f"P{g + 1}", 13, "#03210f" if on else "#fff", "middle")
    for i, (tt, c) in enumerate([("AUTO", "#2f86ff"), ("SEMI", "#1f52d6"), ("RĘCZNY", "#1f52d6")]):
        s.rect(4 + i * 96, 694, 92, 44, c, "#ffd400" if i == 0 else "#5b7fe0", 4 if i == 0 else 2, 8)
        s.text(50 + i * 96, 722, tt, 15, "#fff", "middle", "bold")
    s.rect(292, 694, 184, 44, "#1f52d6", rx=8)
    s.text(384, 722, "START OD PRZERWY", 14, "#fff", "middle")
    s.rect(4, 746, 296, 50, "#1fa34a", rx=10)
    s.text(152, 780, "▶ START", 24, "#fff", "middle", "bold")
    s.rect(306, 746, 170, 50, "#d62828", rx=10)
    s.text(391, 780, "■ STOP", 24, "#fff", "middle", "bold")
    s.group_close()


def pattern_icons():
    """Pojedyncze ikony wzorców do zaimportowania w DGUS Designer jako 'ikona wariantowa'
    (kontrolka wybierająca obrazek numerem indeksu — patrz docs/EKRAN_DGUS.md rozdz. 'Kolumny
    wzorców S1-S10'). Nazwy plików = indeks PatternID (src/config.h) + kod wzorca, więc kolejność
    importu do biblioteki ikon w Designerze jest jednoznaczna: 00=P-1a ... 14=P-7d, 15=WŁASNY.
    Samo tło ikony (bez tła kafelka) — w Designerze nakłada się na kafelek slotu."""
    names = list(PATS.keys())
    icon_dir = os.path.join(OUT, "ikony_dgus")
    os.makedirs(icon_dir, exist_ok=True)
    W, H = 200, 240
    for i, n in enumerate(names):
        s = Svg(W, H, "#1b1b1f")
        glyph(s, 14, 14, W - 28, H - 28, PATS[n], rounded=6)
        fname = f"{i:02d}_{n.replace('Ł', 'L')}.svg"
        body = "\n".join(s.items)
        svg = (f'<svg xmlns="http://www.w3.org/2000/svg" width="{s.w}" height="{s.h}" '
               f'viewBox="0 0 {s.w} {s.h}">\n{body}\n</svg>\n')
        with open(os.path.join(icon_dir, fname), "w", encoding="utf-8") as f:
            f.write(svg)
        print("zapisano ikony_dgus/" + fname)


def glyph_sheet():
    """Arkusz wszystkich wzorców rysowanych w skali (kontrola geometrii)."""
    names = list(PATS.keys())
    s = Svg(1240, 700, "#f8f9fa")
    s.text(30, 38, "Rysunki wzorców w skali — 2 pełne cykle, szerokości i rozstaw jak w rzeczywistości", 20, "#0b1f4a", weight="bold")
    s.text(30, 62, "Miniatura w kolumnie ekranu (34x90 px) i duży rysunek przy kodzie (66x80 px). Okno poziome 72 cm; wzorce krawędziowe z poboczem.",
           13, "#444")
    for i, n in enumerate(names):
        col, row = i % 8, i // 8
        x0, y0 = 30 + col * 150, 90 + row * 300
        s.rect(x0 - 8, y0 - 8, 142, 280, "#1f52d6", "#5b7fe0", 2, 10)
        glyph(s, x0, y0, 66, 160, PATS[n], 4)
        glyph(s, x0 + 76, y0, 34, 90, PATS[n])
        s.text(x0 + 4, y0 + 188, n, 22, "#fff", weight="bold")
        m, wd = spec_text(PATS[n])
        s.text(x0 + 4, y0 + 212, m, 12, "#e9efff")
        s.text(x0 + 4, y0 + 230, wd, 12, "#e9efff")
    s.save("wzorce_w_skali.svg")


def panel_C():
    """Panel pionowy: fizyczne klawisze S1-S10 stoją tuż obok etykiet wzorców na ekranie."""
    W, H = 820, 1120
    s = Svg(W, H, "#f8f9fa")
    s.text(30, 38, "Propozycja C — panel pionowy, klawisze fizyczne obok etykiet wzorców", 22, "#0b1f4a", weight="bold")
    s.text(30, 62, "Ekran 7\" obrócony do pionu (480x800). Każdy klawisz S1-S10 jest na tej samej wysokości co etykieta wzorca. "
                   "Wymaga trybu pionowego UI.", 14, "#444")
    px, py, pw, ph = 40, 90, 740, 990
    panel_frame(s, px, py, pw, ph, "TRASSAR  MPD2026")
    scale = 0.75
    sw, sh = 480 * scale, 800 * scale       # 360 x 600
    sx, sy = px + (pw - sw) / 2, py + 62
    screen_bezel(s, sx, sy, sw, sh)
    ui_mock_portrait(s, sx, sy, scale, 0)
    # fizyczne klawisze dokładnie na wysokości etykiet
    kh = 106 * scale
    for i in range(5):
        yy = sy + (106 + i * 116) * scale
        key(s, sx - 8 - 12 - 108, yy, 108, kh, f"S{i + 1}", "◄ obok etykiety")
        key(s, sx + sw + 8 + 12, yy, 108, kh, f"S{i + 6}", "obok etykiety ►")
        s.line(sx - 20, yy + kh / 2, sx - 8, yy + kh / 2, "#495057", 2)
        s.line(sx + sw + 8, yy + kh / 2, sx + sw + 20, yy + kh / 2, "#495057", 2)
    # dolny blok
    by = sy + sh + 40
    key(s, px + 40, by, 200, 62, "GRUPA", "OŚ ⇄ KRAWĘDŹ", "#7a4b00", "#fff", "#ffd400")
    key(s, px + 270, by, 200, 62, "SELEKTOR", None)
    key(s, px + 500, by, 200, 62, "GAP", "od przerwy")
    s.rect(px + 40, by + 84, 400, 100, "#1fa34a", "#0b6b2e", 3, 16)
    s.text(px + 240, by + 148, "START", 32, "#fff", "middle", "bold")
    s.rect(px + 470, by + 84, 230, 100, "#d62828", "#7a0f0f", 3, 16)
    s.text(px + 585, by + 148, "STOP", 32, "#fff", "middle", "bold")
    s.text(px + 30, py + ph - 16, "Zaleta: klawisz i etykieta w jednej linii, wąska obudowa jak STiM.  Wada: przeróbka UI na tryb pionowy (LVGL).",
           12, "#495057")
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


# ---------------------------------------------------------------------------
# 4. Wiązka okablowania: złącza maszynowe J1-J5 + łącze do modułu 7"
# ---------------------------------------------------------------------------
COLORS = {"czerwony": "#d62828", "czarny": "#222222", "pomarańczowy": "#f77f00", "niebieski": "#1f52d6",
          "zielony": "#1fa34a", "żółty": "#ffd400", "brązowy": "#7b4a12", "biały": "#f8f9fa", "ekran": "#9aa5b1", "-": "#ffffff"}


def connector_card(s, x, y, w, title, subtitle, rows, accent):
    """rows: (pin, sygnał, kolor, wewnątrz, na zewnątrz)"""
    rh = 30
    h = 64 + rh * (len(rows) + 1)
    s.rect(x, y, w, h, "#ffffff", accent, 2.5, 10)
    s.rect(x, y, w, 44, accent, accent, 2.5, 10)
    s.text(x + 14, y + 20, title, 16, "#fff", weight="bold")
    s.text(x + 14, y + 37, subtitle, 12, "#fff")
    hy = y + 64
    cols = [x + 14, x + 62, x + 218, x + 318, x + 470]
    for cx, t in zip(cols, ["Pin", "Sygnał", "Kolor", "Wewnątrz (sterownik)", "Na zewnątrz (maszyna)"]):
        s.text(cx, hy, t, 11, "#555", weight="bold")
    s.line(x + 8, hy + 6, x + w - 8, hy + 6, "#bbb", 1)
    for i, (pin, sig, col, inside, outside) in enumerate(rows):
        yy = hy + 24 + i * rh
        s.circle(cols[0] + 10, yy - 5, 11, "#f1f3f5", "#555", 1.2)
        s.text(cols[0] + 10, yy - 1, str(pin), 12, "#111", "middle", "bold")
        s.text(cols[1], yy, sig, 12, "#111", weight="bold")
        if col != "-":
            s.rect(cols[2], yy - 13, 22, 16, COLORS.get(col, "#ccc"), "#444", 1.2, 3)
            s.text(cols[2] + 28, yy, col, 11, "#333")
        else:
            s.text(cols[2], yy, "rezerwa", 11, "#888")
        s.text(cols[3], yy, inside, 11, "#0b1f4a")
        s.text(cols[4], yy, outside, 11, "#333")
        if i < len(rows) - 1:
            s.line(x + 8, yy + 10, x + w - 8, yy + 10, "#eee", 1)
    return h


def harness():
    W, H = 2000, 900
    s = Svg(W, H, "#f8f9fa")
    s.text(40, 46, "MPD2026 — wiązka okablowania: złącza maszynowe i łącze do modułu 7\"", 28, "#0b1f4a", weight="bold")
    s.text(40, 72, "Kolory przewodów wg zalecanego standardu. Numery GPIO wg src/config.h. Złącza patrz SCHEMAT_PODLACZEN.md rozdz. 7.",
           14, "#444")
    cw = 620
    x1, x2, x3 = 40, 690, 1340
    connector_card(s, x1, 100, cw, "J1 — Zasilanie 5 V (TS13CP03)", "13 A / 250 V, 3 piny; przewód AWG 20-22, min. 3 A", [
        (1, "+5 V DC", "czerwony", "5 V VBUS ESP32, VCC przekaźników", "przetwornica 12/24 V -> 5 V (+)"),
        (2, "GND", "czarny", "GND wspólna (gwiazda przy ESP32)", "przetwornica (-)"),
        (3, "GND", "czarny", "równolegle z pinem 2", "przetwornica (-), zdublowane"),
    ], "#c1121f")
    hj2 = connector_card(s, x2, 100, cw, "J2 — Zawory pistoletów (TS17CP10)", "5 A / 400 V, 10 pinów; obwód mocy 12/24 V, AWG 16-18", [
        (1, "P1 NO", "pomarańczowy", "przekaźnik 1 (GPIO 41)", "zawór P1 (oś lewy, 12 cm)"),
        (2, "P2 NO", "pomarańczowy", "przekaźnik 2 (GPIO 42)", "zawór P2 (oś środek, 12 cm)"),
        (3, "P3 NO", "pomarańczowy", "przekaźnik 3 (GPIO 1)", "zawór P3 (oś prawy, 12 cm)"),
        (4, "P4 NO", "pomarańczowy", "przekaźnik 4 (GPIO 2)", "zawór P4 (oś szeroki, 24 cm)"),
        (5, "P5 NO", "pomarańczowy", "przekaźnik 5 (GPIO 3)", "zawór P5 (krawędź, 12 cm)"),
        (6, "P6 NO", "pomarańczowy", "przekaźnik 6 (GPIO 4)", "zawór P6 (krawędź, 24 cm)"),
        (7, "COM", "czerwony", "COM przekaźników", "+12/24 V instalacji maszyny"),
        (8, "COM", "czerwony", "COM (zdublowany)", "+12/24 V instalacji maszyny"),
        (9, "-", "-", "-", "-"),
        (10, "-", "-", "-", "-"),
    ], "#e65100")
    connector_card(s, x3, 100, cw, "J3 — Enkoder (TS13CP05)", "5 A / 180 V, 5 pinów; skrętka CLK+DT, 100 nF do GND", [
        (1, "CLK (A)", "niebieski", "GPIO 5 (ISR)", "enkoder kanał A"),
        (2, "DT (B)", "zielony", "GPIO 6 (ISR)", "enkoder kanał B"),
        (3, "SW", "żółty", "GPIO 7 (GAP)", "przycisk enkodera"),
        (4, "VCC", "czerwony", "3V3", "zasilanie enkodera"),
        (5, "GND", "czarny", "GND", "masa enkodera"),
    ], "#006064")
    connector_card(s, x1, 314, cw, "J4 — Pilot przewodowy (TS13PS06)", "5 A / 125 V, 6 pinów; równolegle do przycisków panelu", [
        (1, "START / PAUZA", "czerwony", "GPIO 38", "przycisk START pilota (NO -> GND)"),
        (2, "SELEKTOR", "zielony", "GPIO 40", "przycisk SELEKTOR pilota"),
        (3, "STOP", "żółty", "GPIO 39 (+ISR awaryjny)", "przycisk STOP pilota"),
        (4, "GAP", "niebieski", "GPIO 7", "przycisk START OD PRZERWY pilota"),
        (5, "GND", "czarny", "GND", "masa przycisków"),
        (6, "-", "-", "-", "-"),
    ], "#880e4f")
    connector_card(s, x1, 618, cw, "J5 — Przycisk nożny (TS21CP04)", "30 A / 500 V, 4 piny; równolegle do START/STOP", [
        (1, "START / PAUZA", "czerwony", "GPIO 38", "pedał lewy (NO -> GND)"),
        (2, "STOP", "żółty", "GPIO 39 (+ISR awaryjny)", "pedał prawy (NO -> GND)"),
        (3, "GND", "czarny", "GND", "masa pedałów"),
        (4, "-", "-", "-", "-"),
    ], "#880e4f")
    connector_card(s, x2, 524, cw, "J6 — Łącze do modułu 7\" (M12 A, 4 piny) — PROPOZYCJA", "Wariant przewodowy zamiast WiFi; opis w LACZE_PRZEWODOWE.md", [
        (1, "+5 V", "brązowy", "odgałęzienie 5 V (PTC >= 1 A)", "zasilanie modułu 7\" (5 V)"),
        (2, "RS-485 A (+)", "biały", "moduł RS-485 sterownika", "moduł RS-485 wyświetlacza"),
        (3, "GND", "niebieski", "GND", "GND modułu 7\""),
        (4, "RS-485 B (-)", "czarny", "moduł RS-485 sterownika", "moduł RS-485 wyświetlacza"),
        ("PE", "ekran kabla", "ekran", "obudowa / PE (jeden koniec)", "nie łączyć po stronie modułu 7\""),
    ], "#311b92")
    s.rect(x3, 374, cw, 300, "#fffde7", "#f57f17", 2, 10)
    s.text(x3 + 14, 404, "Zasady okablowania", 16, "#111", weight="bold")
    rules = ["1. Zasilanie 5 V i obwody zaworów (J1, J2) prowadzić osobno od sygnałowych (J3-J6).",
             "2. Enkoder (J3): skrętka CLK+DT ekranowana, kondensatory 100 nF przy enkoderze.",
             "3. Masa sygnałowa w jednym punkcie (gwiazda przy ESP32); GND zaworów = minus instalacji maszyny,",
             "    NIE łączyć z GND ESP32 (optoizolacja modułu przekaźników).",
             "4. Wszystkie przyciski (panel, pilot J4, pedał J5) łączą wejście z GND — styk NO.",
             "5. STOP z pilota lub pedału działa tak samo jak przycisk panelu (przerwanie awaryjne).",
             "6. Końcówki tulejkowe na wszystkich przewodach; opisać przewody numerami pinów.",
             "7. PTC 1,5 A + TVS 5,5 V w linii 5 V sterownika, osobne PTC dla modułu 7\".",
             "8. Przed pierwszym uruchomieniem: pomiar 3V3 i 5 V oraz test ciągłości GND."]
    for i, r in enumerate(rules):
        s.text(x3 + 14, 434 + i * 22, r, 12, "#333")
    s.save("schemat_zlacza_wiazka.svg")


# ---------------------------------------------------------------------------
# 5. Łącze przewodowe RS-485 sterownik <-> moduł 7" (propozycja)
# ---------------------------------------------------------------------------
def rs485():
    W, H = 2000, 860
    s = Svg(W, H, "#ffffff")
    s.text(40, 46, "Łącze przewodowe sterownik <-> moduł 7\" (RS-485) — PROPOZYCJA", 28, "#0b1f4a", weight="bold")
    s.text(40, 72, "Zamiast WiFi: UART 3,3 V przez konwerter RS-485 z automatycznym kierunkiem (bez pinu DE)."
                   " Zasilanie modułu 7\" tym samym kablem.", 14, "#444")
    # --- sterownik ---
    s.rect(60, 140, 380, 520, "#e8f0fe", "#0b1f4a", 3, 12)
    s.text(250, 172, "Sterownik Trassar (ESP32-S3 N16R8)", 17, "#0b1f4a", "middle", "bold")
    ports_l = [(250, "GPIO 20  UART1 TX", "#111"), (290, "GPIO 19  UART1 RX", "#111"),
               (330, "3V3", "#111"), (370, "GND", "#111"), (410, "+5 V (odgałęzienie, PTC)", "#c1121f")]
    for yy, lab, col in ports_l:
        s.circle(440, yy - 5, 5, "#0b1f4a")
        s.text(80, yy, lab, 14, col, weight="bold")
    s.text(80, 470, "GPIO 19 i 20 są dziś używane przez joystick KY-023 —", 12, "#b71c1c")
    s.text(80, 488, "w wariancie przewodowym joystick jest pomijany", 12, "#b71c1c")
    s.text(80, 506, "(ekran dotykowy go zastępuje).", 12, "#b71c1c")
    s.text(80, 540, "GPIO 15 zostaje wolny (T_CS / DS18B20).", 12, "#555")
    s.text(80, 574, "Protokół: wiersze JSON, 230400 baud, 8N1,", 12, "#555")
    s.text(80, 592, "status co 100-200 ms, polecenia z sumą CRC.", 12, "#555")
    # --- konwerter po stronie sterownika ---
    s.rect(560, 210, 270, 230, "#e8f5e9", "#1b5e20", 2.5, 10)
    s.text(695, 238, "Konwerter RS-485 (MAX3485)", 15, "#111", "middle", "bold")
    s.text(695, 258, "auto-kierunek, zasilanie 3,3 V", 12, "#333", "middle")
    for yy, lab in [(300, "RXD  <- TX sterownika"), (325, "TXD  -> RX sterownika"), (350, "VCC  3V3"), (375, "GND")]:
        s.circle(560, yy - 4, 4, "#1b5e20")
        s.text(574, yy, lab, 12, "#111")
    s.text(816, 300, "A", 14, "#111", "end", "bold")
    s.text(816, 335, "B", 14, "#111", "end", "bold")
    s.circle(830, 296, 4, "#1b5e20")
    s.circle(830, 331, 4, "#1b5e20")
    s.text(816, 420, "terminator 120 ohm A-B", 11, "#555", "end")
    s.line(440, 245, 470, 245, "#1b5e20", 2); s.line(470, 245, 470, 296, "#1b5e20", 2); s.line(470, 296, 560, 296, "#1b5e20", 2)
    s.line(440, 285, 490, 285, "#1b5e20", 2); s.line(490, 285, 490, 321, "#1b5e20", 2); s.line(490, 321, 560, 321, "#1b5e20", 2)
    s.line(440, 325, 510, 325, "#111", 2); s.line(510, 325, 510, 346, "#111", 2); s.line(510, 346, 560, 346, "#111", 2)
    s.line(440, 365, 530, 365, "#111", 2); s.line(530, 365, 530, 371, "#111", 2); s.line(530, 371, 560, 371, "#111", 2)
    # --- kabel ---
    s.rect(860, 240, 300, 150, "#fff8e1", "#e65100", 2.5, 10)
    s.text(1010, 268, "Kabel ekranowany 2 x 2 x 0,5 mm2", 13, "#111", "middle", "bold")
    s.text(1010, 290, "skrętka A/B + skrętka +5 V/GND", 12, "#333", "middle")
    s.text(1010, 312, "ekran do PE po jednej stronie", 12, "#333", "middle")
    s.text(1010, 334, "złącze M12 A, 4 piny, IP67", 12, "#333", "middle")
    s.text(1010, 356, "długość zwykle 1-5 m (RS-485 do 1200 m)", 12, "#333", "middle")
    s.text(1010, 378, "pin 1 +5 V, 2 A, 3 GND, 4 B", 12, "#333", "middle")
    s.line(830, 296, 860, 296, "#e65100", 3); s.line(830, 331, 860, 331, "#e65100", 3)
    s.line(1160, 296, 1190, 296, "#e65100", 3); s.line(1160, 331, 1190, 331, "#e65100", 3)
    # --- konwerter po stronie modułu ---
    s.rect(1190, 210, 270, 230, "#e8f5e9", "#1b5e20", 2.5, 10)
    s.text(1325, 238, "Konwerter RS-485 (MAX3485)", 15, "#111", "middle", "bold")
    s.text(1325, 258, "auto-kierunek, zasilanie 3,3 V", 12, "#333", "middle")
    s.circle(1190, 296, 4, "#1b5e20"); s.circle(1190, 331, 4, "#1b5e20")
    s.text(1204, 300, "A", 14, "#111", weight="bold")
    s.text(1204, 335, "B", 14, "#111", weight="bold")
    s.text(1204, 420, "terminator 120 ohm A-B", 11, "#555")
    for yy, lab in [(300, "RXD  <- TX modułu"), (325, "TXD  -> RX modułu"), (350, "VCC  3V3"), (375, "GND")]:
        s.circle(1460, yy - 4, 4, "#1b5e20")
        s.text(1446, yy, lab, 12, "#111", "end")
    # --- moduł 7" ---
    s.rect(1560, 140, 380, 520, "#ede7f6", "#311b92", 3, 12)
    s.text(1750, 172, "Moduł 7\" Sunton ESP32-8048S070C", 17, "#311b92", "middle", "bold")
    ports_r = [(250, "GPIO 17  UART1 TX", "#111"), (290, "GPIO 18  UART1 RX", "#111"),
               (330, "3V3 (złącze rozszerzeń)", "#111"), (370, "GND", "#111"), (410, "+5 V (zasilanie płytki)", "#c1121f")]
    for yy, lab, col in ports_r:
        s.circle(1560, yy - 5, 5, "#311b92")
        s.text(1580, yy, lab, 14, col, weight="bold")
    s.text(1580, 470, "GPIO 17/18 są wolne (I2S wzmacniacza i INT dotyku", 12, "#555")
    s.text(1580, 488, "niewykorzystane w projekcie; sprawdzić mostek R17).", 12, "#555")
    s.text(1580, 522, "GPIO 43/44 (UART0) zostają dla USB i programowania.", 12, "#555")
    s.line(1460, 296, 1510, 296, "#1b5e20", 2); s.line(1510, 296, 1510, 285, "#1b5e20", 2); s.line(1510, 285, 1560, 285, "#1b5e20", 2)
    s.line(1460, 321, 1530, 321, "#1b5e20", 2); s.line(1530, 321, 1530, 245, "#1b5e20", 2); s.line(1530, 245, 1560, 245, "#1b5e20", 2)
    s.line(1460, 346, 1490, 346, "#111", 2); s.line(1490, 346, 1490, 325, "#111", 2); s.line(1490, 325, 1560, 325, "#111", 2)
    s.line(1460, 371, 1470, 371, "#111", 2); s.line(1470, 371, 1470, 365, "#111", 2); s.line(1470, 365, 1560, 365, "#111", 2)
    # --- zasilanie i masa przez kabel ---
    s.line(440, 405, 470, 405, "#c1121f", 3); s.line(470, 405, 470, 560, "#c1121f", 3)
    s.line(470, 560, 1540, 560, "#c1121f", 3); s.line(1540, 560, 1540, 405, "#c1121f", 3); s.line(1540, 405, 1560, 405, "#c1121f", 3)
    s.text(1000, 552, "+5 V przez ten sam kabel (żyły +5 V / GND), PTC >= 1 A po stronie sterownika", 12, "#c1121f", "middle", "bold")
    s.line(440, 370, 450, 370, "#111", 2); s.line(450, 370, 450, 600, "#111", 2)
    s.line(450, 600, 1520, 600, "#111", 2); s.line(1520, 600, 1520, 370, "#111", 2); s.line(1520, 370, 1560, 370, "#111", 2)
    s.text(1000, 620, "GND (wspólna masa sygnałowa i zasilania)", 12, "#111", "middle", "bold")
    # --- opis ---
    s.text(40, 700, "Zalety: brak zależności od WiFi (zakłócenia, hasło, limit 4 klientów, czas łączenia), stały niski czas reakcji STOP, zasilanie i dane w jednym kablu.",
           14, "#0b1f4a")
    s.text(40, 726, "Wymagania: nowy moduł komunikacyjny w firmware sterownika i modułu 7\" (rozdz. wdrożenia w LACZE_PRZEWODOWE.md); "
                    "WiFi zostaje jako łącze zapasowe i dla telefonów.", 14, "#0b1f4a")
    s.text(40, 752, "Fizyczny STOP sterownika (panel, pilot, pedał) nadal jest głównym zabezpieczeniem — łącze przewodowe go nie zastępuje.",
           14, "#b71c1c")
    s.save("schemat_lacze_rs485.svg")


# ---------------------------------------------------------------------------
# 6. Komputer kompletny: dwa ekrany + wszystkie przyciski + panel złączy
# ---------------------------------------------------------------------------
def tft_mock(s, x, y, scale):
    """Mały ekran serwisowy ILI9341 320x240: ekran HOME sterownika."""
    s.group_open(f"translate({x},{y}) scale({scale})")
    s.rect(0, 0, 320, 240, "#000")
    s.text(10, 40, "P-3a", 34, "#fff", weight="bold")
    s.text(10, 62, "Przekraczalna dluga", 13, "#c6c6c6")
    s.text(310, 40, "12.5", 34, "#fff", "end", "bold")
    s.text(310, 60, "km/h", 13, "#c6c6c6", "end")
    s.text(310, 80, "148.10 m2", 13, "#c6c6c6", "end")
    s.line(0, 92, 320, 92, "#444", 1)
    s.text(160, 138, "Gotowy", 26, "#35f27a", "middle", "bold")
    s.text(160, 160, "[AUTO]", 14, "#ffd400", "middle")
    for g in range(6):
        on = g in (0, 2)
        s.rect(12 + g * 50, 180, 44, 44, "#07a03a" if on else "#333", "#888", 1)
        s.text(12 + g * 50 + 22, 208, f"P{g + 1}", 14, "#fff", "middle")
    s.group_close()


def badge(s, x, y, n, fill="#c1121f"):
    s.circle(x, y, 13, fill, "#fff", 2)
    s.text(x, y + 5, str(n), 14, "#fff", "middle", "bold")


def complete_computer():
    W, H = 1560, 1420
    s = Svg(W, H, "#f8f9fa")
    s.text(30, 38, "Komputer MPD2026 — widok kompletny: dwa ekrany i wszystkie przyciski", 24, "#0b1f4a", weight="bold")
    s.text(30, 62, "Duży ekran 7\" = praca operatora. Mały ekran 2,8\" = techniczny / serwisowy / awaryjny (+ slot karty SD). "
                   "Układ pionowy (propozycja C), przyciski soft-key 10 + GRUPA.", 14, "#444")
    px, py, pw, ph = 40, 90, 760, 1060
    s.rect(px, py, pw, ph, "#e9ecef", "#495057", 4, 28)
    s.rect(px + 10, py + 10, pw - 20, ph - 20, "#dee2e6", "#adb5bd", 1.5, 22)
    s.text(px + 30, py + 40, "TRASSAR  MPD2026", 17, "#212529", weight="bold")
    # buzzer
    for r in range(3):
        for c in range(5):
            s.circle(px + pw - 190 + c * 9, py + 28 + r * 9, 2, "#6c757d")
    badge(s, px + pw - 205, py + 40, 13)
    # grzybek opcjonalny
    s.add(f'<circle cx="{px + pw - 70}" cy="{py + 42}" r="26" fill="#f4b6b6" stroke="#a01010" stroke-width="3" stroke-dasharray="6 4"/>')
    s.text(px + pw - 70, py + 47, "STOP", 12, "#7a0f0f", "middle", "bold")
    badge(s, px + pw - 100, py + 22, 14, "#7a0f0f")
    # duży ekran pionowy 360x600
    scale = 0.75
    sw, sh = 360, 600
    sx, sy = px + (pw - sw) / 2, py + 76
    screen_bezel(s, sx, sy, sw, sh)
    ui_mock_portrait(s, sx, sy, scale, 0)
    badge(s, sx - 4, sy - 4, 1)
    badge(s, sx + 150, sy + 44, 2, "#1f52d6")
    # klawisze S1-S10 na wysokości etykiet
    kh = 106 * scale
    for i in range(5):
        yy = sy + (106 + i * 116) * scale
        key(s, sx - 8 - 12 - 108, yy, 108, kh, f"S{i + 1}", "◄")
        key(s, sx + sw + 8 + 12, yy, 108, kh, f"S{i + 6}", "►")
        s.line(sx - 20, yy + kh / 2, sx - 8, yy + kh / 2, "#495057", 2)
        s.line(sx + sw + 8, yy + kh / 2, sx + sw + 20, yy + kh / 2, "#495057", 2)
    badge(s, sx - 8 - 12 - 108 + 8, sy + 106 * scale + 6, 3)
    badge(s, sx + sw + 20 + 8, sy + 106 * scale + 6, 4)
    # dolny blok: mały ekran + przyciski
    by = sy + sh + 34
    # mały ekran serwisowy 2,8" (320x240 -> 0.72)
    ts = 0.72
    tw, th = 320 * ts, 240 * ts
    tx, ty = px + 46, by + 8
    s.rect(tx - 12, ty - 12, tw + 24, th + 24, "#111", "#000", 2, 8)
    tft_mock(s, tx, ty, ts)
    badge(s, tx - 10, ty - 10, 6, "#7a4b00")
    s.rect(px + 22, ty + 60, 22, 34, "#adb5bd", "#495057", 2, 3)
    s.text(px + 33, ty + 82, "SD", 11, "#212529", "middle", "bold")
    badge(s, px + 33, ty + 54, 7, "#7a4b00")
    s.text(tx + tw / 2, ty + th + 34, "ekran serwisowy ILI9341 2,8\" (320x240)", 12, "#495057", "middle")
    # przyciski obok małego ekranu
    bx = px + 330
    key(s, bx, by + 6, 190, 62, "GRUPA", "OŚ ⇄ KRAWĘDŹ", "#7a4b00", "#fff", "#ffd400")
    key(s, bx + 210, by + 6, 190, 62, "SELEKTOR", None)
    key(s, bx, by + 88, 190, 62, "GAP", "start od przerwy")
    s.circle(bx + 305, by + 120, 32, "#343a40", "#adb5bd", 3)
    s.circle(bx + 305, by + 120, 12, "#6c757d")
    s.text(bx + 305, by + 168, "joystick (opcja)", 11, "#495057", "middle")
    badge(s, bx + 8, by + 10, 5, "#7a4b00")
    badge(s, bx + 218, by + 10, 8, "#7a4b00")
    badge(s, bx + 8, by + 92, 9, "#7a4b00")
    badge(s, bx + 270, by + 96, 12, "#7a4b00")
    # START / STOP
    ry = by + 200
    s.rect(px + 40, ry, 420, 100, "#1fa34a", "#0b6b2e", 3, 16)
    s.text(px + 250, ry + 65, "START", 34, "#fff", "middle", "bold")
    s.rect(px + 490, ry, 230, 100, "#d62828", "#7a0f0f", 3, 16)
    s.text(px + 605, ry + 65, "STOP", 34, "#fff", "middle", "bold")
    badge(s, px + 52, ry + 14, 10, "#0b6b2e")
    badge(s, px + 502, ry + 14, 11, "#7a0f0f")
    s.text(px + 30, py + ph - 22, "Wymiary orientacyjne. Elementy 1-15 opisane w tabeli po prawej.", 12, "#495057")

    # --- widok od dołu: panel złączy ---
    cy = py + ph + 30
    s.text(px, cy + 4, "Widok od dołu — panel złączy", 15, "#0b1f4a", weight="bold")
    s.rect(px, cy + 14, pw, 190, "#adb5bd", "#495057", 3, 14)
    conns = [
        ("J1", "zasilanie 5 V", "TS13CP03 3 pin", "#c1121f", False),
        ("J2", "zawory P1-P6", "TS17CP10 10 pin", "#e65100", False),
        ("J3", "enkoder", "TS13CP05 5 pin", "#006064", False),
        ("J4", "pilot", "TS13PS06 6 pin", "#880e4f", False),
        ("J5", "pedał", "TS21CP04 4 pin", "#880e4f", False),
        ("J6", "łącze 7\" (opcja)", "M12 A 4 pin", "#311b92", True),
    ]
    cw = (pw - 40) / 6
    for i, (n, fn, typ, col, opt) in enumerate(conns):
        x0 = px + 20 + i * cw
        dash = ' stroke-dasharray="6 4"' if opt else ""
        s.add(f'<circle cx="{x0 + cw / 2 - 6}" cy="{cy + 74}" r="30" fill="#f1f3f5" stroke="{col}" stroke-width="4"{dash}/>')
        s.text(x0 + cw / 2 - 6, cy + 80, n, 17, col, "middle", "bold")
        s.text(x0 + cw / 2 - 6, cy + 128, fn, 11, "#212529", "middle", "bold")
        s.text(x0 + cw / 2 - 6, cy + 144, typ, 10, "#495057", "middle")
    s.text(px + 24, cy + 176, "USB-C sterownika (programowanie) · USB-C modułu 7\" · antena GPS (kabel na zewnątrz) · wentylacja / dławiki", 11, "#212529")
    badge(s, px + 30, cy + 34, 15, "#0b1f4a")

    # --- legenda ---
    lx = 830
    ly = 90
    s.rect(lx, ly, 690, 1060, "#ffffff", "#adb5bd", 2, 12)
    s.text(lx + 20, ly + 32, "Legenda: elementy i ich rola", 17, "#0b1f4a", weight="bold")
    rows = [
        (1, "Ekran roboczy 7\" (dotykowy)", "praca operatora: wzorce z rysunkami w skali, prędkość, droga, liczniki, alarmy, menu", "DUŻY", "#1f52d6"),
        (2, "Zakładki OŚ JEZDNI / KRAWĘDŹ", "dotykowy odpowiednik przycisku GRUPA (wspólna grupa)", "DUŻY", "#1f52d6"),
        (3, "S1–S5 (lewa kolumna)", "wybór wzorca obok etykiety na ekranie; MCP23017 GPA0–GPA4", "przyciski", "#495057"),
        (4, "S6–S10 (prawa kolumna)", "wybór wzorca obok etykiety; MCP23017 GPA5–GPB1", "przyciski", "#495057"),
        (5, "GRUPA", "przełącza OŚ ⇄ KRAWĘDŹ; MCP23017 GPB2", "przyciski", "#7a4b00"),
        (6, "Ekran serwisowy 2,8\" ILI9341", "techniczny/awaryjny: POST, QR i hasło WiFi, menu serwisowe, czyszczenie dysz, SETUP", "MAŁY", "#7a4b00"),
        (7, "Slot karty microSD", "raporty, trasy GPS, kopia ustawień (dostęp z boku obudowy)", "MAŁY", "#7a4b00"),
        (8, "SELEKTOR", "odwróć P-3a/b; w menu nawigacja; długo: Smart/Instant (GPIO 40)", "MAŁY / panel", "#7a4b00"),
        (9, "GAP", "start od przerwy (GPIO 7, równolegle J3/J4)", "panel", "#7a4b00"),
        (10, "START", "start / pauza / wznów; RĘCZNY: trzymaj = strzelaj (GPIO 38)", "panel", "#0b6b2e"),
        (11, "STOP", "zatrzymanie + przerwanie awaryjne, pistolety OFF (GPIO 39)", "panel", "#7a0f0f"),
        (12, "Joystick KY-023 (opcja)", "nawigacja w menu serwisowym małego ekranu (GPIO 19/20/46)", "MAŁY", "#7a4b00"),
        (13, "Buzzer", "sygnały i alarmy (GPIO 8)", "—", "#495057"),
        (14, "Grzybek STOP (opcja)", "styk NO równolegle do STOP; dodatkowe zabezpieczenie", "panel", "#7a0f0f"),
        (15, "Panel złączy J1–J6", "zasilanie, zawory, enkoder, pilot, pedał, łącze do modułu 7\"", "—", "#0b1f4a"),
    ]
    yy = ly + 70
    for n, name, desc, who, col in rows:
        badge(s, lx + 32, yy, n, col)
        s.text(lx + 58, yy - 2, name, 13, "#111", weight="bold")
        s.text(lx + 58, yy + 16, desc, 11, "#444")
        s.rect(lx + 592, yy - 14, 80, 22, "#e4ebf7" if who == "DUŻY" else "#fff3cd" if who.startswith("MAŁY") else "#eee", "#adb5bd", 1, 4)
        s.text(lx + 632, yy + 2, who, 11, "#111", "middle", "bold")
        yy += 46
    yy += 6
    s.text(lx + 20, yy, "Podział ról ekranów", 15, "#0b1f4a", weight="bold")
    s.text(lx + 20, yy + 22, "Duży ekran 7\": praca w terenie, wszystkie funkcje robocze (wzorce, tryby, START/STOP, farba, statystyki, kalibracja).", 12, "#333")
    s.text(lx + 20, yy + 40, "Mały ekran ILI9341: uruchomienie, diagnostyka, serwis, karta SD i praca awaryjna, gdy moduł 7\" lub WiFi nie działa.", 12, "#333")
    s.text(lx + 20, yy + 58, "Sterownik pracuje samodzielnie; fizyczny STOP jest głównym zabezpieczeniem niezależnie od ekranów.", 12, "#b71c1c")
    yy += 92
    s.text(lx + 20, yy, "Liczba przycisków fizycznych", 15, "#0b1f4a", weight="bold")
    s.text(lx + 20, yy + 22, "Na panelu: S1–S10 (10) + GRUPA + START + STOP + SELEKTOR + GAP = 15;  opcjonalnie joystick i grzybek STOP.", 12, "#333")
    s.text(lx + 20, yy + 40, "Zdalne: pilot J4 (START, SELEKTOR, STOP, GAP) i pedał J5 (START, STOP) — równolegle do przycisków panelu.", 12, "#333")
    s.save("komputer_kompletny.svg")



# ---------------------------------------------------------------------------
# 7. Docelowa architektura: sterownik = jedyny mikrokontroler, ekran DGUS (bezposrednio)
# ---------------------------------------------------------------------------
def architecture_target():
    W, H = 2000, 1250
    s = Svg(W, H, "#ffffff")
    s.text(40, 46, "Docelowa architektura MPD2026 — jeden mikrokontroler, ekran inteligentny DWIN DGUS", 26, "#0b1f4a", weight="bold")
    s.text(40, 72, "Cała logika, stan, menu i zapis danych w JEDYNYM mikrokontrolerze (sterownik ESP32-S3). Duży ekran DGUS ma WŁASNY"
                   " procesor i sam renderuje interfejs wg projektu z DGUS Designer — to nie jest nasz firmware ani drugi ESP32."
                   " Bez małego ekranu ILI9341, bez RS-485. Joystick zostaje (nawigacja menu równolegle z dotykiem).", 14, "#444")

    # --- sterownik ---
    s.rect(60, 110, 900, 900, "#e8f0fe", "#0b1f4a", 3, 14)
    s.text(510, 146, "STEROWNIK Trassar — ESP32-S3 N16R8 (JEDYNY mikrokontroler)", 20, "#0b1f4a", "middle", "bold")
    s.text(510, 170, "jedyne źródło prawdy: stan maszyny, logika, menu, ustawienia, dane", 14, "#0b1f4a", "middle")

    # Core 1
    s.rect(90, 200, 400, 330, "#fff", "#1b5e20", 2.5, 10)
    s.text(290, 228, "Core 1 — logika maszyny (bez zmian)", 15, "#1b5e20", "middle", "bold")
    for i, tx in enumerate(["silnik malowania, wzorce, tryby", "sterowanie 6 pistoletami, keepalive 300 ms",
                            "enkoder → dystans / prędkość", "przyciski, ISR awaryjnego STOP",
                            "zabezpieczenia, alarmy, anomalie pistoletów", "statystyki, farba, kalibracja",
                            "dgus_link.cpp: ramki UART do ekranu (nieblokujące)"]):
        s.text(110, 258 + i * 30, "• " + tx, 13, "#222")

    # Core 0 / WWW
    s.rect(520, 200, 410, 330, "#fff", "#0d47a1", 2.5, 10)
    s.text(725, 228, "Core 0 — panel WWW (bez zmian)", 15, "#0d47a1", "middle", "bold")
    for i, tx in enumerate(["serwer HTTP :80 + WebSocket :81", "executeControl() — wspólne dla WWW i ekranu",
                            "panel WWW dla telefonu/laptopa", "raporty CSV, GPX, GeoJSON",
                            "QR i hasło WiFi (na ekranie startowym)"]):
        s.text(540, 258 + i * 30, "• " + tx, 13, "#222")

    # WiFi
    s.rect(90, 560, 400, 110, "#fff", "#4a148c", 2, 10)
    s.text(290, 588, "WiFi AP (bez zmian)", 15, "#4a148c", "middle", "bold")
    s.text(110, 614, "• panel WWW i API dla telefonu / tabletu", 13, "#222")
    s.text(110, 638, "• ekran DGUS NIE korzysta z WiFi (UART)", 13, "#b71c1c")

    # dane
    s.rect(520, 560, 410, 110, "#fff", "#e65100", 2, 10)
    s.text(725, 588, "Dane i pamięć", 15, "#e65100", "middle", "bold")
    s.text(540, 614, "• NVS: ustawienia, kalibracja, statystyki", 13, "#222")
    s.text(540, 638, "• karta SD (osobny moduł): raporty, GPX, log", 13, "#222")

    # piny sterownika
    s.rect(90, 700, 840, 460, "#f8fbff", "#0b1f4a", 2, 10)
    s.text(510, 728, "Piny sterownika po zmianach (27 zajęte, 1 wolny)", 16, "#0b1f4a", "middle", "bold")
    rows = [
        ("Przekaźniki P1–P6", "GPIO 41, 42, 1, 2, 3, 4"),
        ("Enkoder CLK / DT / GAP", "GPIO 5, 6, 7"),
        ("Buzzer", "GPIO 8"),
        ("UART DO EKRANU DGUS (bezpośrednio!)", "TX = GPIO 9, RX = GPIO 10"),
        ("Karta SD (SPI, osobny moduł)", "MOSI 11, SCK 12, MISO 13, CS 16"),
        ("Czujnik temperatury DS18B20 (opcjonalny)", "GPIO 15 (OneWire)"),
        ("I2C: RTC DS1307 + MCP23017 (S1–S10, GRUPA)", "SDA 17, SCL 18"),
        ("Joystick KY-023 (nawigacja menu)", "GPIO 19 (VRx), 20 (VRy), 46 (SW)"),
        ("STOP awaryjny — status pętli (cięcie sprzętowe)", "GPIO 21"),
        ("START / STOP (ISR awaryjny)", "GPIO 38, 39"),
        ("GPS NEO-6M (UART2)", "RX 47, TX 48"),
        ("WOLNE", "GPIO 14 (+40 bez SELEKTORA)"),
        ("Specjalne (unikać)", "GPIO 0, 45 (rozruch), 43/44 (USB), 26–37 (Flash/PSRAM)"),
    ]
    for i, (a, b) in enumerate(rows):
        yy = 758 + i * 30
        col = "#c1121f" if ("UART DO EKRANU" in a or "STOP awaryjny" in a) else ("#1b5e20" if "WOLNE" in a else "#111")
        s.text(110, yy, a, 12.5, col, weight="bold" if col != "#111" else "normal")
        s.text(910, yy, b, 12.5, col, "end", "bold" if col != "#111" else "normal")
        s.line(105, yy + 9, 915, yy + 9, "#dde6f5", 1)

    # łącze
    s.rect(1010, 260, 300, 220, "#fff8e1", "#e65100", 3, 12)
    s.text(1160, 294, "UART BEZPOŚREDNI", 17, "#e65100", "middle", "bold")
    s.text(1160, 320, "115 200 baud, 8N1, 4 przewody", 13, "#333", "middle")
    s.text(1160, 344, "5 V, GND, TX, RX", 13, "#333", "middle")
    s.text(1160, 368, "BEZ RS-485, BEZ konwerterów", 13, "#b71c1c", "middle", "bold")
    s.text(1160, 396, "TX ekranu (5 V) → dzielnik", 13, "#333", "middle")
    s.text(1160, 416, "napięcia → RX sterownika", 13, "#333", "middle")
    s.text(1160, 440, "protokół DGUS (5A A5 …)", 13, "#0b1f4a", "middle", "bold")
    s.text(1160, 460, "zgodny z dokumentacją DWIN", 12, "#333", "middle")
    s.line(960, 380, 1010, 380, "#e65100", 4)
    s.line(1310, 380, 1360, 380, "#e65100", 4)

    # ekran DGUS
    s.rect(1360, 110, 580, 640, "#ede7f6", "#311b92", 3, 14)
    s.text(1650, 146, "WYŚWIETLACZ INTELIGENTNY DWIN DGUS 7\"", 19, "#311b92", "middle", "bold")
    s.text(1650, 170, "np. DMG10600T070_09WTC — WŁASNY procesor, renderuje sam", 13, "#311b92", "middle")
    s.rect(1390, 200, 520, 260, "#fff", "#311b92", 2.5, 10)
    s.text(1650, 228, "Projekt w DGUS Designer (nie nasz kod!)", 15, "#311b92", "middle", "bold")
    for i, tx in enumerate(["strony (Page ID = ScreenID sterownika)", "statyczne etykiety, tła, przyciski, ikony",
                            "sterownik wysyła TYLKO wartości (VP)", "dotyk → kod zdarzenia (VP_TOUCH_EVENT)",
                            "żaden przycisk nie zmienia strony lokalnie"]):
        s.text(1410, 258 + i * 30, "• " + tx, 13, "#222")
    s.rect(1390, 480, 520, 130, "#fff", "#311b92", 2, 10)
    s.text(1650, 508, "Sprzęt ekranu", 15, "#311b92", "middle", "bold")
    s.text(1410, 536, "• 7\" IPS 1024×600, dotyk pojemnościowy", 13, "#222")
    s.text(1410, 560, "• zasilanie 5 V osobnym bezpiecznikiem", 13, "#222")
    s.text(1410, 584, "• brak WiFi, brak karty SD, brak ESP32", 13, "#222")
    s.rect(1390, 630, 520, 100, "#fff3e0", "#e65100", 2, 10)
    s.text(1650, 656, "Zasada bezpieczeństwa", 15, "#b71c1c", "middle", "bold")
    s.text(1410, 680, "• ekran niczym nie steruje — tylko prosi sterownik", 13, "#222")
    s.text(1410, 704, "• fizyczny STOP (panel, pilot, pedał) działa niezależnie", 13, "#222")

    # urządzenia peryferyjne
    s.rect(1010, 520, 300, 190, "#fce4ec", "#880e4f", 2, 10)
    s.text(1160, 550, "Przyciski fizyczne", 16, "#880e4f", "middle", "bold")
    for i, tx in enumerate(["S1–S10 + GRUPA (MCP23017)", "START, STOP, GAP (GPIO)", "joystick KY-023 (nawigacja menu)",
                            "grzybek E-STOP (cięcie sprzętowe, GPIO 21=status)", "pilot J4, pedał J5"]):
        s.text(1030, 586 + i * 26, "• " + tx, 12.5, "#222")
    s.line(960, 615, 1010, 615, "#880e4f", 3)

    s.rect(1010, 750, 300, 130, "#e8f5e9", "#1b5e20", 2, 10)
    s.text(1160, 780, "Wykonawcze i pomiarowe", 15, "#1b5e20", "middle", "bold")
    for i, tx in enumerate(["6 przekaźników → zawory", "enkoder, GPS, buzzer, DS18B20"]):
        s.text(1030, 808 + i * 28, "• " + tx, 13, "#222")

    s.rect(1360, 780, 580, 100, "#f3e5f5", "#4a148c", 2, 10)
    s.text(1650, 806, "Telefon / tablet (opcjonalnie)", 15, "#4a148c", "middle", "bold")
    s.text(1380, 832, "• WiFi sterownika: panel WWW, raporty, API", 13, "#222")
    s.text(1380, 856, "• równolegle do ekranu DGUS, niezależnie od niego", 13, "#222")

    s.text(40, 1200, "Ekran DGUS renderuje LOKALNIE ze swojej własnej pamięci obrazów — sterownik przesyła tylko liczby i teksty"
                     " (adresy VP), nie piksele. To dlatego łącze 115 200 baud wystarcza mimo dużego ekranu.", 13, "#0b1f4a")
    s.text(40, 1223, "Stan: kod sterownika zaimplementowany i skompilowany; projekt ekranu w DGUS Designer jeszcze NIE zbudowany;"
                     " nic nie uruchamiane na sprzęcie — patrz ARCHITEKTURA_TERMINAL.md, rozdz. 11.",
           13, "#b71c1c")
    s.save("architektura_terminal.svg")



if __name__ == "__main__":
    harness()
    rs485()
    wiring()
    wiring_dgus()
    wiring_headless()
    screen_mock()
    panel_A()
    panel_B()
    panel_C()
    panel_D()
    glyph_sheet()
    pattern_icons()
    complete_computer()
    architecture_target()
    from generate_enclosure import enclosure_vertical
    enclosure_vertical(0)
    enclosure_vertical(1)
