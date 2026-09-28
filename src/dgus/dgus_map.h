#pragma once
// ============================================================
// MPD2026 - mapa VP (adresow zmiennych) i stron DGUS dla wyswietlacza DWIN
// (docelowo: DMG10600T070_09WTC, 7" 1024x600, ale mapa jest niezalezna od modelu).
//
// TO JEST RÓWNIEŻ SPECYFIKACJA DO ZBUDOWANIA PROJEKTU W DGUS DESIGNER:
// każdy VP poniżej musi mieć w Designerze dokładnie taki adres, szerokość
// (liczbę słów) i typ kontrolki, jak opisano w komentarzach. Etykiety, tła,
// przyciski nawigacyjne i statyczny tekst są rysowane w Designerze — sterownik
// wysyła tylko WARTOŚCI (patrz docs/ARCHITEKTURA_TERMINAL.md, rozdz. 5-6).
//
// Zasada: sterownik jest jedynym źródłem prawdy o tym, który ekran jest
// aktywny — DGUS nigdy nie zmienia strony lokalnie (żaden przycisk w Designerze
// nie ma ustawionego "jump page"), tylko wysyła kod zdarzenia pod VP_TOUCH_EVENT.
// Sterownik decyduje o zmianie ekranu i sam poleca DGUS przełączenie strony
// (VP systemowe 0x0084, patrz shared/dgus_protocol.h::encodeSwitchPage()).
// ============================================================

#include <stdint.h>

