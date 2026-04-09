/*
 * ============================================================
 *  ESP32 - MCP CHATBOT XIAOZHI - v4.0 (final)
 * ============================================================
 *  Thay đổi so với v3.0:
 *   [🔴 Fix v4] warningTemp → volatile float: bị ghi từ lambda MCP (Core 0)
 *               qua set_temperature_warning và đọc từ loop()/updateOLED() (Core 1)
 *               → cùng pattern race condition như temperature/humidity.
 *   [🟠 Fix v4] set_temperature_warning: thêm validation dải 20–60°C.
 *               Xiaozhi truyền giá trị ngoài dải nhận lỗi rõ ràng,
 *               tránh lưu giá trị vô nghĩa vào NVS.
 *   [🔴 Fix v3] Xóa portMUX / portENTER_CRITICAL — sai context (non-ISR),
 *               có thể gây watchdog reset. Thay bằng volatile float (đủ vì
 *               đọc/ghi 32-bit là atomic tự nhiên trên ESP32 Xtensa LX6).
 *   [🟠 Fix v3] stateFan/stateLight* → volatile bool (đảm bảo compiler
 *               không cache vào register khi đọc xuyên core).
 *   [🟡 Fix v3] Xóa readSensorSafe() gọi thừa mỗi 10ms trong loop().
 *               Thêm cachedTemp cập nhật cùng chu kỳ đọc DHT (2s).
 * ------------------------------------------------------------
 *  Thay đổi từ v1.0 → v2.0 (giữ nguyên):
 *   [🔴 Fix] GPIO 2 → GPIO 25 (tránh xung đột boot ESP32)
 *   [🔴 Fix] Schema Tool 1: xóa required input (tool chỉ trả output)
 *   [🟠 Fix] Thêm validation JSON args cho tất cả tool control
 *   [🟡 Fix] DynamicJsonDocument → StaticJsonDocument
 *   [🟡 Fix] Chu kỳ DHT 1000ms → 2000ms
 *   [🟡 Fix] ToolResponse dùng StaticJsonDocument thay String concat
 *   [➕ Thêm] Timeout WiFi 15s trong setup()
 *   [➕ Thêm] Hiển thị IP lên OLED lúc khởi động
 *   [➕ Thêm] Lưu/nạp warningTemp từ NVS (Preferences)
 *   [➕ Thêm] Tool get_device_status
 * ============================================================
 *  Sơ đồ chân GPIO:
 *   GPIO 23 → DHT11 DATA
 *   GPIO 21 → OLED SDA (I2C mặc định ESP32)
 *   GPIO 22 → OLED SCL (I2C mặc định ESP32)
 *   GPIO  5 → LED cảnh báo quá nhiệt
 *   GPIO 25 → Relay Quạt          ← đổi từ GPIO 2
 *   GPIO  4 → Relay Đèn phòng khách
 *   GPIO 18 → Relay Đèn phòng ngủ
 *   GPIO 19 → Relay Đèn phòng bếp
 * ============================================================
 *  Thư viện cần cài:
 *   - WebSocketMCP (đi kèm dự án)
 *   - DHT sensor library (Adafruit)
 *   - Adafruit GFX Library
 *   - Adafruit SSD1306
 *   - ArduinoJson
 *   - Preferences (có sẵn trong ESP32 Arduino core)
 * ============================================================
 */

#include "WebSocketMCP.h"
#include <WiFi.h>
#include <DHT.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Preferences.h>

// ============================================================
// KHAI BÁO CHÂN GPIO
// ============================================================
#define DHTPIN        23    // DHT11 DATA
#define DHTTYPE       DHT11
#define WARNING_LED    5    // LED nhấp nháy khi quá nhiệt

#define FAN           25    // Relay Quạt (đổi từ GPIO 2 — tránh xung đột boot)
#define LIGHT1         4    // Relay Đèn phòng khách
#define LIGHT2        18    // Relay Đèn phòng ngủ
#define LIGHT3        19    // Relay Đèn phòng bếp

// OLED I2C: SDA=21, SCL=22 (mặc định ESP32, không cần khai báo thêm)
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT  64

// ============================================================
// KHỞI TẠO ĐỐI TƯỢNG
// ============================================================
DHT dht(DHTPIN, DHTTYPE);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
WebSocketMCP mcpClient;
Preferences prefs;

