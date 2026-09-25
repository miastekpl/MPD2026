#pragma once
// ============================================================
// Układ przycisków wzorców: klasyczny (15 przycisków) albo soft-key
// (10 przycisków + przełącznik grupy OŚ / KRAWĘDŹ).
// Czysta logika bez zależności sprzętowych.
//
// Indeksy wzorców zgodne z PatternID:
//   0..4  P-1a..P-1e   5..6 P-2a, P-2b   7..8 P-3a, P-3b   9 P-4      (grupa OŚ)
//   10    P-6          11..14 P-7a..P-7d                              (grupa KRAWĘDŹ)
//   15    WŁASNY (dostępny na stronie KRAWĘDŹ, nie zmienia grupy)
// ============================================================

#include <stdint.h>

enum PatternGroup : uint8_t {
    PGROUP_AXIS = 0,    // oś jezdni (pistolety P1-P4)
    PGROUP_EDGE = 1     // krawędź jezdni (pistolety P5-P6) + wzorzec własny
};

enum PatternBtnLayout : uint8_t {
    PBL_CLASSIC = 0,    // 15 przycisków: przycisk i = wzorzec i
    PBL_SOFTKEY = 1     // 10 przycisków (bity 0..9) + GRUPA (bit 10 = GPB2)
};

constexpr int     SOFTKEY_COUNT     = 10;
constexpr uint8_t SOFTKEY_GROUP_BIT = 10;

// Wzorzec przypisany do przycisku soft-key na danej stronie; -1 = przycisk nieaktywny.
inline int softKeyPattern(uint8_t group, int key) {
    if (key < 0 || key >= SOFTKEY_COUNT) return -1;
    if (group == PGROUP_AXIS) return key;                        // P-1a .. P-4
    static const int8_t edge[SOFTKEY_COUNT] = {10, 11, 12, 13, 14, 15, -1, -1, -1, -1};
    return edge[key];                                            // P-6, P-7a..d, WŁASNY
}

// Grupa, do której należy wzorzec (WŁASNY zachowuje bieżącą grupę).
inline uint8_t patternGroupOf(int patternIdx, uint8_t currentGroup) {
    if (patternIdx >= 0 && patternIdx <= 9)   return PGROUP_AXIS;
    if (patternIdx >= 10 && patternIdx <= 14) return PGROUP_EDGE;
    return currentGroup;
}
