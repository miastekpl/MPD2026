# Alternatywa (nie zalecana jako punkt startowy): sterownik (mózg) + wyświetlacz inteligentny DWIN DGUS

> **Ta architektura NIE jest już wariantem docelowym.** Architektura docelowa dziś to sterownik „headless" (bez
> żadnego ekranu podłączonego bezpośrednio) + moduł wyświetlacza **Sunton 7" po WiFi** (`display-module/`) jako
> jedyny interfejs operatora — cały interfejs to kod, zero pracy w zewnętrznym narzędziu GUI. Opis:
> [README.md](../README.md), [SCHEMAT_PODLACZEN.md](SCHEMAT_PODLACZEN.md) rozdz. 3.2/5.2a. Ten dokument opisuje
> **alternatywę** (ekran DGUS na UART) — zachowaną, bo kod jest gotowy, ale **nie zalecaną jako punkt startowy**,
> bo wymaga ręcznej budowy projektu w DGUS Designer (osobne narzędzie GUI producenta, tylko Windows), co dla
> większości użytkowników okazało się barierą nie do przejścia.

**Decyzja (historyczna, dla tej alternatywy):** jeden duży ekran, podłączony **bezpośrednio** do sterownika (bez
pośredniczącego ESP32). Cała logika, stan, menu, ustawienia i dane są w **sterowniku ESP32-S3 (master)**. Duży
ekran to **wyświetlacz inteligentny DWIN DGUS** (np. 7" `DMG10600T070_09WTC`, 1024×600, IPS, dotyk pojemnościowy,
650 cd/m²) — ma własny procesor i sam renderuje interfejs zaprojektowany w edytorze producenta (DGUS Designer);
sterownik komunikuje się z nim prostym protokołem szeregowym (patrz rozdz. 4). Mały ekran ILI9341 jest usuwany
(joystick zostaje — patrz rozdz. 6), karta SD przechodzi na osobny moduł SPI.

> **Status: zaimplementowane w kodzie (kontroler), niesprawdzone na sprzęcie.** Firmware sterownika kompiluje się
> i ma testy protokołu ramki na hoście (`test/test_dgus_protocol`, zweryfikowane ręcznie względem przykładów z
> oficjalnej dokumentacji DWIN). **Projekt ekranu w DGUS Designer (strony, przyciski, ikony) trzeba dopiero
> zbudować** według specyfikacji w [docs/EKRAN_DGUS.md](EKRAN_DGUS.md) — nie da się tego wygenerować automatycznie,
> to osobne narzędzie producenta; gotowe ikony wzorców: [schematy/ikony_dgus/](schematy/ikony_dgus/). Żaden
> element (ekran + sterownik razem) nie był uruchamiany na fizycznym sprzęcie. Lista prób sprzętowych: rozdz. 11.
> Wariant przejściowy (ILI9341 + WiFi) pozostaje w repozytorium jako działająca alternatywa (rozdz. 9).

## 1. Dlaczego bezpośrednio, bez drugiego ESP32

Wcześniejsza koncepcja (zachowana w historii repozytorium) zakładała kabel RS-485 do **drugiego ESP32** z LVGL,
który renderowałby ekran. DWIN DGUS eliminuje tę potrzebę: to gotowy moduł "wyświetlacz + dotyk + procesor +
firmware renderujący", podłączany **4 przewodami UART** (zasilanie 5 V, GND, TX, RX) wprost do wolnego portu
szeregowego sterownika. Zalety względem wariantu z drugim ESP32:

| Cecha | Wariant z drugim ESP32 (poprzednia koncepcja) | **DWIN DGUS (aktualna decyzja)** |
|-------|--------------------------------------------------|------------------------------------|
| Liczba mikrokontrolerów | 2 (sterownik + terminal LVGL) | **1** (tylko sterownik) |
| Firmware ekranu | własny kod C++/LVGL do napisania i utrzymania | **gotowy**, ekran renderuje sam wg projektu z DGUS Designer |
| Interfejs elektryczny | RS-485 (różnicowy, 2× MAX3485) | **UART TTL bezpośrednio** (+ prosty dzielnik napięcia na 1 linii) |
| Warstwa graficzna | trzeba programować (widgety, layout, fonty) w LVGL | projektuje się **wizualnie** w DGUS Designer (obrazy stron, przyciski) |
| Zmiana wyglądu ekranu | wymaga przeprogramowania i wgrania firmware ekranu | edycja projektu w Designerze, wgranie przez kartę SD ekranu |

