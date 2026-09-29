#pragma once
// ============================================================
// MPD2026 - protokol ramek tekstowych: sterownik <-> modul wyswietlacza
// Sunton po lączu przewodowym (RS-485, patrz docs/LACZE_PRZEWODOWE.md).
//
// Format ramki: $<payload JSON>*<CRC16/CCITT, 4 znaki hex>\n
// Payload to zwykly tekst JSON - ten sam, ktory juz krazy przez WiFi:
//   sterownik -> modul: dokladnie to co GET /api/status (TrassarWebServer::statusJson())
//   modul -> sterownik: {"a":"<akcja>","v":<int|pominiete>,"seq":<numer>}
//                        (te same akcje/wartosci co POST /api/control)
//   sterownik -> modul (odpowiedz na polecenie): {"r":"ok"|"<komunikat>","seq":<numer>}
//
// Bez zaleznosci od Arduino - wspolny dla src/ (sterownik) i display-module/src/
// (modul Sunton), naglowkowy (inline), zeby nie trzeba bylo linkowac osobnego .cpp
// w dwoch niezaleznych projektach PlatformIO.
// ============================================================

#include <stdint.h>
#include <stddef.h>
#include <string.h>

namespace seriallink {

constexpr char   FRAME_START   = '$';
constexpr char   FRAME_CRC_SEP = '*';
constexpr char   FRAME_END     = '\n';
constexpr size_t MAX_PAYLOAD   = 1536;   // z zapasem na pelny status JSON (patrz web_server.cpp getStateJson)

inline uint16_t crc16ccitt(const uint8_t* data, size_t len) {
    uint16_t crc = 0xFFFF;
    for (size_t i = 0; i < len; i++) {
        crc ^= (uint16_t)data[i] << 8;
        for (int b = 0; b < 8; b++) {
            crc = (crc & 0x8000) ? (uint16_t)((crc << 1) ^ 0x1021) : (uint16_t)(crc << 1);
        }
    }
    return crc;
}

// Buduje kompletna ramke gotowa do wyslania (bajt po bajcie identyczny co pojedzie na UART).
// Zwraca dlugosc zapisana do out, albo 0 gdy sie nie zmiescilo (payloadLen za duzy / outCap za mala).
inline size_t encodeFrame(const char* payload, size_t payloadLen, char* out, size_t outCap) {
    // $ + payload + * + 4 hex + \n
    if (payloadLen > MAX_PAYLOAD) return 0;
    size_t need = 1 + payloadLen + 1 + 4 + 1;
    if (need > outCap) return 0;
    size_t p = 0;
    out[p++] = FRAME_START;
    memcpy(out + p, payload, payloadLen);
    p += payloadLen;
    out[p++] = FRAME_CRC_SEP;
    uint16_t crc = crc16ccitt((const uint8_t*)payload, payloadLen);
    static const char HEXDIG[] = "0123456789ABCDEF";
    out[p++] = HEXDIG[(crc >> 12) & 0xF];
    out[p++] = HEXDIG[(crc >> 8) & 0xF];
    out[p++] = HEXDIG[(crc >> 4) & 0xF];
    out[p++] = HEXDIG[crc & 0xF];
    out[p++] = FRAME_END;
    return p;
}

inline int hexVal(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    return -1;
}

// Parser strumieniowy: podawaj bajt po bajcie (feed). Gdy zwroci true, payload()/payloadLen()
// wskazuje na wlasnie zlozona, POPRAWNA (CRC OK) ramke - waznosc tylko do kolejnego feed().
// Bledne ramki (zly CRC, przepelnienie, urwany naglowek) sa ciszej odrzucane i licznik
// badFrames_ rosnie; parser sam resynchronizuje sie na kolejnym '$'.
class Parser {
public:
    void reset() { state_ = WAIT_START; len_ = 0; crcIdx_ = 0; }

    bool feed(uint8_t b) {
        char c = (char)b;
        switch (state_) {
            case WAIT_START:
                if (c == FRAME_START) { len_ = 0; state_ = IN_PAYLOAD; }
                return false;

            case IN_PAYLOAD:
                if (c == FRAME_START) { len_ = 0; return false; }               // resync na nowy naglowek
                if (c == FRAME_CRC_SEP) { crcIdx_ = 0; state_ = IN_CRC; return false; }
                if (len_ >= MAX_PAYLOAD) { badFrames_++; state_ = WAIT_START; return false; }  // przepelnienie
                buf_[len_++] = c;
                return false;

            case IN_CRC: {
                int v = hexVal(c);
                if (v < 0) { badFrames_++; state_ = (c == FRAME_START) ? IN_PAYLOAD : WAIT_START;
                             if (c == FRAME_START) len_ = 0;
                             return false; }
                crcBuf_[crcIdx_++] = c;
                if (crcIdx_ >= 4) state_ = WAIT_END;
                return false;
            }

            case WAIT_END:
                state_ = WAIT_START;
                if (c != FRAME_END) { badFrames_++; return false; }
                buf_[len_] = '\0';
                {
                    uint16_t got = (uint16_t)((hexVal(crcBuf_[0]) << 12) | (hexVal(crcBuf_[1]) << 8) |
                                               (hexVal(crcBuf_[2]) << 4) | hexVal(crcBuf_[3]));
                    uint16_t want = crc16ccitt((const uint8_t*)buf_, len_);
                    if (got != want) { badFrames_++; return false; }
                }
                okFrames_++;
                return true;
        }
        return false;
    }

    const char* payload() const { return buf_; }
    size_t      payloadLen() const { return len_; }
    uint32_t    okFrames() const { return okFrames_; }
    uint32_t    badFrames() const { return badFrames_; }

private:
    enum State { WAIT_START, IN_PAYLOAD, IN_CRC, WAIT_END };
    State  state_ = WAIT_START;
    char   buf_[MAX_PAYLOAD + 1] = {};
    size_t len_ = 0;
    char   crcBuf_[4] = {};
    int    crcIdx_ = 0;
    uint32_t okFrames_ = 0;
    uint32_t badFrames_ = 0;
};

}  // namespace seriallink
