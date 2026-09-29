#pragma once

// Sieć sterownika Trassar (jego punkt dostępowy WiFi)
#define CTRL_SSID        "TrassarV3"
#define CTRL_HOST        "192.168.4.1"
#define CTRL_HTTP_PORT   80
#define CTRL_WS_PORT     81

// Hasło AP sterownika = ostatnie 4 bajty MAC (8 znaków HEX). Domyślnie puste - wpisywane w menu.
#ifndef WIFI_DEFAULT_PASS
#define WIFI_DEFAULT_PASS ""
#endif

// Orientacja interfejsu: poziomo 800x480 (domyślnie) albo pionowo 480x800 (-DUI_PORTRAIT=1)
#ifndef UI_PORTRAIT
#define UI_PORTRAIT 0
#endif
#ifndef UI_ROTATION
#define UI_ROTATION 1
#endif
#if UI_PORTRAIT
constexpr int SCR_W = 480;
constexpr int SCR_H = 800;
#else
constexpr int SCR_W = 800;
constexpr int SCR_H = 480;
#endif

// Ten moduł (Sunton ESP32-8048S070C) jest jedynym ekranem architektury docelowej.
// Łączy się ze sterownikiem DWOMA równoległymi transportami: łączem przewodowym
// (RS-485, UART1, priorytet) i WiFi (zapasowe, jedyne dla telefonu/tabletu w panelu
// WWW) — patrz docs/LACZE_PRZEWODOWE.md i src/serial_link.h w projekcie sterownika.
// Alternatywa (nie zalecana): wyświetlacz DWIN DGUS zamiast tego modułu, na osobnym
// UART bez żadnego pośredniczącego ESP32 — patrz docs/ARCHITEKTURA_TERMINAL.md
// w projekcie sterownika. Ten katalog nie bierze w niej udziału.

// Brak ramek statusu dłużej niż to = utrata łączności
#define LINK_STALE_MS    2500

// Łącze przewodowe (RS-485) do sterownika - UART1 na wolnych pinach płytki Sunton
// (patrz docs/SCHEMAT_PODLACZEN.md sekcja 5.2 - GPIO 17/18 nie są używane przez panel RGB/dotyk/SD)
#define CABLE_TX_PIN     17
#define CABLE_RX_PIN     18
#define CABLE_BAUD       230400
#define CABLE_STALE_MS   1500    // brak ramek dluzej niz to = powrot na WiFi
#define CABLE_HEARTBEAT_MS 300   // wlasny heartbeat, zeby lacze bylo wykrywalne nawet bez polecen

#define DISPLAY_FW_VERSION "0.1.1"

// Tempo odświeżania animacji drogi [ms]
#define ANIM_PERIOD_MS   33