// ============================================================
// CẤU HÌNH WiFi & MCP
// ============================================================
const char* ssid        = "your-ssid";       // Thay bằng tên Wi-Fi của bạn
const char* password    = "your-password";   // Thay bằng mật khẩu Wi-Fi
const char* mcpEndpoint = "wss://api.xiaozhi.me/mcp/?token=xxxx"; // Token từ Xiaozhi

#define WIFI_TIMEOUT_MS 15000   // Timeout kết nối WiFi (15 giây)

// ============================================================
// BIẾN TOÀN CỤC — dual-core safe
// ============================================================
// [v3 Fix1] volatile float là đủ trên ESP32 Xtensa LX6 (32-bit architecture):
//   đọc/ghi float 4-byte là atomic tự nhiên → không cần portMUX hay FreeRTOS mutex.
//   portENTER_CRITICAL tắt interrupt toàn bộ — dùng sai context (non-ISR) có thể
//   gây watchdog reset nếu WebSocket callback giữ lock quá lâu.
volatile float temperature = 0;
volatile float humidity    = 0;
// [v4 Fix] volatile: warningTemp được GHI từ lambda MCP (Core 0) qua set_temperature_warning
//          và ĐỌC từ loop() + updateOLED() (Core 1) → cần volatile như temperature/humidity
volatile float warningTemp = 30.0;

// [v3 Fix2] volatile bool — bool là 1 byte, atomic trên ESP32, nhưng volatile
//   đảm bảo compiler không cache vào register khi đọc xuyên core.
volatile bool stateFan    = false;
volatile bool stateLight1 = false;
volatile bool stateLight2 = false;
volatile bool stateLight3 = false;

// Biến thời gian
unsigned long lastSensorRead = 0;
unsigned long lastBlink      = 0;
bool ledState    = false;
float cachedTemp = 0;  // [v3 Fix3] cache nhiệt độ để blink LED, tránh đọc lặp mỗi 10ms

// ============================================================
// HÀM TIỆN ÍCH: Tạo ToolResponse bằng JSON (không dùng String concat)
// ============================================================
String makeResponse(bool success, const char* key, const char* value) {
    StaticJsonDocument<128> doc;
    doc["success"] = success;
    if (key && value) doc[key] = value;
    String json;
    serializeJson(doc, json);
    return json;
}

String makeResponseFloat(bool success, const char* key, float value) {
    StaticJsonDocument<128> doc;
    doc["success"] = success;
    if (key) doc[key] = value;
    String json;
    serializeJson(doc, json);
    return json;
}

// ============================================================
// CALLBACK TRẠNG THÁI KẾT NỐI MCP
// ============================================================
void onConnectionStatus(bool connected) {
    if (connected) {
        Serial.println("[MCP] ✅ Đã kết nối tới máy chủ");
        registerMcpTools();
    } else {
        Serial.println("[MCP] ⚠️ Mất kết nối với máy chủ MCP");
    }
}