Cena i dostępność: moduł 7" IPS 1024×600 z dotykiem pojemnościowym jest tańszy niż zestaw (ESP32-S3 + panel RGB +
2× konwerter RS-485) i dostępny w Polsce (TME, elty.pl).

## 2. Podział odpowiedzialności

| Warstwa | Sterownik (master) | Ekran DGUS |
|---------|--------------------|------------|
| Pistolety, enkoder, GPS, RTC, zabezpieczenia | tak | nie |
| Stan maszyny, wzorce, menu, ustawienia, funkcje serwisowe | tak (`menu*.cpp`, `control_api.cpp`) | nie |
| Wartości do pokazania (liczby, teksty, flagi) | tak (`dgus_pages.cpp`) | nie — tylko wyświetla |
| Wygląd stron, przyciski, ikony, fonty | nie | **tak** (projekt w DGUS Designer) |
| Zapis danych (NVS, SD, raporty, GPX, log) | tak | nie |
| WiFi AP, panel WWW, API dla telefonu | tak (bez zmian) | nie |
| Dotyk → kod zdarzenia | nie | tak |

Ekran **niczym nie steruje**: zgłasza kody zdarzeń, które przechodzą **te same ścieżki** co przyciski fizyczne
(`menu.handleEvent()`) i panel WWW (`executeControl()`).

## 3. Piny

### 3.1 Sterownik (`esp32s3_terminal`)

| Funkcja | Piny |
|---------|------|
| Przekaźniki P1–P6 | 41, 42, 1, 2, 3, 4 |
| Enkoder CLK/DT, GAP | 5, 6, 7 |
| Buzzer | 8 |
| **UART1 do ekranu DGUS** | **TX = GPIO 9, RX = GPIO 10** |
| SD (osobny moduł): MOSI / SCK / MISO / CS | 11, 12, 13, 16 |
| I2C (RTC + MCP23017: S1–S10, GRUPA) | 17, 18 |
| Joystick KY-023: VRx, VRy, SW | 19, 20, 46 (nawigacja menu — patrz rozdz. 6) |
| Status pętli STOP awaryjnego (E-STOP) | 21 (patrz rozdz. 7 — cięcie zasilania jest sprzętowe, ten pin to tylko odczyt statusu) |
| START, STOP | 38, 39 |
| SELEKTOR (opcjonalny — patrz [WIZUALIZACJE.md](WIZUALIZACJE.md)) | 40 |
| GPS | 47, 48 |
| Czujnik temperatury DS18B20 (opcjonalny, OneWire) | 15 (zajęty zawsze — pin zarezerwowany w firmware, nawet bez czujnika) |
| **Zajęte / wolne** | **27 zajęte, 1 wolny: GPIO 14** (+ GPIO 40 wolny tylko bez SELEKTORA — wtedy 2 wolne) |

GPIO 9/10 to ADC1 — po przejęciu na UART nie służą jako wejścia analogowe. GPIO 19/20 to ADC2 — mogą dawać
zaszumione odczyty przy aktywnym WiFi (`joystick.cpp` uśrednia próbki i stosuje histerezę, żeby to skompensować).
GPIO 46 to pin startowy (strap) — `joystick.cpp` obsługuje to bezpiecznie (patrz rozdz. 9), ale nic INNEGO nie
powinno wymuszać na nim stanu przy zasilaniu.

**Budżet pinów jest teraz bardzo ciasny** (1-2 wolne GPIO) — każda kolejna funkcja wymagająca bezpośredniego
GPIO (nie przez I2C) będzie wymagała albo zwolnienia jednego z powyższych, albo przeniesienia na ekspander I2C.

### 3.2 Ekran DGUS