namespace dgusmap {

// ---- Numery stron DGUS (Page ID w Designerze) = dokladnie ScreenID sterownika ----
// (SCREEN_HOME=0 ... SCREEN_POST=16, patrz src/config.h enum ScreenID).
// Strona DGUS nr N pokazuje to, co dzisiaj ScreenID==N.

// ================================================================
// BLOK 1: status ekranu roboczego (strony 0 = HOME, 1 = PAINTING)
// Pisany co ok. 250 ms (tylko gdy aktywna strona 0 lub 1).
// ================================================================
constexpr uint16_t VP_STATE          = 0x1000;  // 1 slowo: 0 idle,1 painting,2 paused,3 stopped
constexpr uint16_t VP_MODE           = 0x1001;  // 1 slowo: 0 auto,1 semi,2 reczny,3 demo
constexpr uint16_t VP_PATTERN_CODE   = 0x1002;  // 6 slow / 12 znakow ASCII, np. "P-1a"
constexpr uint16_t VP_PATTERN_NAME   = 0x1008;  // 16 slow / 32 znaki ASCII
constexpr uint16_t VP_REVERSED       = 0x1018;  // 1 slowo: 0/1
constexpr uint16_t VP_GAP_START      = 0x1019;  // 1 slowo: 0/1
constexpr uint16_t VP_SPEED_X10      = 0x101A;  // 1 slowo, int16: predkosc*10 [km/h]
constexpr uint16_t VP_DISTANCE_DM    = 0x101B;  // 2 slowa, int32: dystans sesji [dm]
constexpr uint16_t VP_AREA_CM2       = 0x101D;  // 2 slowa, int32: powierzchnia sesji [cm2/100 = m2*100]... patrz opis nizej
constexpr uint16_t VP_ELAPSED_S      = 0x101F;  // 2 slowa, int32: czas sesji [s]
constexpr uint16_t VP_PATDIST_DM     = 0x1021;  // 2 slowa, int32: dystans od startu wzorca [dm] (do prostego paska postepu)
constexpr uint16_t VP_OVERSPEED      = 0x1023;  // 1 slowo: 0/1
constexpr uint16_t VP_LOWSPEED       = 0x1024;  // 1 slowo: 0/1
constexpr uint16_t VP_AUTO_PAUSED    = 0x1025;  // 1 slowo: 0/1
constexpr uint16_t VP_SEMI_COMPLETE  = 0x1026;  // 1 slowo: 0/1
constexpr uint16_t VP_SEMI_SEGMENT   = 0x1027;  // 1 slowo, int16
constexpr uint16_t VP_GUN_BASE       = 0x1028;  // 6 slow, po 1/pistolet: 0 wylaczony/nieuzyty,1 strzela,2 skonfig.-bezczynny
constexpr uint16_t VP_PAINT_PCT      = 0x102E;  // 1 slowo, int16: poziom farby [%]
constexpr uint16_t VP_GPS_SAT        = 0x102F;  // 1 slowo, int16
constexpr uint16_t VP_GPS_FIX        = 0x1030;  // 1 slowo: 0/1
constexpr uint16_t VP_PAT_PENDING    = 0x1031;  // 1 slowo: 0/1
constexpr uint16_t VP_PENDING_CODE   = 0x1032;  // 6 slow / 12 znakow ASCII
constexpr uint16_t VP_PAT_GROUP      = 0x1038;  // 1 slowo: 0=OS,1=KRAWEDZ
constexpr uint16_t VP_NIGHT          = 0x103A;  // 1 slowo: 0/1 (tryb nocny - przyciemnij podswietlenie w DGUS Designer)
constexpr uint16_t VP_LINK_OK        = 0x103B;  // 1 slowo: 0/1 (informacyjnie; samo DGUS wie najlepiej czy odbiera)
constexpr uint16_t VP_SD_READY       = 0x103C;  // 1 slowo: 0/1

// ---- Rozszerzenie bloku 1: pistolety, enkoder, GPS, czujnik temperatury, diagnostyka systemu ----
// (ta sama strona robocza; ciag dalszy adresow, zeby nie kolidowac z blokiem 2 zaczynajacym sie 0x1100)
constexpr uint16_t VP_GUN_ANOMALY        = 0x103D;  // 1 slowo: 0/1 - wykryto anomalie ktoregos pistoletu (alarm)
constexpr uint16_t VP_GUN_ANOMALY_BASE   = 0x103E;  // 6 slow, po 1/pistolet: 0/1 ktory pistolet jest anomalny (0x103E-0x1043)
constexpr uint16_t VP_ENC_CALIBRATED     = 0x1044;  // 1 slowo: 0/1 - enkoder skalibrowany (ostrzezenie gdy 0)
constexpr uint16_t VP_ENC_PPM_X10        = 0x1045;  // 1 slowo, int16: impulsow/metr *10
constexpr uint16_t VP_GPS_SPEED_X10      = 0x1046;  // 1 slowo, int16: predkosc GPS *10 [km/h]
constexpr uint16_t VP_GPS_LAT_X1E6       = 0x1047;  // 2 slowa, int32: szerokosc geogr. *1 000 000
constexpr uint16_t VP_GPS_LNG_X1E6       = 0x1049;  // 2 slowa, int32: dlugosc geogr. *1 000 000
constexpr uint16_t VP_GPS_HDOP_X10       = 0x104B;  // 1 slowo, int16: HDOP *10 (99.9 = brak danych)
constexpr uint16_t VP_GPX_RECORDING      = 0x104C;  // 1 slowo: 0/1 - zapis trasy GPX trwa
constexpr uint16_t VP_GPX_POINTS         = 0x104D;  // 2 slowa, int32: liczba zapisanych punktow trasy
constexpr uint16_t VP_GPX_OVERFLOW       = 0x104F;  // 1 slowo: 0/1 - bufor trasy GPX pelny (ostrzezenie)
constexpr uint16_t VP_TEMP_AVAILABLE     = 0x1050;  // 1 slowo: 0/1 - czujnik DS18B20 wykryty
constexpr uint16_t VP_TEMP_X10           = 0x1051;  // 1 slowo, int16 (ze znakiem): temperatura *10 [C]
constexpr uint16_t VP_FREE_HEAP_KB       = 0x1052;  // 1 slowo, int16: wolna pamiec RAM [KB] (diagnostyka)
constexpr uint16_t VP_UPTIME_MIN         = 0x1053;  // 1 slowo, int16: czas pracy od wlaczenia [min]
constexpr uint16_t VP_WWW_CLIENTS        = 0x1054;  // 1 slowo, int16: liczba polaczonych klientow WiFi (telefon/laptop)
constexpr uint16_t VP_CUSTOM_VALID       = 0x1055;  // 1 slowo: 0/1 - czy wzorzec WLASNY jest zapisany (do wyszarzenia ikony)
constexpr uint16_t VP_ESTOP_TRIGGERED    = 0x1056;  // 1 slowo: 0/1 - petla E-STOP otwarta TERAZ (wysylane na KAZDEJ stronie, nie tylko HOME)
constexpr uint16_t VP_ESTOP_AWAIT_ACK    = 0x1057;  // 1 slowo: 0/1 - bylo zadzialanie, czeka na potwierdzenie operatora (patrz KEY_ESTOP_ACK)

// Uwaga jednostek: DGUS "value display" najczesciej pokazuje liczby calkowite lub
// stalo-przecinkowe (ustawienie liczby miejsc po przecinku w kontrolce, nie w danych).
// distance_dm/10 = metry (1 miejsce po przecinku), area_cm2 jest w rzeczywistosci
// m2*100 (2 miejsca po przecinku) - nazwa pola to skrot mysli "wartosc x100 jak cm2".

// ================================================================
// BLOK 2: kolumny wzorcow S1..S10 (soft-key), widoczne na stronach 0 i 1
// ================================================================
constexpr uint16_t VP_SLOT_PATIDX_BASE = 0x1100;  // 10 slow, po 1/slot: indeks wzorca 0-15, 255 = slot pusty
constexpr uint16_t VP_SLOT_SEL_BASE    = 0x110A;  // 10 slow, po 1/slot: 0/1 czy to aktualnie wybrany wzorzec
constexpr uint16_t VP_SLOT_CODE_BASE   = 0x1120;  // 10 x 6 slow (12 znakow) = 60 slow: kod wzorca w slocie, "" gdy pusty

// ================================================================
// BLOK 3: generyczne pola ekranow serwisowych (strony 2..16)
// Tylko JEDNA taka strona jest widoczna naraz - adresy sa wspoldzielone
// (kontroler pisze wg biezacego ScreenID, DGUS pokazuje wg wlasnej strony).
// ================================================================
constexpr int      DGUS_ROW_COUNT   = 10;              // 10 wierszy - miesci np. wszystkie 8 pozycji POST + margines
constexpr int      DGUS_ROW_WORDS   = 16;              // 16 slow = 32 znaki na wartosc wiersza
constexpr uint16_t VP_ROW_BASE      = 0x1210;          // wiersz i: VP_ROW_BASE + i*DGUS_ROW_WORDS (0x1210..0x12AF)
constexpr uint16_t VP_ROW_SELECTED  = 0x12B0;          // 1 slowo, int16: podswietlony wiersz (-1 = brak)
constexpr uint16_t VP_MSG           = 0x12B1;          // 32 slowa / 64 znaki: komunikat pod tytulem strony
constexpr uint16_t VP_FLAGS         = 0x12D1;          // 1 slowo, bitowo: bit0=flagA, bit1=flagB (znaczenie zalezy od strony)
constexpr uint16_t VP_NUM1          = 0x12D2;          // 2 slowa, int32: liczba pomocnicza 1 (znaczenie zalezy od strony)
constexpr uint16_t VP_NUM2          = 0x12D4;          // 2 slowa, int32: liczba pomocnicza 2

// Znaczenie VP_FLAGS/VP_NUM1/VP_NUM2 per strona (patrz src/dgus_pages.cpp):
//   CALIBRATION:    flagA=w trakcie, NUM1=impulsy*10
//   DISTANCE_METER: flagA=pomiar trwa, NUM1=zmierzony dystans*100 [cm]
//   NOZZLE_CLEAN:   NUM1=indeks wzorca (do podswietlenia ikony)
//   STATS_EXPORT:   flagA=zakonczono, flagB=sukces
//   TANKOWANIE:     flagA=zatwierdzono, NUM1=ilosc dolewki*10 [L], NUM2=poziom w zbiorniku*10 [L]

// ================================================================
// BLOK 4: zdarzenia dotyku (T -> S) - WSPÓLNY rejestr dla wszystkich stron.
// Każdy przycisk w DGUS Designer (na dowolnej stronie) jest skonfigurowany
// jako "Return key code" / "Momentary button" piszacy WLASNY, STALY kod
// pod TEN SAM adres VP_TOUCH_EVENT. Sterownik odczytuje kod (patrz
// src/dgus_link.cpp) i wywoluje dokladnie te sama logike, co przycisk
// fizyczny (executeControl / menu.handleEvent) - terminal niczym nie steruje.
// ================================================================
constexpr uint16_t VP_TOUCH_EVENT = 0x1300;   // 1 slowo: kod zdarzenia (patrz tabela nizej)

// VP_PING: adres UŻYWANY WYŁĄCZNIE do sprawdzania żywotności łącza (sterownik okresowo
// wysyła zapytanie odczytu tego adresu; sama odpowiedź, niezależnie od treści, jest
// dowodem, że ekran żyje). CELOWO oddzielony od VP_TOUCH_EVENT: gdybyśmy odpytywali
// (READ) adres zdarzeń dotyku, dostawalibyśmy w odpowiedzi jego OSTATNIĄ zapisaną
// wartość i błędnie interpretowali ją jako nowe, powtórzone naciśnięcie klawisza.
constexpr uint16_t VP_PING = 0x1301;   // 1 slowo, dowolna wartość - trescia się nie posługujemy

// ---- Kody zdarzen (wartosc pod VP_TOUCH_EVENT) ----
// 1-7: DOKLADNIE enum ButtonEvent sterownika (src/button_handler.h) - ta sama
//      sciezka co przyciski fizyczne: menu.handleEvent((ButtonEvent)kod).
constexpr uint16_t KEY_START_SHORT   = 1;
constexpr uint16_t KEY_START_LONG    = 2;   // np. FACTORY_RESET/COUNTER_RESET "potwierdz"
constexpr uint16_t KEY_STOP_SHORT    = 3;
constexpr uint16_t KEY_STOP_LONG     = 4;   // z HOME: wejscie do serwisu; na ekranach serwisowych: wyjdz
constexpr uint16_t KEY_SELECT_SHORT  = 5;
constexpr uint16_t KEY_SELECT_LONG   = 6;
constexpr uint16_t KEY_GAP_START     = 7;

// 30-40: bezposrednie wejscie do pozycji menu serwisowego (executeControl "set_screen").
// Zamiast przewijac lista jak na malym ekranie, operator dotyka wprost pozycji.
constexpr uint16_t KEY_GOTO_CALIBRATION    = 30;
constexpr uint16_t KEY_GOTO_DISTANCE_METER = 31;
constexpr uint16_t KEY_GOTO_REPORTS        = 32;
constexpr uint16_t KEY_GOTO_NOZZLE_CLEAN   = 33;
constexpr uint16_t KEY_GOTO_LIFETIME_STATS = 34;
constexpr uint16_t KEY_GOTO_CUSTOM_PATTERN = 35;
constexpr uint16_t KEY_GOTO_STATS_EXPORT   = 36;
constexpr uint16_t KEY_GOTO_SESSION_RESET  = 37;
constexpr uint16_t KEY_GOTO_COUNTER_RESET  = 38;
constexpr uint16_t KEY_GOTO_TANKOWANIE     = 39;
constexpr uint16_t KEY_GOTO_FACTORY_RESET  = 40;

// 41: potwierdzenie operatora po ustapieniu STOP-u awaryjnego (patrz VP_ESTOP_AWAIT_ACK,
// estop.h). Przycisk w DGUS Designer powinien byc aktywny/widoczny tylko gdy
// VP_ESTOP_AWAIT_ACK=1 (i VP_ESTOP_TRIGGERED=0 - petla juz zamknieta).
constexpr uint16_t KEY_ESTOP_ACK           = 41;

// 70-79: soft-key S1..S10 (slot = kod-70); 80: GRUPA (przelacz OS/KRAWEDZ)
constexpr uint16_t KEY_SOFTKEY_BASE = 70;   // S1=70 ... S10=79
constexpr uint16_t KEY_GRUPA        = 80;

// 90/91: "martwy czlowiek" czyszczenia dysz (ekran NOZZLE_CLEAN). Przycisk w DGUS
// Designer skonfigurowany jako "press value"=90, "release value"=91 (typowa
// funkcja przyciskow momentary/jog w HMI - do zweryfikowania dokladnej nazwy
// opcji w uzywanej wersji Designera). Sterownik dodatkowo pilnuje twardego limitu
// czasu przytrzymania (DGUS_NOZZLE_HOLD_MAX_MS w dgus_link.cpp) na wypadek
// zgubienia komunikatu zwolnienia.
constexpr uint16_t KEY_NOZZLE_HOLD_ON  = 90;
constexpr uint16_t KEY_NOZZLE_HOLD_OFF = 91;

}  // namespace dgusmap
