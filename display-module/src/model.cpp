#include "model.h"
#include <ArduinoJson.h>

#define G_OFF        {GM_OFF, 0, 0}
#define G_CONT       {GM_CONT, 0, 0}
#define G_DASH(l, g) {GM_DASH, l, g}

// Zgodne z src/patterns.cpp sterownika
const PatternInfo PATTERNS[NPAT_PREDEF] = {
    { "P-1a", "Przerywana dluga",     false, { G_OFF, G_DASH(4.0f, 8.0f), G_OFF, G_OFF, G_OFF, G_OFF } },
    { "P-1b", "Przerywana krotka",    false, { G_OFF, G_DASH(2.0f, 4.0f), G_OFF, G_OFF, G_OFF, G_OFF } },
    { "P-1c", "Wydzielajaca",         false, { G_OFF, G_DASH(2.0f, 2.0f), G_OFF, G_OFF, G_OFF, G_OFF } },
    { "P-1d", "Prowadzaca waska",     false, { G_OFF, G_DASH(1.0f, 1.0f), G_OFF, G_OFF, G_OFF, G_OFF } },
    { "P-1e", "Prowadzaca szeroka",   false, { G_OFF, G_OFF, G_OFF, G_DASH(1.0f, 1.0f), G_OFF, G_OFF } },
    { "P-2a", "Ciagla waska",         false, { G_OFF, G_CONT, G_OFF, G_OFF, G_OFF, G_OFF } },
    { "P-2b", "Ciagla szeroka",       false, { G_OFF, G_OFF, G_OFF, G_CONT, G_OFF, G_OFF } },
    { "P-3a", "Przekraczalna dluga",  true,  { G_CONT, G_OFF, G_DASH(4.0f, 2.0f), G_OFF, G_OFF, G_OFF } },
    { "P-3b", "Przekraczalna krotka", true,  { G_CONT, G_OFF, G_DASH(1.0f, 1.0f), G_OFF, G_OFF, G_OFF } },
    { "P-4",  "Podwojna ciagla",      false, { G_CONT, G_OFF, G_CONT, G_OFF, G_OFF, G_OFF } },
    { "P-6",  "Ostrzegawcza",         false, { G_OFF, G_OFF, G_OFF, G_OFF, G_DASH(4.0f, 2.0f), G_OFF } },
    { "P-7a", "Krawedz przeryw.szer", false, { G_OFF, G_OFF, G_OFF, G_OFF, G_OFF, G_DASH(1.0f, 1.0f) } },
    { "P-7b", "Krawedz ciagla szer",  false, { G_OFF, G_OFF, G_OFF, G_OFF, G_OFF, G_CONT } },
    { "P-7c", "Krawedz przeryw.wask", false, { G_OFF, G_OFF, G_OFF, G_OFF, G_DASH(1.0f, 1.0f), G_OFF } },
    { "P-7d", "Krawedz ciagla wask",  false, { G_OFF, G_OFF, G_OFF, G_OFF, G_CONT, G_OFF } },
};

// Lateralne położenie pasów (ułamek szerokości jezdni od środka) i szerokości - do wizualizacji
const float GUN_LATERAL[NGUNS]  = { -0.075f, 0.0f, 0.075f, 0.0f, 0.40f, 0.40f };
const float GUN_WIDTH_FR[NGUNS] = {  0.030f, 0.030f, 0.030f, 0.060f, 0.030f, 0.060f };
const uint8_t GUN_WIDTH_CM[NGUNS] = { 12, 12, 12, 24, 12, 24 };
const int8_t  GUN_CENTER_CM[NGUNS] = { -12, 0, 12, 0, 30, 30 };

void patternSpecText(const GunCfg cfg[NGUNS], char* modes, size_t nm, char* widths, size_t nw) {
    modes[0] = 0;
    widths[0] = 0;
    size_t lm = 0, lw = 0;
    int n = 0;
    for (int g = 0; g < NGUNS; g++) {
        if (cfg[g].mode == GM_OFF) continue;
        char tok[24];
        if (cfg[g].mode == GM_CONT) snprintf(tok, sizeof(tok), "ciagla");
        else snprintf(tok, sizeof(tok), "%g/%g m", (double)cfg[g].line, (double)cfg[g].gap);
        lm += snprintf(modes + lm, nm - lm, "%s%s", n ? " + " : "", tok);
        lw += snprintf(widths + lw, nw - lw, "%s%d", n ? "+" : "", (int)GUN_WIDTH_CM[g]);
        n++;
        if (lm >= nm - 1 || lw >= nw - 1) break;
    }
    if (n == 0) { snprintf(modes, nm, "brak"); return; }
    if (lw + 4 < nw) snprintf(widths + lw, nw - lw, " cm");
}

// ---------- pomocnicze parsowanie ----------
static float jf(JsonVariantConst v, float def = 0.0f) {
    if (v.is<float>()) return v.as<float>();
    if (v.is<const char*>()) {
        const char* p = v.as<const char*>();
        return p ? (float)atof(p) : def;
    }
    return def;
}

