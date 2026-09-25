#pragma once
// Ustawienia modułu wyświetlacza zapisywane w NVS (Preferences)

#include <Arduino.h>
#include <Preferences.h>
#include "app_config.h"

constexpr int NUM_FAV = 9;   // 9 ulubionych wzorców + przycisk "WSZYSTKIE"

struct DisplaySettings {
    char    wifiPass[24] = WIFI_DEFAULT_PASS;
    uint8_t brightness = 220;
    uint8_t fav[NUM_FAV] = {0, 1, 5, 7, 8, 9, 11, 12, 15};   // domyślne indeksy wzorców

    void load() {
        Preferences p;
        if (!p.begin("mpddisp", true)) return;
        String s = p.getString("wpass", WIFI_DEFAULT_PASS);
        strlcpy(wifiPass, s.c_str(), sizeof(wifiPass));
        brightness = p.getUChar("bright", brightness);
        if (brightness < 20) brightness = 20;
        uint8_t tmp[NUM_FAV];
        if (p.getBytes("fav", tmp, NUM_FAV) == NUM_FAV) {
            for (int i = 0; i < NUM_FAV; i++) fav[i] = (tmp[i] < 16) ? tmp[i] : 0;
        }
        p.end();
    }

    void save() {
        Preferences p;
        if (!p.begin("mpddisp", false)) return;
        p.putString("wpass", wifiPass);
        p.putUChar("bright", brightness);
        p.putBytes("fav", fav, NUM_FAV);
        p.end();
    }
};

extern DisplaySettings g_settings;
