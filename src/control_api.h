#pragma once
// ============================================================
// MPD2026 - wspolne wykonywanie polecen sterowania (POST /api/control i lacze terminala)
// Logika przeniesiona 1:1 z TrassarWebServer::handleControl(); nie zalezy od WebServer.
// ============================================================

#include <Arduino.h>

// Zrodlo argumentow polecenia (formularz HTTP albo JSON z lacza)
class ControlArgs {
public:
    virtual ~ControlArgs() {}
    virtual bool   has(const char* key) const = 0;
    virtual String get(const char* key) const = 0;
};

struct ControlResult {
    int    httpCode;   // 200 / 400 / 503
    String body;       // gotowy JSON odpowiedzi: {"result":"..."} | {"error":"..."} | dane (get_slot_config)
    bool   ok;         // true gdy result == "ok" (do komunikatow terminala)
    String message;    // tekst result/error (krotki)
};

ControlResult executeControl(const String& action, const ControlArgs& args);