// ============================================================
// ĐĂNG KÝ MCP TOOLS
// ============================================================
void registerMcpTools() {

    // --- Tool 1: Đọc nhiệt độ & độ ẩm ---
    // [Fix] Schema input rỗng — tool này chỉ trả output, không nhận tham số từ AI
    mcpClient.registerTool(
        "temperature_and_humidity_values",
        "Báo giá trị nhiệt độ và độ ẩm hiện tại từ cảm biến DHT11",
        R"({
            "type": "object",
            "properties": {}
        })",
        [](const String& args) {
            // [v3 Fix1] đọc trực tiếp volatile — atomic trên ESP32 32-bit, không cần mutex
            float t = temperature;
            float h = humidity;

            Serial.println("\n=== Xiaozhi yêu cầu giá trị cảm biến ===");
            Serial.print("Nhiệt độ: "); Serial.print(t); Serial.println(" °C");
            Serial.print("Độ ẩm: ");    Serial.print(h); Serial.println(" %");

            StaticJsonDocument<128> doc;
            doc["temperature"] = t;
            doc["humidity"]    = h;
            String json;
            serializeJson(doc, json);
            return WebSocketMCP::ToolResponse(json);
        }
    );
    Serial.println("[MCP] 🛠️ Đã đăng ký tool: temperature_and_humidity_values");

    // --- Tool 2: Cài ngưỡng cảnh báo nhiệt độ ---
    mcpClient.registerTool(
        "set_temperature_warning",
        "Cài đặt nhiệt độ ngưỡng cảnh báo (°C). Khi vượt ngưỡng, LED sẽ nhấp nháy.",
        R"({
            "type": "object",
            "properties": {
                "max_temperature": { "type": "number" }
            },
            "required": ["max_temperature"]
        })",
        [](const String& args) {
            StaticJsonDocument<64> doc;  // [Fix] Static
            auto err = deserializeJson(doc, args);

            // [Fix] Validation: kiểm tra JSON hợp lệ và có key cần thiết
            if (err || !doc.containsKey("max_temperature")) {
                return WebSocketMCP::ToolResponse(
                    makeResponse(false, "error", "invalid args")
                );
            }

            float newTemp = doc["max_temperature"].as<float>();

            // [v4 Fix] Giới hạn dải hợp lệ cho môi trường nhà ở (20–60°C)
            // Ngoài dải này Xiaozhi nhận lỗi rõ ràng, tránh lưu giá trị vô nghĩa vào NVS
            if (newTemp < 20.0 || newTemp > 60.0) {
                return WebSocketMCP::ToolResponse(
                    makeResponse(false, "error", "out of range: 20-60 C")
                );
            }

            warningTemp = newTemp;
            prefs.putFloat("warnTemp", warningTemp);  // [Thêm] Lưu vào NVS

            Serial.print("[MCP] Ngưỡng cảnh báo mới: ");
            Serial.print(warningTemp); Serial.println(" °C (đã lưu NVS)");

            return WebSocketMCP::ToolResponse(
                makeResponseFloat(true, "new_limit", warningTemp)
            );
        }
    );
    Serial.println("[MCP] 🛠️ Đã đăng ký tool: set_temperature_warning");

    // --- Tool 3: Điều khiển quạt ---
    mcpClient.registerTool(
        "fan_control",
        "Bật hoặc tắt quạt",
        R"({
            "type": "object",
            "properties": {
                "state": { "type": "string", "enum": ["on", "off"] }
            },
            "required": ["state"]
        })",
        [](const String& args) {
            StaticJsonDocument<64> doc;
            auto err = deserializeJson(doc, args);

            // [Fix] Validation
            if (err || !doc.containsKey("state")) {
                return WebSocketMCP::ToolResponse(
                    makeResponse(false, "error", "invalid args")
                );
            }

            String state = doc["state"].as<String>();
            if (state == "on") { digitalWrite(FAN, HIGH); stateFan = true; }
            else               { digitalWrite(FAN, LOW);  stateFan = false; }

            Serial.println("[MCP] Quạt: " + state);
            return WebSocketMCP::ToolResponse(makeResponse(true, "state", state.c_str()));
        }
    );
    Serial.println("[MCP] 🛠️ Đã đăng ký tool: fan_control");

    // --- Tool 4: Điều khiển đèn phòng khách ---
    mcpClient.registerTool(
        "living_room_lights_control",
        "Bật hoặc tắt đèn phòng khách",
        R"({
            "type": "object",
            "properties": {
                "state": { "type": "string", "enum": ["on", "off"] }
            },
            "required": ["state"]
        })",
        [](const String& args) {
            StaticJsonDocument<64> doc;
            auto err = deserializeJson(doc, args);
            if (err || !doc.containsKey("state")) {
                return WebSocketMCP::ToolResponse(makeResponse(false, "error", "invalid args"));
            }
            String state = doc["state"].as<String>();
            if (state == "on") { digitalWrite(LIGHT1, HIGH); stateLight1 = true; }
            else               { digitalWrite(LIGHT1, LOW);  stateLight1 = false; }

            Serial.println("[MCP] Đèn phòng khách: " + state);
            return WebSocketMCP::ToolResponse(makeResponse(true, "state", state.c_str()));
        }
    );
    Serial.println("[MCP] 🛠️ Đã đăng ký tool: living_room_lights_control");

    // --- Tool 5: Điều khiển đèn phòng ngủ ---
    mcpClient.registerTool(
        "bedroom_lights_control",
        "Bật hoặc tắt đèn phòng ngủ",
        R"({
            "type": "object",
            "properties": {
                "state": { "type": "string", "enum": ["on", "off"] }
            },
            "required": ["state"]
        })",
        [](const String& args) {
            StaticJsonDocument<64> doc;
            auto err = deserializeJson(doc, args);
            if (err || !doc.containsKey("state")) {
                return WebSocketMCP::ToolResponse(makeResponse(false, "error", "invalid args"));
            }
            String state = doc["state"].as<String>();
            if (state == "on") { digitalWrite(LIGHT2, HIGH); stateLight2 = true; }
            else               { digitalWrite(LIGHT2, LOW);  stateLight2 = false; }

            Serial.println("[MCP] Đèn phòng ngủ: " + state);
            return WebSocketMCP::ToolResponse(makeResponse(true, "state", state.c_str()));
        }
    );
    Serial.println("[MCP] 🛠️ Đã đăng ký tool: bedroom_lights_control");

    // --- Tool 6: Điều khiển đèn phòng bếp ---
    mcpClient.registerTool(
        "kitchen_lights_control",
        "Bật hoặc tắt đèn phòng bếp",
        R"({
            "type": "object",
            "properties": {
                "state": { "type": "string", "enum": ["on", "off"] }
            },
            "required": ["state"]
        })",
        [](const String& args) {
            StaticJsonDocument<64> doc;
            auto err = deserializeJson(doc, args);
            if (err || !doc.containsKey("state")) {
                return WebSocketMCP::ToolResponse(makeResponse(false, "error", "invalid args"));
            }
            String state = doc["state"].as<String>();
            if (state == "on") { digitalWrite(LIGHT3, HIGH); stateLight3 = true; }
            else               { digitalWrite(LIGHT3, LOW);  stateLight3 = false; }

            Serial.println("[MCP] Đèn phòng bếp: " + state);
            return WebSocketMCP::ToolResponse(makeResponse(true, "state", state.c_str()));
        }
    );
    Serial.println("[MCP] 🛠️ Đã đăng ký tool: kitchen_lights_control");

    // --- Tool 7: Điều khiển tất cả thiết bị ---
    mcpClient.registerTool(
        "all_devices_control",
        "Bật hoặc tắt tất cả thiết bị cùng lúc (quạt + 3 đèn)",
        R"({
            "type": "object",
            "properties": {
                "state": { "type": "string", "enum": ["on", "off"] }
            },
            "required": ["state"]
        })",
        [](const String& args) {
            StaticJsonDocument<64> doc;
            auto err = deserializeJson(doc, args);
            if (err || !doc.containsKey("state")) {
                return WebSocketMCP::ToolResponse(makeResponse(false, "error", "invalid args"));
            }
            String state = doc["state"].as<String>();
            bool on = (state == "on");

            digitalWrite(FAN,    on ? HIGH : LOW);
            digitalWrite(LIGHT1, on ? HIGH : LOW);
            digitalWrite(LIGHT2, on ? HIGH : LOW);
            digitalWrite(LIGHT3, on ? HIGH : LOW);
            stateFan = stateLight1 = stateLight2 = stateLight3 = on;

            Serial.println("[MCP] Tất cả thiết bị: " + state);
            return WebSocketMCP::ToolResponse(makeResponse(true, "state", state.c_str()));
        }
    );
    Serial.println("[MCP] 🛠️ Đã đăng ký tool: all_devices_control");

    // --- Tool 8: Truy vấn trạng thái thiết bị ---
    // [Thêm mới] Xiaozhi hỏi thiết bị nào đang bật/tắt
    mcpClient.registerTool(
        "get_device_status",
        "Truy vấn trạng thái hiện tại của tất cả thiết bị",
        R"({
            "type": "object",
            "properties": {}
        })",
        [](const String& args) {
            StaticJsonDocument<192> doc;
            doc["fan"]          = stateFan    ? "on" : "off";
            doc["living_room"]  = stateLight1 ? "on" : "off";
            doc["bedroom"]      = stateLight2 ? "on" : "off";
            doc["kitchen"]      = stateLight3 ? "on" : "off";
            String json;
            serializeJson(doc, json);
            Serial.println("[MCP] Trả về trạng thái thiết bị: " + json);
            return WebSocketMCP::ToolResponse(json);
        }
    );
    Serial.println("[MCP] 🛠️ Đã đăng ký tool: get_device_status");
}

