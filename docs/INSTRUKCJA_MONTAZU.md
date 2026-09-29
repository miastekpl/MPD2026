# MPD2026 — Instrukcja montażu

Instrukcja krok po kroku dla osoby montującej komputer pokładowy **od zera**: kolejność prac, narzędzia,
bezpieczeństwo i checklisty. Dane elektryczne (piny, prądy, schematy) są w [SCHEMAT_PODLACZEN.md](SCHEMAT_PODLACZEN.md)
i na diagramach SVG — ten dokument mówi **w jakiej kolejności** i **na co uważać**, nie powtarza tabel pinów.

**Architektura, którą montujesz:** sterownik **bez żadnego ekranu podłączonego bezpośrednio** ("headless") +
moduł wyświetlacza **Sunton 7"**, połączony **łączem przewodowym UART** (priorytet — patrz
[LACZE_PRZEWODOWE.md](LACZE_PRZEWODOWE.md)) **oraz WiFi** (automatyczny fallback, gdy kabel odłączony, i jedyne
łącze dla telefonu/tabletu). To jest wariant **docelowy**. Joystick fizyczny na panelu sterownika jest zachowany.

## Spis treści

1. [Zakres i założenia](#1-zakres-i-założenia)
2. [Narzędzia i materiały](#2-narzędzia-i-materiały)
3. [Bezpieczeństwo podczas montażu](#3-bezpieczeństwo-podczas-montażu)
4. [Krok 1 — Obudowa i mocowanie](#krok-1--obudowa-i-mocowanie)
5. [Krok 2 — Elektronika wewnątrz obudowy](#krok-2--elektronika-wewnątrz-obudowy)
6. [Krok 3 — Zasilanie](#krok-3--zasilanie)
7. [Krok 4 — Przekaźniki i zawory](#krok-4--przekaźniki-i-zawory)
8. [Krok 5 — Enkoder dystansu](#krok-5--enkoder-dystansu)
9. [Krok 6 — GPS](#krok-6--gps)
10. [Krok 7 — STOP awaryjny (E-STOP)](#krok-7--stop-awaryjny-e-stop)
11. [Krok 8 — Przyciski wzorców (MCP23017)](#krok-8--przyciski-wzorców-mcp23017)
12. [Krok 9 — Pilot i pedał (opcjonalnie)](#krok-9--pilot-i-pedał-opcjonalnie)
13. [Krok 10 — Moduł wyświetlacza Sunton 7"](#krok-10--moduł-wyświetlacza-sunton-7)
14. [Krok 11 — Pierwsze uruchomienie](#krok-11--pierwsze-uruchomienie)
15. [Krok 12 — Parowanie WiFi i kalibracja](#krok-12--parowanie-wifi-i-kalibracja)
16. [Checklist końcowy](#checklist-końcowy)
17. [Częste błędy montażowe](#częste-błędy-montażowe)

---

## 1. Zakres i założenia

Montujesz dwa niezależne urządzenia:

| Urządzenie | Gdzie w kabinie | Połączenie ze sterownikiem |
|---|---|---|
| **Sterownik** (ESP32-S3, w obudowie pionowej) | blisko przekaźników/zaworów — krótsze przewody mocy | — (to on jest centrum) |
| **Moduł Sunton 7"** (osobna obudowa/ramka) | w zasięgu wzroku operatora | **kabel UART (priorytet) + WiFi (fallback)** |

Obudowa sterownika: [WIZUALIZACJE.md](WIZUALIZACJE.md), rysunki
[obudowa_pionowa_os.svg](schematy/obudowa_pionowa_os.svg) / [obudowa_pionowa_krawedz.svg](schematy/obudowa_pionowa_krawedz.svg)
(ten sam sprzęt, różni się tylko tym, co akurat pokazuje ekran — nie ma to wpływu na montaż).
Pełny schemat elektryczny: [schemat_polaczen_docelowy.svg](schematy/schemat_polaczen_docelowy.svg).

> Jeżeli budujesz alternatywę z ekranem DGUS na przewodzie (nie zalecane, wymaga DGUS Designer) — ta instrukcja
> nadal obowiązuje dla kroków 1–9, ale krok 10 zastępujesz podłączeniem ekranu DGUS wg
> [ARCHITEKTURA_TERMINAL.md](ARCHITEKTURA_TERMINAL.md) i schematu [schemat_polaczen_dgus.svg](schematy/schemat_polaczen_dgus.svg).

## 2. Narzędzia i materiały

**Narzędzia:**
- Lutownica + cyna (albo złączki skręcane/WAGO, jeśli wolisz bezlutowo)
- Multimetr (obowiązkowy — do sprawdzenia napięć **przed** pierwszym uruchomieniem)
- Wkrętak precyzyjny (obudowa, złącza)
- Ściągacz izolacji, opaski zaciskowe
- Komputer z PlatformIO (do wgrania firmware) i kablem USB-C

**Materiały (BOM pełny w [SCHEMAT_PODLACZEN.md](SCHEMAT_PODLACZEN.md) i [WIZUALIZACJE.md](WIZUALIZACJE.md)):**
ESP32-S3 N16R8 DevKitC-1, moduł Sunton ESP32-8048S070C, moduł przekaźników 6 kanałów, enkoder KY-040,
GPS GY-NEO6MV2, RTC DS1307 + bateria CR2032, ekspander MCP23017, moduł microSD (osobny) + karta microSD (FAT32),
buzzer pasywny 5 V, czujnik DS18B20 (opcjonalnie), przycisk grzybkowy E-STOP (styk NC), przewody, złącza
maszynowe J1–J5, przetwornica DC-DC 12/24 V → 5 V (min. 3 A), bezpieczniki/PTC.

## 3. Bezpieczeństwo podczas montażu

> **Cała elektronika montowana i lutowana przy ODŁĄCZONYM zasilaniu.** Podłączaj zasilanie dopiero na końcu
> (krok 11), po sprawdzeniu wszystkiego multimetrem.

- Przewód zasilania **12/24 V od strony akumulatora** podłączaj jako **ostatnią** czynność, z odłączonym biegunem
  do momentu zakończenia montażu.
- Sprawdź polaryzację **przed** podłączeniem przetwornicy DC-DC — odwrotna polaryzacja niszczy elektronikę
  natychmiast i nieodwracalnie.
- Masa zaworów (COM przekaźników) to **minus instalacji maszyny**, NIE masa (GND) ESP32 — pomylenie tych dwóch
  mas jest częstym błędem i może uszkodzić sterownik (patrz [schemat_polaczen_docelowy.svg](schematy/schemat_polaczen_docelowy.svg),
  czerwone adnotacje przy module przekaźników).
- Grzybek E-STOP: zamontuj i okabluj **przed** pierwszym testem przekaźników, nie po.

## Krok 1 — Obudowa i mocowanie

1. Zmontuj/wydrukuj obudowę wg [WIZUALIZACJE.md](WIZUALIZACJE.md) (tabela wycięć w rysunkach
   `obudowa_pionowa_*.svg`, pozycje 1–11).
2. Zamocuj obudowę na uchwycie VESA 75 (4× M5) — pozycja 9 w tabeli wycięć.
3. Wywierć/przygotuj otwór na grzybek E-STOP w **prawej ściance bocznej** (~90 mm od góry, Ø22 mm) —
   pozycja 11. Zrób to teraz, zanim wnętrze się zapełni przewodami — dostęp później jest trudniejszy.
4. Zamontuj odpowietrznik (membrana Gore) w bocznej ściance — pozycja 10.
5. Przygotuj otwór na slot karty microSD z boku (dostęp bez otwierania obudowy) — pozycja 7.

## Krok 2 — Elektronika wewnątrz obudowy

1. Zamocuj płytę ESP32-S3 DevKitC-1 wewnątrz obudowy (patrz rozmieszczenie w
   [SCHEMAT_PODLACZEN.md](SCHEMAT_PODLACZEN.md) rozdz. 4, widok od tyłu w `obudowa_pionowa_*.svg`).
2. Zamontuj moduł microSD **jako osobny moduł** (nie dzieli już magistrali z żadnym ekranem) — 4 przewody:
   MOSI (GPIO 11), SCK (GPIO 12), MISO (GPIO 13), CS (GPIO 16), plus 3V3/GND.
3. Zamontuj RTC DS1307 i włóż baterię CR2032 — magistrala I2C: SDA (GPIO 17), SCL (GPIO 18), zasilanie 5 V.
4. Zamontuj MCP23017 na **tej samej** magistrali I2C (adres 0x20, A0=A1=A2=GND), zasilanie 3V3.
5. Zamontuj buzzer pasywny — GPIO 8, zasilanie 5 V.
6. **Opcjonalnie:** czujnik DS18B20 — GPIO 15 (OneWire), rezystor podciągający 4,7 kΩ do 3V3. Ten pin jest
   zarezerwowany w firmware **niezależnie od tego, czy czujnik jest fizycznie zamontowany** — jeśli go
   pomijasz, po prostu zostaw GPIO 15 niepodłączony.

Nie podłączaj jeszcze niczego do zasilania 5 V/12 V — to krok 3.

## Krok 3 — Zasilanie

1. Zamontuj przetwornicę DC-DC 12/24 V → 5 V (min. 3 A) w pobliżu złącza J1.
2. Zamontuj bezpiecznik/PTC (1,5 A) i warystor/TVS na linii +5 V zaraz za przetwornicą — **przed**
   rozgałęzieniem do ESP32, przekaźników i RTC.
3. Przygotuj **osobny odczep 5 V** (własny bezpiecznik/PTC ≥ 1 A) do zasilania modułu Sunton — moduł 7"
   NIE jest zasilany z tej samej gałęzi co logika sterownika, żeby jego pobór prądu (0,3–0,5 A) nie wpływał
   na stabilność ESP32.
4. **Jeszcze nie podłączaj** do akumulatora/instalacji maszyny — to ostatni krok (11).

Pełny bilans prądu i schemat zasilania: [SCHEMAT_PODLACZEN.md](SCHEMAT_PODLACZEN.md) rozdz. 6.

## Krok 4 — Przekaźniki i zawory

1. Zamontuj moduł przekaźników 6-kanałowy. Wejścia sterujące (IN1–IN6) do GPIO: 41, 42, 1, 2, 3, 4
   (P1–P6, patrz tabela w schemacie połączeń — kolejność ma znaczenie, IN1=P1 oś lewy itd.).
2. Zasilanie modułu przekaźników (VCC): 5 V z linii logiki (razem z ESP32).
3. **COM przekaźników (J2/7, J2/8) podłącz do +12/24 V instalacji maszyny** — NIE do 5 V logiki.
4. Wyjścia NO1–NO6 → złącze J2 → zawory pistoletów P1–P6.
5. **Masa zaworów = minus instalacji maszyny.** Nie łącz jej z GND ESP32.

## Krok 5 — Enkoder dystansu

1. Zamontuj enkoder KY-040 na kole pomiarowym — musi się swobodnie obracać, dociskane do jezdni.
2. Podłącz skrętką (żeby ograniczyć zakłócenia, maks. 2 m): CLK → GPIO 5, DT → GPIO 6, SW → GPIO 7 (to też
   wejście GAP — start od przerwy).
3. Dodaj kondensator 100 nF między każdą linię sygnałową a GND, blisko wejścia ESP32 (filtr przeciwzakłóceniowy).
4. Kalibrację (dokładny przelicznik impulsów na metr) robisz na końcu, w kroku 12 — teraz tylko podłącz.

## Krok 6 — GPS

1. Zamontuj antenę GPS **na zewnątrz kabiny** (widok nieba) — wewnątrz metalowej kabiny GPS nie złapie fixa.
2. Podłącz moduł GY-NEO6MV2: TX modułu → GPIO 47 (RX sterownika), RX modułu → GPIO 48 (TX sterownika),
   zasilanie 3V3.
3. Pierwsze uzyskanie fixa po włączeniu (zimny start) trwa 30–60 s — to normalne, nie oznacza usterki.

## Krok 7 — STOP awaryjny (E-STOP)

To jedyny element, który **fizycznie tnie zasilanie** pistoletów/pomp — najważniejsze połączenie w całym
montażu z punktu widzenia bezpieczeństwa. Rób to uważnie i przetestuj multimetrem przed uruchomieniem.

1. Zamontuj grzybek E-STOP (styk **NC** — normalnie zamknięty) w otworze przygotowanym w kroku 1
   (prawa ścianka boczna).
2. **Tor mocy:** wepnij styk NC grzybka **w serii** między +12/24 V instalacji a wejściem COM modułu
   przekaźników (z kroku 4) — tak, żeby wciśnięcie grzybka fizycznie przerywało ten obwód, niezależnie od
   tego, czy sterownik działa, zawiesił się, czy jest wyłączony.
3. **Pętla statusu (osobna, równoległa):** dołącz dodatkowe dwa przewody z tego samego styku NC do GPIO 21
   i do GND sterownika. To wejście z wewnętrznym pull-up — pętla zamknięta czyta się jako LOW (stan normalny),
   otwarta (grzybek wciśnięty **lub przerwany przewód**) jako HIGH (alarm). Zerwanie przewodu też uruchamia
   alarm — to celowe (fail-safe).
4. Sprawdź multimetrem **przed podłączeniem zasilania**: rezystancja między stykami grzybka w stanie
   spoczynkowym powinna być bliska 0 Ω, po wciśnięciu — nieskończoność (obwód otwarty).

Szczegóły elektryczne i zasada działania: [ARCHITEKTURA_TERMINAL.md](ARCHITEKTURA_TERMINAL.md) rozdz. 7.

## Krok 8 — Przyciski wzorców (MCP23017)

1. Podłącz 10 przycisków S1–S10 + GRUPA do MCP23017 (GPA0–GPA7, GPB0–GPB2) — każdy przycisk zwiera do GND
   (wewnętrzny pull-up, nie potrzeba zewnętrznych rezystorów).
2. Podłącz przyciski **START** (GPIO 38) i **STOP** (GPIO 39) bezpośrednio do ESP32 (nie przez MCP23017) —
   to jest fizycznie inny, niezależny obwód, celowo (STOP musi działać nawet gdyby I2C padł).
3. **Opcjonalnie:** SELEKTOR (GPIO 40) — jeśli go pomijasz, zostaw pin niepodłączony; odwracanie P-3a/b jest
   też dostępne dotykiem na module Sunton.

## Krok 9 — Pilot i pedał (opcjonalnie)

Pilot (złącze J4) i pedał (J5) są **elektrycznie równoległe** do przycisków fizycznych (START/STOP/SELEKTOR/GAP)
— podłączasz je do **tych samych** linii GPIO co odpowiadające przyciski, nie do osobnych pinów. Sterownik nie
rozróżnia, czy sygnał przyszedł z panelu, pilota czy pedału.

## Krok 10 — Moduł wyświetlacza Sunton 7" i łącze przewodowe

Moduł Sunton łączy się ze sterownikiem **kablem UART** (priorytet — patrz
[LACZE_PRZEWODOWE.md](LACZE_PRZEWODOWE.md)) i zasilaniem; WiFi (krok 12) jest automatycznym łączem zapasowym,
uruchamianym gdy kabel jest odłączony, oraz jedynym łączem dla telefonu/tabletu w panelu WWW.

1. Zamontuj moduł Sunton w miejscu widocznym dla operatora (osobna obudowa/ramka, niezależnie od obudowy
   sterownika).
2. Podłącz zasilanie 5 V z **osobnego odczepu** przygotowanego w kroku 3 (nie z linii logiki sterownika).
3. Podłącz kabel danych (skrętka, 2 żyły + wspólna masa, do kilku metrów bez konwerterów RS-485 — patrz
   [LACZE_PRZEWODOWE.md](LACZE_PRZEWODOWE.md) §2/§4 dla dłuższych odcinków):
   - **Sterownik GPIO 9 (TX) → moduł Sunton GPIO 18 (RX)**
   - **Sterownik GPIO 10 (RX) ← moduł Sunton GPIO 17 (TX)**
   - GND wspólna (może iść tym samym kablem co zasilanie 5 V modułu).
4. Poprowadź kabel danych **osobno** od przewodów mocy zaworów/przekaźników (patrz [SCHEMAT_PODLACZEN.md](SCHEMAT_PODLACZEN.md)
   §9.1) — ogranicza to zakłócenia, choć UART działa również bez tego na krótkich odcinkach.
5. Po pierwszym uruchomieniu (krok 11) sprawdź na ekranie Sunton wskaźnik **„KABEL"** w pasku górnym — jeśli
   pokazuje **„POLACZONO"** (WiFi), sprawdź okablowanie z punktu 3 (styk, kolejność TX/RX, wspólna masa).
   Parowanie WiFi (na wypadek odłączenia kabla) robisz w kroku 12.

## Krok 11 — Pierwsze uruchomienie

1. **Jeszcze raz sprawdź multimetrem** przed podłączeniem: polaryzację zasilania, brak zwarć na +5 V do GND,
   ciągłość pętli E-STOP (krok 7.4).
2. Wgraj firmware sterownika: `pio run -e esp32s3 -t upload` (środowisko domyślne — headless + Sunton).
3. Wgraj firmware modułu Sunton: `cd display-module && pio run -t upload`.
4. Podłącz zasilanie 12/24 V od strony akumulatora — **ostatnia czynność montażu**.
5. Sterownik nie ma ekranu, więc diagnostyka startowa (POST) trafia **do portu szeregowego (USB)**: podłącz
   laptop, otwórz monitor portu (115200 baud, `pio device monitor`), sprawdź linie `[POST]` — status karty SD,
   RTC, GPS, MCP23017, enkodera, temperatury, oraz **SSID i hasło WiFi** (potrzebne w następnym kroku).
6. Jeśli którykolwiek moduł pokazuje błąd — sprawdź jego połączenie z odpowiedniego kroku wyżej, zanim
   pójdziesz dalej.

## Krok 12 — Parowanie WiFi i kalibracja

1. Włącz moduł Sunton — pokaże **BRAK ŁĄCZNOŚCI ZE STEROWNIKIEM** i przycisk **USTAW POŁĄCZENIE WiFi**.
2. Wpisz hasło odczytane z logu USB w kroku 11.5, zapisz. Moduł zapamiętuje hasło na stałe — kolejne
   uruchomienia łączą się automatycznie.
3. Skalibruj enkoder: **MENU → KALIBRACJA** na module Sunton, przejedź dokładnie zmierzony odcinek (zalecane
   10 m), potwierdź. Bez tego długości kresek/przerw będą błędne — patrz [INSTRUKCJA_OBSLUGI.md](INSTRUKCJA_OBSLUGI.md)
   rozdz. 8.

## Checklist końcowy

- [ ] Multimetr: brak zwarć, poprawna polaryzacja, pętla E-STOP ciągła w spoczynku
- [ ] POST przez USB: SD OK, RTC OK, MCP OK (GPS może pokazywać brak fixa — normalne na start)
- [ ] Moduł Sunton połączony kablem (wskaźnik „KABEL" u góry ekranu; WiFi tylko jako zapasowe)
- [ ] Enkoder skalibrowany
- [ ] Test E-STOP: wciśnięcie grzybka **natychmiast** odcina napięcie na wyjściach przekaźników (zmierz
      multimetrem albo obserwuj zawory) — zrób to **przed** pierwszym wyjazdem w teren
- [ ] Test fizycznego STOP (GPIO 39) niezależnie od E-STOP
- [ ] Test każdego z 6 pistoletów pojedynczo (np. przez czyszczenie dysz w menu Serwis)
- [ ] Antena GPS na zewnątrz, docelowo złapany fix
- [ ] Karta microSD włożona, dostępna bez otwierania obudowy
- [ ] Wszystkie przewody mocy (zawory, zasilanie) odseparowane od przewodów sygnałowych (enkoder, I2C)

## Częste błędy montażowe

| Objaw | Najczęstsza przyczyna |
|---|---|
| Sterownik się nie uruchamia / restartuje w pętli | Odwrotna polaryzacja zasilania albo brak PTC/bezpiecznika — sprawdź multimetrem **przed** ponownym podłączeniem |
| Zawory strzelają losowo / migają | Wspólna masa zaworów i GND ESP32 (powinny być rozdzielone — masa zaworów to minus instalacji) |
| MCP/RTC „BŁĄD" w diagnostyce startowej | Przerwa na magistrali I2C, zła adresacja (A0/A1/A2 nie do GND), brak zasilania 3V3 |
| Enkoder pokazuje losowe skoki dystansu | Brak kondensatorów 100 nF, zbyt długi przewód (>2 m), przewód nieskrętkowy |
| E-STOP nie odcina zasilania | Grzybek wpięty tylko w pętlę statusu (GPIO 21), a nie fizycznie w tor mocy — wróć do kroku 7.2 |
| Moduł Sunton pokazuje „POLACZONO" (WiFi) zamiast „KABEL" | Sprawdź okablowanie kroku 10.3: TX/RX zamienione miejscami, brak wspólnej masy, przerwa w kablu |
| Moduł Sunton nie widzi sterownika wcale (ani kablem, ani WiFi) | Kabel: sprawdź krok 10.3. WiFi (zapasowe): zbyt duża odległość / przeszkody dla 2,4 GHz; sprawdź hasło (wielkość liter nie ma znaczenia) |
| Brak fixa GPS przez długi czas | Antena wewnątrz metalowej kabiny — musi mieć widok nieba |

---

Powiązane dokumenty: [SCHEMAT_PODLACZEN.md](SCHEMAT_PODLACZEN.md) (piny, schematy modułów, zasilanie),
[WIZUALIZACJE.md](WIZUALIZACJE.md) (obudowa, wycięcia), [INSTRUKCJA_OBSLUGI.md](INSTRUKCJA_OBSLUGI.md) (obsługa
po montażu), [ARCHITEKTURA_TERMINAL.md](ARCHITEKTURA_TERMINAL.md) (szczegóły E-STOP i alternatywa DGUS).
