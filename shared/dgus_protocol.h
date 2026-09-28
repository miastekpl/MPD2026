#pragma once
// ============================================================
// MPD2026 - protokol DWIN DGUS II (rodzina T5L/T5L2) po UART
// Wspolny naglowek (sterownik + testy na PC). Bez zaleznosci od Arduino.
//
// Zrodlo formatu ramki: oficjalna dokumentacja DWIN "T5L_DGUSII Application
// Development Guide" (rozdzial "Serial Communication Protocol"):
//   <5A> <A5> <BC> <CMD> [<DANE>...] [<CRC_H> <CRC_L>]
//   BC = liczba bajtow OD bajtu CMD do konca ramki (bez naglowka i bez BC).
//   CMD 0x82 = zapis VP (RAM zmiennych ekranu), 0x83 = odczyt VP.
//   Adresy VP i wartosci sa 16-bitowe, big-endian ("slowo" = 2 bajty).
//
// Zapis:  5A A5 <BC> 82 <VP_H><VP_L> <W0_H><W0_L> [<W1_H><W1_L> ...]
// Odczyt (zadanie):    5A A5 04 83 <VP_H><VP_L> <LICZBA_SLOW>
// Odczyt (odpowiedz):  5A A5 <BC> 83 <VP_H><VP_L> <LICZBA_SLOW> <W0_H><W0_L> ...
//
// WAZNE: ekran DGUS wysyla WLASNYM zaniechaniem (bez zadania) ramke w TYM SAMYM
// formacie co "odpowiedz 0x83" po dotknieciu klawisza/kontrolki skonfigurowanej
// w DGUS Designer na zapis do VP ("Return key code"/"Momentary button" itp.) -
// sterownik odbiera wiec zdarzenia dotyku jako zwykle ramki 0x83 pod adresem VP,
// ktory sam sobie zaprojektujesz jako "rejestr zdarzen dotyku" (patrz src/dgus_map.h).
//
// CRC jest w DGUS opcjonalne (rejestr FCRC w konfiguracji ekranu, domyslnie
// wylaczone i w tym stanie zostawiamy projekt) - domyslnie NIE uzywane tutaj.
// ============================================================

#include <stdint.h>
#include <stddef.h>

namespace dgus {

constexpr uint32_t BAUD          = 115200;   // domyslny baudrate DGUS (do zmiany w CONFIG.TXT/DGUS Designer)
constexpr uint8_t  HDR1          = 0x5A;
constexpr uint8_t  HDR2          = 0xA5;
constexpr uint8_t  CMD_WRITE     = 0x82;
constexpr uint8_t  CMD_READ      = 0x83;
constexpr size_t   MAX_WORDS     = 120;                    // limit slow danych w jednej ramce (BC<=255)
constexpr size_t   MAX_FRAME     = 3 + 3 + MAX_WORDS * 2;   // naglowek+BC + cmd+vp + dane

// Adres systemowy przelaczania strony (patrz DWIN "T5L_DGUSII Application
// Development Guide", rozdz. "System variable interface address"):
// zapis 2 slow pod VP 0x0084: {0x5A01, <numer_strony>} przelacza ekran.
constexpr uint16_t VP_PAGE_SWITCH = 0x0084;
constexpr uint16_t PAGE_SWITCH_MAGIC = 0x5A01;

// ---- Kodowanie ----

// Zapisuje N slow (big-endian) pod adresem vp. Zwraca dlugosc ramki w out (0 = blad/za duzo danych).
inline size_t encodeWrite(uint16_t vp, const uint16_t* words, size_t count, uint8_t* out, size_t cap) {
    if (!words || !out || count == 0 || count > MAX_WORDS) return 0;
    size_t bc = 1 + 2 + count * 2;         // cmd + vp + dane
    size_t total = 3 + bc;                 // naglowek(2)+BC(1) + bc
    if (bc > 0xFF || total > cap) return 0;
    size_t p = 0;
    out[p++] = HDR1;
    out[p++] = HDR2;
    out[p++] = (uint8_t)bc;
    out[p++] = CMD_WRITE;
    out[p++] = (uint8_t)(vp >> 8);
    out[p++] = (uint8_t)(vp & 0xFF);
    for (size_t i = 0; i < count; i++) {
        out[p++] = (uint8_t)(words[i] >> 8);
        out[p++] = (uint8_t)(words[i] & 0xFF);
    }
    return p;
}

inline size_t encodeWriteWord(uint16_t vp, uint16_t value, uint8_t* out, size_t cap) {
    return encodeWrite(vp, &value, 1, out, cap);
}

// strnlen bez zaleznosci od <cstring> (unikamy konfliktu z ewentualnym strnlen systemowym)
inline size_t dgusStrnlen(const char* s, size_t maxLen) {
    size_t n = 0;
    while (n < maxLen && s[n] != '\0') n++;
    return n;
}

// Pakuje tekst ASCII po 2 znaki/slowo (big-endian), dopelnia zerami do fixedWords slow
// (fixedWords*2 bajtow) - NADPISUJE cale pole, wiec krotszy nowy tekst nie zostawia
// resztek starego. Zwraca dlugosc ramki (0 = tekst/pole za duze).
inline size_t encodeWriteText(uint16_t vp, const char* text, size_t fixedWords, uint8_t* out, size_t cap) {
    if (fixedWords == 0 || fixedWords > MAX_WORDS) return 0;
    uint16_t buf[MAX_WORDS];
    size_t len = text ? dgusStrnlen(text, fixedWords * 2) : 0;
    for (size_t w = 0; w < fixedWords; w++) {
        uint8_t c0 = (2 * w < len) ? (uint8_t)text[2 * w] : 0x00;
        uint8_t c1 = (2 * w + 1 < len) ? (uint8_t)text[2 * w + 1] : 0x00;
        buf[w] = (uint16_t)((c0 << 8) | c1);
    }
    return encodeWrite(vp, buf, fixedWords, out, cap);
}

inline size_t encodeReadRequest(uint16_t vp, uint8_t wordCount, uint8_t* out, size_t cap) {
    if (wordCount == 0 || cap < 7) return 0;
    out[0] = HDR1;
    out[1] = HDR2;
    out[2] = 0x04;             // BC: cmd+vp(2)+len(1)
    out[3] = CMD_READ;
    out[4] = (uint8_t)(vp >> 8);
    out[5] = (uint8_t)(vp & 0xFF);
    out[6] = wordCount;
    return 7;
}

inline size_t encodeSwitchPage(uint16_t pageId, uint8_t* out, size_t cap) {
    uint16_t words[2] = {PAGE_SWITCH_MAGIC, pageId};
    return encodeWrite(VP_PAGE_SWITCH, words, 2, out, cap);
}

// ---- Parser strumieniowy ----
// feed() zwraca true, gdy zlozono poprawna ramke 0x82 (zapis, rzadko odbierany od
// ekranu) lub 0x83 (odpowiedz NA ODCZYT albo, tak samo sformatowane, PUSH zdarzenia
// dotyku). Po true: cmd()/vp()/wordCount()/word(i) sa wazne do kolejnego feed().
class Parser {
public:
    bool feed(uint8_t b) {
        switch (state_) {
            case WAIT_H1:
                if (b == HDR1) state_ = WAIT_H2;
                return false;
            case WAIT_H2:
                if (b == HDR2) { state_ = WAIT_BC; }
                else if (b != HDR1) { state_ = WAIT_H1; }   // inaczej zostajemy w WAIT_H2 (b==HDR1)
                return false;
            case WAIT_BC:
                bc_ = b;
                if (bc_ < 3 || bc_ > (uint8_t)(3 + MAX_WORDS * 2)) {
                    // nieprawdopodobna dlugosc - odrzuc i szukaj naglowka od nowa
                    overflow_++;
                    state_ = WAIT_H1;
                    return false;
                }
                got_ = 0;
                state_ = READ_BODY;
                return false;
            case READ_BODY:
                body_[got_++] = b;
                if (got_ < bc_) return false;
                state_ = WAIT_H1;
                return parseBody();
        }
        return false;
    }

