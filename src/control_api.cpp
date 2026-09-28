#include "sys_log.h"
// ============================================================
// MPD2026 - wspolne wykonywanie polecen sterowania
// Ciało przeniesione z TrassarWebServer::handleControl() (bez zmiany zachowania);
// uzywane przez HTTP (Core 0) oraz lacze terminala (Core 1).
// ============================================================

#include "control_api.h"
#include "config.h"
#include "dgus/dgus_link.h"
#include <cmath>
#include <ArduinoJson.h>
#include "painting_engine.h"
#include "menu.h"
#include "encoder_distance.h"
#include "guns.h"
#include "patterns.h"
#include "pattern_buttons.h"
#include "storage.h"
#include "paint_consumption.h"
#include "estop.h"
#include "statistics.h"
#include "event_log.h"

namespace {

ControlResult makeResult(const String& message) {
    ControlResult r;
    r.httpCode = 200;
    r.ok = (message == "ok");
    r.message = message;
    r.body = "{\"result\":\"" + message + "\"}";
    return r;
}

ControlResult makeError(int code, const char* message) {
    ControlResult r;
    r.httpCode = code;
    r.ok = false;
    r.message = message;
    r.body = String("{\"error\":\"") + message + "\"}";
    return r;
}

}  // namespace

ControlResult executeControl(const String& action, const ControlArgs& args) {
    String result = "ok";

    // Atomowy snapshot stanu (wymagany do decyzji o akcji)
    // Fix #25/#30: Trylock — jesli inny watek trzyma mutex, zwroc blad zamiast blokowac.
    if (!STATE_TRYLOCK(200)) {
        return makeError(503, "serwer zajety — sprobuj ponownie");
    }
    MachineState snapState = g_state.machineState;
    ScreenID snapScreen = g_state.currentScreen;
    STATE_UNLOCK();

    if (action == "start") {
#if HAS_ESTOP
        if (estop.isTriggered()) {
            result = "STOP awaryjny aktywny - zwolnij grzybek";
        } else if (estop.isAwaitingAck()) {
            result = "potwierdz STOP awaryjny przed wznowieniem";
        } else
#endif
        // Jesli ekran QR startowy jest aktywny — zamknij go zamiast startowac malowanie
        if (snapScreen == SCREEN_POST) {
            if (STATE_TRYLOCK(500)) { g_state.qrDismissed = true; STATE_UNLOCK(); }
        } else if (snapState == STATE_PAUSED) {
            paintEngine.resume();
        } else if (snapState == STATE_IDLE || snapState == STATE_STOPPED) {
            paintEngine.start();
        }
    } else if (action == "start_from_gap") {
        paintEngine.startFromGap();
    } else if (action == "pause") {
        paintEngine.pause();
    } else if (action == "stop") {
        // Fix #30: requestStop() natychmiast wylacza pistolety i zmienia stan,
        // a Core 1 wykonuje zapis danych w nastepnym update().
        paintEngine.requestStop();
    } else if (action == "set_pattern") {
        if (args.has("value")) {
            int val = args.get("value").toInt();
            if (val >= 0 && val < PAT_COUNT) {
                paintEngine.setPattern((PatternID)val);
            } else {
                result = "nieprawidlowy wzorzec";
            }
        } else {
            result = "brak parametru value";
        }
    } else if (action == "toggle_reverse") {
        paintEngine.toggleReverse();
    } else if (action == "cal_start") {
        encoderDist.startCalibration();
    } else if (action == "cal_finish") {
        encoderDist.finishCalibration();
    } else if (action == "set_max_speed") {
        if (args.has("value")) {
            float val = args.get("value").toFloat();
            // Fix #8: walidacja NaN/Inf + cross-check z minSpeed
            if (isnan(val) || isinf(val)) {
                result = "nieprawidlowa wartosc";
            } else if (val >= 5.0f && val <= 30.0f) {
                if (val <= paintEngine.getMinSpeed()) {
                    result = "maxSpeed musi byc > minSpeed";
                } else {
                    paintEngine.setMaxSpeed(val);
                    storage.saveMaxSpeed(val);
                }
            } else {
                result = "zakres 5-30 km/h";
            }
        } else {
            result = "brak parametru value";
        }
    } else if (action == "set_min_speed") {
        if (args.has("value")) {
            float val = args.get("value").toFloat();
            // Fix #8: walidacja NaN/Inf + cross-check z maxSpeed
            if (isnan(val) || isinf(val)) {
                result = "nieprawidlowa wartosc";
            } else if (val >= 0.0f && val <= 10.0f) {
                if (val >= paintEngine.getMaxSpeed()) {
                    result = "minSpeed musi byc < maxSpeed";
                } else {
                    paintEngine.setMinSpeed(val);
                    storage.saveMinSpeed(val);
                }
            } else {
                result = "zakres 0-10 km/h";
            }
        } else {
            result = "brak parametru value";
        }
    } else if (action == "set_mode") {
        if (args.has("value")) {
            int val = args.get("value").toInt();
            if (val >= 0 && val <= 3) {
                // Nie zmieniaj trybu podczas malowania — niebezpieczne
                if (STATE_TRYLOCK(200)) {
                    MachineState modeState = g_state.machineState;
                    if (modeState == STATE_IDLE || modeState == STATE_STOPPED) {
                        MachineMode newMode = (MachineMode)val;
                        g_state.machineMode = newMode;
                        STATE_UNLOCK();
                        storage.saveMode(newMode);
                        DBG_PRINTF("[CTRL] Tryb pracy: %d\n", val);
                    } else {
                        STATE_UNLOCK();
                        result = "nie mozna zmienic trybu podczas malowania";
                    }
                } else {
                    result = "serwer zajety — sprobuj ponownie";
                }
            } else {
                result = "nieprawidlowy tryb (0-3)";
            }
        } else {
            result = "brak parametru value";
        }
    } else if (action == "save_custom_pattern") {
        // Parametry: g0..g5, ln0..ln5, gp0..gp5, slot (0-2)
        CustomPatternCfg cfg = {};
        cfg.structVersion = CUSTOM_PAT_STRUCT_VER;
        cfg.valid = true;
        bool validationError = false;
        for (int i = 0; i < NUM_GUNS; i++) {
            String gKey = "g" + String(i);
            String lKey = "ln" + String(i);
            String pKey = "gp" + String(i);
            if (args.has(gKey.c_str())) {
                int gm = args.get(gKey.c_str()).toInt();
                if (gm < 0 || gm > 2) gm = 0;
                cfg.gunModes[i] = (uint8_t)gm;
            }
            float ln = 4.0f, gp = 8.0f;
            if (args.has(lKey.c_str())) ln = args.get(lKey.c_str()).toFloat();
            if (args.has(pKey.c_str())) gp = args.get(pKey.c_str()).toFloat();
            // Walidacja: odrzuc NaN/Inf i wartosci spoza zakresu
            if (isnan(ln) || isinf(ln) || isnan(gp) || isinf(gp)) {
                validationError = true;
                break;
            }
            if (ln < 0.1f) ln = 0.1f;
            if (ln > 50.0f) ln = 50.0f;
            if (gp < 0.1f) gp = 0.1f;
            if (gp > 50.0f) gp = 50.0f;
            cfg.lineLen[i] = ln;
            cfg.gapLen[i] = gp;
        }
        if (validationError) {
            return makeError(400, "nieprawidlowe wartosci lineLen/gapLen");
        }
        int slot = 0;
        if (args.has("slot")) {
            slot = args.get("slot").toInt();
            if (slot < 0 || slot >= NUM_CUSTOM_SLOTS) slot = 0;
        }
        patternMgr.saveSlot(slot, cfg);
        patternMgr.activateSlot(slot);
        DBG_PRINTF("[CTRL] Wzorzec wlasny slot %d zapisany\n", slot);
    } else if (action == "activate_slot") {
        if (args.has("value")) {
            int slot = args.get("value").toInt();
            if (slot >= 0 && slot < NUM_CUSTOM_SLOTS && patternMgr.isSlotValid(slot)) {
                patternMgr.activateSlot(slot);
            } else {
                result = "slot pusty lub nieprawidlowy";
            }
        }
    } else if (action == "get_slot_config") {
        int slot = 0;
        if (args.has("slot")) {
            slot = args.get("slot").toInt();
            if (slot < 0 || slot >= NUM_CUSTOM_SLOTS) slot = 0;
        }
        CustomPatternCfg cfg = patternMgr.loadSlot(slot);
        JsonDocument slotDoc;
        JsonArray gunsArr = slotDoc["guns"].to<JsonArray>();
        for (int i = 0; i < NUM_GUNS; i++) {
            JsonObject g = gunsArr.add<JsonObject>();
            g["mode"] = cfg.gunModes[i];
            g["ln"]   = serialized(String(cfg.lineLen[i], 1));
            g["gp"]   = serialized(String(cfg.gapLen[i], 1));
        }
        slotDoc["valid"] = cfg.valid;
        ControlResult r;
        r.httpCode = 200;
        r.ok = true;
        r.message = "ok";
        serializeJson(slotDoc, r.body);
        return r;
    } else if (action == "semi_next_line") {
        paintEngine.semiNextLine();
    } else if (action == "send_event") {
        // Wirtualne przyciski — kolejkowanie zdarzenia do petli Core 1.
        // NIE wywoluj menu.handleEvent() bezposrednio z Core 0 — race condition
        // z obsluga przyciskow/joysticka w loop() na Core 1.
        if (args.has("value")) {
            int val = args.get("value").toInt();
            if (val > 0 && val <= (int)EVT_GAP_START) {
                if (STATE_TRYLOCK(500)) {
                    g_state.pendingWebEvent = (ButtonEvent)val;
                    STATE_UNLOCK();
                }
                DBG_PRINTF("[CTRL] Event kolejkowany: %d\n", val);
            } else {
                result = "nieprawidlowy event";
            }
        } else {
            result = "brak parametru value";
        }
    } else if (action == "set_screen") {
        if (args.has("value")) {
            int val = args.get("value").toInt();
            if (val == (int)SCREEN_NOZZLE_CLEAN && snapState != STATE_IDLE && snapState != STATE_STOPPED) {
                result = "czyszczenie dysz tylko w stanie GOTOWY";
            } else if (val >= 0 && val <= (int)SCREEN_STATS_EXPORT) {
                menu.goToScreen((ScreenID)val);
                DBG_PRINTF("[CTRL] Ekran: %d\n", val);
            } else {
                result = "nieprawidlowy ekran";
            }
        } else {
            result = "brak parametru value";
        }
    } else if (action == "set_switch_mode") {
        if (args.has("value")) {
            int val = args.get("value").toInt();
            bool smart = (val == 0);  // 0=smart, 1=instant
            paintEngine.setSmartSwitch(smart);
            storage.saveSwitchMode(smart);
            DBG_PRINTF("[CTRL] Tryb przelaczania: %s\n", smart ? "SMART" : "INSTANT");
        }
    } else if (action == "set_pattern_group") {
        if (args.has("value")) {
            int val = args.get("value").toInt();
            if (val == 0 || val == 1) patternButtons.setGroup((uint8_t)val);
            else result = "grupa 0 (os) lub 1 (krawedz)";
        } else {
            result = "brak parametru value";
        }
    } else if (action == "set_pattern_layout") {
        if (args.has("value")) {
            int val = args.get("value").toInt();
            if (val == 0 || val == 1) patternButtons.setLayout((uint8_t)val);
            else result = "uklad 0 (klasyczny) lub 1 (soft-key)";
        } else {
            result = "brak parametru value";
        }
    } else if (action == "set_tank_capacity") {
        if (args.has("value")) {
            float val = args.get("value").toFloat();
            if (val >= 1.0f && val <= 1000.0f) {
                paintConsumption.setTankCapacity(val);
                storage.saveTankCapacity(val);
            } else {
                result = "zakres 1-1000 litrow";
            }
        }
    } else if (action == "set_paint_rate") {
        if (args.has("value")) {
            float val = args.get("value").toFloat();
            if (val >= 0.1f && val <= 5.0f) {
                paintConsumption.setConsumptionRate(val);
                storage.saveConsumptionRate(val);
            } else {
                result = "zakres 0.1-5.0 l/m2";
            }
        }
    } else if (action == "set_auto_resume") {
        if (args.has("value")) {
            bool en = (args.get("value").toInt() != 0);
            paintEngine.setAutoResumeEnabled(en);
            storage.saveAutoResume(en);
        }
    } else if (action == "refuel") {
        if (args.has("value")) {
            float val = args.get("value").toFloat();
            if (val >= 1.0f && val <= 1000.0f) {
                paintConsumption.refuel(val);
                DBG_PRINTF("[CTRL] Tankowanie: +%.0f L, poziom: %.1f L\n",
                              val, paintConsumption.getCurrentLevel());
            } else {
                result = "zakres 1-1000 litrow";
            }
        }
    } else if (action == "session_reset") {
        // Reset etapu (dystans/powierzchnia/czas biezacej sesji) - odpowiednik SCREEN_SESSION_RESET
        // + START z panelu fizycznego. value=1 to jawne potwierdzenie (zamiast dwuetapowej
        // nawigacji ekranowej: wejdz na ekran -> nacisnij START).
        if (snapState != STATE_IDLE && snapState != STATE_STOPPED) {
            result = "najpierw zatrzymaj malowanie";
        } else if (args.has("value") && args.get("value").toInt() == 1) {
            menu.goToScreen(SCREEN_SESSION_RESET);
            if (STATE_TRYLOCK(500)) { g_state.pendingWebEvent = (uint8_t)EVT_START_SHORT; STATE_UNLOCK(); }
        } else {
            result = "wymagane potwierdzenie: value=1";
        }
    } else if (action == "counter_reset") {
        // Reset wszystkich licznikow oprocz kalibracji - odpowiednik SCREEN_COUNTER_RESET + START.
        if (snapState != STATE_IDLE && snapState != STATE_STOPPED) {
            result = "najpierw zatrzymaj malowanie";
        } else if (args.has("value") && args.get("value").toInt() == 1) {
            menu.goToScreen(SCREEN_COUNTER_RESET);
            if (STATE_TRYLOCK(500)) { g_state.pendingWebEvent = (uint8_t)EVT_START_SHORT; STATE_UNLOCK(); }
        } else {
            result = "wymagane potwierdzenie: value=1";
        }
    } else if (action == "factory_reset") {
        // Pelny reset NVS + restart - odpowiednik SCREEN_FACTORY_RESET + dlugie START (3s) z panelu
        // fizycznego. value=1 zastepuje fizyczne przytrzymanie jawnym potwierdzeniem z UI (WWW/Sunton
        // musza pokazac wlasne "na pewno?" PRZED wyslaniem tego zadania - tu nie ma cofniecia).
        if (snapState != STATE_IDLE && snapState != STATE_STOPPED) {
            result = "najpierw zatrzymaj malowanie";
        } else if (args.has("value") && args.get("value").toInt() == 1) {
            menu.goToScreen(SCREEN_FACTORY_RESET);
            if (STATE_TRYLOCK(500)) { g_state.pendingWebEvent = (uint8_t)EVT_START_LONG; STATE_UNLOCK(); }
        } else {
            result = "wymagane potwierdzenie: value=1";
        }
    } else if (action == "stats_export") {
        // Synchroniczny eksport (w odroznieniu od powyzszych) - caller od razu dostaje wynik
        // zamiast musiec dopytywac o rezultat zdarzenia przetworzonego pozniej na Core 1.
        if (stats.exportLifetimeCsv()) {
            eventLog.log("CTRL", "Eksport statystyk na SD: /stats/lifetime_stats.csv");
        } else {
            result = "eksport nieudany — sprawdz karte SD";
        }
    } else if (action == "nozzle_hold_on") {
        // "Martwy czlowiek" czyszczenia dysz z panelu WWW/Sunton — patrz menu.cpp update()
        // (SCREEN_NOZZLE_CLEAN) i NOZZLE_HOLD_MAX_MS w config.h (twardy limit przytrzymania).
        if (snapScreen != SCREEN_NOZZLE_CLEAN) {
            result = "wejdz najpierw na ekran czyszczenia dysz";
        } else if (STATE_TRYLOCK(500)) {
            g_state.wwwNozzleHoldOn = true;
            g_state.wwwNozzleHoldSetMs = millis();
            STATE_UNLOCK();
        }
    } else if (action == "nozzle_hold_off") {
        if (STATE_TRYLOCK(500)) {
            g_state.wwwNozzleHoldOn = false;
            STATE_UNLOCK();
        }
    } else if (action == "set_dist_target") {
        // Cel pomiaru dystansu (SCREEN_DISTANCE_METER), w decymetrach (value=500 -> 50.0 m),
        // zeby uniknac przesylania ulamkow przez formularz. Precyzja 0,1 m.
        if (args.has("value")) {
            int dm = args.get("value").toInt();
            if (dm >= 0 && dm <= 99990) {
                menu.setDistMeterTarget(dm / 10.0f);
            } else {
                result = "zakres 0-9999.0 m (w decymetrach: 0-99990)";
            }
        } else {
            result = "brak parametru value";
        }
#if HAS_ESTOP
    } else if (action == "ack_estop") {
        // Potwierdzenie operatora po ustapieniu STOP-u awaryjnego (panel WWW) — patrz estop.h.
        // Bez efektu, dopoki petla pozostaje otwarta.
        estop.acknowledge();
#endif
#if HAS_DGUS_LINK
    } else if (action == "set_term_policy") {
        // 0 = kontynuuj malowanie + alarm (domyslnie), 1 = automatyczna pauza przy utracie terminala
        if (args.has("value")) {
            int val = args.get("value").toInt();
            if (val == 0 || val == 1) dgusLink.setLossPolicy((uint8_t)val);
            else result = "polityka 0 (kontynuuj) lub 1 (pauza)";
        } else {
            result = "brak parametru value";
        }
#endif
    } else {
        result = "nieznana akcja";
    }

    if (STATE_TRYLOCK(500)) { g_state.displayNeedsUpdate = true; STATE_UNLOCK(); }
    return makeResult(result);
}