// ============================================================
// CẬP NHẬT HIỂN THỊ OLED
// ============================================================
void updateOLED() {
    // [v3 Fix1] đọc trực tiếp volatile float — atomic trên ESP32
    float t = temperature;
    float h = humidity;

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    display.setCursor(0, 0);
    display.print("Nhiet do: ");
    display.print(t, 1);
    display.println(" C");

    display.setCursor(0, 16);
    display.print("Do am   : ");
    display.print(h, 1);
    display.println(" %");

    display.setCursor(0, 32);
    display.print("Canh bao: ");
    display.print(warningTemp, 1);
    display.println(" C");

    display.setCursor(0, 48);
    if (t > warningTemp) {
        display.print("!!! QUA NHIET !!!");
    } else {
        display.print("Trang thai OK");
    }

    display.display();
}

// ============================================================
// HIỂN THỊ THÔNG TIN KHỞI ĐỘNG LÊN OLED
// ============================================================
void showBootScreen(const String& ip) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    display.setCursor(0, 0);
    display.println("ESP32 MCP v4.0");

    display.setCursor(0, 16);
    display.println("WiFi: OK");

    display.setCursor(0, 32);
    display.println("IP:");
    display.setCursor(0, 44);
    display.println(ip);

    display.display();
    delay(3000);  // Hiển thị 3 giây rồi chuyển sang màn hình cảm biến
}

