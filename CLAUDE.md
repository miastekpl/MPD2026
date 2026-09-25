# MPD2026 — Komputer pokładowy malowarki pasów drogowych

## Opis projektu
Repozytorium zawiera dwa firmware'y i aplikację:
- **Sterownik Trassar** (`src/`) — ESP32-S3 (N16R8), steruje 6 pistoletami wg 16 wzorców (P-1 do P-7),
  tryby AUTO / SEMI-AUTO / MANUAL / DEMO. Jedyny element sterujący pistoletami.
- **Moduł wyświetlacza 7"** (`display-module/`) — Sunton ESP32-8048S070C, klient WiFi sterownika, LVGL.
  Nie steruje pistoletami; używa istniejącego API (WebSocket :81 + `POST /api/control`).
- **Aplikacja Android** (`android-app/`).

**Zasada pracy:** to repozytorium (MPD2026) jest rozwojową kopią `Trassar_251v3`. Repozytorium
`miastekpl/Trassar_251v3` pozostaje niezmienioną referencją — nowe zmiany zapisujemy wyłącznie tutaj.
Zmiany w sterowniku mają być **addytywne** (nie usuwać funkcjonalności). Dokumentacja: `docs/`
(instrukcja, schemat połączeń, API, moduł 7") — aktualizować razem ze zmianami w kodzie.

## Platforma i build
- **MCU**: ESP32-S3 N16R8 (dual-core, 16MB flash, 8MB PSRAM)
- **Framework**: Arduino (PlatformIO)
- **Build**: `pio run`
- **Upload**: `pio run -t upload`
- **Monitor**: `pio device monitor` (115200 baud)
- **Plik konfiguracji**: `platformio.ini`

## Struktura katalogów
```
src/                    # Cały kod źródłowy (.cpp + .h w jednym katalogu)
  config.h              # Piny, stałe, enumy, struktury globalne
  main.cpp              # setup() + loop() — Core 1
  web_server.cpp/.h     # WiFi AP + REST API + WebSocket (Core 0, FreeRTOS task)
  web_html.h            # HTML/CSS/JS panelu sterowania (PROGMEM)
  display_manager.cpp/.h        # Rdzeń modułu wyświetlacza ILI9341
  display_internal.h            # Stałe layoutu i makra czcionek (wewnętrzne)
  display_screens_main.cpp      # Ekrany HOME + PAINTING
  display_screens_service.cpp   # Ekrany serwisowe (kalibracja, raporty, itp.)
  menu.cpp/.h           # System menu — rdzeń (begin, goToScreen, handleEvent, update)
  menu_handlers.cpp     # Handlery zdarzeń per ekran
  painting_engine.cpp/.h  # Silnik malowania (sterowanie pistoletami wg wzorca)
  encoder_distance.cpp/.h # Enkoder obrotowy (dystans/prędkość)
  patterns.cpp/.h       # Definicje 16 wzorców malowania
  guns.cpp/.h           # Sterowanie 6 przekaźnikami pistoletów
  statistics.cpp/.h     # Liczniki sesji + lifetime
  storage.cpp/.h        # NVS (pamięć trwała)
  buzzer.cpp/.h         # Buzzer pasywny (LEDC PWM)
  button_handler.cpp/.h # 3 przyciski fizyczne (START, STOP, SELECT)
  joystick.cpp/.h       # Joystick analogowy KY-023
  pattern_buttons.cpp/.h # 15 przycisków wzorców (MCP23017 I2C)
  rtc_handler.cpp/.h    # Zegar RTC DS1307
  gps_handler.cpp/.h    # GPS NEO-6M (UART2)
  gps_track.cpp/.h      # Zapis trasy GPS (GPX na SD, bufor PSRAM)
  report_logger.cpp/.h  # Raporty sesji (CSV na karcie SD)
  event_log.cpp/.h      # Log zdarzeń systemowych (SD)
  nvs_backup.cpp/.h     # Backup/restore NVS na kartę SD
data/                   # Zasoby LittleFS (panel WWW)
docs/                   # Dokumentacja
display-module/         # Firmware modułu 7" (osobny projekt PlatformIO)
  platformio.ini        # espressif32@6.3.1, LovyanGFX + LVGL 8.3 + WebSockets + ArduinoJson
  src/main.cpp          # LovyanGFX + LVGL + dotyk, start zadań
  src/link.cpp/.h       # WiFi STA + WebSocket + HTTP + kolejka poleceń (zadanie FreeRTOS, Core 0)
  src/model.cpp/.h      # Status/StatsData, parsery JSON, tabela wzorców (kopia patterns.cpp!)
  src/ui_*.cpp          # Ekran roboczy, widgety własne (7-seg, droga), nakładki (menu, wzory, ustawienia)
  src/lgfx_sunton7.h    # Piny i timingi panelu RGB + GT911
```

Build modułu 7": `cd display-module && pio run` (upload: `pio run -t upload`).

## Architektura dual-core
- **Core 1** (Arduino loop): Enkoder, przyciski, joystick, pistolety, wyświetlacz, GPS, buzzer
- **Core 0** (FreeRTOS task): Serwer HTTP + WebSocket (web_server.cpp)
- **Synchronizacja**: `portMUX_TYPE g_stateMux` z makrami `STATE_LOCK()`/`STATE_UNLOCK()`

## Kluczowe biblioteki
- `TFT_eSPI` — wyświetlacz ILI9341 (SPI)
- `ArduinoJson` v7 — API JSON
- `WebSockets` — WebSocket server (push co 500ms)
- `RTClib` — zegar DS1307
- `TinyGPSPlus` — parser NMEA GPS

## Konwencje kodu
- Komentarze i nazwy UI po polsku (interfejs operatora)
- Nazwy zmiennych/funkcji po angielsku (camelCase)
- Klasy: PascalCase (np. `DisplayManager`, `PaintEngine`)
- Stałe/define: UPPER_SNAKE_CASE
- Piny GPIO: `PIN_` prefix
- Enumy: `STATE_`, `MODE_`, `SCREEN_`, `GUN_`, `PAT_` prefiksy
- Brak katalogu include/ — wszystkie headery w src/
- Headery: `#pragma once`

## Globalny stan
`SystemState g_state` (config.h) — centralna struktura stanu maszyny:
- `machineState` (IDLE/PAINTING/PAUSED/STOPPED)
- `machineMode` (AUTO/SEMI_AUTO/MANUAL)
- `currentScreen` (14 ekranów)
- `currentPattern` (16 wzorców)
- `displayNeedsUpdate` / `forceFullRedraw` — flagi odświeżania TFT

## Ważne uwagi
- Watchdog timer 5 s (`WDT_TIMEOUT_SEC`) — nie blokować loop()
- SPI współdzielone: TFT + karta SD (PIN_SD_CS musi być HIGH przed TFT)
- GPIO 26-37 zajęte przez Flash/PSRAM — nie używać!
- PIN_JOY_SW (GPIO 46) to strap pin — nie trzymać przy starcie
- Pamięć: ~300KB heap wolne, PSRAM na bufor GPS (135KB)
- Hasło WiFi AP = ostatnie 4 bajty MAC (8 znaków HEX), generowane w `web_server.cpp`
- Długie naciśnięcie przycisków: 1,5 s (`BTN_LONG_PRESS_MS`)
- Tryb RĘCZNY: pistolety strzelają tylko przy fizycznie trzymanym START (`buttons.isStartHeld()`) — moduł 7" nie może tego obejść (celowo)
- Pole `patDist` w `/api/status` (dystans od startu wzorca) służy modułowi 7" do synchronizacji animacji; nie usuwać
- Tabela `PATTERNS` w `display-module/src/model.cpp` musi być zgodna z `src/patterns.cpp`
- Tekst UI modułu 7" wyłącznie ASCII (czcionki wbudowane LVGL nie mają polskich znaków)
