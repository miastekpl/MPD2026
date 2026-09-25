# Moduł wyświetlacza 7" (MPD2026)

Osobny moduł **Sunton ESP32-8048S070C** (ESP32-S3, 800x480, dotyk GT911), który łączy się przez WiFi
z punktem dostępowym sterownika Trassar i działa jako duży panel operatora. Sterownik pozostaje bez zmian
(ma nadal własny TFT, przyciski i panel WWW). Moduł używa istniejącego API: WebSocket `:81` (status)
i `POST /api/control` (polecenia).

## Budowanie i wgranie

```bash
cd display-module
pio run            # kompilacja
pio run -t upload  # wgranie przez USB
pio device monitor
```

## Pierwsze uruchomienie

1. Włącz sterownik (uruchamia AP `TrassarV3`).
2. Na module: **MENU → POLACZENIE WiFi**, wpisz hasło AP (8 znaków HEX = ostatnie 4 bajty MAC sterownika,
   widoczne na jego ekranie startowym) i zapisz. Hasło jest pamiętane w NVS.

## Ekran roboczy

- Lewa/prawa kolumna: 9 szybkich wzorców (przytrzymaj 0,9 s, aby przypisać inny) + **WSZYSTKIE** (16 wzorców).
- Środek: prędkość (7-seg), dystans/powierzchnia/czas, droga w perspektywie z animacją kreska/przerwa,
  stan pistoletów P1–P6, alarmy (za szybko/wolno, auto-pauza, anomalia).
- Dół: AUTO / SEMI / RECZNY, START·PAUZA·WZNOW·NASTEPNA LINIA, STOP, START OD PRZERWY.
- MENU: statystyki, wzór własny (3 sloty), kalibracja, farba/zbiornik, ustawienia, WiFi, informacje.

## Bezpieczeństwo

STOP na module działa przez WiFi. Przy utracie łączności moduł pokazuje pełnoekranowe ostrzeżenie:
**używaj fizycznego przycisku STOP na sterowniku.** Fizyczny STOP sterownika pozostaje głównym zabezpieczeniem.

## Uwagi

- Tekst bez polskich znaków diakrytycznych (czcionki wbudowane w LVGL); własna czcionka to kolejny krok.
- Tabela wzorców w `src/model.cpp` musi być zgodna z `src/patterns.cpp` sterownika.
- Animacja drogi używa pola `patDist` ze statusu (dodane w sterowniku MPD2026); ze starszym sterownikiem
  działa przybliżenie z dystansu sesji.