| Funkcja | Pin modułu |
|---------|------------|
| Zasilanie | 5 V, GND (osobny bezpiecznik/PTC — pobór do ~500 mA przy pełnej jasności) |
| UART RX (dane od sterownika) | wejście 5 V-tolerant — ESP32 TX (3,3 V) można podłączyć **wprost** |
| UART TX (dane do sterownika) | wyjście **5 V** — **wymaga dzielnika/level-shiftera** przed wejściem RX sterownika (patrz rozdz. 10) |

## 4. Łącze i protokół

### 4.1 Warstwa fizyczna
UART TTL, **115 200 baud 8N1** (domyślna prędkość DGUS — do zmiany w konfiguracji ekranu razem z
`dgus::BAUD` w kodzie, jeśli potrzeba innej). Połączenie **bezpośrednie** (bez RS-485/konwerterów) — zakładany
krótki przewód wewnątrz jednej obudowy (patrz [WIZUALIZACJE.md](WIZUALIZACJE.md)). Dla dłuższych przewodów
(ekran montowany osobno) rozważ podniesienie napięcia sygnału przez RS-485 (2× MAX3485) — patrz
[LACZE_PRZEWODOWE.md](LACZE_PRZEWODOWE.md), które opisuje tę opcję jako uzupełnienie.

### 4.2 Ramka (`shared/dgus_protocol.h`)

