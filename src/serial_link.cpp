#include "sys_log.h"
// ============================================================
// MPD2026 - lacze przewodowe (RS-485) z modulem Sunton - patrz serial_link.h
// ============================================================

#include "serial_link.h"

SerialLink serialLink;

#if HAS_SERIAL_LINK

#include <driver/uart.h>
#include <esp_idf_version.h>
#include <ArduinoJson.h>
#include "serial_link_protocol.h"
#include "control_api.h"
#include "web_server.h"

namespace {

constexpr uart_port_t LINK_UART = UART_NUM_1;
constexpr int RX_BUF = 4096;
constexpr int TX_BUF = 4096;
constexpr int RX_BUDGET = 2048;   // maks. bajtow przetwarzanych w jednym update()

seriallink::Parser g_parser;
char g_frame[seriallink::MAX_PAYLOAD + 16];

void sendFrame(const char* payload, size_t len) {
    size_t n = seriallink::encodeFrame(payload, len, g_frame, sizeof(g_frame));
    if (n > 0) uart_write_bytes(LINK_UART, g_frame, n);
}

// Argumenty polecenia z ramki {"a":"...","v":...} - odpowiednik jednej wartosci
// "value", tak jak wszystkie akcje w control_api.cpp jej oczekuja (patrz np.
// DgusArgs w dgus_link.cpp - ten sam, sprawdzony wzorzec).
class FrameArgs : public ControlArgs {
public:
    FrameArgs(int v, bool has) : val(v), hasVal(has) {}
    bool has(const char* key) const override { return hasVal && strcmp(key, "value") == 0; }
    String get(const char* key) const override {
        return (hasVal && strcmp(key, "value") == 0) ? String(val) : String();
    }
private:
    int  val;
    bool hasVal;
};

}  // namespace

void SerialLink::begin() {
    uart_config_t cfg = {};
    cfg.baud_rate = SERIAL_LINK_BAUD;
    cfg.data_bits = UART_DATA_8_BITS;
    cfg.parity    = UART_PARITY_DISABLE;
    cfg.stop_bits = UART_STOP_BITS_1;
    cfg.flow_ctrl = UART_HW_FLOWCTRL_DISABLE;
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
    cfg.source_clk = UART_SCLK_DEFAULT;
#else
    cfg.source_clk = UART_SCLK_APB;
#endif

    if (uart_driver_install(LINK_UART, RX_BUF, TX_BUF, 0, nullptr, 0) != ESP_OK ||
        uart_param_config(LINK_UART, &cfg) != ESP_OK ||
        uart_set_pin(LINK_UART, PIN_SERIAL_LINK_TX, PIN_SERIAL_LINK_RX, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE) != ESP_OK) {
        DBG_PRINTLN("[LINK] BLAD: inicjalizacja UART1 (kabel do modulu Sunton) nie powiodla sie");
        return;
    }
    started = true;
    g_parser.reset();
    DBG_PRINTF("[LINK] Lacze kablowe: UART1 TX=%d RX=%d %u 8N1\n",
               PIN_SERIAL_LINK_TX, PIN_SERIAL_LINK_RX, (unsigned)SERIAL_LINK_BAUD);
}

uint32_t SerialLink::framesOk()  const { return g_parser.okFrames(); }
uint32_t SerialLink::framesBad() const { return g_parser.badFrames(); }

void SerialLink::handleFrame(const char* payload, size_t len) {
    JsonDocument doc;
    if (deserializeJson(doc, payload, len) != DeserializationError::Ok) return;
    const char* action = doc["a"] | "";
    if (!action[0]) return;

    bool hasVal = doc["v"].is<int>();
    int  val    = hasVal ? doc["v"].as<int>() : 0;
    long seq    = doc["seq"] | 0;

    FrameArgs args(val, hasVal);
    ControlResult r = executeControl(String(action), args);

    JsonDocument out;
    out["r"]   = r.message;
    out["seq"] = seq;
    String outStr;
    serializeJson(out, outStr);
    sendFrame(outStr.c_str(), outStr.length());
}

void SerialLink::sendStatus(uint32_t now) {
    STATE_LOCK();
    MachineState ms = g_state.machineState;
    STATE_UNLOCK();
    uint32_t period = (ms == STATE_PAINTING) ? SERIAL_LINK_STATUS_PAINT_MS : SERIAL_LINK_STATUS_IDLE_MS;
    if (now - lastStatusMs < period) return;
    lastStatusMs = now;

    // Dokladnie ten sam JSON co GET /api/status - modul Sunton parsuje go tym samym
    // parseStatus(), ktorego uzywa juz dla WiFi (zero zmian po stronie parsera).
    String json = webServer.statusJson();
    if (json.length() > seriallink::MAX_PAYLOAD) {
        DBG_PRINTF("[LINK] Status JSON za dlugi (%u > %u) - pomijam ramke\n",
                   (unsigned)json.length(), (unsigned)seriallink::MAX_PAYLOAD);
        return;
    }
    sendFrame(json.c_str(), json.length());
}

void SerialLink::update() {
    if (!started) return;
    uint32_t now = millis();

    uint8_t rx[128];
    int budget = RX_BUDGET;
    while (budget > 0) {
        int want = budget < (int)sizeof(rx) ? budget : (int)sizeof(rx);
        int n = uart_read_bytes(LINK_UART, rx, want, 0);
        if (n <= 0) break;
        budget -= n;
        for (int i = 0; i < n; i++) {
            if (g_parser.feed(rx[i])) {
                lastRxMs = now;
                handleFrame(g_parser.payload(), g_parser.payloadLen());
            }
        }
    }

    linkUp = (lastRxMs != 0) && ((uint32_t)(now - lastRxMs) < SERIAL_LINK_LOSS_MS);

    sendStatus(now);
}

#endif  // HAS_SERIAL_LINK
