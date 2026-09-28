# Ikony wzorców do DGUS Designer

16 gotowych obrazków (`.png`, tło `#1b1b1f` — ciemne, jak reszta interfejsu), po jednym na każdy wzorzec.
Wygenerowane skryptem `docs/schematy/generate_svgs.py` (funkcja `pattern_icons()`), z tej samej funkcji
rysującej `glyph()`, która produkuje rysunki wzorców w `wzorce_w_skali.svg` i w makietach ekranu — więc
wyglądają identycznie jak wszędzie indziej w dokumentacji.

## Kolejność = indeks `PatternID` (src/config.h)

Nazwa pliku zaczyna się od dwucyfrowego indeksu, dokładnie w kolejności enuma `PatternID`:

| Plik | Indeks | Wzorzec |
|------|--------|---------|
| `00_P-1a.png` | 0 | P-1a Przerywana długa |
| `01_P-1b.png` | 1 | P-1b Przerywana krótka |
| `02_P-1c.png` | 2 | P-1c Wydzielająca |
| `03_P-1d.png` | 3 | P-1d Prowadząca wąska |
| `04_P-1e.png` | 4 | P-1e Prowadząca szeroka |
| `05_P-2a.png` | 5 | P-2a Ciągła wąska |
| `06_P-2b.png` | 6 | P-2b Ciągła szeroka |
| `07_P-3a.png` | 7 | P-3a Przekraczalna długa |
| `08_P-3b.png` | 8 | P-3b Przekraczalna krótka |
| `09_P-4.png`  | 9 | P-4 Podwójna ciągła |
| `10_P-6.png`  | 10 | P-6 Ostrzegawcza (z poboczem) |
| `11_P-7a.png` | 11 | P-7a Krawędziowa przeryw. szeroka |
| `12_P-7b.png` | 12 | P-7b Krawędziowa ciągła szeroka |
| `13_P-7c.png` | 13 | P-7c Krawędziowa przeryw. wąska |
| `14_P-7d.png` | 14 | P-7d Krawędziowa ciągła wąska |
| `15_WLASNY.png` | 15 | Wzorzec WŁASNY (użytkownika) |

Ta kolejność musi się zgadzać z tym, co sterownik wysyła pod `VP_SLOT_PATIDX_BASE` (0x1100+i) —
patrz `docs/EKRAN_DGUS.md`, sekcja „Kolumny wzorców S1–S10".

## Jak zaimportować w DGUS Designer

1. Otwórz projekt, wejdź do menedżera obrazów/ikon (zwykle „Picture" / „ICON" w bibliotece zasobów).
2. Utwórz nową grupę ikon (variable icon group) i zaimportuj pliki **w tej kolejności** (00 → 15) —
   Designer numeruje je automatycznie 0, 1, 2… w kolejności importu, więc kolejność ma znaczenie.
3. Na stronie roboczej (kafelek slotu S1–S10, strona 0/1) dodaj kontrolkę „ikona wariantowa" (variable icon),
   powiąż ją z adresem `VP_SLOT_PATIDX_BASE + i` (jedna kontrolka na slot, i = 0..9) — Designer sam pokaże
   właściwy obrazek z biblioteki na podstawie wartości pod tym adresem.
4. Tło ikony jest już ciemne (`#1b1b1f`) — kafelek slotu w tle NIE musi mieć własnego wypełnienia w tym samym
   miejscu, w przeciwnym razie zobaczysz podwójną ramkę. Podświetlenie wybranego slotu (VP_SLOT_SEL_BASE)
   najlepiej zrobić jako osobną ramkę/obwódkę WOKÓŁ ikony, nie jako zmianę tła pod nią.

## Format i rozmiar

200×240 px, PNG (bez przezroczystości — pełne tło). Jeśli w Designerze kafelek wychodzi wyraźnie mniejszy
lub większy niż te proporcje (5:6), przeskaluj przy wstawianiu — źródłowe pliki `.svg` (ten sam folder) mają
te same proporcje i można je łatwo wyeksportować w innej rozdzielczości, jeśli PNG okaże się za małe/za duże
(regeneracja: `python docs/schematy/generate_svgs.py`, zmień `W, H` w `pattern_icons()`).
