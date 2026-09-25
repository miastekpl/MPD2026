#pragma once
// ============================================================
// TrassarV3 - Fizyczne przyciski wzorców (MCP23017 I2C)
// 15 przycisków → 15 wzorców predefiniowanych (P-1a...P-7d)
// Expander MCP23017 na magistrali I2C (wspólnej z RTC DS1307)
// ============================================================

#include "config.h"

class PatternButtonHandler {
public:
    void begin();
    void update();    // Odczyt MCP23017, debounce, generowanie zdarzeń

    // Diagnostyka
    bool isReady() const { return ready; }
    uint16_t getRawState() const { return lastRaw; }

    // Układ przycisków (klasyczny 15 / soft-key 10 + GRUPA) i aktywna grupa (OŚ / KRAWĘDŹ)
    uint8_t getLayout() const { return layout; }
    void    setLayout(uint8_t l);          // zapis w NVS
    uint8_t getGroup() const { return group; }
    void    setGroup(uint8_t g) { group = (g == 1) ? 1 : 0; }

private:
    bool ready = false;
    volatile uint8_t layout = 0;           // PatternBtnLayout
    volatile uint8_t group = 0;            // PatternGroup
    int8_t lastPatSeen = -1;               // do automatycznego przełączania grupy za wzorcem

    // Stan przycisków (16 bitów: PortA[7:0] + PortB[7:0])
    uint16_t lastRaw = 0xFFFF;        // Poprzedni odczyt (pull-up = HIGH = 1)
    uint16_t debouncedState = 0xFFFF;  // Stan po debounce
    uint16_t lastDebouncedState = 0xFFFF;
    unsigned long lastReadMs = 0;

    // MCP23017 I2C helpers
    bool writeReg(uint8_t reg, uint8_t val);
    uint8_t readReg(uint8_t reg);
    uint16_t readAllPins();

    // Mapowanie: bit index → PatternID
    static PatternID bitToPattern(uint8_t bit);
};

extern PatternButtonHandler patternButtons;
