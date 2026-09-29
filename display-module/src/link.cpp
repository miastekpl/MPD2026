#include "link.h"
#include "app_config.h"
#include "settings_store.h"

#include <WiFi.h>
#include <HTTPClient.h>
#include <WebSocketsClient.h>
#include <HardwareSerial.h>
#include <ArduinoJson.h>
#include "serial_link_protocol.h"
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>

namespace {

struct Cmd { char body[192]; };

SemaphoreHandle_t g_mtx = nullptr;
QueueHandle_t     g_cmdQ = nullptr;

// ---------- Lacze przewodowe (RS-485), patrz docs/LACZE_PRZEWODOWE.md ----------
HardwareSerial       CableSerial(1);
seriallink::Parser   g_cableParser;
uint32_t             g_cableRxMs = 0;
uint32_t             g_cableHbMs = 0;
uint32_t             g_cableSeq = 0;

// Korelacja odpowiedzi na polecenie wyslane kablem (przez numer sekwencji)
volatile uint32_t    g_cableRespSeq = 0;
volatile bool        g_cableRespPending = false;
bool                 g_cableRespOk = false;
char                 g_cableRespMsg[64] = "";

bool cableLinkUp(uint32_t now) {
    return g_cableRxMs != 0 && (now - g_cableRxMs) < CABLE_STALE_MS;
}

void cableSendRaw(const char* payload, size_t len) {
    char frame[seriallink::MAX_PAYLOAD + 16];
    size_t n = seriallink::encodeFrame(payload, len, frame, sizeof(frame));
    if (n > 0) CableSerial.write((const uint8_t*)frame, n);
}

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

void cableHandleFrame(const char* payload, size_t len) {
    // Odpowiedz na polecenie: {"r":"...","seq":N}. Status sterownika (GET /api/status,
    // dokladnie ten sam parser co dla WiFi): wszystko inne (w tym heartbeat {"hb":1}).
    bool looksLikeResponse = (strstr(payload, "\"r\":") != nullptr) && (strstr(payload, "\"seq\":") != nullptr);
    if (looksLikeResponse) {
        JsonDocument doc;
        if (deserializeJson(doc, payload, len) == DeserializationError::Ok) {
            uint32_t seq = doc["seq"] | 0;
            const char* msg = doc["r"] | "";
            if (g_cableRespPending && seq == g_cableRespSeq) {
                g_cableRespOk = (strcmp(msg, "ok") == 0);
                strlcpy(g_cableRespMsg, msg, sizeof(g_cableRespMsg));
                g_cableRespPending = false;
            }
        }
        return;
    }
    Status tmp;
    if (parseStatus(payload, len, tmp)) storeStatus(tmp);
}

void cablePoll(uint32_t now) {
    while (CableSerial.available()) {
        uint8_t b = (uint8_t)CableSerial.read();
        if (g_cableParser.feed(b)) {
            g_cableRxMs = now;
            cableHandleFrame(g_cableParser.payload(), g_cableParser.payloadLen());
        }
    }
    if (now - g_cableHbMs >= CABLE_HEARTBEAT_MS) {
        g_cableHbMs = now;
        static const char HB[] = "{\"hb\":1}";
        cableSendRaw(HB, sizeof(HB) - 1);
    }
}

// Rozbiera "action=xxx&value=yyy" (format uzywany juz dziś przez linkSend()/httpPost)
// na para akcja+opcjonalna wartosc calkowita.
bool parseFormBody(const char* body, String& action, bool& hasValue, int& value) {
    String s(body);
    int ai = s.indexOf("action=");
    if (ai < 0) return false;
    ai += 7;
    int amp = s.indexOf('&', ai);
    action = (amp < 0) ? s.substring(ai) : s.substring(ai, amp);
    hasValue = false;
    value = 0;
    int vi = s.indexOf("value=");
    if (vi >= 0) {
        vi += 6;
        int amp2 = s.indexOf('&', vi);
        String vs = (amp2 < 0) ? s.substring(vi) : s.substring(vi, amp2);
        value = vs.toInt();
        hasValue = true;
    }
    return true;
}

// Wysyla polecenie kablem i czeka (nieblokujaco dla reszty systemu - tylko ten task)
// na odpowiedz po numerze sekwencji. STOP ponawiany agresywniej, jak w httpPost/processCommand.
void cableSendCommand(const Cmd& c) {
    String action;
    bool hasValue;
    int value;
    if (!parseFormBody(c.body, action, hasValue, value)) {
        setResult(false, "zly format polecenia");
        return;
    }
    bool isStop = (action == "stop");
    int attempts = isStop ? 3 : 1;
    for (int a = 0; a < attempts; a++) {
        uint32_t seq = ++g_cableSeq;
        JsonDocument doc;
        doc["a"] = action;
        if (hasValue) doc["v"] = value;
        doc["seq"] = seq;
        String json;
        serializeJson(doc, json);

        g_cableRespPending = true;
        g_cableRespSeq = seq;
        cableSendRaw(json.c_str(), json.length());

        uint32_t waitStart = millis();
        while (millis() - waitStart < 300) {
            cablePoll(millis());
            if (!g_cableRespPending) break;
            vTaskDelay(pdMS_TO_TICKS(5));
        }
        if (!g_cableRespPending) {
            setResult(g_cableRespOk, g_cableRespMsg);
            return;
        }
        g_cableRespPending = false;
    }
    setResult(false, isStop ? "STOP NIE DOTARL (kabel) - uzyj fizycznego STOP" : "brak odpowiedzi (kabel)");
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
        if (cableLinkUp(millis())) {
            cableSendCommand(c);
        } else {
            processCommand(c);
        }
    }
}

void netTask(void*) {
    CableSerial.begin(CABLE_BAUD, SERIAL_8N1, CABLE_RX_PIN, CABLE_TX_PIN);
    g_cableParser.reset();

    WiFi.persistent(false);
    WiFi.mode(WIFI_STA);
    WiFi.setSleep(false);
    WiFi.setAutoReconnect(true);

    uint32_t lastBeginMs = 0;
    uint32_t lastPollMs = 0;
    uint32_t lastStatsMs = 0;
    bool     wsStarted = false;

    for (;;) {
        cablePoll(millis());

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
    if (cableLinkUp(millis())) {
        lock();
        bool have = g_haveStatus;
        uint32_t age = millis() - g_rxMs;
        unlock();
        return (have && age < LINK_STALE_MS) ? LS_ONLINE : LS_NO_DATA;
    }
    if (g_settings.wifiPass[0] == 0) return LS_NO_PASSWORD;
    if (WiFi.status() != WL_CONNECTED) return LS_WIFI_CONNECTING;
    lock();
    bool have = g_haveStatus;
    uint32_t age = millis() - g_rxMs;
    unlock();
    return (have && age < LINK_STALE_MS) ? LS_ONLINE : LS_NO_DATA;
}

bool linkIsCable() { return cableLinkUp(millis()); }

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
