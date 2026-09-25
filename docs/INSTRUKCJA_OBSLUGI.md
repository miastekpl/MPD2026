# MPD2026 — Instrukcja obsługi

**Komputer pokładowy malowarki pasów drogowych** — sterownik Trassar (firmware 2.52.0) i moduł
wyświetlacza 7" (display-module 0.1.0).

## Spis treści

1. [Zasada działania i podział urządzeń](#1-zasada-działania-i-podział-urządzeń)
2. [Szybki start — pierwsze uruchomienie](#2-szybki-start--pierwsze-uruchomienie)
3. [Moduł wyświetlacza 7" — obsługa](#3-moduł-wyświetlacza-7--obsługa)
4. [Sterownik — panel fizyczny](#4-sterownik--panel-fizyczny)
5. [Wzorce malowania](#5-wzorce-malowania)
6. [Tryby pracy](#6-tryby-pracy)
7. [Start od przerwy](#7-start-od-przerwy)
8. [Kalibracja enkodera](#8-kalibracja-enkodera)
9. [Farba i zbiornik](#9-farba-i-zbiornik)
10. [Zabezpieczenia, alarmy i sygnały dźwiękowe](#10-zabezpieczenia-alarmy-i-sygnały-dźwiękowe)
11. [Raporty, GPS, statystyki, kopia zapasowa](#11-raporty-gps-statystyki-kopia-zapasowa)
12. [Przykłady krok po kroku](#12-przykłady-krok-po-kroku)
13. [Rozwiązywanie problemów](#13-rozwiązywanie-problemów)
14. [BHP i konserwacja](#14-bhp-i-konserwacja)
15. [Dane techniczne](#15-dane-techniczne)

---

## 1. Zasada działania i podział urządzeń

System składa się z **dwóch urządzeń**:

| Urządzenie | Rola | Ważne |
|------------|------|-------|
| **Sterownik** (ESP32-S3, mały ekran 2,8", przyciski fizyczne) | Steruje 6 pistoletami, mierzy dystans i prędkość, zapisuje raporty. **Jedyne urządzenie sterujące pistoletami.** | Działa samodzielnie, także bez modułu 7". |
| **Moduł wyświetlacza 7"** (ekran dotykowy 800×480) | Duży, czytelny panel operatora: podgląd na żywo, wybór wzorców, tryby, START/STOP, ustawienia. | Łączy się ze sterownikiem przez WiFi. Nie steruje pistoletami bezpośrednio. |

Trzeci sposób obsługi to **panel WWW** (telefon/laptop, `http://192.168.4.1`) — pełny opis w [API_WWW.md](API_WWW.md).

> **ZASADA BEZPIECZEŃSTWA:** przycisk STOP na module 7" działa przez WiFi. **Fizyczny STOP na sterowniku
> (panel, pilot, pedał) jest głównym zabezpieczeniem** i działa zawsze — niezależnie od WiFi.

---

## 2. Szybki start — pierwsze uruchomienie

### Krok 1 — Sprawdź montaż
- Karta MicroSD (FAT32) włożona do slotu wyświetlacza sterownika.
- Bateria CR2032 w module RTC.
- Koło pomiarowe z enkoderem obraca się swobodnie; antena GPS wyniesiona na zewnątrz kabiny.
- Zasilanie 5 V (min. 3 A) podłączone do J1; moduł 7" zasilony z osobnego odgałęzienia 5 V.

### Krok 2 — Włącz sterownik
1. Po uruchomieniu pojawia się ekran powitalny, a następnie **ekran WiFi z kodem QR**. Zapisz z niego:
   - **SSID:** `TrassarV3`
   - **Hasło:** 8 znaków (np. `A1B2C3D4`) — unikalne dla każdego sterownika, generowane z jego adresu MAC
   - **Adres panelu:** `http://192.168.4.1`
2. Naciśnij **START** (lub poczekaj), aby przejść dalej. Ekran **POST** pokaże stan modułów:

| Pozycja | Znaczenie |
|---------|-----------|
| SD: OK / FAIL | karta SD |
| RTC: OK / FAIL | zegar |
| GPS: BRAK | normalne na początku (zimny start 30–60 s) |
| MCP: OK / FAIL | ekspander przycisków wzorców |
| ENK: Domyślny | enkoder wymaga kalibracji |

3. Ekran **HOME** oznacza gotowość.

### Krok 3 — Połącz moduł 7" ze sterownikiem
1. Włącz moduł 7". Pierwszy raz zobaczysz napis **BRAK ŁĄCZNOŚCI ZE STEROWNIKIEM** i przycisk
   **USTAW POŁĄCZENIE WiFi**.
2. Naciśnij go (albo **MENU → POŁĄCZENIE WiFi**), wpisz **8-znakowe hasło** ze sterownika na klawiaturze
   ekranowej (wielkość liter nie ma znaczenia — moduł zamienia na wielkie) i naciśnij **ZAPISZ I POŁĄCZ**.
3. W górnym pasku pojawi się zielone **POŁĄCZONO**, a ekran ożyje. Hasło jest zapamiętane — przy kolejnych
   uruchomieniach moduł łączy się sam.

### Krok 4 — Skalibruj enkoder (OBOWIĄZKOWO przed pierwszym malowaniem)
Patrz [sekcja 8](#8-kalibracja-enkodera). Bez kalibracji długości kresek i przerw będą błędne.

### Krok 5 — Pierwsze malowanie
1. Naciśnij wzorzec (np. **P-1a**) w kolumnie po bokach.
2. Naciśnij zielony **START**.
3. Ruszaj — po przekroczeniu **3 km/h** pistolety zaczną malować automatycznie.
4. **STOP** kończy pracę i zapisuje raport na karcie SD.

---

## 3. Moduł wyświetlacza 7" — obsługa

Ekran jest dotykowy. Wszystkie przyciski są duże (obsługa w rękawicach); ekran jest zoptymalizowany pod
wysoki kontrast w słońcu.

### 3.1 Ekran roboczy

Makieta graficzna ekranu: [schematy/ekran_roboczy.svg](schematy/ekran_roboczy.svg).

```
┌────────────────────────────────────────────────────────────────────────────┐
│ ⌂POŁĄCZONO  GPS 8 sat  [▓▓▓▓░ FARBA 72%]  [ OŚ ][KRAWĘDŹ]      [ MENU ]  │ ← pasek górny
├───────────┬────────────────────────────────────────────────────┬───────────┤
│ S1 P-1a   │  12.5 km/h                                   P-3a   │ S6 P-2a   │
│ S2 P-1b   │  (duże cyfry 7-segmentowe)         AUTO  MALOWANIE  │ S7 P-2b   │
│ S3 P-1c   │  DYST 1234.5 m   POW 148.1 m2   CZAS 12:34         │ S8 P-3a ◄ │
│ S4 P-1d   │ ┌───────────── widok drogi w perspektywie ───────┐ │ S9 P-3b   │
│ S5 P-1e   │ │  ░  ┃  ░     OCZEKUJE: P-2a   [ ODWRÓĆ ]       │ │ S10 P-4   │
│           │ │   ▮  ┃  ▮     kreski i przerwy jadą do maszyny  │ │           │
│           │ │ ──────── linia bieżącej pozycji ────────────── │ │           │
│           │ └────────────────────────────────────────────────┘ │           │
│           │  (P1) (P2) (P3) (P4) (P5) (P6)  ← stan pistoletów   │           │
├───────────┴────────────────────────────────────────────────────┴───────────┤
│ [AUTO] [SEMI] [RĘCZNY]     [ ▶ START ]        [ ■ STOP ]   [START OD PRZERWY] │ ← pasek dolny
└────────────────────────────────────────────────────────────────────────────┘
   (strona „OŚ" — 10 wzorców w 10 przyciskach S1–S10; strona „KRAWĘDŹ" — patrz 3.2)
```

**Pasek górny**

| Element | Znaczenie |
|---------|-----------|
| POŁĄCZONO (zielony) | łączność ze sterownikiem OK |
| ŁĄCZENIE... / BRAK DANYCH (pomarańczowy) | łączenie z siecią / brak ramek statusu |
| BRAK HASŁA (czerwony) | nie wpisano hasła WiFi |
| GPS *N* sat (zielony) / brak (pomarańczowy) | fix GPS i liczba satelitów |
| Pasek FARBA % | poziom farby w zbiorniku; pomarańczowy poniżej 15 %, czerwony poniżej 5 % |
| **OŚ / KRAWĘDŹ** | przełącznik strony wzorców ([3.2](#32-wybór-wzorca)); aktywna strona ma żółtą ramkę |
| MENU | otwiera menu ([3.6](#36-menu)) |

Stan maszyny (GOTOWY, MALOWANIE, PAUZA, ZATRZYMANY) jest wyświetlany w linii pod kodem wzorca: zielony = malowanie,
pomarańczowy = pauza.

**Środek**

- **Prędkość** — duże cyfry 7-segmentowe; **czerwone** przy przekroczeniu prędkości maksymalnej,
  **pomarańczowe** poniżej minimalnej w trakcie malowania.
- **Kod wzorca** (prawy górny róg strefy) i znacznik `<>` gdy wzorzec jest odwrócony; pod spodem tryb i stan
  (np. `SEMI #3 MALOWANIE`, `(od przerwy)`).
- **Liczniki:** dystans i powierzchnia bieżącego etapu oraz czas.
- **Widok drogi** — animowana perspektywa. Żółte kreski przed maszyną to **plan** (co zostanie namalowane),
  kreski za linią bieżącej pozycji to **już namalowane**. Zielone bloki przy linii pozycji pokazują pistolety,
  które **aktualnie strzelają** (stan rzeczywisty ze sterownika). Pozycja w cyklu kreska/przerwa jest
  zsynchronizowana z dystansem od startu wzorca.
- **Kapsuły P1–P6** — zielone = strzela; żółta ramka = pistolet użyty we wzorcu; czerwona ramka = anomalia.

**Pasek dolny** — tryby, START, STOP, START OD PRZERWY (opisane niżej).

### 3.2 Wybór wzorca

Wzorce są podzielone na **dwie strony według rodzaju malowania**. Przełączasz je przyciskami **OŚ** i
**KRAWĘDŹ** w górnym pasku. Boczne kolumny (5 + 5 przycisków, **S1–S10**) zawsze pokazują wzorce aktywnej strony:

| Strona | S1–S5 (lewa kolumna) | S6–S10 (prawa kolumna) |
|--------|----------------------|------------------------|
| **OŚ jezdni** (pistolety P1–P4), 10 wzorców | P-1a, P-1b, P-1c, P-1d, P-1e | P-2a, P-2b, P-3a, P-3b, P-4 |
| **KRAWĘDŹ** (pistolety P5–P6), 5 wzorców + własny | P-6, P-7a, P-7b, P-7c, P-7d | WŁASNY (S6); S7–S10 puste |

W ten sposób **wszystkie 16 wzorców jest dostępnych** bez przycisku na każdy z nich.

- **Dotknięcie przycisku S1–S10** wybiera wzorzec. Zaznaczony wzorzec ma żółtą ramkę. Przyciski puste (strona
  KRAWĘDŹ) są nieaktywne.
- **Strona podąża za wzorcem:** gdy wzorzec zostanie wybrany z innego miejsca (panel WWW, przyciski fizyczne),
  strona przełącza się sama na tę, do której wzorzec należy (WŁASNY nie zmienia strony).
- **MENU → WSZYSTKIE WZORCE** — siatka 4 × 4 wszystkich 16 wzorców z miniaturą i nazwą (dla wygody).
  Wzorzec **WŁASNY** jest aktywny tylko po zapisaniu ([3.6](#36-menu), Wzór własny).
- **Przyciski fizyczne:** w układzie **soft-key** te same przyciski S1–S10 oraz przycisk **GRUPA** (przełącznik
  OŚ/KRAWĘDŹ) są fizycznymi przyciskami przy ekranie — patrz [4.1](#41-przyciski) i [sekcja 5.5](#55-podział-na-grupy-oś-i-krawędź).
- **Zmiana podczas malowania** zależy od ustawienia **Smart/Instant** (MENU → USTAWIENIA):
  - **SMART** — bieżący cykl kreska+przerwa jest dokańczany, nowy wzorzec zaczyna się po nim. Na drodze widać
    żółty napis `OCZEKUJE: P-2a (po cyklu)`.
  - **INSTANT** — zmiana natychmiast (bieżąca kreska jest ucinana).
- **ODWRÓĆ** — przycisk widoczny na drodze tylko dla **P-3a** i **P-3b**: zamienia role pistoletów P1 ↔ P3
  (która linia jest ciągła, a która przerywana). Działa też w trakcie malowania.

### 3.3 Tryby pracy

Przyciski **AUTO / SEMI / RĘCZNY** w lewym dolnym rogu. Zaznaczony tryb ma żółtą ramkę.
**Tryb można zmienić tylko gdy maszyna stoi** (stan GOTOWY lub ZATRZYMANY); podczas malowania i pauzy
przyciski są wyszarzone. Wybrany tryb jest pamiętany po restarcie. Opis trybów: [sekcja 6](#6-tryby-pracy).

### 3.4 START, PAUZA, STOP

Przycisk **START** zmienia się zależnie od sytuacji:

| Sytuacja | Napis i kolor | Działanie |
|----------|---------------|-----------|
| GOTOWY / ZATRZYMANY | ▶ START (zielony) | rozpoczyna malowanie od początku wzorca |
| MALOWANIE (AUTO) | ⏸ PAUZA (pomarańczowy) | zatrzymuje pistolety, zachowuje pozycję |
| PAUZA | ▶ WZNÓW (zielony) | wznawia malowanie |
| MALOWANIE w SEMI, kreska gotowa | ⏭ NASTĘPNA LINIA (niebieski) | rozpoczyna kolejną kreskę |
| MALOWANIE w RĘCZNYM | ⏸ PAUZA | patrz uwaga niżej |

**STOP** (czerwony, zawsze widoczny) kończy malowanie, zapisuje raport na karcie SD i wyłącza pistolety.
W każdym oknie menu **STOP jest dostępny w lewym górnym rogu**.

**START OD PRZERWY** (wąski przycisk po prawej) — dostępny gdy maszyna stoi; rozpoczyna cykl od fazy przerwy
([sekcja 7](#7-start-od-przerwy)).

> **Nowy etap:** START ze stanu GOTOWY lub ZATRZYMANY rozpoczyna **nowy etap** — liczniki sesji (dystans,
> powierzchnia, czas) startują od zera; raport poprzedniego etapu został zapisany przy STOP. PAUZA/WZNÓW nie
> zeruje liczników.

> **Tryb RĘCZNY:** pistolety strzelają wyłącznie gdy operator **trzyma fizyczny przycisk START na sterowniku**
> (albo na pilocie/pedale) i prędkość jest odpowiednia. Jest to zabezpieczenie sprzętowe — moduł 7" nie może
> uruchomić strzału. Na drodze wyświetla się niebieski napis **RĘCZNY: TRZYMAJ FIZYCZNY START**. Start sesji,
> pauzę i STOP wykonasz normalnie z ekranu.

### 3.5 Komunikaty i alarmy

W górnej części widoku drogi pojawia się kolorowy baner (najwyższy priorytet pokazywany jest pierwszy):

| Baner | Znaczenie | Co robić |
|-------|-----------|----------|
| **ZA SZYBKO!** (czerwony, miga) | prędkość powyżej maksymalnej — **pistolety wyłączone** | zwolnij; po spadku prędkości pistolety włączą się same |
| **ANOMALIA PISTOLETU P*n*** (czerwony, miga) | pistolet skonfigurowany, ale nie maluje | sprawdź dyszę, przekaźnik, przewód, zbiornik farby |
| **AUTO-PAUZA (postój)** (pomarańczowy) | maszyna stoi | ruszaj — po ruszeniu wznowi się automatycznie (jeśli włączone) |
| **ZA WOLNO** (pomarańczowy) | prędkość poniżej minimalnej | przyspiesz powyżej progu (domyślnie 3 km/h) |
| **LINIA GOTOWA – NASTĘPNA LINIA** (niebieski) | tryb SEMI, kreska skończona | naciśnij NASTĘPNA LINIA |
| **RĘCZNY: TRZYMAJ FIZYCZNY START** (niebieski) | tryb RĘCZNY | trzymaj START na sterowniku |

Krótkie **czerwone komunikaty** u góry ekranu (znikają po ok. 3 s) informują o odrzuconym poleceniu, np.
„nie można zmienić trybu podczas malowania", „sterownik zajęty", „brak łączności ze sterownikiem".

**Utrata łączności:** gdy przez ponad ok. 2,5 s brak danych, ekran pokrywa czerwony napis
**BRAK ŁĄCZNOŚCI ZE STEROWNIKIEM** z ostrzeżeniem **UŻYJ FIZYCZNEGO PRZYCISKU STOP NA STEROWNIKU**.
Sterownik działa dalej samodzielnie (pistolety pracują wg wzorca). Moduł łączy się ponownie sam i ekran wraca.
Polecenie STOP wysłane z modułu jest powtarzane kilkukrotnie; jeśli nie dotrze, pojawi się komunikat
„STOP NIE DOTARŁ – użyj fizycznego STOP".

### 3.6 Menu

**MENU** (prawy górny róg) otwiera siatkę funkcji. W każdym oknie: **ZAMKNIJ** (prawy górny róg) i **STOP**
(lewy górny róg).

| Pozycja | Zawartość |
|---------|-----------|
| **STATYSTYKI** | Sesja: dystans, powierzchnia, czas. Łącznie: dystans, powierzchnia, czas pracy. Dystans i liczba strzałów każdego pistoletu. Farba: poziom, zużycie, tankowania. Karta SD i liczba raportów. Odświeżane co ok. 1,5 s. |
| **WZÓR WŁASNY** | Edytor 3 slotów. Wybierz **SLOT 1–3** (gwiazdka `*` = zapisany). Dla każdego pistoletu P1–P6 wybierz **WYŁ / CIĄGŁY / PRZERYW.**; dla przerywanego ustaw **KRESKĘ** i **PRZERWĘ** w metrach (przyciski − / +, krok 0,5 m, zakres 0,5–50 m). **ZAPISZ** zapisuje w sterowniku; **ZAPISZ I UŻYJ** dodatkowo wybiera wzorzec WŁASNY. Wymaga łączności. |
| **KALIBRACJA** | Prowadzi przez procedurę 10 m ([sekcja 8](#8-kalibracja-enkodera)). Pokazuje status i impulsy/metr. |
| **FARBA / ZBIORNIK** | Poziom w zbiorniku, zużycie w sesji, liczba tankowań. Pojemność zbiornika (− / + co 10 L). **Tankowanie:** +10, +25, +50, +100 L lub **DO PEŁNA**. |
| **USTAWIENIA** | Maks. prędkość malowania (5–30 km/h, krok 1); min. prędkość (0–10 km/h, krok 0,5); **Smart / Instant**; **auto-wznowienie**; **przyciski wzorców (sterownik): KLASYCZNE 15 / SOFT-KEY 10 + GRUPA**; jasność ekranu. Wartości progów odczytywane są ze sterownika; błędne kombinacje (np. min ≥ max) sterownik odrzuca z komunikatem. |
| **WSZYSTKIE WZORCE** | Siatka 4 × 4 wszystkich 16 wzorców — wybór dowolnego wzorca bez przełączania stron OŚ/KRAWĘDŹ. |
| **POŁĄCZENIE WiFi** | Hasło sieci sterownika (min. 8 znaków), stan połączenia i siła sygnału. |
| **INFORMACJE** | Wersje firmware, wolna pamięć, czas pracy sterownika, klienci WiFi, GPS (fix, satelity, HDOP, prędkość GPS, zapis trasy), enkoder, progi prędkości, poziom farby. |

**Funkcje dostępne tylko na sterowniku lub w panelu WWW** (nie ma ich w module 7"): pobieranie raportów SD,
czyszczenie dysz, pomiar dystansu, reset etapu, reset liczników, eksport statystyk, factory reset,
tryb nocny, tryb DEMO. Pełna obsługa w [sekcji 4](#4-sterownik--panel-fizyczny).

---

## 4. Sterownik — panel fizyczny

Sterownik ma własny ekran 2,8" (320×240) i przyciski. Działa niezależnie od modułu 7".

### 4.1 Przyciski

| Przycisk | Krótkie naciśnięcie | Długie (1,5 s) |
|----------|---------------------|-----------------|
| **START** (GPIO 38) | start / pauza / wznowienie; SEMI: następna linia; RĘCZNY: **trzymaj = strzelaj** | HOME: ekran SETUP |
| **STOP** (GPIO 39) | zatrzymanie malowania; w menu: poprzednia pozycja | HOME: menu serwisowe; w menu: powrót |
| **SELEKTOR** (GPIO 40) | HOME/malowanie: odwróć P-3a/P-3b; w menu: następna pozycja | HOME: przełącz Smart/Instant; w menu: wejdź |
| **GAP** (GPIO 7, przycisk enkodera) | HOME: start od przerwy | — |
| **Przyciski wzorców** (MCP23017) | zależnie od układu: **klasyczny — 15 przycisków**, każdy wybiera jeden wzorzec P-1a … P-7d; **soft-key — 10 przycisków S1–S10 + GRUPA** (patrz niżej). Potwierdza buzzer. | — |
| **START + STOP razem** (1,5 s) | menu serwisowe | |

**Układ przycisków wzorców** (ustawienie w module 7": MENU → USTAWIENIA → *Przyciski wzorców*; panel WWW/API:
`set_pattern_layout`; zapis trwały):

| Układ | Przyciski | Działanie |
|-------|-----------|-----------|
| **KLASYCZNE 15** *(domyślny)* | 15 | jeden przycisk = jeden wzorzec (działa samodzielnie, bez modułu 7") |
| **SOFT-KEY 10 + GRUPA** | 10 + 1 | S1–S10 wybierają wzorce **aktywnej grupy** (OŚ albo KRAWĘDŹ); przycisk **GRUPA** przełącza grupę (krótki ton 1,8 kHz, wyższy niż przy wyborze wzorca). **Etykiety wzorców są na ekranie modułu 7" obok przycisków**, więc ten układ wymaga modułu 7". |

Podział na grupy i numeracja przycisków — [sekcja 5.5](#55-podział-na-grupy-oś-i-krawędź). Aktywna grupa podąża za wybranym
wzorcem i jest wspólna dla przycisków fizycznych i ekranu 7".

STOP dodatkowo uruchamia **sprzętowe przerwanie awaryjne** — pistolety są wyłączane natychmiast, bez
oczekiwania na pętlę programu.

**Elementy fizyczne na panelu** (układ soft-key, propozycja A z [WIZUALIZACJE.md](WIZUALIZACJE.md)):
10 przycisków wzorców **S1–S10** przy ekranie (S1–S5 lewa kolumna, S6–S10 prawa, od góry), **GRUPA**, **START**,
**STOP**, **SELEKTOR**, **GAP**, opcjonalnie joystick — razem 15 przycisków fizycznych (w układzie klasycznym było
19). Dodatkowo pilot (J4) i pedał (J5) działają równolegle do START/STOP/SELEKTOR/GAP. Rysunki: [WIZUALIZACJE.md](WIZUALIZACJE.md);
schemat elektryczny: [SCHEMAT_PODLACZEN.md](SCHEMAT_PODLACZEN.md), sekcja 8.

### 4.2 Joystick KY-023

| Ruch | Odpowiednik |
|------|-------------|
| Góra | STOP (krótko) — poprzednia pozycja |
| Dół | SELEKTOR (krótko) — następna pozycja |
| Prawo | SELEKTOR (długo) — wejdź / zmień |
| Lewo | STOP (długo) — cofnij |
| Przycisk (krótko) | SELEKTOR (długo) |
| Przycisk (długo) | START (długo) — SETUP |

> **Nie wciskaj joysticka przy włączaniu zasilania** (pin rozruchowy — może uniemożliwić start).

### 4.3 Ekrany

- **HOME** — wzorzec, prędkość, powierzchnia, tryb `[AUTO]/[SEMI]/[RECZNY]`, „Gotowy", 6 prostokątów pistoletów.
- **PAINTING** — jak HOME plus flagi `[ODW]` `[GAP]`; „Malowanie" (zielony) lub „Pauza" (żółty). Prostokąty pistoletów:
  zielony = maluje, żółty migający = pauza, szary = nieużywany.
- **SETUP** (START 1,5 s na HOME): kursor `►` na opcjach *Tryb pracy* (AUTO → SEMI → RĘCZNY → DEMO), *Przełączanie*
  (Smart ↔ Instant), *Start* (Normalny ↔ Od przerwy). SELEKTOR krótko = kursor, SELEKTOR długo = zmiana wartości,
  START = maluj, STOP długo = powrót.
- **SUMMARY** (po STOP): wzorzec, dystans, powierzchnia, czas, średnia prędkość, GPS. START = kontynuuj,
  STOP krótko = nowy etap (zeruje liczniki sesji), STOP długo = HOME.

### 4.4 Menu serwisowe (STOP 1,5 s na HOME)

Nawigacja: SELEKTOR krótko = w dół, STOP krótko = w górę, SELEKTOR długo = wejdź, STOP długo = powrót.
START/START długo w menu przełącza **tryb nocny** (bursztynowa paleta).

| # | Pozycja | Opis |
|---|---------|------|
| 1 | Kalibracja enkodera | procedura 10 m — START = początek / koniec pomiaru |
| 2 | Pomiar dystansu | ręczny miernik: START = start/wstrzymaj, STOP = zeruj |
| 3 | Raporty | status karty SD, liczba raportów, ostatni wpis |
| 4 | Czyszczenie dysz | wybierz wzorzec SELEKTOREM, **trzymaj START** — pistolety strzelają (nawet na postoju), puść = OFF |
| 5 | Statystyki lifetime | łączny dystans, powierzchnia, czas malowania |
| 6 | Wzorzec własny | edycja 3 slotów na sterowniku |
| 7 | Eksport statystyk | zapis na SD: `/stats/lifetime_stats.csv` |
| 8 | Reset etapu | zeruje liczniki **sesji** (START = TAK, STOP = NIE) |
| 9 | Reset liczników | zeruje wszystkie liczniki oprócz kalibracji |
| 10 | Tankowanie farby | uzupełnienie zbiornika |
| 11 | Factory reset | przywrócenie ustawień fabrycznych (kasuje NVS) |

---

## 5. Wzorce malowania

### 5.1 Tabela wzorców

| Wzorzec | Nazwa | Kreska / przerwa | Pistolety | Szer. |
|---------|-------|------------------|-----------|-------|
| P-1a | Przerywana długa | 4 m / 8 m | P2 | 12 cm |
| P-1b | Przerywana krótka | 2 m / 4 m | P2 | 12 cm |
| P-1c | Wydzielająca | 2 m / 2 m | P2 | 12 cm |
| P-1d | Prowadząca wąska | 1 m / 1 m | P2 | 12 cm |
| P-1e | Prowadząca szeroka | 1 m / 1 m | P4 | 24 cm |
| P-2a | Ciągła wąska | ciągła | P2 | 12 cm |
| P-2b | Ciągła szeroka | ciągła | P4 | 24 cm |
| P-3a | Przekraczalna długa | ciągła + 4 m / 2 m | P1 (ciągły) + P3 (przerywany) | 12 cm |
| P-3b | Przekraczalna krótka | ciągła + 1 m / 1 m | P1 (ciągły) + P3 (przerywany) | 12 cm |
| P-4 | Podwójna ciągła | ciągła + ciągła | P1 + P3 | 12 cm |
| P-6 | Ostrzegawcza | 4 m / 2 m | P5 | 12 cm |
| P-7a | Krawędziowa przerywana szeroka | 1 m / 1 m | P6 | 24 cm |
| P-7b | Krawędziowa ciągła szeroka | ciągła | P6 | 24 cm |
| P-7c | Krawędziowa przerywana wąska | 1 m / 1 m | P5 | 12 cm |
| P-7d | Krawędziowa ciągła wąska | ciągła | P5 | 12 cm |
| WŁASNY | Wzorzec własny | dowolnie | dowolne z P1–P6 | wg pistoletów |

### 5.2 Pistolety

| Pistolet | Szerokość | Rola |
|----------|-----------|------|
| P1 | 12 cm | oś jezdni — lewy |
| P2 | 12 cm | oś jezdni — środek |
| P3 | 12 cm | oś jezdni — prawy |
| P4 | 24 cm | oś jezdni — szeroki |
| P5 | 12 cm | krawędź — wąski |
| P6 | 24 cm | krawędź — szeroki |

### 5.3 Odwracanie P-3a / P-3b
Domyślnie P1 (lewy) maluje linię ciągłą, a P3 (prawy) przerywaną. **Odwrócenie** zamienia je miejscami —
przydatne przy jeździe w przeciwnym kierunku, aby linia ciągła była po właściwej stronie jezdni.
Moduł 7": przycisk **ODWRÓĆ**. Sterownik: SELEKTOR (krótko). Panel WWW: przycisk odwracania.

### 5.4 Wzorzec własny
Trzy sloty pamięci. Dla każdego pistoletu: wyłączony / ciągły / przerywany (z własną długością kreski i przerwy,
0,5–50 m). Edycja: moduł 7" (MENU → WZÓR WŁASNY), sterownik (menu serwisowe, pozycja 6) lub panel WWW.
Zapis trwały (NVS), przetrwa restart.

### 5.5 Podział na grupy OŚ i KRAWĘDŹ

Wzorce dzielą się według pistoletów, które malują — **oś jezdni** (P1–P4) i **krawędź** (P5–P6). Podział obowiązuje
w module 7" oraz w układzie fizycznych przycisków soft-key. **Żaden wzorzec nie zostaje pominięty** (10 + 5 + WŁASNY = 16).

| Nr przycisku | Grupa **OŚ** | Grupa **KRAWĘDŹ** |
|--------------|--------------|-------------------|
| S1 | P-1a | P-6 |
| S2 | P-1b | P-7a |
| S3 | P-1c | P-7b |
| S4 | P-1d | P-7c |
| S5 | P-1e | P-7d |
| S6 | P-2a | WŁASNY |
| S7 | P-2b | — |
| S8 | P-3a | — |
| S9 | P-3b | — |
| S10 | P-4 | — |
| **GRUPA** | przełącza OŚ ⇄ KRAWĘDŹ | |

Fizyczne rozmieszczenie przy ekranie: **S1–S5 w lewej kolumnie** (od góry do dołu), **S6–S10 w prawej kolumnie**
(od góry do dołu), przycisk **GRUPA** przy ekranie (np. narożnik). Na ekranie 7" etykiety wzorców widnieją
dokładnie obok przycisków. Podłączenie do MCP23017: [SCHEMAT_PODLACZEN.md](SCHEMAT_PODLACZEN.md), sekcja 4.3.

---

## 6. Tryby pracy

| Tryb | Działanie | Zastosowanie |
|------|-----------|--------------|
| **AUTO** | Pistolety sterowane automatycznie z dystansu (cykl kreska/przerwa). Ciągłe malują nieprzerwanie (od minimalnej prędkości). Auto-pauza na postoju. | Standardowe malowanie linii |
| **SEMI** | Kreska malowana automatycznie na pełną długość, po czym pistolety wyłączają się (krótki sygnał). **Przerwę kontroluje operator** — kolejną kreskę zaczyna **START / NASTĘPNA LINIA**. Pistolety ciągłe działają normalnie. | Przejścia dla pieszych, skrzyżowania, ręczne pozycjonowanie |
| **RĘCZNY** | Pistolety strzelają **tylko przy trzymanym fizycznym START** i odpowiedniej prędkości; puszczenie = natychmiast OFF. Pistolety wyłączone we wzorcu pozostają wyłączone. | Strzałki, symbole, ręczne oznaczenia |
| **DEMO** | Logika jak AUTO, ale pistolety **nie strzelają fizycznie** — pokazywane jest, które by strzelały. | Szkolenie operatorów, sprawdzenie wzorca bez zużycia farby |

Zmiana trybu: moduł 7" (przyciski dolne: AUTO/SEMI/RĘCZNY), sterownik (SETUP: także DEMO), panel WWW.
Tryb DEMO wybierany jest na sterowniku (SETUP); moduł 7" wyświetla go jako `DEMO`. Tryb jest pamiętany po restarcie.

---

## 7. Start od przerwy

Normalny start zaczyna cykl od **kreski**; start od przerwy zaczyna od **przerwy**:

```
Normalny START:        ▮▮▮▮░░░░░░░░▮▮▮▮░░░░░░░░
Start OD PRZERWY:      ░░░░░░░░▮▮▮▮░░░░░░░░▮▮▮▮
```

Służy do kontynuacji istniejącego oznakowania w fazie zgodnej z tym, co już jest na jezdni. Dla wzorców
ciągłych (P-2a, P-2b, P-4, P-7b, P-7d) działa jak zwykły start; w SEMI oznacza rozpoczęcie od oczekiwania
na START; w RĘCZNYM jak zwykły start.

Włączenie: moduł 7" — **START OD PRZERWY**; sterownik — przycisk **GAP** na HOME lub opcja *Start: Od przerwy* w SETUP;
panel WWW — żółty przycisk. W trakcie pracy widoczny jest znacznik `(od przerwy)` / `[GAP]`.

---

## 8. Kalibracja enkodera

Wykonaj po montażu, po zmianie koła pomiarowego i gdy dystanse odbiegają od rzeczywistych.

1. Odmierz na prostym podłożu **dokładnie 10 m** i zaznacz początek i koniec.
2. Ustaw maszynę na początku odcinka; maszyna musi stać (stan GOTOWY).
3. **Moduł 7":** MENU → KALIBRACJA → **START KALIBRACJI**.
   **Sterownik:** menu serwisowe → *Kalibracja enkodera* → START.
4. Przejedź **dokładnie 10 m po prostej**.
5. **KONIEC KALIBRACJI** (moduł 7") lub START (sterownik). Nowa wartość impulsów/metr = zliczone impulsy / 10.
6. Sprawdź wynik: status „Enkoder skalibrowany" i wartość impulsów/metr. Zapis trwały (NVS).
7. Kontrola: przejedź znany odcinek i porównaj z dystansem na ekranie.

Anulowanie na sterowniku: STOP (1,5 s). Domyślnie 100 imp/m (niekalibrowane).

---

## 9. Farba i zbiornik

System szacuje poziom farby z powierzchni namalowanej i współczynnika zużycia (domyślnie **0,6 l/m²**,
zbiornik **200 L**).

- **Ustawienie pojemności:** moduł 7" (MENU → FARBA / ZBIORNIK) lub panel WWW.
- **Współczynnik zużycia:** panel WWW / API (`set_paint_rate`, 0,1–5,0 l/m²).
- **Tankowanie:** po dolaniu farby wpisz ilość (moduł 7": +10/+25/+50/+100 L lub DO PEŁNA; sterownik: menu
  serwisowe, pozycja 10).
- **Ostrzeżenia:** poniżej **15 %** zbiornika ostrzeżenie (żółty/pomarańczowy), poniżej **5 %** alarm (czerwony).
  Poziom zapisywany jest w NVS co 30 s malowania.

Poziom jest szacunkiem z powierzchni — po każdym tankowaniu i przy zmianie farby sprawdź go z rzeczywistym stanem.

---

## 10. Zabezpieczenia, alarmy i sygnały dźwiękowe

### 10.1 Zabezpieczenia

| Zabezpieczenie | Działanie |
|----------------|-----------|
| Minimalna prędkość | Poniżej progu (domyślnie **3 km/h**, nastawa 0–10) pistolety są wyłączone; ostrzeżenie „ZA WOLNO". Wyjątek: czyszczenie dysz. |
| Maksymalna prędkość | Powyżej progu (domyślnie **15 km/h**, nastawa 5–30) — **pistolety wyłączane** + alarm. Po spadku prędkości włączają się same. |
| Auto-pauza | Prędkość < 0,5 km/h przez 1,5 s (lub brak impulsów enkodera 5 s) → pauza. Opcjonalne auto-wznowienie po ruszeniu; po wznowieniu przez 2 s auto-pauza jest blokowana (ochrona przed oscylacją pauza/wznowienie). |
| Gun keepalive | Brak aktualizacji silnika malowania > 300 ms → wszystkie pistolety OFF. |
| Watchdog | 5 s; przy zawieszeniu reset, wyjścia w stan niski. |
| Shutdown handler | Przed każdym resetem pistolety są wyłączane w rejestrach GPIO. |
| Izolacja awarii WWW | Zawieszenie serwera WWW nie przerywa malowania (restart zadania po 10 s). |
| Niski heap | < 64 KB ostrzeżenie; < 32 KB automatyczne zatrzymanie malowania. |
| Zablokowany przekaźnik | Ciągłe ON > 60 s w trybie przerywanym → alarm i wpis w logu. |
| Anomalia pistoletu | Po 50 m jazdy, sprawdzana co 10 s: pistolet użyty we wzorcu z dystansem strzału < 1 m → alarm. |

### 10.2 Sygnały dźwiękowe (buzzer sterownika)

| Zdarzenie | Sygnał |
|-----------|--------|
| Start / wznowienie | 1 krótki beep (2 kHz, 100 ms) |
| Pauza / stop | 2 krótkie beepy |
| Niska prędkość (co 3 s) | 2 pulsy (1,5 kHz) |
| Przekroczenie prędkości (co 2 s) | 3 szybkie tony (3 kHz) |
| Anomalia pistoletu | niski–wysoki–niski (800→1200→800 Hz) |
| Błąd startowy (SD/RTC) | opadający ton (1000→800→600 Hz) |
| SEMI: kreska gotowa | 1 krótki beep (1 kHz, 50 ms) |
| SEMI: nowa linia / wybór trybu / wybór wzorca | 1 krótki beep (1,5 kHz, 80 ms) |

Moduł 7" nie ma własnych sygnałów dźwiękowych — alarmy słychać z buzzera sterownika.

---

## 11. Raporty, GPS, statystyki, kopia zapasowa

- **Raport CSV** — po każdym STOP zapis na karcie SD: data, godzina, wzorzec, dystans, powierzchnia, pozycja GPS.
  Pobieranie: panel WWW (Menu serwisowe → Raporty SD) lub `GET /api/reports/download`.
- **Raport HTML sesji** — stylizowane podsumowanie (wzorce, zużycie farby, GPS), `/html_reports/`.
- **Trasa GPS** — punkt co 5 s w trakcie malowania, po STOP eksport **GPX** i **GeoJSON** (`/tracks/`);
  bufor ok. 6 godzin.
- **Statystyki** — sesja i lifetime; moduł 7" (MENU → STATYSTYKI), sterownik, panel WWW. Motogodziny zapisywane co 5 min.
- **Kopia zapasowa NVS** — automatycznie co 30 min i przy starcie na SD (`/backup/nvs_backup.json`); przy pustym
  NVS ustawienia odtwarzają się z kopii.
- **Karta SD:** FAT32, min. 1 GB. Bez karty system pracuje, ale nie zapisuje raportów (ostrzeżenie w POST).

---

## 12. Przykłady krok po kroku

### Przykład 1 — Linia przerywana P-1a na nowej drodze (moduł 7")

**Cel:** oś jezdni, kreska 4 m / przerwa 8 m, 12 cm.

1. Włącz sterownik i moduł; upewnij się, że pasek pokazuje **POŁĄCZONO** i FARBA > 15 %.
2. Naciśnij **P-1a** w lewej kolumnie (żółta ramka wokół niego; u góry `P-1a`, tryb `AUTO GOTOWY`).
3. Ustaw maszynę na początku odcinka i naciśnij zielony **START** — usłyszysz beep, stan zmieni się na
   `MALOWANIE`, na drodze pojawią się żółte kreski przed maszyną.
4. Ruszaj. Przy prędkości ≥ 3 km/h zielona kapsuła **P2** zaświeci się podczas kreski (4 m) i zgaśnie w przerwie (8 m).
5. Na końcu odcinka naciśnij **STOP**. Sterownik zapisze raport na SD; liczniki pokażą dystans i powierzchnię etapu.

**Oczekiwane wartości:** dla 1000 m linii P-1a: ok. 333 m namalowane (cykl 12 m, kreska 4 m), powierzchnia ok. 40 m².

### Przykład 2 — Linia przekraczalna P-3a z odwróceniem

**Cel:** oś dwukierunkowej drogi, w drugą stronę linia ciągła ma być po przeciwnej stronie.

1. Wybierz **P-3a**. Na drodze pojawią się dwie linie: ciągła (P1) i przerywana (P3, 4 m / 2 m). Na drodze widać przycisk **ODWRÓĆ**.
2. **START**, maluj pierwszy kierunek. Kapsuły P1 (ciągły) i P3 (cyklicznie) świecą na zielono.
3. **STOP** na końcu odcinka. Zawróć maszynę.
4. Dotknij **ODWRÓĆ** — przy kodzie wzorca pojawi się `<>`, a role P1/P3 zamienią się miejscami.
5. **START** i maluj w drugą stronę. Ponowne dotknięcie ODWRÓĆ przywraca układ podstawowy.

Odwrócenie działa również w trakcie malowania, bez zatrzymywania maszyny.

### Przykład 3 — Kontynuacja istniejącej linii (start od przerwy)

**Cel:** na jezdni jest linia P-1b (2 m / 4 m); maszyna stoi w miejscu, gdzie zaczyna się przerwa.

1. Wybierz **P-1b**.
2. Ustaw maszynę dokładnie na początku przerwy.
3. Naciśnij **START OD PRZERWY** (nie START).
4. Ruszaj: przez pierwsze **4 m** pistolet nie maluje (pełna faza przerwy), potem maluje kreskę 2 m, potem znów 4 m przerwy…
   (start od przerwy przesuwa cykl o długość kreski, więc pierwsza przerwa ma **pełną** długość przerwy wzorca).

Wizualnie: istniejące `▮▮░░░░▮▮░░░░`, maszyna maluje `░░░░▮▮░░░░▮▮` — fazy się pokrywają.

### Przykład 4 — Tryb SEMI: przejście dla pieszych

**Cel:** kreski 1 m / 1 m malowane w miejscach wskazywanych przez operatora.

1. Maszyna stoi → dotknij **SEMI** (żółta ramka).
2. Wybierz **P-1d** (1 m / 1 m). **START**.
3. Ruszaj: pistolet maluje kreskę 1 m, po czym wyłącza się (krótki beep); na drodze pojawia się niebieski
   baner **LINIA GOTOWA – NASTĘPNA LINIA**, a przycisk START zmienia się w **⏭ NASTĘPNA LINIA**.
4. Dojedź do miejsca kolejnej kreski i dotknij **NASTĘPNA LINIA** (beep potwierdzenia).
5. Powtarzaj; **STOP** kończy pracę. Numer segmentu widać w opisie trybu (`SEMI #5`).

### Przykład 5 — Tryb RĘCZNY: strzałki i symbole

1. Maszyna stoi → **RĘCZNY**. Wybierz wzorzec z odpowiednimi pistoletami (np. P-4 dla P1+P3).
2. **START** na ekranie rozpoczyna sesję — pojawi się napis **RĘCZNY: TRZYMAJ FIZYCZNY START**.
3. Ruszaj powyżej minimalnej prędkości. **Trzymaj fizyczny START na sterowniku** (lub pilocie/pedale) —
   pistolety strzelają; **puść — natychmiast wyłączone**.
4. Dystans i powierzchnia liczą się normalnie. **STOP** kończy pracę.

### Przykład 6 — Wzorzec własny (edytor na module 7")

**Cel:** dwie linie: P1 ciągła oraz P5 przerywana 3 m / 2 m.

1. **MENU → WZÓR WŁASNY.** Wybierz **SLOT 1** (jeśli ma `*`, jest już zajęty i zostanie nadpisany po zapisie).
2. Przy **P1** dotknij **CIĄGŁY**. Przy **P5** dotknij **PRZERYW.**; ustaw **KRESKA 3.0**, **PRZERWA 2.0** przyciskami − / +.
   Pozostałe pistolety zostaw na **WYŁ**.
3. Dotknij **ZAPISZ I UŻYJ** (pojawi się zielony komunikat), a następnie **ZAMKNIJ**. Wzorzec **WŁASNY** jest
   teraz zapisany i wybrany — zaznaczony na liście wzorców.
4. **START** i maluj. Na drodze widać obie linie zgodnie z konfiguracją.

Zapis dotyczy sterownika (pamięć NVS) — wzorzec jest dostępny także z panelu WWW i po restarcie.

### Przykład 7 — Kalibracja po wymianie koła pomiarowego

1. Odmierz 10 m taśmą, zaznacz początek i koniec.
2. **MENU → KALIBRACJA**, ustaw maszynę na początku odcinka.
3. **START KALIBRACJI** — ekran pokazuje „KALIBRACJA TRWA" i rosnący licznik impulsów.
4. Przejedź prosto 10 m, **KONIEC KALIBRACJI**.
5. Sprawdź nową wartość impulsów/metr. Wróć na ekran roboczy i przejedź znany odcinek jako kontrolę.

### Przykład 8 — Tankowanie farby w trakcie dnia

1. Po zakończeniu odcinka **STOP**. Dolej farbę do zbiornika.
2. **MENU → FARBA / ZBIORNIK.** Odczytaj poziom i sprawdź pojemność zbiornika (popraw − / + jeśli inna).
3. Jeśli zbiornik jest pełny: **DO PEŁNA** — moduł dolewa różnicę do pojemności. Jeśli dolano część: użyj **+10/+25/+50/+100 L**.
4. Pasek FARBA w górnej części ekranu przyjmuje nową wartość.

### Przykład 9 — Zmiana wzorca w trakcie malowania (Smart)

1. Maluj P-1a. Dotknij **P-2a**.
2. Przy ustawieniu **SMART** na drodze pojawi się `OCZEKUJE: P-2a (po cyklu)`; bieżąca kreska dokończy się, po
   zakończeniu cyklu (kreska+przerwa) maszyna przejdzie na P-2a.
3. Aby zmieniać natychmiast: **MENU → USTAWIENIA → Zmiana wzorca: INSTANT** (dotknij, aż pokaże `INSTANT`).

### Przykład 10 — Oś i krawędzie jezdni (przełączanie grup)

**Cel:** namalować oś linią przerywaną P-1a, a potem prawą krawędź linią ciągłą szeroką P-7b.

1. Strona **OŚ** (żółta ramka na OŚ). Dotknij **S1 = P-1a**, **START**, maluj oś, **STOP**.
2. Dotknij **KRAWĘDŹ** — kolumny pokażą P-6, P-7a … P-7d i WŁASNY. Dotknij **S3 = P-7b**.
3. **START** i maluj krawędź (pistolet P6, 24 cm). **STOP**.
4. Wrócisz do osi dotknięciem **OŚ** albo wybierając dowolny wzorzec osi — strona zmieni się sama.

Z fizycznymi przyciskami (układ soft-key): naciśnij **GRUPA** (krótki wyższy ton), potem **S3**. Na ekranie
zobaczysz, że strona i wzorzec zmieniły się tak samo.

### Przykład 11 — Utrata łączności w trakcie pracy

1. Ekran pokrywa czerwony napis **BRAK ŁĄCZNOŚCI ZE STEROWNIKIEM**.
2. Sterownik **maluje dalej** wg wzorca. Jeśli musisz zatrzymać maszynę — naciśnij **fizyczny STOP na sterowniku**
   (panel, pilot lub pedał).
3. Po powrocie sygnału (moduł łączy się sam, zwykle w kilka sekund) ekran wraca do normalnego widoku. Jeśli nie wraca:
   sprawdź zasięg, hasło (**MENU → POŁĄCZENIE WiFi**) oraz czy do sterownika nie jest podłączonych już 4 klientów.

---

## 13. Rozwiązywanie problemów

| Problem | Przyczyna | Rozwiązanie |
|---------|-----------|-------------|
| Moduł 7": „BRAK HASŁA" | nie wpisano hasła | MENU → POŁĄCZENIE WiFi, hasło z ekranu QR sterownika |
| Moduł 7": ciągle „ŁĄCZENIE..." | złe hasło lub poza zasięgiem | sprawdź hasło (8 znaków HEX), zbliż się do sterownika |
| Moduł 7": „BRAK DANYCH" | WiFi OK, sterownik nie wysyła statusu | zrestartuj sterownik; sprawdź liczbę klientów (max 4) |
| Moduł 7": polecenia nie działają | link nieaktywny / sterownik zajęty | poczekaj na komunikat; ponów po chwili |
| Moduł 7": nie można zmienić trybu | trwa malowanie lub pauza | **STOP**, potem zmień tryb |
| Moduł 7": wzorzec WŁASNY niedostępny | brak zapisanego wzorca | MENU → WZÓR WŁASNY → ZAPISZ |
| Moduł 7": obraz miga / przesunięty | ustawienia panelu | patrz [SCHEMAT_PODLACZEN.md](SCHEMAT_PODLACZEN.md) sekcja 5 |
| Tryb RĘCZNY: pistolety nie strzelają | nie trzymasz fizycznego START | trzymaj START na sterowniku / pilocie / pedale |
| Pistolety nie włączają się | prędkość < min. lub > max | sprawdź prędkość i progi (USTAWIENIA) |
| Pistolety nie włączają się na postoju | zabezpieczenie prędkości | to normalne; do testu użyj *Czyszczenie dysz* na sterowniku |
| Złe długości kresek/przerw | brak/zła kalibracja | powtórz kalibrację 10 m |
| Wyświetlacz sterownika nie świeci | podświetlenie | GPIO 21 (PWM) |
| Brak daty i czasu | DS1307 / bateria | I2C (SDA 17, SCL 18), wymień CR2032 |
| Enkoder nie liczy | okablowanie | CLK = GPIO 5, DT = GPIO 6 |
| Przyciski nie działają | okablowanie | GPIO 38 / 39 / 40 / 7 do GND |
| Pętla restartów | użyte piny PSRAM lub zwarty GPIO 46 | odłącz GPIO 26–37; nie wciskaj joysticka przy starcie |
| Karta SD nie działa | format / CS | FAT32, CS = GPIO 16 |
| Brak raportów | raport zapisuje się po STOP | zakończ malowanie STOP-em |
| Anomalia pistoletu (buzzer 800 Hz) | pistolet nie strzela | sprawdź przekaźnik, przewód, dyszę, farbę |
| Alarm prędkości miga stale | próg zbyt niski | USTAWIENIA → maks. prędkość ↑ |
| Panel WWW nie odpowiada | 4 klientów WiFi | odłącz zbędne urządzenia |

---

## 14. BHP i konserwacja

- Pistolety i zawory to elementy wykonawcze z farbą pod ciśnieniem — przed pracą przy pistoletach **zatrzymaj
  maszynę fizycznym STOP i odłącz zasilanie**.
- Przed każdym dniem pracy: test *Czyszczenie dysz* (wszystkie 6 przekaźników), kontrola poziomu farby, test
  fizycznego STOP (panel, pilot, pedał) oraz STOP z modułu 7".
- Regularnie czyść dysze; obserwuj liczniki strzałów pistoletów (STATYSTYKI) jako wskazówkę do serwisu.
- Kontroluj kabel enkodera i mocowanie koła pomiarowego (wpływ na dokładność).
- Wymień CR2032 w module RTC raz na 2–3 lata.
- Raporty i kopię ustawień archiwizuj z karty SD.
- Aktualizację firmware sterownika i modułu 7" wykonuj przy zatrzymanej maszynie.

---

## 15. Dane techniczne

| Parametr | Wartość |
|----------|---------|
| Sterownik | ESP32-S3 N16R8 (16 MB Flash, 8 MB PSRAM), firmware 2.52.0 |
| Wyświetlacz sterownika | ILI9341 2,8", 320×240 |
| Moduł wyświetlacza | Sunton ESP32-8048S070C, 7", 800×480, dotyk pojemnościowy |
| Pistolety | 6 (P1–P6), przekaźniki, logika HIGH = ON |
| Wzorce | 16 (15 normowych + własny, 3 sloty) |
| Tryby | AUTO, SEMI, RĘCZNY (+ DEMO) |
| Prędkość min. / maks. | 3 km/h / 15 km/h (nastawy 0–10 / 5–30) |
| Auto-pauza | < 0,5 km/h przez 1,5 s |
| WiFi | AP `TrassarV3`, kanał 6, max 4 klientów, hasło z MAC |
| API | HTTP :80, WebSocket :81 (status co 500 ms) |
| GPS | NEO-6M, UART2, 9600 baud |
| Przyciski wzorców | MCP23017 (I2C 0x20): układ klasyczny 15 przycisków albo soft-key 10 + GRUPA (11 przycisków) |
| Zasilanie | 5 V DC, min. 3 A (sterownik + moduł 7" na osobnych odgałęzieniach) |
| Watchdog | 5 s |
| Keepalive pistoletów | 300 ms |
| Kalibracja | odcinek 10 m, zapis NVS |

Szczegóły elektryczne: [SCHEMAT_PODLACZEN.md](SCHEMAT_PODLACZEN.md). API: [API_WWW.md](API_WWW.md).
Moduł 7" (dla programistów): [MODUL_WYSWIETLACZA.md](MODUL_WYSWIETLACZA.md).