// ============================================================
// SETUP
// ============================================================
void setup() {
    Serial.begin(115200);

    // Khởi tạo chân OUTPUT
    pinMode(WARNING_LED, OUTPUT);
    pinMode(FAN,         OUTPUT);
    pinMode(LIGHT1,      OUTPUT);
    pinMode(LIGHT2,      OUTPUT);
    pinMode(LIGHT3,      OUTPUT);

    // Tắt tất cả thiết bị khi khởi động
    digitalWrite(WARNING_LED, LOW);
    digitalWrite(FAN,         LOW);
    digitalWrite(LIGHT1,      LOW);
    digitalWrite(LIGHT2,      LOW);
    digitalWrite(LIGHT3,      LOW);

    // Khởi tạo DHT11
    dht.begin();

    // Khởi tạo OLED
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println("[OLED] Khởi tạo thất bại!");
        while (1);
    }
    display.clearDisplay();
    display.display();
    Serial.println("[OLED] ✅ Khởi tạo thành công");

    // [Thêm] Nạp warningTemp từ NVS (giữ giá trị sau reset nguồn)
    prefs.begin("smarthome", false);
    warningTemp = prefs.getFloat("warnTemp", 30.0);
    Serial.print("[NVS] Ngưỡng cảnh báo nạp từ NVS: ");
    Serial.print(warningTemp); Serial.println(" °C");

    // [Fix] Kết nối WiFi có timeout — không treo vô hạn
    Serial.print("Đang kết nối Wi-Fi: ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);

    unsigned long wifiStart = millis();
    while (WiFi.status() != WL_CONNECTED) {
        if (millis() - wifiStart > WIFI_TIMEOUT_MS) {
            Serial.println("\n[WiFi] ❌ Timeout kết nối WiFi! Khởi động lại...");
            ESP.restart();
        }
        delay(200);
        Serial.print(".");
    }
    Serial.println("\n✅ Wi-Fi đã kết nối");
    Serial.println("IP thiết bị: " + WiFi.localIP().toString());

    // [Thêm] Hiển thị IP lên OLED khi khởi động
    showBootScreen(WiFi.localIP().toString());

    // Kết nối MCP
    mcpClient.begin(mcpEndpoint, onConnectionStatus);
}

// ============================================================
// LOOP
// ============================================================
void loop() {
    mcpClient.loop();

    unsigned long now = millis();

    // [Fix v2] Đọc cảm biến mỗi 2000ms (tăng từ 1000ms để DHT11 ổn định hơn)
    if (now - lastSensorRead >= 2000) {
        lastSensorRead = now;

        float t = dht.readTemperature();
        float h = dht.readHumidity();

        if (!isnan(t) && !isnan(h)) {
            // [v3 Fix1] ghi trực tiếp volatile — atomic trên ESP32 32-bit
            temperature = t;
            humidity    = h;
            // [v3 Fix3] cache lại để dùng cho blink LED, không cần đọc lại mỗi 10ms
            cachedTemp  = t;
        } else {
            Serial.println("[DHT] Lỗi đọc cảm biến!");
        }

        updateOLED();
    }

    // [v3 Fix3] Dùng cachedTemp (cập nhật mỗi 2s) thay vì đọc volatile mỗi 10ms
    if (cachedTemp > warningTemp) {
        if (now - lastBlink >= 300) {
            lastBlink = now;
            ledState  = !ledState;
            digitalWrite(WARNING_LED, ledState);
        }
    } else {
        ledState = false;
        digitalWrite(WARNING_LED, LOW);
    }

    delay(10);
}
