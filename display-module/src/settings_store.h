#pragma once
// Ustawienia modułu wyświetlacza zapisywane w NVS (Preferences)

#include <Arduino.h>
#include <Preferences.h>
#include "app_config.h"

struct DisplaySettings {
    char    wifiPass[24] = WIFI_DEFAULT_PASS;
    uint8_t brightness = 220;

    void load() {
        Preferences p;
        if (!p.begin("mpddisp", true)) return;
        String s = p.getString("wpass", WIFI_DEFAULT_PASS);
        strlcpy(wifiPass, s.c_str(), sizeof(wifiPass));
        brightness = p.getUChar("bright", brightness);
        if (brightness < 20) brightness = 20;
        p.end();
    }

    void save() {
        Preferences p;
        if (!p.begin("mpddisp", false)) return;
        p.putString("wpass", wifiPass);
        p.putUChar("bright", brightness);
        p.end();
    }
};

extern DisplaySettings g_settings;
