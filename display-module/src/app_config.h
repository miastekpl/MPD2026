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

// Ten moduł (Sunton ESP32-8048S070C) jest teraz WYŁĄCZNIE wariantem WiFi ("gruby klient").
// Wariant docelowy dużego ekranu to sterownik + wyświetlacz inteligentny DWIN DGUS
// (DMG10600T070_09WTC) na łączu UART bez pośredniczącego ESP32 — patrz src/dgus_link.cpp
// w projekcie sterownika i docs/ARCHITEKTURA_TERMINAL.md. Ten katalog nie bierze w tym udziału.

// Brak ramek statusu dłużej niż to = utrata łączności
#define LINK_STALE_MS    2500

#define DISPLAY_FW_VERSION "0.1.1"

// Tempo odświeżania animacji drogi [ms]
#define ANIM_PERIOD_MS   33
