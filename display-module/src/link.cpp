#include "link.h"
#include "app_config.h"
#include "settings_store.h"

#include <WiFi.h>
#include <HTTPClient.h>
#include <WebSocketsClient.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>

namespace {

struct Cmd { char body[192]; };

SemaphoreHandle_t g_mtx = nullptr;
QueueHandle_t     g_cmdQ = nullptr;

Status            g_status;
uint32_t          g_rxMs = 0;
bool              g_haveStatus = false;

StatsData         g_stats;
volatile bool     g_wantStats = false;

SlotCfg           g_slot;
volatile int      g_slotReq = -1;
volatile int      g_slotReady = -1;

volatile uint32_t g_resultSeq = 0;
volatile bool     g_resultOk = true;
char              g_resultMsg[64] = "";

volatile bool     g_reconnect = false;
volatile bool     g_wsConnected = false;
WebSocketsClient  g_ws;

inline void lock()   { xSemaphoreTake(g_mtx, portMAX_DELAY); }
inline void unlock() { xSemaphoreGive(g_mtx); }

void storeStatus(const Status& s) {
    lock();
    g_status = s;
    g_rxMs = millis();
    g_haveStatus = true;
    unlock();
}

void setResult(bool ok, const char* msg) {
    lock();
    g_resultOk = ok;
    strlcpy(g_resultMsg, msg ? msg : "", sizeof(g_resultMsg));
    g_resultSeq = g_resultSeq + 1;
    unlock();
}

void onWsEvent(WStype_t type, uint8_t* payload, size_t length) {
    switch (type) {
        case WStype_CONNECTED:
            g_wsConnected = true;
            break;
        case WStype_DISCONNECTED:
            g_wsConnected = false;
            break;
        case WStype_TEXT: {
            Status tmp;
            if (parseStatus((const char*)payload, length, tmp)) storeStatus(tmp);
            break;
        }
        default:
            break;
    }
}

// Wyciąga wartość pola "result" z odpowiedzi {"result":"..."}; zwraca false gdy brak.
bool extractResult(const String& resp, char* out, size_t n) {
    int i = resp.indexOf("\"result\":\"");
    if (i < 0) {
        int e = resp.indexOf("\"error\":\"");
        if (e < 0) return false;
        i = e + 9;
    } else {
        i += 10;
    }
    int j = resp.indexOf('"', i);
    if (j < 0) return false;
    String v = resp.substring(i, j);
    strlcpy(out, v.c_str(), n);
    return true;
}

bool httpPost(const char* body, String& resp, int& code) {
    WiFiClient wc;
    HTTPClient http;
    http.setConnectTimeout(900);
    http.setTimeout(1800);
    if (!http.begin(wc, CTRL_HOST, CTRL_HTTP_PORT, "/api/control")) return false;
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    code = http.POST((uint8_t*)body, strlen(body));
    if (code > 0) resp = http.getString();
    http.end();
    return code > 0;
}

bool httpGet(const char* path, String& resp) {
    WiFiClient wc;
    HTTPClient http;
    http.setConnectTimeout(700);
    http.setTimeout(1200);
    if (!http.begin(wc, CTRL_HOST, CTRL_HTTP_PORT, path)) return false;
    int code = http.GET();
    bool ok = (code == 200);
    if (ok) resp = http.getString();
    http.end();
    return ok;
}

void processCommand(const Cmd& c) {
    bool isStop = strstr(c.body, "action=stop") != nullptr;
    int attempts = isStop ? 4 : 2;   // STOP powtarzany agresywniej
    for (int a = 0; a < attempts; a++) {
        String resp;
        int code = 0;
        if (httpPost(c.body, resp, code)) {
            char msg[64];
            if (code == 200 && extractResult(resp, msg, sizeof(msg))) {
                setResult(strcmp(msg, "ok") == 0, msg);
            } else if (code == 503) {
                if (a + 1 < attempts) { vTaskDelay(pdMS_TO_TICKS(120)); continue; }
                setResult(false, "sterownik zajety");
            } else {
                setResult(false, extractResult(resp, msg, sizeof(msg)) ? msg : "blad polecenia");
            }
            return;
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    setResult(false, isStop ? "STOP NIE DOTARL - uzyj fizycznego STOP" : "brak odpowiedzi sterownika");
}

void drainCommands() {
    Cmd c;
    while (xQueueReceive(g_cmdQ, &c, 0) == pdTRUE) {
        processCommand(c);
    }
}

void netTask(void*) {
    WiFi.persistent(false);
    WiFi.mode(WIFI_STA);
    WiFi.setSleep(false);
    WiFi.setAutoReconnect(true);

    uint32_t lastBeginMs = 0;
    uint32_t lastPollMs = 0;
    uint32_t lastStatsMs = 0;
    bool     wsStarted = false;

    for (;;) {
        bool haveCreds = g_settings.wifiPass[0] != 0;

        if (g_reconnect) {
            g_reconnect = false;
            WiFi.disconnect();
            lastBeginMs = 0;
        }

        wl_status_t st = WiFi.status();
        uint32_t now = millis();

        if (st != WL_CONNECTED) {
            if (wsStarted) { g_ws.disconnect(); wsStarted = false; g_wsConnected = false; }
            if (haveCreds && (lastBeginMs == 0 || now - lastBeginMs > 8000)) {
                WiFi.disconnect();
                WiFi.begin((const char*)CTRL_SSID, (const char*)g_settings.wifiPass);
                lastBeginMs = now ? now : 1;
            }
            vTaskDelay(pdMS_TO_TICKS(200));
            continue;
        }

        if (!wsStarted) {
            g_ws.begin(CTRL_HOST, CTRL_WS_PORT, "/");
            g_ws.onEvent(onWsEvent);
            g_ws.setReconnectInterval(2000);
            g_ws.enableHeartbeat(4000, 2000, 2);
            wsStarted = true;
        }
        g_ws.loop();

        drainCommands();

        // Awaryjny polling HTTP, gdy WebSocket nie dostarcza ramek
        lock();
        uint32_t age = g_haveStatus ? (now - g_rxMs) : 0xFFFFFFFFu;
        unlock();
        if (age > 1500 && now - lastPollMs > 700) {
            lastPollMs = now;
            String resp;
            if (httpGet("/api/status", resp)) {
                Status tmp;
                if (parseStatus(resp.c_str(), resp.length(), tmp)) storeStatus(tmp);
            }
        }

        if (g_wantStats && now - lastStatsMs > 1500) {
            lastStatsMs = now;
            String resp;
            if (httpGet("/api/stats", resp)) {
                StatsData tmp;
                if (parseStats(resp.c_str(), resp.length(), tmp)) {
                    lock();
                    g_stats = tmp;
                    unlock();
                }
            }
        }

        int req = g_slotReq;
        if (req >= 0) {
            char body[64];
            snprintf(body, sizeof(body), "action=get_slot_config&slot=%d", req);
            String resp;
            int code = 0;
            if (httpPost(body, resp, code) && code == 200) {
                SlotCfg tmp;
                if (parseSlotConfig(resp.c_str(), resp.length(), tmp)) {
                    lock();
                    g_slot = tmp;
                    g_slotReady = req;
                    unlock();
                }
            }
            g_slotReq = -1;
        }

        vTaskDelay(pdMS_TO_TICKS(4));
    }
}

}  // namespace

void linkBegin() {
    g_mtx = xSemaphoreCreateMutex();
    g_cmdQ = xQueueCreate(8, sizeof(Cmd));
    xTaskCreatePinnedToCore(netTask, "net", 10240, nullptr, 2, nullptr, 0);
}

void linkSetPassword(const char* pass) {
    strlcpy(g_settings.wifiPass, pass ? pass : "", sizeof(g_settings.wifiPass));
    g_settings.save();
    g_reconnect = true;
}

LinkState linkState() {
    if (g_settings.wifiPass[0] == 0) return LS_NO_PASSWORD;
    if (WiFi.status() != WL_CONNECTED) return LS_WIFI_CONNECTING;
    lock();
    bool have = g_haveStatus;
    uint32_t age = millis() - g_rxMs;
    unlock();
    return (have && age < LINK_STALE_MS) ? LS_ONLINE : LS_NO_DATA;
}

int linkRssi() {
    return WiFi.status() == WL_CONNECTED ? WiFi.RSSI() : -127;
}

bool linkGetStatus(Status& out, uint32_t* ageMs) {
    lock();
    bool have = g_haveStatus;
    if (have) {
        out = g_status;
        if (ageMs) *ageMs = millis() - g_rxMs;
    }
    unlock();
    return have;
}

void linkWantStats(bool on) { g_wantStats = on; }

bool linkGetStats(StatsData& out) {
    lock();
    bool ok = g_stats.valid;
    if (ok) out = g_stats;
    unlock();
    return ok;
}

void linkRequestSlot(int slot) {
    if (slot < 0 || slot >= NSLOTS) return;
    g_slotReady = -1;
    g_slotReq = slot;
}

bool linkGetSlot(int slot, SlotCfg& out) {
    bool ok = false;
    lock();
    if (g_slotReady == slot) { out = g_slot; ok = true; }
    unlock();
    return ok;
}

bool linkSend(const char* formBody, bool urgent) {
    if (!g_cmdQ || !formBody) return false;
    Cmd c;
    strlcpy(c.body, formBody, sizeof(c.body));
    BaseType_t r = urgent ? xQueueSendToFront(g_cmdQ, &c, 0) : xQueueSend(g_cmdQ, &c, 0);
    if (r != pdTRUE && urgent) {
        // Kolejka pełna: STOP ma pierwszeństwo - zwolnij najstarsze polecenie
        Cmd drop;
        xQueueReceive(g_cmdQ, &drop, 0);
        r = xQueueSendToFront(g_cmdQ, &c, 0);
    }
    return r == pdTRUE;
}

uint32_t linkResultSeq() { return g_resultSeq; }
bool     linkResultOk()  { return g_resultOk; }

void linkResultMsg(char* buf, size_t n) {
    lock();
    strlcpy(buf, g_resultMsg, n);
    unlock();
}