static void jstr(JsonVariantConst v, char* dst, size_t n) {
    const char* p = v.is<const char*>() ? v.as<const char*>() : nullptr;
    strlcpy(dst, p ? p : "", n);
}

static MState parseState(const char* s) {
    if (!s) return MS_UNKNOWN;
    if (!strcmp(s, "idle")) return MS_IDLE;
    if (!strcmp(s, "painting")) return MS_PAINTING;
    if (!strcmp(s, "paused")) return MS_PAUSED;
    if (!strcmp(s, "stopped")) return MS_STOPPED;
    return MS_UNKNOWN;
}

static MMode parseMode(const char* s) {
    if (!s) return MM_AUTO;
    if (!strcmp(s, "semi")) return MM_SEMI;
    if (!strcmp(s, "manual")) return MM_MANUAL;
    if (!strcmp(s, "demo")) return MM_DEMO;
    return MM_AUTO;
}

bool parseStatus(const char* json, size_t len, Status& o) {
    JsonDocument doc;
    if (deserializeJson(doc, json, len) != DeserializationError::Ok) return false;
    if (!doc["state"].is<const char*>()) return false;   // np. "{}" z pominiętego broadcastu

    o.state = parseState(doc["state"].as<const char*>());
    o.mode  = parseMode(doc["mode"].is<const char*>() ? doc["mode"].as<const char*>() : nullptr);

    o.patternIdx = doc["patternIdx"] | 0;
    if (o.patternIdx < 0 || o.patternIdx >= NPAT) o.patternIdx = 0;
    jstr(doc["pattern"], o.patternCode, sizeof(o.patternCode));
    jstr(doc["patternName"], o.patternName, sizeof(o.patternName));
    o.reversed    = doc["reversed"] | false;
    o.gapStart    = doc["gapStart"] | false;
    o.customValid = doc["customValid"] | false;
    o.activeSlot  = doc["activeSlot"] | 0;
    for (int i = 0; i < NSLOTS; i++) o.slotsValid[i] = doc["slotsValid"][i] | false;

    for (int i = 0; i < NGUNS; i++) o.custom[i] = {GM_OFF, 0, 0};
    JsonArrayConst cg = doc["customGuns"].as<JsonArrayConst>();
    for (JsonVariantConst g : cg) {
        const char* nm = g[0].is<const char*>() ? g[0].as<const char*>() : nullptr;
        if (!nm || nm[0] != 'P') continue;
        int gi = atoi(nm + 1) - 1;
        if (gi < 0 || gi >= NGUNS) continue;
        float ln = jf(g[2]);
        float gp = jf(g[3]);
        if (ln > 0.0f) o.custom[gi] = {GM_DASH, ln, gp};
        else           o.custom[gi] = {GM_CONT, 0, 0};
    }

    o.speed    = jf(doc["speed"]);
    o.distance = jf(doc["distance"]);
    o.area     = jf(doc["area"]);
    o.elapsed  = doc["elapsed"] | 0;
    o.hasPatDist = doc["patDist"].is<float>() || doc["patDist"].is<const char*>();
    o.patDist  = o.hasPatDist ? jf(doc["patDist"]) : 0.0f;

    for (int i = 0; i < NGUNS; i++) {
        o.guns[i]       = doc["guns"][i] | false;
        o.gunAnomaly[i] = doc["gunAnomaly"][i] | false;
    }
    o.anomalyDetected = doc["gunAnomalyDetected"] | false;

    o.overspeed        = doc["overspeed"] | false;
    o.lowSpeed         = doc["lowSpeed"] | false;
    o.autoPaused       = doc["autoPaused"] | false;
    o.autoResume       = doc["autoResumeEnabled"] | true;
    o.semiLineComplete = doc["semiLineComplete"] | false;
    o.semiSegment      = doc["semiSegment"] | 0;

    o.hasPatGroup    = doc["patGroup"].is<int>();
    o.patGroup       = (doc["patGroup"] | 0) == 1 ? 1 : 0;
    o.patBtnLayout   = (doc["patBtnLayout"] | 0) == 1 ? 1 : 0;

    o.smartSwitch    = doc["smartSwitch"] | true;
    o.patternPending = doc["patternPending"] | false;
    jstr(doc["pendingPattern"], o.pendingCode, sizeof(o.pendingCode));

    o.calibrated  = doc["calibrated"] | false;
    o.calibrating = doc["calibrating"] | false;
    o.calPulses   = jf(doc["calPulses"]);
    o.ppm         = jf(doc["ppm"]);
    o.maxSpeed    = jf(doc["maxSpeed"]);
    o.minSpeed    = jf(doc["minSpeed"]);

    o.gpsFix   = doc["gpsFix"] | false;
    o.gpsSat   = doc["gpsSat"] | 0;
    o.gpsHdop  = jf(doc["gpsHdop"]);
    o.gpsSpeed = jf(doc["gpsSpeed"]);
    o.gpxRec   = doc["gpxRec"] | false;

    o.paintLevelL   = jf(doc["paintLevelL"]);
    o.paintLevelPct = doc["paintLevelPct"] | 0;

    jstr(doc["firmware"], o.firmware, sizeof(o.firmware));
    o.uptime  = doc["uptime"] | 0;
    o.clients = doc["clients"] | 0;

    o.valid = true;
    return true;
}

