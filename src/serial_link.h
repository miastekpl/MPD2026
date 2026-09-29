#pragma once
// ============================================================
// MPD2026 - lacze przewodowe (RS-485) z modulem wyswietlacza Sunton.
// Uzupelnienie/alternatywa WiFi - patrz docs/LACZE_PRZEWODOWE.md.
//
// Sterownik okresowo wysyla status (dokladnie ten sam JSON co GET /api/status)
// ramkami po UART1 (GPIO 9/10, przez konwertery RS-485). Modul Sunton odpowiada
// poleceniami w formie {"a":"<akcja>","v":<int>,"seq":N} - te same akcje/wartosci
// co POST /api/control, przetwarzane przez WSPOLNA funkcje executeControl()
// (control_api.cpp) - dokladnie ta sama sciezka co WWW i (w innym wariancie) ekran DGUS.
//
// Dziala w petli Core 1 (update() nie blokuje). WiFi zostaje NIEZALEZNIE jako
// lacze zapasowe dla modulu Sunton i jedyne dla telefonu/tabletu.
// ============================================================

#include "config.h"

#if HAS_SERIAL_LINK

class SerialLink {
public:
    void begin();
    void update();   // wolac w kazdej iteracji loop()

    // Lacze zywe: poprawna ramka od modulu < SERIAL_LINK_LOSS_MS temu
    bool isLinkUp() const { return linkUp; }

    uint32_t framesOk() const;
    uint32_t framesBad() const;

private:
    void sendStatus(uint32_t now);
    void handleFrame(const char* payload, size_t len);

    bool     started = false;
    bool     linkUp = false;
    uint32_t lastRxMs = 0;
    uint32_t lastStatusMs = 0;
};

extern SerialLink serialLink;

#else   // HAS_SERIAL_LINK == 0: pusty odpowiednik, zeby kod wspolny kompilowal sie bez #if

class SerialLink {
public:
    void begin() {}
    void update() {}
    bool isLinkUp() const { return false; }
    uint32_t framesOk() const { return 0; }
    uint32_t framesBad() const { return 0; }
};

extern SerialLink serialLink;

#endif
