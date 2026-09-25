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

// Brak ramek statusu dłużej niż to = utrata łączności
#define LINK_STALE_MS    2500

#define DISPLAY_FW_VERSION "0.1.0"

// Tempo odświeżania animacji drogi [ms]
#define ANIM_PERIOD_MS   33