Format **zgodny z oficjalną dokumentacją DWIN** ("T5L_DGUSII Application Development Guide", rozdział "Serial
Communication Protocol") — nie autorski, więc jest w 100% kompatybilny z prawdziwym ekranem:

```
5A A5 <BC> <CMD> <dane...>
```
- `5A A5` — stały nagłówek ramki.
- `BC` — liczba bajtów od `CMD` do końca ramki.
- `CMD = 0x82` — zapis VP (adresu zmiennej w RAM ekranu): `<VP_H><VP_L> <słowo0> [<słowo1> ...]` (słowa 16-bit,
  big-endian).
- `CMD = 0x83` — odczyt VP: żądanie `<VP_H><VP_L><liczba_słów>`, odpowiedź `<VP_H><VP_L><liczba_słów><dane...>`.

**Kluczowe:** ekran wysyła **niezażądaną** ramkę w **tym samym formacie co odpowiedź 0x83**, gdy dotknięty zostanie
element skonfigurowany w DGUS Designer na zapis do VP (przycisk, "return key"). Sterownik odbiera więc zdarzenia
dotyku jako zwykłe ramki `0x83` pod adresem `VP_TOUCH_EVENT` — nie trzeba niczego "żądać", wystarczy nasłuchiwać.

Przełączanie strony: zapis 2 słów `{0x5A01, numer_strony}` pod systemowy adres `VP 0x0084` (udokumentowany przez
DWIN jako stały adres przełączania stron) — `dgus::encodeSwitchPage()`.

CRC jest w DGUS **opcjonalne** (rejestr konfiguracyjny ekranu, domyślnie wyłączone) — projekt zakłada, że
pozostaje wyłączone (nie komplikujemy pierwszej wersji); `shared/dgus_protocol.h` nie go nie wysyła ani nie
wymaga.

### 4.3 Zasada: sterownik jedynym źródłem prawdy o ekranie

**Żaden przycisk w DGUS Designer nie ma ustawionego lokalnego "przejdź do strony X".** Każdy przycisk pisze tylko
kod zdarzenia pod `VP_TOUCH_EVENT` (0x1300); to **sterownik** decyduje, czy i dokąd przejść (`goToScreen()`), i
**sam** poleca ekranowi zmianę strony (zapis pod `VP 0x0084`). Dzięki temu stan `currentScreen` w sterowniku i
strona pokazywana przez ekran nigdy się nie rozjadą — nawet gdy zdarzenie dotyku zostanie odrzucone (np. próba
wejścia w czyszczenie dysz podczas malowania).

### 4.4 Niezawodność / wykrywanie utraty łącza

DGUS **nie wysyła własnego, okresowego "jestem żywy"** — odzywa się tylko gdy dotknięty jest przycisk albo w
odpowiedzi na żądanie odczytu. Sterownik więc **sam odpytuje** adres `VP_PING` co ok. 700 ms (`dgus_link.cpp`);
sama odpowiedź (niezależnie od treści) jest dowodem, że ekran żyje. Adres do odpytywania jest **celowo różny**
od `VP_TOUCH_EVENT` — odpytywanie adresu zdarzeń dotyku zwróciłoby jego ostatnią zapisaną wartość i sterownik
błędnie potraktowałby to jako nowe naciśnięcie.

- **Utrata łącza:** brak jakiejkolwiek ramki > **1,2 s**. Sterownik ustawia alarm, dzwoni co 3 s podczas
  malowania i loguje zdarzenie. Polityka (jak w NVS): **kontynuuj + alarm** (domyślnie) albo **auto-pauza**
  (`POST /api/control action=set_term_policy`).
- **STOP z ekranu** idzie tą samą ścieżką co `send_event` z panelu WWW — `menu.handleEvent(EVT_STOP_*)`.
- **Reset ekranu w trakcie pracy:** sterownik cały czas wysyła status (co 250 ms na ekranie roboczym); po
  restarcie ekran od razu dostaje aktualny stan. Malowanie nie jest przerywane.

## 5. Ekrany

**Ekran roboczy** (HOME/PAINTING) i **kolumny wzorców S1–S10** to strony w DGUS Designer wypełniane blokiem
statusu (`VP_STATE`..`VP_CUSTOM_VALID`, `dgus_link.cpp::sendHomeStatus()`) i blokiem soft-key
(`VP_SLOT_*`, `sendSoftkeys()`) — odświeżane ok. 4×/s. Blok statusu obejmuje **komplet** danych sterownika
przydatnych operatorowi w polu: stan i tryb pracy, wzorzec, prędkość/dystans/powierzchnia/czas, pistolety (w tym
**anomalia pistoletu** — czerwony alarm jak dawny baner LVGL), poziom farby, **pełne dane GPS** (pozycja, prędkość,
HDOP, zapis i przepełnienie trasy GPX), **stan kalibracji enkodera**, **odczyt czujnika temperatury** DS18B20
(opcjonalny), tryb nocny i podstawową diagnostykę systemu (wolna pamięć, czas pracy, liczba klientów WiFi) —
pełna lista pól: [docs/EKRAN_DGUS.md](EKRAN_DGUS.md), rozdz. 3. **Pilot przewodowy (J4) i pedał (J5) nie mają
osobnej reprezentacji na ekranie** — elektrycznie są równoległe do przycisków fizycznych, więc z punktu widzenia
oprogramowania to te same zdarzenia co przyciski START/STOP/SELEKTOR/GAP.

**Ekrany serwisowe** (SERVICE_MENU, CALIBRATION, DISTANCE_METER, REPORTS, NOZZLE_CLEAN, SETUP, SESSION_RESET,
COUNTER_RESET, SUMMARY, LIFETIME_STATS, CUSTOM_PATTERN, STATS_EXPORT, FACTORY_RESET, TANKOWANIE, POST) — jedna
strona DGUS na ekran, z **etykietami i przyciskami narysowanymi na stałe w Designerze**; sterownik wypełnia tylko
dziesięć generycznych "wartości wiersza" (`MenuSystem::fillDgusPage()`, `dgus_pages.cpp`) — dokładna specyfikacja
pól i przycisków każdej strony: **[docs/EKRAN_DGUS.md](EKRAN_DGUS.md)**.

Wejście do serwisu: przycisk **"SERWIS"** na ekranie roboczym wysyła kod `4` (`EVT_STOP_LONG`) — dokładnie to, co
dziś robi długie przytrzymanie fizycznego STOP.

## 6. Zdarzenia dotyku

Pełna tabela kodów: `src/dgus/dgus_map.h` (jedno źródło prawdy, używane przez firmware) i
[docs/EKRAN_DGUS.md](EKRAN_DGUS.md) (czytelna wersja + instrukcja konfiguracji przycisków w Designerze). Skrót:

| Zakres kodów | Znaczenie |
|---|---|
| 1–7 | dokładnie `enum ButtonEvent` sterownika — ta sama ścieżka co przycisk fizyczny |
| 30–40 | bezpośrednie wejście do pozycji menu serwisowego (`set_screen`) |
| 70–79 | soft-key S1–S10 (numer slotu = kod−70) |
| 80 | GRUPA (przełącz OŚ ⇄ KRAWĘDŹ) |
| 90 / 91 | "martwy człowiek" czyszczenia dysz: naciśnięto / puszczono |
| 41 | potwierdzenie operatora po ustąpieniu STOP-u awaryjnego (`KEY_ESTOP_ACK`) — patrz rozdz. 7 |

**Nawigacja menu również przez joystick.** Fizyczny joystick KY-023 (GPIO 19/20/46, `src/joystick.cpp`) generuje
dokładnie ten sam `enum ButtonEvent` co kody dotyku 1–7 powyżej i trafia do **tej samej** funkcji
`menu.handleEvent()` — ekran DGUS niczego o tym nie wie i nie musi. Góra/dół = poprzedni/następny element
(auto-powtarzanie przy przytrzymaniu), lewo/prawo = cofnij/wejdź, SW krótko = potwierdź, SW długo = SETUP z
HOME. Na ekranach roboczych (HOME/PAINTING/SUMMARY) zdarzenia z osi analogowych są blokowane (tylko przycisk SW
przechodzi) — zapobiega to przypadkowemu „cofnij” z szumu ADC2 podczas malowania (`main.cpp`).

## 7. Bezpieczeństwo

| Zagrożenie | Środek | Gdzie |
|------------|--------|-------|
| Ekran wysyła błędny/nieznany kod | kody poza zdefiniowanym zakresem są ignorowane i logowane | `dgus_link.cpp::handleTouchEvent()` |
| Zawieszenie/odłączenie ekranu | ping co 700 ms, próg 1,2 s, alarm; polityka utraty (rozdz. 4.4); fizyczny STOP niezależny | `dgus_link.cpp` |
| Test dysz z ekranu | **martwy człowiek**: kod 90 (naciśnięto) / 91 (puszczono) + **twardy limit 8 s** na wypadek zgubienia komunikatu zwolnienia; wejście tylko w stanie GOTOWY/ZATRZYMANY | `dgus_link.cpp`, `control_api.cpp` |
| Reset liczników / factory reset z dotyku | te same kody `EVT_START_LONG`/`EVT_STOP_LONG` co fizyczne przyciski; wielostopniowa nawigacja (menu → pozycja → potwierdzenie) jest już zabezpieczeniem przed przypadkowym dotknięciem; jeśli używana wersja DGUS Designer pozwala skonfigurować wyzwolenie po przytrzymaniu przycisku — **zalecane** dodatkowe zabezpieczenie (do zweryfikowania w Designerze) | `menu_handlers.cpp` |
| Tryb RĘCZNY | strzał tylko przy **fizycznie** trzymanym START — ekran nie może tego obejść | `painting_engine.cpp` (bez zmian) |
| STOP | przerwanie sprzętowe (GPIO 39) bez zmian; STOP z ekranu to `menu.handleEvent(EVT_STOP_*)` jak z panelu fizycznego | — |
| Uszkodzona ramka | parser odrzuca nieprawidłową długość/format i resynchronizuje się na kolejnym nagłówku `5A A5` | `dgus_protocol.h::Parser` |
| STOP awaryjny (E-STOP) | **cięcie zasilania pistoletów/pomp jest sprzętowe** (grzybek NC w torze zasilania modułu przekaźników — nie zależy od firmware); GPIO 21 tylko odczytuje status tej pętli. Obrona w głąb: ten sam ISR co STOP (GPIO 39) zeruje przekaźniki programowo w mikrosekundy po zboczu na GPIO 21; `estop.cpp::update()` (odszumione) wymusza `STATE_STOPPED`, alarm dźwiękowy co 1,5 s i log zdarzeń; zerwany przewód pętli jest interpretowany jako zadziałanie (fail-safe). Wznowienie malowania (`action=start`) jest **zablokowane** dopóki operator nie potwierdzi ustąpienia (kod dotyku 41 / `action=ack_estop`) | `estop.h/.cpp`, `guns.cpp::beginEmergencyStop()`, `control_api.cpp` |

**Polityka utraty ekranu podczas malowania** (zapisywana w NVS): `0` = **kontynuuj + alarm** (domyślnie), `1` =
automatyczna pauza. Ustawienie: `POST /api/control action=set_term_policy&value=0|1`. Alarm włącza się tylko, gdy
łącze było wcześniej nawiązane (start bez podłączonego ekranu nie alarmuje).

## 8. Mapa kodu

| Element | Plik | Uwagi |
|---------|------|-------|
| Ramka DGUS (5A A5, VP, odczyt/zapis) | `shared/dgus_protocol.h` | zweryfikowane ręcznie względem przykładów z dokumentacji DWIN; wspólne dla sterownika i testów |
| Mapa VP / stron / kodów zdarzeń | `src/dgus/dgus_map.h` | **jedyne źródło prawdy** — także specyfikacja do DGUS Designer |
| Łącze po stronie sterownika | `src/dgus/dgus_link.cpp/.h` | UART1 (ESP-IDF), pętla Core 1, nie blokuje; ping, polityka utraty, martwy człowiek |
| Dane dla ekranu (status roboczy, soft-key, strony serwisowe) | `src/dgus/dgus_pages.cpp`, `src/dgus/dgus_pages.h` | `MenuSystem::fillDgusPage()` używa tych samych pól co obsługa ILI9341 |
| Wspólne polecenia | `src/control_api.cpp/.h` | `executeControl()` — HTTP, WWW i ekran DGUS wołają to samo |
| Status STOP-u awaryjnego | `src/estop.h/.cpp` | Odczyt pętli (GPIO 21), debounce, log/buzzer/`requestStop()`; rzeczywiste cięcie zasilania jest sprzętowe — patrz rozdz. 7 |
| Flagi wariantu | `src/config.h` | `HAS_SMALL_TFT`, `HAS_JOYSTICK`, `HAS_DGUS_LINK`, `HAS_ESTOP`; `#error` przy kolizji pinów |
| Headless (docelowy) | `platformio.ini` `[env:esp32s3_terminal]` | bez `display_*.cpp`, bez TFT_eSPI |
| Testy | `test/test_dgus_protocol/` | ramka, VP, przełączanie strony, resynchronizacja, limity — przykłady 1:1 z dokumentacji DWIN |
| **Specyfikacja projektu ekranu** | [docs/EKRAN_DGUS.md](EKRAN_DGUS.md) | strony, pola, przyciski — do ręcznego zbudowania w DGUS Designer |

## 9. Wariant przejściowy — zachowany

Środowisko `esp32s3` (sterownik z ILI9341 i joystickiem, WiFi) oraz moduł 7" `display-module/` (`sunton7`,
`sunton7_portrait` — WiFi, gruby klient) **nadal się kompilują i działają jak dotąd**, niezmienione tą pracą.
Jedyna zmiana zachowania wariantu przejściowego: wejście do czyszczenia dysz jest zablokowane podczas
malowania/pauzy (zabezpieczenie wprowadzone razem z tą architekturą, dotąd nieosiągalne z panelu fizycznego).

