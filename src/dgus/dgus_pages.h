#pragma once
// ============================================================
// MPD2026 - dane dla ekranow DGUS: struktury posrednie miedzy stanem
// sterownika a zapisami VP (konwersja na bajty/slowa robi dgus_link.cpp).
// ============================================================

#include "config.h"

#if HAS_DGUS_LINK

#include "dgus_map.h"

// ---- Ekran roboczy (strony HOME/PAINTING) ----
struct DgusHomeStatus {
    int      state = 0, mode = 0;
    char     patternCode[13] = {};
    char     patternName[33] = {};
    bool     reversed = false, gapStart = false;
    float    speedKmh = 0, distanceM = 0, areaM2 = 0, patDistM = 0;
    uint32_t elapsedS = 0;
    bool     overspeed = false, lowSpeed = false, autoPaused = false;
    bool     semiComplete = false;
    int      semiSegment = 0;
    uint8_t  gunState[NUM_GUNS] = {};   // 0 off/nieuzyty, 1 strzela, 2 skonfig.-bezczynny
    int      paintPct = 0;
    int      gpsSat = 0;
    bool     gpsFix = false;
    bool     patternPending = false;
    char     pendingCode[13] = {};
    int      patGroup = 0;
    bool     night = false;
    bool     linkOk = false;
    bool     sdReady = false;

    // Pistolety - anomalia (brak ruchu mimo aktywnego pistoletu)
    bool     gunAnomalyDetected = false;
    bool     gunAnomaly[NUM_GUNS] = {};

    // Enkoder
    bool     encCalibrated = false;
    float    encPpm = 0;

    // GPS (pelny zestaw - pozycja, predkosc, jakosc, zapis trasy)
    float    gpsSpeedKmh = 0;
    double   gpsLat = 0, gpsLng = 0;
    float    gpsHdop = 0;
    bool     gpxRecording = false;
    uint32_t gpxPoints = 0;
    bool     gpxOverflow = false;

    // Czujnik temperatury DS18B20 (opcjonalny)
    bool     tempAvailable = false;
    float    tempC = 0;

    // Diagnostyka systemu (przydatne w terenie bez laptopa)
    int      freeHeapKB = 0;
    uint32_t uptimeMin = 0;
    int      wwwClients = 0;
};
void dgusBuildHomeStatus(DgusHomeStatus& out);

// ---- Kolumny S1..S10 ----
struct DgusSoftkeys {
    int8_t  patternIdx[10] = {};   // -1 = slot pusty
    bool    selected[10] = {};
    char    code[10][13] = {};
    bool    customValid = false;   // czy slot WLASNY jest zapisany (do wyszarzenia ikony w Designerze)
};
void dgusBuildSoftkeys(DgusSoftkeys& out);

// ---- Ekrany serwisowe (strony 2..16), wypelniane przez MenuSystem::fillDgusPage() ----
struct DgusPageData {
    char    row[dgusmap::DGUS_ROW_COUNT][33] = {};   // wartosc wiersza (etykieta jest statyczna w DGUS Designer)
    int     selected = -1;
    char    message[65] = {};
    bool    flagA = false, flagB = false;
    int32_t num1 = 0, num2 = 0;
};

#endif  // HAS_DGUS_LINK
