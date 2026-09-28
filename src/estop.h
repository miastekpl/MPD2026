#pragma once
// ============================================================
// MPD2026 - Status STOP-u awaryjnego (E-STOP)
//
// Rzeczywiste ciecie zasilania pistoletow/pomp jest SPRZETOWE (grzybek E-STOP,
// styk NC, wpiety w tor zasilania modulu przekaznikow) - nie zalezy od tego
// modulu ani od firmware w ogole. Ten modul tylko OBSERWUJE status petli przez
// GPIO (PIN_ESTOP_STATUS, patrz config.h) i - jako obrona w glab, nie jako
// jedyne zabezpieczenie - dodatkowo wymusza bezpieczny stan maszyny w
// oprogramowaniu (STATE_STOPPED, alarm dzwiekowy, log zdarzen).
//
// Natychmiastowe zerowanie przekaznikow przy zboczu na tym pinie robi juz
// GunController::emergencyStopISR (patrz guns.cpp beginEmergencyStop) - ten
// modul dochodzi pozniej (w loop(), odszumione) i domyka stan maszyny/UI/log.
// ============================================================

#include "config.h"

class EstopHandler {
public:
    void begin();
    void update();

    // Petla E-STOP aktualnie otwarta (odszumione, potwierdzone przez ESTOP_DEBOUNCE_MS)
    bool isTriggered() const { return triggered; }

    // Bylo zadzialanie i operator jeszcze go nie potwierdzil (nawet jesli petla
    // juz sie zamknela) - blokuje wznowienie malowania, patrz control_api.cpp "start"
    bool isAwaitingAck() const { return awaitingAck; }

    // Potwierdzenie operatora po ustapieniu STOP-u (z ekranu DGUS/panelu WWW).
    // Bez efektu, dopoki petla pozostaje otwarta.
    void acknowledge();

private:
    bool rawReading() const;

    bool     triggered = false;
    bool     awaitingAck = false;
    bool     lastRaw = false;
    uint32_t lastChangeMs = 0;
    uint32_t lastBuzzMs = 0;
};

extern EstopHandler estop;