## 10. Sprzęt (zmiany BOM i schematu)

| Element | Zmiana |
|---------|--------|
| ILI9341 2,8" + Touch | **usunięte** (zastąpione ekranem DGUS) |
| Joystick KY-023 | **zachowany** — nawigacja menu równolegle z dotykiem ekranu (rozdz. 6), piny 19/20/46 bez zmian |
| Moduł microSD (SPI) | **dodać** (MOSI 11, SCK 12, MISO 13, CS 16), gniazdo wyprowadzone na obudowę |
| **Wyświetlacz DGUS 7" (np. `DMG10600T070_09WTC`)** | **dodać** — zasilanie 5 V (osobny bezpiecznik), UART bezpośrednio do sterownika |
| **Dzielnik napięcia / level-shifter** na linii TX ekranu → RX sterownika | **dodać** (np. 2 rezystory 10 kΩ/20 kΩ, albo gotowy moduł I2C/UART level-shifter) — ekran ma wyjście 5 V, GPIO ESP32 toleruje max ~3,6 V |
| **Przycisk grzybkowy E-STOP (styk NC)** | **dodać** — wpięty **fizycznie w tor zasilania** modułu przekaźników (tnie prąd niezależnie od firmware); dodatkowa pętla statusu do GPIO 21 (INPUT_PULLUP, patrz rozdz. 7) |
| Przyciski fizyczne | S1–S10 + GRUPA + START + STOP + GAP = **14**, plus opcjonalny SELEKTOR = 15, plus joystick (4 kierunki + SW) i grzybek E-STOP (pilot J4, pedał J5 — opcje) |

