# Moduł wyświetlacza 7" (MPD2026)

Osobny moduł **Sunton ESP32-8048S070C** (ESP32-S3, 800×480, dotyk GT911), który łączy się przez WiFi
z punktem dostępowym sterownika Trassar i działa jako duży panel operatora. Sterownik pozostaje bez zmian
funkcjonalnych (ma nadal własny TFT, przyciski i panel WWW).

```bash
cd display-module
pio run            # kompilacja
pio run -t upload  # wgranie przez USB
pio device monitor
```

Pierwsze uruchomienie: **MENU → POŁĄCZENIE WiFi**, hasło AP sterownika (8 znaków HEX z jego ekranu startowego).

Dokumentacja:
- Obsługa: [docs/INSTRUKCJA_OBSLUGI.md](../docs/INSTRUKCJA_OBSLUGI.md), rozdz. 3
- Architektura i rozszerzanie: [docs/MODUL_WYSWIETLACZA.md](../docs/MODUL_WYSWIETLACZA.md)
- Piny i zasilanie: [docs/SCHEMAT_PODLACZEN.md](../docs/SCHEMAT_PODLACZEN.md), rozdz. 5–6
- API sterownika: [docs/API_WWW.md](../docs/API_WWW.md)

**Bezpieczeństwo:** STOP na module działa przez WiFi. Fizyczny STOP na sterowniku jest głównym zabezpieczeniem.
