#pragma once
// ============================================================
// Model danych modułu wyświetlacza: stan sterownika Trassar,
// tabela wzorców (kopia patterns.cpp) i parsery JSON API.
// UWAGA: tabela PATTERNS musi być zgodna z src/patterns.cpp sterownika.
// ============================================================

#include <Arduino.h>

constexpr int NGUNS       = 6;
constexpr int NPAT_PREDEF = 15;
constexpr int NPAT        = 16;   // 15 normowych + WŁASNY (indeks 15)
constexpr int PAT_CUSTOM_IDX = 15;
constexpr int NSLOTS      = 3;

enum MState : uint8_t { MS_IDLE, MS_PAINTING, MS_PAUSED, MS_STOPPED, MS_UNKNOWN };
enum MMode  : uint8_t { MM_AUTO, MM_SEMI, MM_MANUAL, MM_DEMO };
enum GMode  : uint8_t { GM_OFF = 0, GM_CONT = 1, GM_DASH = 2 };

struct GunCfg {
    uint8_t mode;   // GMode
    float   line;   // [m]
    float   gap;    // [m]
};

struct PatternInfo {
    const char* code;
    const char* name;
    bool        hasReverse;
    GunCfg      guns[NGUNS];
};

extern const PatternInfo PATTERNS[NPAT_PREDEF];

// Geometria pistoletów do wizualizacji (ułamek szerokości jezdni)
extern const float GUN_LATERAL[NGUNS];   // środek pasa
extern const float GUN_WIDTH_FR[NGUNS];  // szerokość pasa
extern const uint8_t GUN_WIDTH_CM[NGUNS];

struct Status {
    bool     valid = false;

    MState   state = MS_UNKNOWN;
    MMode    mode  = MM_AUTO;
    int      patternIdx = 0;
    char     patternCode[12] = "";
    char     patternName[32] = "";
    bool     reversed = false;
    bool     gapStart = false;
    bool     customValid = false;
    int      activeSlot = 0;
    bool     slotsValid[NSLOTS] = {false, false, false};
    GunCfg   custom[NGUNS] = {};        // konfiguracja wzorca własnego

    float    speed = 0;                 // km/h
    float    distance = 0;              // m (sesja)
    float    area = 0;                  // m2 (sesja)
    uint32_t elapsed = 0;               // s
    float    patDist = 0;               // m od startu wzorca
    bool     hasPatDist = false;

    bool     guns[NGUNS] = {};
    bool     gunAnomaly[NGUNS] = {};
    bool     anomalyDetected = false;

    bool     overspeed = false;
    bool     lowSpeed = false;
    bool     autoPaused = false;
    bool     autoResume = true;
    bool     semiLineComplete = false;
    int      semiSegment = 0;

    bool     hasPatGroup = false;       // sterownik zwraca patGroup (starsze wersje nie)
    int      patGroup = 0;              // 0 = OS jezdni, 1 = KRAWEDZ
    int      patBtnLayout = 0;          // 0 = klasyczne 15 przyciskow, 1 = soft-key 10 + GRUPA

    bool     smartSwitch = true;
    bool     patternPending = false;
    char     pendingCode[12] = "";

    bool     calibrated = false;
    bool     calibrating = false;
    float    calPulses = 0;
    float    ppm = 0;
    float    maxSpeed = 0;
    float    minSpeed = 0;

    bool     gpsFix = false;
    int      gpsSat = 0;
    float    gpsHdop = 0;
    float    gpsSpeed = 0;
    bool     gpxRec = false;

    float    paintLevelL = 0;
    int      paintLevelPct = 0;

    char     firmware[16] = "";
    uint32_t uptime = 0;
    int      clients = 0;
};

struct StatsData {
    bool     valid = false;
    float    lifeDistance = 0;
    float    lifeArea = 0;
    uint32_t lifeTimeSec = 0;
    float    sessDistance = 0;
    float    sessArea = 0;
    uint32_t sessTimeSec = 0;
    float    gunDist[NGUNS] = {};
    uint32_t gunShots[NGUNS] = {};
    bool     sdReady = false;
    int      reportCount = 0;
    float    paintUsedL = 0;
    float    paintRemainingL = 0;
    float    paintTankL = 0;
    float    paintLevelL = 0;
    int      refuelCount = 0;
    float    totalRefueledL = 0;
};

struct SlotCfg {
    bool   valid = false;
    GunCfg guns[NGUNS] = {};
};

bool parseStatus(const char* json, size_t len, Status& out);
bool parseStats(const char* json, size_t len, StatsData& out);
bool parseSlotConfig(const char* json, size_t len, SlotCfg& out);

// Konfiguracja pistoletu w aktualnym wzorcu (z uwzględnieniem odwrócenia P1<->P3)
GunCfg effectiveGun(const Status& s, int gun);
// Cykl główny wzorca (pierwszy pistolet przerywany), 0 = wzorzec ciągły
float patternCycle(const Status& s);

// Konfiguracja pistoletu dla dowolnego wzorca (do podglądów w wyborze wzorca)
GunCfg patternGun(int patIdx, bool reversed, int gun, const Status& s);

// Strony wzorców (zgodne z src/pattern_layout.h sterownika):
//   grupa 0 = OŚ jezdni (P1-P4): przyciski 0..9 = P-1a..P-4
//   grupa 1 = KRAWĘDŹ (P5-P6): P-6, P-7a..P-7d, WŁASNY (przyciski 0..5)
constexpr int SOFTKEY_COUNT = 10;
int softKeyPattern(int group, int key);                 // -1 = przycisk nieaktywny
int patternGroupOf(int patIdx, int currentGroup);       // WŁASNY zachowuje bieżącą grupę

const char* stateName(MState s);
const char* modeName(MMode m);