Pełny schemat wszystkich połączeń (ta alternatywa): [schematy/schemat_polaczen_dgus.svg](schematy/schemat_polaczen_dgus.svg)
(patrz [SCHEMAT_PODLACZEN.md](SCHEMAT_PODLACZEN.md), rozdz. 3.3/5.2b). Obudowa z ekranem pionowym:
[WIZUALIZACJE.md](WIZUALIZACJE.md), dwie wersje (OŚ/KRAWĘDŹ): [obudowa_pionowa_os.svg](schematy/obudowa_pionowa_os.svg),
[obudowa_pionowa_krawedz.svg](schematy/obudowa_pionowa_krawedz.svg).

## 11. Próby sprzętowe (do wykonania przed użyciem w terenie)

Nic z poniższej listy nie zostało jeszcze sprawdzone:

1. **Zbudowanie projektu w DGUS Designer** wg [EKRAN_DGUS.md](EKRAN_DGUS.md) i wgranie go na ekran (karta SD ekranu).
2. Napięcia: potwierdzić multimetrem, że dzielnik na linii TX ekranu → RX sterownika faktycznie ogranicza sygnał
   do ~3,3 V (nie za nisko — ESP32 musi jeszcze rozpoznać stan wysoki).
3. Wymiana ramek sterownik ↔ ekran 24 h: brak błędów ramki, licznik `framesBad()` bliski zeru, brak restartów.
4. Wszystkie strony serwisowe z tabeli [EKRAN_DGUS.md](EKRAN_DGUS.md): nawigacja, wartości, przełączanie stron
   przez sterownik (nie lokalnie przez ekran).
