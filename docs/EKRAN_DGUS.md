# Specyfikacja ekranu DWIN DGUS — projekt do zbudowania w DGUS Designer

Ten dokument jest **specyfikacją budowy** projektu ekranu (strony, przyciski, pola wartości) dla wyświetlacza
inteligentnego DWIN DGUS (np. `DMG10600T070_09WTC`, 7" 1024×600), używanego jako duży ekran sterownika Trassar.
Kontekst i uzasadnienie architektury: [ARCHITEKTURA_TERMINAL.md](ARCHITEKTURA_TERMINAL.md). Adresy poniżej są
**jeden do jednego** zgodne z `src/dgus/dgus_map.h` (kod firmware) — zmiana jednego bez drugiego zerwie komunikację.

> Projekt w DGUS Designer **nie został jeszcze zbudowany** — ten dokument to specyfikacja do zrobienia tego ręcznie
> w narzędziu producenta (Windows, bezpłatne, do pobrania ze strony DWIN). Nie da się tego wygenerować automatycznie.

## 1. Ustawienia projektu

| Parametr | Wartość |
|----------|---------|
| Model / rozdzielczość | wg posiadanego modułu (np. `DMG10600T070_09WTC`, 1024×600) |
| Prędkość UART | **115 200 baud, 8N1** (musi zgadzać się z `dgus::BAUD` w `shared/dgus_protocol.h`) |
| CRC ramki | **wyłączone** (domyślnie) |
| Orientacja | pozioma 1024×600 **lub** obrócona 90°/270° do pionu 600×1024, jeśli ekran montowany pionowo (patrz [WIZUALIZACJE.md](WIZUALIZACJE.md)) — do ustawienia w konfiguracji ekranu (rejestr kierunku wyświetlania); zweryfikuj w używanej wersji Designera |
| Auto-upload kodu klawisza (touch) | **włączone** dla wszystkich przycisków — inaczej sterownik nie dostanie zdarzeń dotyku |

## 2. Numeracja stron (Page ID)

Numer strony w DGUS Designer **musi być równy** wartości `ScreenID` sterownika (`src/config.h`). Sterownik sam
poleca przełączenie strony (rozdz. 4.3 w ARCHITEKTURA_TERMINAL.md) — nie ustawiaj w Designerze żadnego przycisku
z lokalnym "jump page".

| Page ID | Ekran | Zawartość strony |
|---|---|---|
| 0 | HOME | ekran roboczy — patrz rozdz. 3 |
| 1 | PAINTING | ekran roboczy — patrz rozdz. 3 (ten sam layout co HOME, różni się tylko danymi) |
| 2 | SERVICE_MENU | rozdz. 5.1 |
| 3 | CALIBRATION | rozdz. 5.2 |
| 4 | DISTANCE_METER | rozdz. 5.3 |
| 5 | REPORTS | rozdz. 5.4 |
| 6 | NOZZLE_CLEAN | rozdz. 5.5 |
| 7 | SETUP | rozdz. 5.6 |
| 8 | SESSION_RESET | rozdz. 5.7 |
| 9 | COUNTER_RESET | rozdz. 5.8 |
| 10 | SUMMARY | rozdz. 5.9 |
| 11 | LIFETIME_STATS | rozdz. 5.10 |
| 12 | CUSTOM_PATTERN | rozdz. 5.11 |
| 13 | STATS_EXPORT | rozdz. 5.12 |
| 14 | FACTORY_RESET | rozdz. 5.13 |
| 15 | TANKOWANIE | rozdz. 5.14 |
| 16 | POST | rozdz. 5.15 (ekran startowy) |

## 3. Strona robocza (0 = HOME, 1 = PAINTING)

Jeden layout na obie strony (albo dwie osobne strony z tą samą treścią — HOME różni się od PAINTING głównie tym,
że w PAINTING pistolety realnie strzelają). Pola do umieszczenia (kontrolka "Basic"/"Number"/"Text" w Designerze,
związana ze wskazanym adresem VP):

| Pole | VP | Typ / szerokość | Uwagi |
|------|----|-----------------|-------|
| Stan | `0x1000` | liczba, 1 słowo | 0 GOTOWY, 1 MALOWANIE, 2 PAUZA, 3 ZATRZYMANY — użyj kontrolki "ikona wariantowa" (4 obrazki) albo tekstu warunkowego |
| Tryb | `0x1001` | liczba, 1 słowo | 0 AUTO, 1 SEMI, 2 RĘCZNY, 3 DEMO |
| Kod wzorca | `0x1002` | tekst ASCII, 6 słów (12 zn.) | np. "P-1a" |
| Nazwa wzorca | `0x1008` | tekst ASCII, 16 słów (32 zn.) | np. "Przerywana dluga" |
| Odwrócony | `0x1018` | liczba, 1 słowo | 0/1 — pokaż znacznik "⇄" gdy 1 |
| Start od przerwy | `0x1019` | liczba, 1 słowo | 0/1 |
| Prędkość ×10 | `0x101A` | liczba (int16), 1 słowo | kontrolka z 1 miejscem po przecinku (dzielnik wyświetlania = 10) |
| Dystans [dm] | `0x101B`–`0x101C` | liczba (int32), 2 słowa | dzielnik wyświetlania = 10 → metry |
| Powierzchnia ×100 [m²] | `0x101D`–`0x101E` | liczba (int32), 2 słowa | dzielnik = 100 |
| Czas [s] | `0x101F`–`0x1020` | liczba (int32), 2 słowa | format mm:ss/hh:mm:ss po stronie Designera (albo tekst już sformatowany — do rozważenia) |
| Dystans od startu wzorca [dm] | `0x1021`–`0x1022` | liczba (int32), 2 słowa | do prostego paska postępu cyklu (nie ma animowanej "drogi" jak w LVGL — patrz ARCHITEKTURA_TERMINAL.md rozdz. 12) |
| Przekroczona prędkość | `0x1023` | liczba, 1 słowo | 0/1 — steruje kolorem/alarmem |
| Za wolno | `0x1024` | liczba, 1 słowo | 0/1 |
| Auto-pauza | `0x1025` | liczba, 1 słowo | 0/1 |
| SEMI: linia gotowa | `0x1026` | liczba, 1 słowo | 0/1 |
| SEMI: numer segmentu | `0x1027` | liczba, 1 słowo | |
| Pistolety P1–P6 | `0x1028`–`0x102D` | 6× liczba, 1 słowo każda | 0 wyłączony/nieużyty, 1 strzela (zielony), 2 skonfigurowany-bezczynny (żółta obwódka) |
| Poziom farby [%] | `0x102E` | liczba, 1 słowo | pasek/wskaźnik |
| Satelity GPS | `0x102F` | liczba, 1 słowo | |
| GPS fix | `0x1030` | liczba, 1 słowo | 0/1 |
| Oczekująca zmiana wzorca | `0x1031` | liczba, 1 słowo | 0/1 |
| Kod oczekującego wzorca | `0x1032` | tekst ASCII, 6 słów | |
| Grupa | `0x1038` | liczba, 1 słowo | 0 OŚ, 1 KRAWĘDŹ — podświetl odpowiednią zakładkę |
| Tryb nocny | `0x103A` | liczba, 1 słowo | 0/1 — jeśli chcesz przyciemnienie sterowane przez sterownik, powiąż z jasnością podświetlenia (do zweryfikowania w Designerze) |
| Łącze OK | `0x103B` | liczba, 1 słowo | zawsze 1 gdy ramka dotarła (informacyjne — ekran i tak wie najlepiej, czy odbiera) |
| Karta SD gotowa | `0x103C` | liczba, 1 słowo | 0/1 — mała ikonka ostrzeżenia gdy 0 |
| **Anomalia pistoletu (zbiorczo)** | `0x103D` | liczba, 1 słowo | 0/1 — czerwony baner/alarm, gdy któryś aktywny pistolet nie strzela mimo dystansu (jak dawny czerwony baner LVGL) |
| Anomalia pistoletów P1–P6 | `0x103E`–`0x1043` | 6× liczba, 1 słowo każda | 0/1 który konkretnie — do podświetlenia właściwej kapsuły pistoletu na czerwono |
| **Enkoder skalibrowany** | `0x1044` | liczba, 1 słowo | 0/1 — pokaż małe ostrzeżenie "NIESKALIBROWANY", gdy 0 (dystans/prędkość będą błędne) |
| Enkoder: impulsy/metr ×10 | `0x1045` | liczba (int16), 1 słowo | dzielnik wyświetlania = 10 |
| **GPS: prędkość ×10 [km/h]** | `0x1046` | liczba (int16), 1 słowo | z odbiornika GPS (niezależnie od prędkości z enkodera) |
| GPS: szerokość geogr. ×1 000 000 | `0x1047`–`0x1048` | liczba (int32), 2 słowa | dzielnik = 1 000 000 (6 miejsc po przecinku); 0 gdy brak fixa |
| GPS: długość geogr. ×1 000 000 | `0x1049`–`0x104A` | liczba (int32), 2 słowa | jw. |
| GPS: HDOP ×10 | `0x104B` | liczba (int16), 1 słowo | dzielnik = 10; 99.9 = brak danych (jakość fixa — im niżej, tym lepiej) |
| GPS: zapis trasy GPX trwa | `0x104C` | liczba, 1 słowo | 0/1 |
| GPS: liczba zapisanych punktów trasy | `0x104D`–`0x104E` | liczba (int32), 2 słowa | |
| **GPS: bufor trasy pełny** | `0x104F` | liczba, 1 słowo | 0/1 — ostrzeżenie (dalsze punkty trasy nie są zapisywane) |
| **Czujnik temperatury: wykryty** | `0x1050` | liczba, 1 słowo | 0/1 — DS18B20 jest opcjonalny, może nie być zamontowany |
| Czujnik temperatury: wartość ×10 [°C] | `0x1051` | liczba (int16, ze znakiem), 1 słowo | dzielnik = 10; pokazuj tylko gdy pole wyżej = 1 |
| Wolna pamięć RAM [KB] | `0x1052` | liczba, 1 słowo | diagnostyka (opcjonalnie, np. na osobnym ekranie "informacje") |
| Czas pracy od włączenia [min] | `0x1053` | liczba, 1 słowo | jw. |
| Klienci WiFi (telefon/laptop) | `0x1054` | liczba, 1 słowo | jw. |
| Wzorzec WŁASNY zapisany | `0x1055` | liczba, 1 słowo | 0/1 — użyj do wyszarzenia ikony/kafelka WŁASNY, gdy pusty |

> **Pilot przewodowy (złącze J4) i pedał (J5):** to fizyczne wejścia **równoległe** do przycisków START/STOP/SELEKTOR/GAP
> na sterowniku (ten sam GPIO) — nie mają osobnej reprezentacji software'owej ani VP, bo elektrycznie są tym samym
> sygnałem co przycisk fizyczny. Ekran DGUS nie musi (i nie może) nic o nich wiedzieć.

> **Joystick fizyczny (nawigacja menu):** działa **równolegle** do dotyku — generuje te same kody zdarzeń co
> przyciski 1–7 powyżej (`menu.handleEvent()`), więc **nie wymaga żadnej konfiguracji w Designerze**. Operator
> może obsłużyć cały interfejs zarówno dotykiem, jak i joystickiem — patrz ARCHITEKTURA_TERMINAL.md rozdz. 6.

### Kolumny wzorców S1–S10

| Pole | VP | Typ / szerokość |
|------|----|------|
| Indeks wzorca slotu *i* (i=0..9) | `0x1100 + i` | liczba, 1 słowo (`0xFFFF` = slot pusty — ukryj kafelek) |
| Zaznaczony slot *i* | `0x110A + i` | liczba, 1 słowo (0/1 — podświetl ramką) |
| Kod wzorca slotu *i* | `0x1120 + i*6` | tekst ASCII, 6 słów |

Ikona/rysunek wzorca w kafelku: **16 gotowych obrazków PNG już przygotowanych** —
[`docs/schematy/ikony_dgus/`](schematy/ikony_dgus/) (wygenerowane z tego samego kodu, co rysunki w
`wzorce_w_skali.svg`, więc wyglądają identycznie). Zaimportuj je w podanej tam kolejności (plik `README.md`
w tym folderze) do biblioteki ikon w Designerze i powiąż z polem "indeks wzorca slotu" jako "ikona wariantowa"
(DGUS Designer: kontrolka wybierająca obrazek numerem z listy — kolejność importu = indeks `PatternID`).
Pole `0x1055` (WŁASNY zapisany) możesz powiązać z wyglądem kafelka WŁASNY (np. wyszarzona ikona / dopisek
"(pusty)"), gdy slot nie jest jeszcze zapisany.

### Przyciski strony roboczej (piszą pod `VP_TOUCH_EVENT` = `0x1300`)

| Przycisk | Kod |
|----------|-----|
| START | 1 (krótkie) |
| START (SETUP — przytrzymanie) | 2 |
| STOP | 3 (krótkie) |
| SERWIS (wejście do menu serwisowego) | 4 |
| Odwróć wzorzec (P-3a/P-3b) | 5 |
| Start od przerwy (GAP) | 7 |
| S1…S10 | 70…79 |
| GRUPA (OŚ ⇄ KRAWĘDŹ) | 80 |

## 4. Rejestr żywotności łącza

| VP | Kierunek | Opis |
|----|----------|------|
| `0x1300` (`VP_TOUCH_EVENT`) | ekran → sterownik | kod zdarzenia dotyku (patrz tabele wyżej i niżej) — **żaden inny cel**, nie odpytuj go z Designera |
| `0x1301` (`VP_PING`) | sterownik → ekran, odczyt | sterownik okresowo (~700 ms) wysyła żądanie odczytu tego adresu; nie trzeba niczego tu umieszczać w Designerze — wystarczy, że adres istnieje (dowolna wartość) |

## 4b. STOP awaryjny (E-STOP) — alarm widoczny na KAŻDEJ stronie

W odróżnieniu od wszystkich pól w rozdz. 3 i 5 (które są wysyłane tylko, gdy dana strona jest aktywna), poniższe
dwa pola sterownik wysyła **cały czas, niezależnie od tego, jaka strona jest akurat wyświetlana** (co ~200 ms) —
patrz `DgusLink::sendEstopStatus()`. Rzeczywiste cięcie zasilania pistoletów/pomp jest **sprzętowe** (grzybek E-STOP
w torze zasilania, nie zależy od ekranu ani firmware) — te pola służą wyłącznie do poinformowania operatora.

| Pole | VP | Typ / szerokość | Uwagi |
|------|----|-----------------|-------|
| **STOP awaryjny aktywny** | `0x1056` | liczba, 1 słowo | 0/1 — gdy 1, pokaż pełnoekranowy/nachodzący czerwony alarm "STOP AWARYJNY AKTYWNY" |
| Oczekuje potwierdzenia | `0x1057` | liczba, 1 słowo | 0/1 — 1 gdy grzybek już zwolniony, ale operator jeszcze nie potwierdził (pokaż przycisk POTWIERDŹ, kod 41) |

**Jak zbudować w Designerze:** ponieważ te pola nie są związane z żadną konkretną stroną, alarm musi być widoczny
zawsze — najprościej powielić ten sam element (baner + przycisk POTWIERDŹ) **na każdej stronie** projektu, związany
z tymi samymi adresami VP (jeśli używana wersja DGUS Designer ma funkcję warstwy/elementu wspólnego dla wszystkich
stron — "global"/"system reserved area" — tym lepiej, użyj jej zamiast kopiowania). Przycisk POTWIERDŹ powinien być
widoczny/aktywny tylko, gdy `0x1057=1` (a nie tylko `0x1056=0` — dopóki pętla jest otwarta, potwierdzenie nie ma
znaczenia, sterownik i tak je zignoruje).

| Przycisk | Kod (pod `VP_TOUCH_EVENT`) |
|----------|-----|
| POTWIERDŹ (po ustąpieniu STOP-u) | 41 |

## 5. Strony serwisowe — generyczne pola

Każda strona serwisowa ma **własne, statyczne** etykiety, tło i przyciski (rysujesz raz w Designerze); sterownik
wypełnia tylko wspólny zestaw pól, którego znaczenie zmienia się w zależności od strony (bo tylko jedna strona
serwisowa jest widoczna naraz):

| Pole | VP | Typ / szerokość |
|------|----|------|
| Wiersz 0–9 (wartość) | `0x1210 + i*16` (i=0..9) | tekst ASCII, 16 słów (32 zn.) każdy |
| Zaznaczony wiersz | `0x12B0` | liczba (int16), 1 słowo (−1 = brak) |
| Komunikat | `0x12B1` | tekst ASCII, 32 słowa (64 zn.) |
| Flagi (bit0=A, bit1=B) | `0x12D1` | liczba, 1 słowo |
| Liczba pomocnicza 1 | `0x12D2`–`0x12D3` | liczba (int32), 2 słowa |
| Liczba pomocnicza 2 | `0x12D4`–`0x12D5` | liczba (int32), 2 słowa |

(10 wierszy zamiast pierwotnych 8, żeby ekran startowy — rozdz. 5.15 — pomieścił wszystkie pozycje diagnostyki
bez ucinania. Adresy poniżej `VP_ROW_SELECTED` w dół przesuwają się automatycznie razem z `dgusmap::DGUS_ROW_COUNT`
w kodzie — ta tabela jest zawsze zgodna z `src/dgus/dgus_map.h`, sprawdź tam, jeśli kiedyś zmienisz liczbę wierszy.)

Etykiety wierszy (np. "Impulsy", "Dystans", "Tryb") są **statycznym tekstem w Designerze** — sterownik wysyła
tylko wartości. Poniżej znaczenie pól per strona (kolejność = kolejność wierszy).

### 5.1 SERVICE_MENU (strona 2)
Lista 11 pozycji — **statyczna** w Designerze, każda pozycja to osobny przycisk z własnym kodem (30–40, patrz
tabela w rozdz. 6). Pole `VP_ROW_SELECTED` (`0x12B0`) może podświetlać aktualnie "podświetloną" pozycję, jeśli
zdecydujesz się zachować nawigację strzałkami zamiast (albo obok) bezpośrednich przycisków.
Przyciski: ^/v (kody 5/3, opcjonalnie), WEJDŹ (6), TRYB NOCNY (1), WYJDŹ (4) — albo, prościej, **11 osobnych
przycisków** z kodami 30–40 (zalecane, wygodniejsze na ekranie dotykowym).

### 5.2 CALIBRATION (strona 3)
- Wiersz 0: stan kalibracji ("W TOKU"/"nieaktywna") — flagA
- Wiersz 1: impulsy (tekst), NUM1 = impulsy×10
- Wiersz 2: impulsy/metr
- Wiersz 3: "TAK"/"NIE" (skalibrowany)
- Przyciski: START/KONIEC (kod 1), WYJDŹ (4)

### 5.3 DISTANCE_METER (strona 4)
- Wiersz 0: zmierzony dystans (tekst "%.2f m"), NUM1 = dystans×100 [cm]
- Wiersz 1: "TRWA"/"zatrzymany" — flagA
- Przyciski: START/STOP (1), ZERUJ (3), WYJDŹ (4)

### 5.4 REPORTS (strona 5)
- Wiersz 0: stan karty SD ("OK"/"BRAK")
- Wiersz 1: liczba raportów
- Wiersz 2: ostatni raport (nazwa/skrót)
- Przycisk: WYJDŹ (4)

### 5.5 NOZZLE_CLEAN (strona 6) — **martwy człowiek**
- Wiersz 0: kod + nazwa wzorca
- Wiersz 1: dysze wzorca (lista "P1 P3" itp.)
- Wiersz 2: dysze aktualnie otwarte
- NUM1 = indeks wzorca (do podświetlenia ikony, jeśli chcesz pokazać rysunek)
- Przyciski: NASTĘPNY (5), POPRZEDNI (6), **duży przycisk "TRZYMAJ = PSIKAJ"** skonfigurowany z **osobną wartością
  naciśnięcia (90) i zwolnienia (91)** — to jest jedyny przycisk w całym projekcie wymagający takiej konfiguracji
  (typowa funkcja przycisków "momentary"/"jog" w edytorach HMI; dokładna nazwa opcji zależy od wersji Designera),
  WYJDŹ (4)

### 5.6 SETUP (strona 7)
- Wiersz 0: tryb (AUTO/SEMI-AUTO/RĘCZNY/DEMO)
- Wiersz 1: przełączanie (SMART/INSTANT)
- Wiersz 2: start (NORMALNY/OD PRZERWY)
- `VP_ROW_SELECTED`: podświetlony wiersz (kursor)
- Przyciski: ^/v (3/5), ZMIEŃ (6), START MALOWANIA (1), WYJDŹ (4)

### 5.7 SESSION_RESET (strona 8)
- Wiersz 0: dystans sesji, Wiersz 1: powierzchnia, Wiersz 2: czas
- Przyciski: TAK-RESET (1), NIE (3)

### 5.8 COUNTER_RESET (strona 9)
- Wiersz 0: dystans całkowity, Wiersz 1: powierzchnia, Wiersz 2: czas
- Przyciski: TAK-RESET (**2** = `EVT_START_LONG` — zalecane przytrzymanie, patrz uwaga bezpieczeństwa niżej), NIE (3)

### 5.9 SUMMARY (strona 10)
- Wiersz 0: kod wzorca, 1: dystans, 2: powierzchnia, 3: czas, 4: średnia prędkość, 5: GPS (jeśli dostępny)
- Przyciski: KONTYNUUJ (1), NOWY ETAP (3), STRONA GŁÓWNA (4)

### 5.10 LIFETIME_STATS (strona 11)
- Wiersz 0: dystans, 1: powierzchnia, 2: czas malowania, 3: motogodziny, 4–7 (do 4 pistoletów): liczba strzałów — dla pełnych 6 pistoletów rozważ 2 kolumny po 3 wiersze w Designerze albo przewijanie
- Przycisk: WYJDŹ (4)

### 5.11 CUSTOM_PATTERN (strona 12)
- Wiersz 0: numer pistoletu (P1–P6), 1: tryb (WYŁĄCZONY/CIĄGŁY/PRZERYWANY), 2: kreska [m], 3: przerwa [m]
- `VP_ROW_SELECTED`: pole edytowane (0 pistolet, 1 tryb, 2 kreska, 3 przerwa, 4 zapisz)
- Przyciski: ^/v (3/5), + (6), − (1), WYJDŹ (4) — pole "zapisz" aktywowane przez + (6) gdy zaznaczone

### 5.12 STATS_EXPORT (strona 13)
- Wiersz 0: stan karty SD, Wiersz 1: status eksportu ("gotowy"/"ZAPISANO"/"BŁĄD")
- flagA = zakończono, flagB = sukces
- Przyciski: EKSPORTUJ (1), WYJDŹ (4)

### 5.13 FACTORY_RESET (strona 14)
- Brak wierszy — sam komunikat ostrzegawczy (statyczny tekst w Designerze + `VP_MSG`, jeśli chcesz go zmieniać)
- Przyciski: PRZYTRZYMAJ-RESET (**2** = `EVT_START_LONG`, **zalecane przytrzymanie** — to najbardziej niebezpieczna
  akcja w całym systemie), ANULUJ (3)

### 5.14 TANKOWANIE (strona 15)
- Wiersz 0: ilość dolewki [L], Wiersz 1: poziom w zbiorniku [L], Wiersz 2: pojemność zbiornika [L]
- NUM1 = ilość dolewki×10, NUM2 = poziom×10; flagA = zatwierdzono
- Przyciski: +10 L (5), −10 L (3), +5 L (6), POTWIERDŹ (1), WYJDŹ (4)

### 5.15 POST (strona 16) — ekran startowy
- `VP_MSG`: "Nacisnij START, aby przejsc do pracy"
- Wiersze 0–9: 10 pozycji diagnostyki startowej ("WiFi (SSID, dla telefonu): TrassarV3", "Hasło WiFi: ...", "Adres: ...",
  "Karta SD: OK/BRAK", "Zegar RTC: OK/BŁĄD", "GPS: FIX/brak fix", "Przyciski MCP: OK/BŁĄD", "Enkoder: skalibrowany/NIESKALIBROWANY",
  "Temperatura: xx.x C / brak czujnika", "Firmware: 2.53.0")
- Przycisk: START – DALEJ (1)

## 6. Pełna tabela kodów zdarzeń dotyku

Wartość zapisywana pod `VP_TOUCH_EVENT` (`0x1300`) przez przycisk w DGUS Designer:

| Kod | Znaczenie | Dokładnie jak |
|-----|-----------|---------------|
| 1 | START krótko | fizyczny przycisk START |
| 2 | START długo (potwierdzenie) | fizyczny przycisk START przytrzymany |
| 3 | STOP krótko | fizyczny przycisk STOP |
| 4 | STOP długo / wyjście | fizyczny przycisk STOP przytrzymany |
| 5 | SELECT krótko | fizyczny SELEKTOR |
| 6 | SELECT długo | fizyczny SELEKTOR przytrzymany |
| 7 | GAP (start od przerwy) | fizyczny przycisk GAP |
| 30 | wejdź: KALIBRACJA | — (nowość, tylko dotyk) |
| 31 | wejdź: POMIAR DYSTANSU | — |
| 32 | wejdź: RAPORTY | — |
| 33 | wejdź: CZYSZCZENIE DYSZ | — |
| 34 | wejdź: STATYSTYKI LIFETIME | — |
| 35 | wejdź: WZORZEC WŁASNY | — |
| 36 | wejdź: EKSPORT STATYSTYK | — |
| 37 | wejdź: RESET ETAPU | — |
| 38 | wejdź: RESET LICZNIKÓW | — |
| 39 | wejdź: TANKOWANIE | — |
| 40 | wejdź: FACTORY RESET | — |
| 41 | POTWIERDŹ ustąpienie STOP-u awaryjnego | — (nowość, tylko dotyk — patrz rozdz. 4b) |
| 70–79 | soft-key S1–S10 (slot = kod−70) | fizyczne S1–S10 |
| 80 | GRUPA | fizyczny przycisk GRUPA |
| 90 | martwy człowiek: naciśnięto (tylko NOZZLE_CLEAN) | fizyczny START trzymany na ekranie czyszczenia dysz |
| 91 | martwy człowiek: puszczono | — |

Kody spoza tej listy są ignorowane i logowane przez sterownik (`DBG_PRINTF("[DGUS] Nieznany kod zdarzenia...")`) —
bezpieczny domyślny fallback przy pomyłce w konfiguracji przycisku.

## 7. Kolejność prac przy budowie projektu

1. Utwórz projekt w DGUS Designer dla posiadanego modelu ekranu, ustaw UART 115200 8N1, CRC wyłączone.
2. Zaimportuj **gotowe ikony wzorców** — [`docs/schematy/ikony_dgus/`](schematy/ikony_dgus/) (16 plików PNG,
   kolejność importu opisana w README tego folderu) — przyda się od razu przy budowie strony 0/1.
3. Zbuduj **stronę 0/1** (robocza) wg rozdz. 3 — to najważniejszy i najczęściej używany ekran.
4. Zbuduj **stronę 2** (SERVICE_MENU) z 11 przyciskami (kody 30–40).
5. Zbuduj pozostałe strony serwisowe (3–16) wg rozdz. 5 — mogą być prostsze wizualnie na start (sam tekst, bez
   grafik), dopracowanie wyglądu może poczekać.
6. Dodaj alarm STOP-u awaryjnego (rozdz. 4b) na **każdej** stronie — nie pomiń, to jedyny element bezpieczeństwa
   po stronie ekranu.
7. Wgraj projekt na ekran (karta SD ekranu — procedura opisana w dokumentacji DWIN dla posiadanego modelu).
8. Podłącz ekran do sterownika **z działającym dzielnikiem napięcia** na linii TX ekranu → RX sterownika (patrz
   [ARCHITEKTURA_TERMINAL.md](ARCHITEKTURA_TERMINAL.md), rozdz. 10) i wykonaj próby z rozdz. 11 tego dokumentu.
