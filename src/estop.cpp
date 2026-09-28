#include "sys_log.h"
// ============================================================
// MPD2026 - Status STOP-u awaryjnego (E-STOP) - patrz estop.h
// ============================================================

#include "estop.h"
#include "painting_engine.h"
#include "buzzer.h"
#include "event_log.h"

EstopHandler estop;

bool EstopHandler::rawReading() const {
    // HIGH = petla otwarta = zadzialanie GRZYBKA lub przerwany przewod (fail-safe) - patrz config.h
    return digitalRead(PIN_ESTOP_STATUS) == HIGH;
}

void EstopHandler::begin() {
    pinMode(PIN_ESTOP_STATUS, INPUT_PULLUP);
    delay(50);  // ustabilizowanie odczytu tuz po wlaczeniu, zanim zaczniemy debounce

    lastRaw = rawReading();
    triggered = lastRaw;
    awaitingAck = lastRaw;
    lastChangeMs = millis();

    if (triggered) {
        eventLog.log("ESTOP", "STOP awaryjny AKTYWNY juz przy starcie sterownika (petla otwarta)");
    }
    DBG_PRINTF("[ESTOP] Monitorowanie GPIO %d, stan poczatkowy: %s\n",
               PIN_ESTOP_STATUS, triggered ? "AKTYWNY" : "OK");
}

void EstopHandler::update() {
    uint32_t now = millis();
    bool raw = rawReading();

    if (raw != lastRaw) {
        // Zmiana surowego odczytu - zacznij odliczanie debounce od nowa
        lastRaw = raw;
        lastChangeMs = now;
    } else if (raw != triggered && (now - lastChangeMs) >= ESTOP_DEBOUNCE_MS) {
        // Odczyt stabilny przez ESTOP_DEBOUNCE_MS -> potwierdzona zmiana stanu petli
        triggered = raw;
        if (triggered) {
            awaitingAck = true;
            // ISR (guns.cpp) juz wyzerowal przekazniki mikrosekundy wczesniej;
            // requestStop() domyka stan maszyny (STATE_STOPPED) i finalizacje na Core 1.
            paintEngine.requestStop();
            eventLog.log("ESTOP", "STOP awaryjny AKTYWNY (petla otwarta)");
            buzzer.play(BUZ_ESTOP);
            lastBuzzMs = now;
        } else {
            eventLog.log("ESTOP", "Petla STOP-u awaryjnego zamknieta - oczekiwanie na potwierdzenie operatora");
        }
    }

    if (triggered && (now - lastBuzzMs >= ESTOP_BUZZ_REPEAT_MS)) {
        lastBuzzMs = now;
        buzzer.play(BUZ_ESTOP);
    }
}

void EstopHandler::acknowledge() {
    if (triggered) return;  // Petla wciaz otwarta - nie ma czego potwierdzac
    if (awaitingAck) {
        awaitingAck = false;
        eventLog.log("ESTOP", "Operator potwierdzil ustapienie STOP-u awaryjnego");
    }
}