5. **Test dysz z martwym człowiekiem:** puszczenie przycisku, odłączenie kabla w trakcie, zawieszenie ekranu —
   dysze zamykają się w rozsądnym czasie (docelowo < 400 ms, ograniczone przez `NOZZLE_HOLD_MAX_MS`=8 s jako
   twardy limit, nie normalny czas reakcji).
6. Utrata ekranu w trakcie malowania: alarm dźwiękowy, obie polityki (kontynuuj / pauza), powrót łącza.
7. Fizyczny STOP przy zawieszonym/odłączonym ekranie.
8. Karta SD jako osobny moduł na pinach 11/12/13/16 (raporty, GPX, backup NVS) — bez kolizji z niczym (ekran nie
   dzieli już z nią magistrali SPI).
9. Start bez podłączonego ekranu (sterownik czeka na START z panelu fizycznego — patrz `main.cpp`) i podłączenie
   ekranu już po starcie.
10. Czytelność w słońcu i odpowiedź dotyku w rękawicach (dotyk pojemnościowy — może wymagać rękawic dotykowych
    albo zmiany na wariant rezystancyjny, jeśli okaże się niepraktyczny w terenie).
11. **Joystick:** nawigacja po wszystkich ekranach serwisowych, brak fałszywych zdarzeń z szumu ADC2 przy
    aktywnym WiFi (histereza w `joystick.cpp`), brak błędnego trybu bootowania od GPIO 46 (strap pin) przy
    różnych stanach przycisku SW w momencie włączenia zasilania.
