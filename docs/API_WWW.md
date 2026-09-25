# MPD2026 — API sterownika (HTTP + WebSocket)

**Firmware sterownika:** 2.52.0 (MPD2026). Z API korzystają: panel WWW, aplikacja Android, **moduł wyświetlacza 7"**
oraz dowolny klient (curl, skrypty).

## 1. Informacje ogólne

| Parametr | Wartość |
|----------|---------|
| Tryb sieci | WiFi Access Point sterownika |
| SSID | `TrassarV3` |
| Hasło | **8 znaków HEX = ostatnie 4 bajty adresu MAC ESP32** (unikalne dla urządzenia; widoczne na ekranie startowym sterownika w kodzie QR i tekście) |
| Adres IP | `192.168.4.1` |
| HTTP | port 80 |
| WebSocket | port 81 (`ws://192.168.4.1:81`), broadcast statusu co 500 ms |
| Kanał WiFi | 6 |
| Maks. klientów | 4 (moduł 7" zajmuje jednego) |
| Format danych | JSON (ArduinoJson v7) |
| Uwierzytelnianie | brak (dostęp chroniony hasłem WiFi) |

> Poprzednie wersje dokumentacji podawały hasło `12345678` — jest nieaktualne. Hasło jest generowane z MAC
> w `web_server.cpp` (`generatePassword()`).

## 2. Endpointy

| Metoda | Ścieżka | Opis |
|--------|---------|------|
| GET | `/` | Panel WWW (HTML) |
| GET | `/api/status` | Pełny status maszyny (JSON) — ten sam co broadcast WebSocket |
| GET | `/api/stats` | Statystyki sesji i lifetime, pistolety, farba |
| GET | `/api/reports` | Lista raportów CSV z karty SD (cache 15 s) |
| GET | `/api/reports/download?file=` | Pobranie raportu CSV |
| GET | `/api/reports/geojson` | Eksport raportów jako GeoJSON |
| GET | `/api/tracks` | Lista tras GPS (GPX / GeoJSON) |
| GET | `/api/tracks/download?file=` | Pobranie trasy |
| GET | `/api/html_reports` | Lista raportów HTML sesji |
| GET | `/api/html_reports/download?file=` | Pobranie raportu HTML |
| POST | `/api/control` | Polecenia sterujące (form-urlencoded) |

Nazwy plików w parametrze `file` mogą zawierać wyłącznie litery, cyfry, `.`, `_`, `-` (inaczej HTTP 400).

## 3. GET /api/status

Wszystkie wartości liczbowe (`speed`, `distance`, `area`, `ppm`, `gpsLat`, …) są zwracane jako **liczby JSON**
(formatowane z ustaloną liczbą miejsc po przecinku). Klient powinien tolerować także zapis tekstowy.

```json
{
  "state": "painting",
  "screen": 1, "menuIndex": 0,
  "mode": "auto", "semiLineComplete": false,
  "pattern": "P-3a", "patternName": "Przekraczalna dluga", "patternIdx": 7,
  "reversed": false, "gapStart": false,
  "customValid": true, "activeSlot": 0, "slotsValid": [true, false, false],
  "customGuns": [["P2", 12, 3.0, 2.0], ["P5", 12, 0, 0]],
  "speed": 6.4, "distance": 152.3, "area": 18.28, "elapsed": 96,
  "patDist": 152.3,
  "firmware": "2.52.0", "freeHeap": 245760, "minFreeHeap": 210000,
  "uptime": 3600, "clients": 2, "webStackHWM": 2048, "littleFs": true,
  "calibrated": true, "ppm": 100.0, "calibrating": false, "calPulses": 0,
  "maxSpeed": 15.0, "minSpeed": 3.0, "overspeed": false, "lowSpeed": false,
  "guns": [true, false, true, false, false, false],
  "autoPaused": false, "autoResumeEnabled": true, "semiSegment": 1,
  "smartSwitch": true, "patternPending": false, "pendingPattern": "P-2a",
  "gpsFix": true, "gpsLat": 52.229676, "gpsLng": 21.012229, "gpsSat": 8,
  "gpsSpeed": 6.2, "gpsHdop": 1.2, "gpxRec": true, "gpxPts": 20,
  "paintLevelL": 172.5, "paintLevelPct": 86,
  "gunAnomalyDetected": false, "gunAnomaly": [false, false, false, false, false, false]
}
```

| Pole | Typ | Opis |
|------|-----|------|
| `state` | string | `idle`, `painting`, `paused`, `stopped` |
| `screen` | int | aktualny ekran TFT sterownika (enum `ScreenID`) |
| `menuIndex` | int | pozycja kursora w menu |
| `mode` | string | `auto`, `semi`, `manual`, `demo` |
| `semiLineComplete` | bool | SEMI: kreska zakończona, czeka na START |
| `semiSegment` | int | SEMI: numer segmentu (linii) w etapie |
| `pattern`, `patternName` | string | kod i nazwa aktualnego wzorca |
| `patternIdx` | int | indeks 0–15 (15 = WŁASNY) |
| `reversed` | bool | wzorzec odwrócony (P-3a/P-3b) |
| `gapStart` | bool | aktywny start od przerwy |
| `customValid` | bool | wzorzec własny zapisany i gotowy |
| `activeSlot` | int | aktywny slot wzorca własnego (0–2) |
| `slotsValid` | bool[3] | które sloty są zapisane |
| `customGuns` | array | konfiguracja wzorca własnego: `[nazwa, szer.cm, kreska_m, przerwa_m]`; kreska = 0 → pistolet ciągły; tylko pistolety aktywne (obecne, gdy `customValid`) |
| `speed` | number | prędkość [km/h] |
| `distance`, `area`, `elapsed` | number | dystans [m], powierzchnia [m²], czas [s] bieżącego etapu |
| **`patDist`** | number | **dystans od startu wzorca [m]** (0 poza malowaniem/pauzą). Pistolet przerywany strzela, gdy `fmod(patDist, kreska+przerwa) < kreska`. Używany przez moduł 7" do synchronizacji animacji drogi |
| `firmware`, `freeHeap`, `minFreeHeap`, `uptime`, `clients`, `webStackHWM`, `littleFs` | — | diagnostyka |
| `calibrated`, `ppm`, `calibrating`, `calPulses` | — | kalibracja enkodera |
| `maxSpeed`, `minSpeed` | number | progi prędkości [km/h] |
| `overspeed`, `lowSpeed` | bool | alarmy prędkości |
| `guns` | bool[6] | stan pistoletów P1–P6 (true = strzela) |
| `autoPaused`, `autoResumeEnabled` | bool | auto-pauza aktywna / auto-wznowienie włączone |
| `smartSwitch` | bool | true = Smart (dokończ cykl), false = Instant |
| `patternPending`, `pendingPattern` | bool, string | oczekująca zmiana wzorca i jego kod |
| `gpsFix`, `gpsLat`, `gpsLng`, `gpsSat`, `gpsSpeed`, `gpsHdop` | — | GPS |
| `gpxRec`, `gpxPts` | bool, int | zapis trasy GPS |
| `paintLevelL`, `paintLevelPct` | number, int | poziom farby w zbiorniku |
| `gunAnomalyDetected`, `gunAnomaly` | bool, bool[6] | detekcja anomalii pistoletów |

`patDist` jest dodane w MPD2026; starsze sterowniki go nie zwracają (klient powinien przyjąć rezerwę).

## 4. GET /api/stats

```json
{
  "lifetimeDistanceM": 12500.5, "lifetimeAreaM2": 3200.75, "lifetimePaintTimeSec": 86400,
  "sessionDistanceM": 250.3, "sessionAreaM2": 30.04, "sessionTimeSec": 180,
  "gunDistances": [250.3, 0.0, 250.3, 0.0, 0.0, 0.0],
  "gunShotCounts": [1250, 0, 1248, 0, 0, 0],
  "sdReady": true, "reportCount": 12,
  "paintUsedL": 18.0, "paintRemainingL": 182.0, "paintTankL": 200,
  "paintUsedPct": 9, "paintCurrentLevelL": 182.0,
  "refuelCount": 2, "totalRefueledL": 150.0
}
```

## 5. POST /api/control

Parametry: `action` (wymagany) oraz `value` lub parametry szczegółowe. Odpowiedź: `{"result":"ok"}` albo
`{"result":"<komunikat błędu>"}` (HTTP 200); HTTP 503 `{"error":"serwer zajety — sprobuj ponownie"}` gdy stan
jest chwilowo zablokowany (klient powinien ponowić); HTTP 400 przy braku `action`.

| Akcja | Wartość | Opis / walidacja |
|-------|---------|------------------|
| `start` | — | START z GOTOWY/ZATRZYMANY; wznowienie z PAUZA; zamknięcie ekranu QR startowego |
| `start_from_gap` | — | start od przerwy (tylko GOTOWY/ZATRZYMANY) |
| `pause` | — | pauza |
| `stop` | — | zatrzymanie (`requestStop()`: pistolety OFF natychmiast, zapis danych w pętli Core 1) |
| `set_pattern` | 0–15 | wybór wzorca (15 = WŁASNY, wymaga `customValid`). W trakcie malowania: Smart kolejkuje do końca cyklu, Instant zmienia od razu |
| `toggle_reverse` | — | odwróć P-3a/P-3b |
| `set_mode` | 0–3 | 0 AUTO, 1 SEMI, 2 RĘCZNY, 3 DEMO. **Tylko gdy stan GOTOWY/ZATRZYMANY**; zapis NVS |
| `semi_next_line` | — | SEMI: następna kreska (tylko gdy `semiLineComplete`) |
| `cal_start` / `cal_finish` | — | początek / koniec kalibracji enkodera (10 m) |
| `set_max_speed` | 5–30 | próg maksymalnej prędkości [km/h]; musi być > `minSpeed`; zapis NVS |
| `set_min_speed` | 0–10 | próg minimalnej prędkości [km/h]; musi być < `maxSpeed`; zapis NVS |
| `set_switch_mode` | 0 / 1 | 0 = Smart, 1 = Instant; zapis NVS |
| `set_auto_resume` | 0 / 1 | auto-wznowienie po auto-pauzie; zapis NVS |
| `set_tank_capacity` | 1–1000 | pojemność zbiornika [L]; zapis NVS |
| `set_paint_rate` | 0.1–5.0 | współczynnik zużycia [l/m²]; zapis NVS |
| `refuel` | 1–1000 | dolanie farby [L] (dodaje do bieżącego poziomu) |
| `save_custom_pattern` | patrz niżej | zapis wzorca własnego do slotu i aktywacja slotu |
| `activate_slot` | 0–2 | aktywuj zapisany slot |
| `get_slot_config` | `slot`=0–2 | zwraca `{"guns":[{"mode":0..2,"ln":..,"gp":..}, ...],"valid":bool}` (odpowiedź JSON, nie `result`) |
| `send_event` | 1–7 | wirtualny przycisk fizyczny (kolejka do Core 1): 1 START krótko, 2 START długo, 3 STOP krótko, 4 STOP długo, 5 SELEKTOR krótko, 6 SELEKTOR długo, 7 GAP |
| `set_screen` | 0–`SCREEN_STATS_EXPORT` | przejście do ekranu TFT sterownika |

### save_custom_pattern

| Parametr | Zakres | Opis |
|----------|--------|------|
| `g0`…`g5` | 0 / 1 / 2 | tryb pistoletu P1–P6: 0 wyłączony, 1 ciągły, 2 przerywany |
| `ln0`…`ln5` | 0.1–50.0 | długość kreski [m] (przerywany) |
| `gp0`…`gp5` | 0.1–50.0 | długość przerwy [m] (przerywany) |
| `slot` | 0–2 | slot (domyślnie 0) |

Po zapisie slot jest aktywowany (wzorzec własny staje się ważny); aby go użyć, wyślij `set_pattern` z wartością 15.

### Indeksy wzorców

| # | Wzorzec | # | Wzorzec |
|---|---------|---|---------|
| 0 | P-1a | 8 | P-3b |
| 1 | P-1b | 9 | P-4 |
| 2 | P-1c | 10 | P-6 |
| 3 | P-1d | 11 | P-7a |
| 4 | P-1e | 12 | P-7b |
| 5 | P-2a | 13 | P-7c |
| 6 | P-2b | 14 | P-7d |
| 7 | P-3a | 15 | WŁASNY |

### Przykłady (curl)

```bash
curl http://192.168.4.1/api/status
curl -X POST -d "action=set_pattern&value=7"    http://192.168.4.1/api/control   # P-3a
curl -X POST -d "action=set_mode&value=1"       http://192.168.4.1/api/control   # SEMI
curl -X POST -d "action=start"                  http://192.168.4.1/api/control
curl -X POST -d "action=stop"                   http://192.168.4.1/api/control
curl -X POST -d "action=set_max_speed&value=12" http://192.168.4.1/api/control
curl -X POST -d "action=refuel&value=50"        http://192.168.4.1/api/control
# wzorzec własny: P1 ciągły, P5 przerywany 3 m / 2 m, slot 1 (indeks 0)
curl -X POST -d "action=save_custom_pattern&slot=0&g0=1&g1=0&g2=0&g3=0&g4=2&g5=0&ln4=3.0&gp4=2.0&ln0=4&gp0=8&ln1=4&gp1=8&ln2=4&gp2=8&ln3=4&gp3=8&ln5=4&gp5=8" http://192.168.4.1/api/control
curl -X POST -d "action=set_pattern&value=15"   http://192.168.4.1/api/control
```

## 6. WebSocket (port 81)

- Adres: `ws://192.168.4.1:81`. Serwer wysyła co 500 ms ramkę tekstową o **identycznej treści jak `GET /api/status`**.
- Przy krytycznie niskiej pamięci (< 32 KB) broadcast jest wyłączany; zdarza się też ramka `{}` (pominięty
  cykl przy zajętym stanie) — klient powinien ją ignorować.
- Klient nie wysyła ramek do serwera (polecenia idą przez HTTP).
- Zalecany fallback: jeśli przez ponad 1,5 s brak ramek, odpytywać `GET /api/status` co ok. 700 ms
  (tak robi moduł 7").

## 7. Uwagi implementacyjne

- Serwer działa na Core 0 (zadanie FreeRTOS), logika pistoletów na Core 1; dostęp do stanu przez spinlock. Dlatego
  `POST /api/control` może zwrócić HTTP 503 przy zajętym stanie — należy ponowić po kilkuset ms.
- `stop` nie wykonuje zapisu NVS/SD na Core 0: wyłącza pistolety natychmiast i deleguje zapis do Core 1.
- Zmiana trybu (`set_mode`) jest odrzucana w trakcie malowania i pauzy (komunikat w polu `result`).
- Klient nie powinien wysyłać poleceń częściej niż kilka na sekundę.
- Zawieszenie serwera WWW (Core 0) nie przerywa malowania — zadanie jest restartowane po 10 s.
