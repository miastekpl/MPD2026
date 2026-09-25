# Wizualizacje kontrolera — propozycje graficzne

Grafiki są w formacie SVG (skalowalne, otwierają się w przeglądarce i VS Code) i generowane skryptem
[`schematy/generate_svgs.py`](schematy/generate_svgs.py). Wymiary są **orientacyjne** (do ustalenia przy projekcie obudowy).

Wszystkie propozycje mają tę samą elektronikę: 10 przycisków wzorców **S1–S10** + przycisk **GRUPA** (MCP23017,
[SCHEMAT_PODLACZEN.md](SCHEMAT_PODLACZEN.md) sekcja 4.3), fizyczne **START, STOP, SELEKTOR, GAP** oraz ekran dotykowy 7".
Etykiety wzorców widnieją na ekranie obok przycisków; wzorce są podzielone na **OŚ** (10) i **KRAWĘDŹ** (5 + własny).

## Ekran roboczy (moduł 7")

Makieta ekranu 800×480: prędkość 7-segmentowa, widok drogi, 10 wzorców w bocznych kolumnach, zakładki OŚ / KRAWĘDŹ.

![Ekran roboczy](schematy/ekran_roboczy.svg)

## Propozycja A — układ kabinowy 10 + 1 (poziomy, jak STiM)

![Propozycja A](schematy/panel_A_kabinowy.svg)

- Ekran w środku, **S1–S5 po lewej, S6–S10 po prawej**, etykiety na ekranie dokładnie obok klawiszy.
- GRUPA w narożniku, na dole SELEKTOR, GAP, duże START i STOP, opcjonalny joystick.
- Orientacyjnie ok. 340 × 215 mm.
- **Zalety:** identyczny z obecnym ekranem (zero zmian w oprogramowaniu), klawisze przy etykietach, znany operatorom STiM.
- **Wady:** szeroka obudowa.

## Propozycja B — klawisze w rzędzie pod ekranem (niska obudowa)

![Propozycja B](schematy/panel_B_pas_pod_ekranem.svg)

- Rząd 10 klawiszy S1–S10 pod ekranem; GRUPA, SELEKTOR, GAP po lewej, START i STOP po prawej.
- Orientacyjnie ok. 340 × 190 mm.
- **Zalety:** niski pulpit, wygodny do montażu przed operatorem.
- **Wady:** klawisze poza osią etykiet — potrzebny wariant UI z etykietami wzdłuż dolnej krawędzi ekranu (do wykonania w module 7").

## Propozycja C — panel pionowy (jak na zdjęciu STiM)

![Propozycja C](schematy/panel_C_pionowy.svg)

- Ekran obrócony do pionu (480 × 800), S1–S5 i S6–S10 po bokach, blok START / STOP / GRUPA / SELEKTOR / GAP na dole.
- Orientacyjnie ok. 227 × 327 mm.
- **Zalety:** wąska obudowa, kształt najbardziej zbliżony do zdjęcia referencyjnego.
- **Wady:** wymaga przeróbki interfejsu modułu 7" na tryb pionowy (LVGL); układ drogi i kolumn trzeba przeprojektować.

## Propozycja D — duży grzybek awaryjnego STOP + blok klawiszy 2 × 5

![Propozycja D](schematy/panel_D_estop.svg)

- Czerwony **grzybek STOP** po lewej, klawisze S1–S10 zgrupowane w bloku 2 × 5 po prawej.
- Orientacyjnie ok. 340 × 215 mm.
- **Zalety:** najlepsza widoczność i dostępność STOP, klawisze wzorców zgrupowane.
- **Wady:** większa obudowa; klawisze są dalej od etykiet na ekranie (ekran można przesunąć w stronę bloku klawiszy).
- **Uwaga elektryczna:** firmware traktuje zwarcie GPIO 39 do GND jako naciśnięcie STOP (zbocze opadające). Grzybek
  podłączaj **stykiem NO równolegle do przycisku STOP** (jak pilot J4 i pedał J5). Grzybek zatrzaskowy pozostawałby
  w stanie „wciśnięty" — zachowanie przy trwale zwartym STOP (blokada START) wymaga sprawdzenia na sprzęcie, zanim
  zastosujesz zatrzask.

## Porównanie

| | A kabinowy | B pas pod ekranem | C pionowy | D grzybek + blok |
|---|-----------|--------------------|-----------|------------------|
| Przyciski fizyczne (S + GRUPA + START/STOP/SEL/GAP) | 15 | 15 | 15 | 15 (STOP jako grzybek) |
| Zmiany w oprogramowaniu | brak | wariant UI (etykiety u dołu) | tryb pionowy UI | brak |
| Szerokość / wysokość (orient.) | 340 × 215 mm | 340 × 190 mm | 227 × 327 mm | 340 × 215 mm |
| Czytelność etykiet obok klawiszy | najlepsza | średnia | najlepsza | dobra |
| Dostępność STOP | dobra | dobra | dobra | najlepsza |
| Ryzyko | niskie | średnie | wysokie (UI) | niskie |

**Rekomendacja:** zacząć od **A** (nie wymaga żadnych zmian w oprogramowaniu) i rozważyć **grzybek STOP z D** jako
element bezpieczeństwa niezależnie od wybranego układu. Wariant **C** ma sens dopiero jako osobny etap (przeróbka UI).

## Regeneracja grafik

```bash
python docs/schematy/generate_svgs.py
```

Skrypt tworzy w katalogu `docs/schematy/`: `schemat_polaczen.svg` (schemat elektryczny), `ekran_roboczy.svg`
i cztery propozycje `panel_*.svg`. Piny w schemacie odpowiadają `src/config.h`.