    void reset() { state_ = WAIT_H1; }

    uint8_t  cmd() const { return cmd_; }
    uint16_t vp() const { return vp_; }
    size_t   wordCount() const { return nWords_; }
    uint16_t word(size_t i) const { return (i < nWords_) ? words_[i] : 0; }

    uint32_t okFrames() const  { return ok_; }
    uint32_t badFrames() const { return bad_; }
    uint32_t overflows() const { return overflow_; }

private:
    bool parseBody() {
        uint8_t c = body_[0];
        if (c == CMD_WRITE) {
            if (bc_ < 3 || ((bc_ - 3) % 2) != 0) { bad_++; return false; }
            cmd_ = CMD_WRITE;
            vp_ = (uint16_t)((body_[1] << 8) | body_[2]);
            nWords_ = (bc_ - 3) / 2;
            if (nWords_ > MAX_WORDS) { bad_++; return false; }
            for (size_t i = 0; i < nWords_; i++) {
                words_[i] = (uint16_t)((body_[3 + i * 2] << 8) | body_[3 + i * 2 + 1]);
            }
            ok_++;
            return true;
        }
        if (c == CMD_READ) {
            // odpowiedz odczytu / auto-push dotyku: cmd, vp(2), len(1), dane(len*2)
            if (bc_ < 4) { bad_++; return false; }
            uint8_t declLen = body_[3];
            if ((size_t)(4 + declLen * 2) != bc_ || declLen == 0 || declLen > MAX_WORDS) { bad_++; return false; }
            cmd_ = CMD_READ;
            vp_ = (uint16_t)((body_[1] << 8) | body_[2]);
            nWords_ = declLen;
            for (size_t i = 0; i < nWords_; i++) {
                words_[i] = (uint16_t)((body_[4 + i * 2] << 8) | body_[4 + i * 2 + 1]);
            }
            ok_++;
            return true;
        }
        bad_++;
        return false;
    }

    enum State : uint8_t { WAIT_H1, WAIT_H2, WAIT_BC, READ_BODY };
    State    state_ = WAIT_H1;
    uint8_t  bc_ = 0;
    uint8_t  body_[3 + MAX_WORDS * 2] = {};
    size_t   got_ = 0;

    uint8_t  cmd_ = 0;
    uint16_t vp_ = 0;
    uint16_t words_[MAX_WORDS] = {};
    size_t   nWords_ = 0;

    uint32_t ok_ = 0, bad_ = 0, overflow_ = 0;
};

}  // namespace dgus