12. **STOP awaryjny:** zmierzyć multimetrem czas między wciśnięciem grzybka a zanikiem napięcia na wyjściach
    przekaźników (powinien być natychmiastowy — cięcie sprzętowe); potwierdzić, że GPIO 21 poprawnie odczytuje
    stan pętli w obu kierunkach (zamknięta/otwarta) i że przerwany przewód pętli też jest sygnalizowany jako
    zadziałanie; sprawdzić blokadę `action=start` do czasu potwierdzenia (`ack_estop`).

## 12. Ograniczenia i dalsze prace

- **Projekt w DGUS Designer nie istnieje jeszcze** — kod po stronie sterownika jest gotowy i czeka na strony
  zbudowane według specyfikacji (rozdz. 8, [EKRAN_DGUS.md](EKRAN_DGUS.md)).
- Rysunki wzorców "w skali" (jak w LVGL) nie są renderowane dynamicznie — DGUS pokazuje **statyczne ikony**
  wybierane numerem wzorca (0–15); wygląda dobrze, ale nie jest to żywy podgląd geometrii jak w module WiFi.
  Zestaw ikon (16 wzorców + WŁASNY) jest **już gotowy** jako pliki PNG —
  [`docs/schematy/ikony_dgus/`](schematy/ikony_dgus/) — zostaje tylko zaimportować je w Designerze (patrz README
  w tym folderze).
  "Droga" (animacja perspektywiczna) — analogicznie: nie da się jej łatwo odtworzyć w DGUS; `VP_PATDIST_DM`
  zostaje jako dana liczbowa (np. pod prosty pasek postępu), a nie animowana grafika.
- Kod QR z danymi WiFi nie jest rysowany — ekran startowy pokazuje SSID, hasło i adres tekstem (8 wierszy).
- Kody zdarzeń dotyku (`ev`) nie mają potwierdzeń — pojedyncza zgubiona ramka wymaga ponownego dotknięcia;
  polecenia idące przez `set` (np. `set_pattern`) **mają** odpowiedź (`rs`) w poprzedniej koncepcji RS-485, ale
  w obecnym, prostszym modelu DGUS-owym (rozdz. 4) nie ma kanału potwierdzeń — błędy sygnalizowane są tylko
  dźwiękiem błędu (`BUZ_ERROR`).
- Tryb nocny (`VP_NIGHT`) trzeba spiąć z jasnością podświetlenia w konfiguracji DGUS (rejestr backlightu) —
  do zweryfikowania dokładnego mechanizmu w używanej wersji Designera.
- Dokładna prędkość transmisji i ewentualne CRC muszą się zgadzać **jeden do jednego** między `dgus::BAUD` w
  kodzie a konfiguracją ekranu (`CONFIG.TXT`/ustawienia w Designerze) — zmiana jednej strony bez drugiej zerwie
  łącze.

## 13. Decyzje (przyjęte)

| # | Decyzja | Wdrożenie |
|---|---------|-----------|
| D1 | Medium łącza | **UART bezpośrednio do ekranu inteligentnego DGUS** (nie RS-485/drugi ESP32); WiFi zostaje tylko dla telefonu |
| D2 | Utrata ekranu w trakcie malowania | kontynuuj + alarm (domyślnie), opcja auto-pauza |
| D3 | Karta SD | osobny moduł SPI |
| D4 | Joystick i SELEKTOR | joystick **przywrócony** — nawigacja menu współdzielona z dotykiem (`menu.handleEvent()`, rozdz. 6); SELEKTOR opcjonalny |
| D5 | WiFi AP sterownika dla telefonu | zachowane |
| D6 | Wariant przejściowy | zachowany (`esp32s3`, `sunton7*`) |
| D7 | Test dysz z ekranu | martwy człowiek (kody 90/91 + twardy limit 8 s) + tylko gdy maszyna nie maluje |
| D8 | Renderer ekranu | **gotowy silnik DGUS** (projekt w Designerze), nie własny kod LVGL na drugim ESP32 |
| D9 | STOP awaryjny (E-STOP) | cięcie zasilania **sprzętowe** (grzybek NC w torze zasilania przekaźników); GPIO 21 tylko status + ISR programowy jako obrona w głąb (rozdz. 7); wznowienie wymaga potwierdzenia operatora |