bool parseStats(const char* json, size_t len, StatsData& o) {
    JsonDocument doc;
    if (deserializeJson(doc, json, len) != DeserializationError::Ok) return false;
    if (doc["sessionDistanceM"].isNull() && doc["lifetimeDistanceM"].isNull()) return false;

    o.lifeDistance = jf(doc["lifetimeDistanceM"]);
    o.lifeArea     = jf(doc["lifetimeAreaM2"]);
    o.lifeTimeSec  = doc["lifetimePaintTimeSec"] | 0;
    o.sessDistance = jf(doc["sessionDistanceM"]);
    o.sessArea     = jf(doc["sessionAreaM2"]);
    o.sessTimeSec  = doc["sessionTimeSec"] | 0;
    for (int i = 0; i < NGUNS; i++) {
        o.gunDist[i]  = jf(doc["gunDistances"][i]);
        o.gunShots[i] = doc["gunShotCounts"][i] | 0;
    }
    o.sdReady     = doc["sdReady"] | false;
    o.reportCount = doc["reportCount"] | 0;
    o.paintUsedL       = jf(doc["paintUsedL"]);
    o.paintRemainingL  = jf(doc["paintRemainingL"]);
    o.paintTankL       = jf(doc["paintTankL"]);
    o.paintLevelL      = jf(doc["paintCurrentLevelL"]);
    o.refuelCount      = doc["refuelCount"] | 0;
    o.totalRefueledL   = jf(doc["totalRefueledL"]);
    o.valid = true;
    return true;
}

bool parseSlotConfig(const char* json, size_t len, SlotCfg& o) {
    JsonDocument doc;
    if (deserializeJson(doc, json, len) != DeserializationError::Ok) return false;
    if (!doc["guns"].is<JsonArrayConst>()) return false;
    o.valid = doc["valid"] | false;
    for (int i = 0; i < NGUNS; i++) {
        int m = doc["guns"][i]["mode"] | 0;
        if (m < 0 || m > 2) m = 0;
        float ln = jf(doc["guns"][i]["ln"], 4.0f);
        float gp = jf(doc["guns"][i]["gp"], 8.0f);
        o.guns[i] = {(uint8_t)m, ln, gp};
    }
    return true;
}

GunCfg patternGun(int patIdx, bool reversed, int gun, const Status& s) {
    if (gun < 0 || gun >= NGUNS) return {GM_OFF, 0, 0};
    if (patIdx == PAT_CUSTOM_IDX) return s.custom[gun];
    if (patIdx < 0 || patIdx >= NPAT_PREDEF) return {GM_OFF, 0, 0};
    const PatternInfo& p = PATTERNS[patIdx];
    if (reversed && p.hasReverse) {
        if (gun == 0) return p.guns[2];
        if (gun == 2) return p.guns[0];
    }
    return p.guns[gun];
}

GunCfg effectiveGun(const Status& s, int gun) {
    return patternGun(s.patternIdx, s.reversed, gun, s);
}

float patternCycle(const Status& s) {
    for (int i = 0; i < NGUNS; i++) {
        GunCfg c = effectiveGun(s, i);
        if (c.mode == GM_DASH && c.line > 0) return c.line + c.gap;
    }
    return 0;
}

int softKeyPattern(int group, int key) {
    if (key < 0 || key >= SOFTKEY_COUNT) return -1;
    if (group == 0) return key;
    static const int edge[SOFTKEY_COUNT] = {10, 11, 12, 13, 14, 15, -1, -1, -1, -1};
    return edge[key];
}

int patternGroupOf(int patIdx, int currentGroup) {
    if (patIdx >= 0 && patIdx <= 9) return 0;
    if (patIdx >= 10 && patIdx <= 14) return 1;
    return currentGroup;
}

const char* stateName(MState s) {
    switch (s) {
        case MS_IDLE:     return "GOTOWY";
        case MS_PAINTING: return "MALOWANIE";
        case MS_PAUSED:   return "PAUZA";
        case MS_STOPPED:  return "ZATRZYMANY";
        default:          return "---";
    }
}

const char* modeName(MMode m) {
    switch (m) {
        case MM_AUTO:   return "AUTO";
        case MM_SEMI:   return "SEMI";
        case MM_MANUAL: return "RECZNY";
        case MM_DEMO:   return "DEMO";
    }
    return "AUTO";
}
