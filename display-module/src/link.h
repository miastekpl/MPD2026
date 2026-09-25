#pragma once
// ============================================================
// Łącze ze sterownikiem Trassar: WiFi (STA) + WebSocket (status)
// + HTTP POST /api/control (polecenia). Cała komunikacja sieciowa
// działa w osobnym zadaniu FreeRTOS (Core 0), UI nigdy nie blokuje.
// ============================================================

#include "model.h"

enum LinkState : uint8_t {
    LS_NO_PASSWORD,      // brak hasła WiFi
    LS_WIFI_CONNECTING,  // łączenie z AP sterownika
    LS_NO_DATA,          // WiFi OK, brak ramek statusu
    LS_ONLINE            // dane na bieżąco
};

void      linkBegin();
void      linkSetPassword(const char* pass);   // zapis w ustawieniach i ponowne łączenie
LinkState linkState();
int       linkRssi();

// Ostatni status; zwraca false gdy nigdy nie otrzymano ramki. ageMs = wiek ramki.
bool      linkGetStatus(Status& out, uint32_t* ageMs);

// Statystyki (odświeżane co ~1.5 s, tylko gdy linkWantStats(true))
void      linkWantStats(bool on);
bool      linkGetStats(StatsData& out);

// Konfiguracja slotu wzorca własnego (asynchronicznie)
void      linkRequestSlot(int slot);
bool      linkGetSlot(int slot, SlotCfg& out);   // true gdy dane dla slotu gotowe

// Polecenie POST /api/control, body np. "action=set_pattern&value=3".
// urgent=true wstawia na początek kolejki (STOP).
bool      linkSend(const char* formBody, bool urgent = false);

// Wynik ostatniego polecenia (do wyświetlenia komunikatu)
uint32_t  linkResultSeq();
bool      linkResultOk();
void      linkResultMsg(char* buf, size_t n);
