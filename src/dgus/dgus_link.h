#pragma once
// ============================================================
// MPD2026 - lacze sterownika z ekranem DWIN DGUS (UART, bez posredniczacego ESP32)
// Sterownik = MASTER (logika, stan, menu). Ekran = inteligentny wyswietlacz DGUS.
// Protokol: shared/dgus_protocol.h, mapa VP/stron: src/dgus_map.h,
// opis calosci: docs/ARCHITEKTURA_TERMINAL.md.
//
// Wszystko dziala w petli Core 1 (update() nie blokuje). Ekran NIGDY nie steruje
// pistoletami bezposrednio - tylko zglasza kody zdarzen, ktore przechodza te same
// sciezki co przyciski fizyczne (menu.handleEvent) i panel WWW (executeControl).
// ============================================================

#include "config.h"
#include "button_handler.h"

#if HAS_DGUS_LINK

class DgusLink {
public:
    void begin();
    void update();                       // wolac w kazdej iteracji loop() i w petlach oczekiwania w setup()

    // Lacze zywe: poprawna ramka od ekranu < DGUS_LINK_LOSS_MS temu
    bool isLinkUp() const { return linkUp; }
    // Alarm utraty ekranu (po tym jak lacze bylo nawiazane); zerowany po powrocie lacza
    bool isLossAlarm() const { return lossAlarm; }

    // Polityka utraty ekranu podczas malowania: 0 = kontynuuj + alarm, 1 = auto-pauza
    uint8_t getLossPolicy() const { return lossPolicy; }
    void setLossPolicy(uint8_t p);

    // "Martwy czlowiek" czyszczenia dysz: true = przycisk na ekranie trzymany
    bool isHoldActive() const;

    uint32_t framesOk() const;
    uint32_t framesBad() const;

private:
    void handleTouchEvent(uint16_t code, uint32_t now);
    void sendHomeStatus();
    void sendSoftkeys();
    void sendServicePage(int screen);
    void sendEstopStatus(uint32_t now);  // wysylany na KAZDEJ stronie (nie tylko HOME) - alarm musi byc zawsze widoczny
    void sendPing(uint32_t now);
    void onLinkChange(bool up, uint32_t now);
    void applyLossPolicy(uint32_t now);
    void dispatchAction(const char* action, int value);

    bool     started = false;
    bool     linkUp = false;
    bool     everUp = false;
    bool     lossAlarm = false;
    bool     lossPauseDone = false;
    uint8_t  lossPolicy = 0;

    uint32_t lastRxMs = 0;
    uint32_t lastPingMs = 0;
    uint32_t lastHomeMs = 0;
    uint32_t lastServiceMs = 0;
    uint32_t lastEstopMs = 0;
    uint32_t lastAlarmBeepMs = 0;
    int      lastPageSent = -1;

    bool     nozzleHoldOn = false;
    uint32_t nozzleHoldSetMs = 0;

    static const int EVQ = 8;
    uint16_t evq[EVQ] = {};
    uint8_t  evHead = 0, evTail = 0;
};

extern DgusLink dgusLink;

#else   // HAS_DGUS_LINK == 0: pusty odpowiednik, zeby kod wspolny kompilowal sie bez #if

class DgusLink {
public:
    void begin() {}
    void update() {}
    bool isLinkUp() const { return false; }
    bool isLossAlarm() const { return false; }
    uint8_t getLossPolicy() const { return 0; }
    void setLossPolicy(uint8_t) {}
    bool isHoldActive() const { return false; }
};

extern DgusLink dgusLink;

#endif
