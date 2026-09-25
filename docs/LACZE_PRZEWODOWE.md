# Łącze przewodowe sterownik ↔ moduł 7" (zamiast WiFi)

**Pytanie:** czy nie lepiej połączyć fizycznie główny moduł ESP32 z modułem wyświetlacza zamiast używać WiFi?

**Krótka odpowiedź: tak — w maszynie pracującej w terenie łącze przewodowe jest lepsze niż WiFi.** Rekomenduję **RS-485
(UART 3,3 V przez konwertery) w jednym kablu z zasilaniem**, z WiFi jako łączem zapasowym i dla telefonów/tabletów.
Nie polecam natomiast wciskania wszystkiego w jeden ESP32 (patrz [niżej](#dlaczego-nie-jedna-płytka)).

> **Status:** to **propozycja projektowa**. Łącze przewodowe **nie jest jeszcze zaimplementowane** w oprogramowaniu —
> obecnie moduł 7" pracuje przez WiFi ([MODUL_WYSWIETLACZA.md](MODUL_WYSWIETLACZA.md)). Schemat elektryczny:
> [schemat_lacze_rs485.svg](schematy/schemat_lacze_rs485.svg), wiązka: [schemat_zlacza_wiazka.svg](schematy/schemat_zlacza_wiazka.svg) (złącze J6).

![Łącze RS-485](schematy/schemat_lacze_rs485.svg)

## 1. Porównanie: WiFi a kabel

| Kryterium | WiFi (obecnie) | Kabel RS-485 (propozycja) |
|-----------|----------------|----------------------------|
| Odporność na zakłócenia | podatne (silniki, przekaźniki, zawory, inne sieci 2,4 GHz) | wysoka (transmisja różnicowa, skrętka, ekran) |
| Czas uzyskania połączenia po włączeniu | kilka–kilkanaście s (skanowanie, asocjacja, WebSocket) | natychmiast po zasilaniu |
| Opóźnienie i przewidywalność (STOP) | zmienne; przy problemach ponowienia | stałe, kilkanaście ms |
| Konfiguracja przez użytkownika | trzeba wpisać hasło (zmienne z MAC) | brak — „włóż i działa" |
| Limit urządzeń | AP sterownika: 4 klienty łącznie z telefonami | 1 port dedykowany (telefony dalej przez WiFi) |
| Zasilanie modułu 7" | osobny przewód | ten sam kabel (5 V + GND) |
| Wyświetlanie ostrzeżenia „BRAK ŁĄCZNOŚCI" | możliwe z powodu radia | tylko przy fizycznym przerwaniu kabla |
| Wygoda (zdejmowany moduł, telefon) | duża | mniejsza — kabel i złącze; telefon nadal przez WiFi |
| Koszt / złożoność | zero (już działa) | 2 konwertery, kabel, złącza M12, praca programistyczna |
| Niezawodność mechaniczna | brak części ruchomych | złącze/kabel narażone na wibracje i wodę (M12 IP67 rozwiązuje) |

**Wniosek:** WiFi jest wygodne do prototypu i dla telefonów, ale to łącze przewodowe daje przewidywalność, jakiej
oczekuje się od panelu operatora maszyny. Po awarii łącza sterownik i tak dalej pracuje samodzielnie, więc łącze
przewodowe poprawia **komfort i pewność obsługi**, a nie zastępuje zabezpieczeń (fizyczny STOP pozostaje głównym).

## Dlaczego nie jedna płytka

Połączenie sterownika i wyświetlacza w jednym ESP32-S3 jest **niewykonalne bez dużej przeróbki**:

- Panel RGB 800 × 480 zajmuje ok. 20 GPIO (16 danych + DE, VSYNC, HSYNC, PCLK) plus podświetlenie i dotyk. ESP32-S3
  N16R8 ma po odjęciu GPIO 26–37 (Flash/PSRAM) niewiele ponad 30 użytecznych pinów.
- Sterownik potrzebuje ok. 30 pinów (6 przekaźników, SPI TFT + SD, I2C, enkoder, przyciski, GPS, buzzer, joystick).
- Przeniesienie części wejść/wyjść na ekspandery I2C (np. przekaźniki) oznaczałoby sterowanie pistoletami przez magistralę
  I2C — gorsze opóźnienia i nowe tryby awarii w torze bezpieczeństwa.
- Awaria lub restart jednego procesu (grafika, WiFi, pamięć) wpływałaby na sterowanie pistoletami. Dziś wyświetlacz
  można zawiesić lub zresetować bez wpływu na malowanie.

Dlatego **dwa układy z łączem przewodowym** to najlepszy kompromis.

## 2. Wybór medium

| Opcja | Ocena |
|-------|-------|
| UART 3,3 V wprost (3 przewody) | działa na 1–2 m w spokojnym otoczeniu; w kabinie z zaworami i przekaźnikami ryzykowne — **niezalecane** |
| **RS-485 (konwertery MAX3485, auto-kierunek)** | **zalecane**: odporne, tanie, do 1200 m, prosty protokół |
| CAN (TWAI w ESP32) | równie odporne, standard motoryzacyjny; wymaga transceiverów i więcej pracy programistycznej; sensowne, gdy dojdą kolejne urządzenia na magistrali |
| Ethernet | nadmiarowe |
| USB | nie (ograniczony zasięg, brak izolacji) |

## 3. Piny

| Strona | Piny UART1 | Uwagi |
|--------|-----------|-------|
| **Moduł 7" (Sunton)** | **GPIO 17 = TX, GPIO 18 = RX** | GPIO 17/18 są wolne (I2S wzmacniacza dźwięku i linia INT dotyku nie są w tym projekcie używane). Sprawdzić mostek R17 (INT dotyku ↔ GPIO 18). UART0 (GPIO 43/44) zostaje dla USB/programowania. |
| **Sterownik (ESP32-S3)** | **GPIO 20 = TX, GPIO 19 = RX** | To piny joysticka KY-023. Przy ekranie dotykowym joystick jest zbędny — w wariancie przewodowym pomijany. GPIO 15 (T_CS / DS18B20) zostaje wolny; GPIO 0/45/46 to piny rozruchowe — nie używać. |

Wersja bez rezygnacji z joysticka: brak drugiego wolnego pinu w sterowniku bez przenoszenia innych funkcji, dlatego
rekomenduję rezygnację z joysticka (jego funkcje przejmuje ekran dotykowy i przyciski S1–S10 / GRUPA).

## 4. Elektryka

- **Konwertery:** MAX3485 (3,3 V) z automatycznym przełączaniem kierunku (moduł „TTL ↔ RS485 auto flow control"), po jednym
  po każdej stronie. Zasilanie 3,3 V (moduł 7" ma wyprowadzony 3V3 na złączu rozszerzeń; sterownik z pinu 3V3).
- **Terminatory:** 120 Ω między A i B po obu końcach (jeden na konwerterze sterownika, jeden na module 7").
- **Kabel:** ekranowany, 2 × 2 × 0,5 mm² (para A/B skręcona, para +5 V/GND). Ekran do PE **po jednej stronie** (sterownik).
- **Złącze:** M12 A-coded, 4 piny, IP67 (J6): 1 = +5 V (brązowy), 2 = A (biały), 3 = GND (niebieski), 4 = B (czarny).
- **Spadek napięcia zasilania** (obciążenie ok. 0,5 A, miedź 0,5 mm² ≈ 36 Ω/km, pętla tam i z powrotem):

| Długość kabla | Rezystancja pętli | Spadek przy 0,5 A |
|---------------|-------------------|--------------------|
| 2 m | 0,14 Ω | 0,07 V |
| 3 m | 0,22 Ω | 0,11 V |
| 5 m | 0,36 Ω | 0,18 V |

Przy 5 V spadek do 0,2 V jest akceptowalny; dla dłuższych kabli zwiększyć przekrój lub zasilić moduł lokalnie.
- **Zabezpieczenia:** PTC ≥ 1 A i dioda TVS na odgałęzieniu 5 V modułu; przy długich kablach lub dużych różnicach mas
  rozważyć **izolowany** konwerter RS-485 (np. z izolacją ADM2483).
- **Masa:** GND kabla łączy GND sterownika i modułu; zaworów nie łączyć z GND ESP32.

## 5. Protokół (propozycja)

- Warstwa fizyczna: 230 400 baud, 8N1, ramki tekstowe zakończone `\n`.
- Format ramki: `$<JSON>*<CRC16-hex>\n` (suma kontrolna CRC-16/CCITT z treści JSON). Ramki z błędnym CRC są odrzucane.
- **Sterownik → moduł:** ramka statusu o **tej samej zawartości co `GET /api/status`** ([API_WWW.md](API_WWW.md)): co 100 ms podczas
  malowania, co 500 ms w spoczynku. Dzięki temu parser `parseStatus()` w module 7" pozostaje bez zmian.
- **Moduł → sterownik:** polecenia w formie `{"a":"set_pattern","v":7}` (te same akcje co `POST /api/control`) z numerem
  sekwencji; sterownik odpowiada `{"r":"ok"|"komunikat","seq":N}`. STOP jest powtarzany do skutku (jak dziś w łączu WiFi).
- **Heartbeat:** moduł wysyła `{"hb":1}` co 200 ms; brak heartbeatu > 1 s oznacza „brak panelu" (sterownik **nie** zatrzymuje
  malowania — tak jak przy utracie WiFi, decyzja o zatrzymaniu należy do operatora / fizycznego STOP).
- **Wybór łącza w module 7":** priorytet kabel, jeśli ramki dochodzą; w przeciwnym razie automatyczny powrót do WiFi.
  Na ekranie wskaźnik „KABEL" lub „WiFi".

## 6. Zmiany w oprogramowaniu (plan wdrożenia)

| Etap | Zakres | Ryzyko |
|------|--------|--------|
| 1 | **Sterownik:** refaktoryzacja `handleControl()` w `web_server.cpp` do współdzielonej funkcji `executeControl(action, args)` (bez zmiany zachowania HTTP) | niskie, dobrze testowalne |
| 2 | **Sterownik:** nowy moduł `serial_link.cpp/.h` (zadanie na Core 0): ramki statusu, odbiór poleceń, CRC, heartbeat; flaga włączenia w NVS; pominięcie joysticka przy włączonym łączu | średnie (piny 19/20 — najpierw wyłączyć joystick) |
| 3 | **Moduł 7":** transport UART1 (GPIO 17/18) w `link.cpp` obok WiFi, wspólny parser, automatyczny wybór łącza, wskaźnik w pasku górnym | niskie |
| 4 | Testy: odłączenie kabla w trakcie malowania, zakłócenia od pistoletów, pomiar opóźnienia STOP, jednoczesne WiFi (telefon) | wymaga sprzętu |
| 5 | Dokumentacja i schemat po potwierdzeniu na maszynie | — |

Szacunek kosztów sprzętu (orientacyjnie, do zweryfikowania): 2 konwertery RS-485 ok. 10–30 zł sztuka, złącza M12 z kablem
ok. 100–250 zł, PTC/TVS kilka złotych.

## 7. Zalecenia

1. **Wdrożyć łącze RS-485** jako etap po pierwszych testach panelu na sprzęcie (najpierw sprawdzić samą płytkę i ekran przez WiFi).
2. Zachować WiFi jako łącze zapasowe oraz dla telefonów/tabletów (raporty, panel WWW).
3. Fizyczny STOP (panel, pilot, pedał lub grzybek) pozostaje niezależnym zabezpieczeniem, niezwiązanym z żadnym łączem.
4. Zaplanować w obudowie miejsce na gniazdo M12 i konwerter RS-485 (patrz [WIZUALIZACJE.md](WIZUALIZACJE.md)).

Jeśli zdecydujesz się na wdrożenie, kolejnym krokiem jest etap 1–3 powyżej.
