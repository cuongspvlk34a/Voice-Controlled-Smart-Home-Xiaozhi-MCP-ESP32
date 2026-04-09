# 🏠 Nhà Thông Minh Điều Khiển Bằng Giọng Nói — Xiaozhi + MCP + ESP32

# 🏠 語音控制智慧家居 — Xiaozhi + MCP + ESP32

<p align="center">
  <img src="https://img.shields.io/badge/Platform-ESP32-blue?logo=espressif" />
  <img src="https://img.shields.io/badge/AI-Xiaozhi%20%7C%20Qwen%2FDeepSeek-orange" />
  <img src="https://img.shields.io/badge/Protocol-MCP%20WebSocket-green" />
  <img src="https://img.shields.io/badge/License-MIT-lightgrey" />
  <img src="https://img.shields.io/badge/Version-v4.0-brightgreen" />
</p>

-----

## 📖 Giới thiệu / 專案介紹

**Tiếng Việt:**  
Dự án biến ngôi nhà của bạn thành nhà thông minh chỉ bằng giọng nói — không cần chạm tay vào bất kỳ nút bấm nào. Bạn nói “Bật quạt”, AI Xiaozhi sẽ hiểu và ra lệnh cho ESP32 bật relay quạt trong tích tắc.

Hệ thống gồm 2 thiết bị phần cứng: **Xiaozhi ESP32-S3** (bộ phận AI giọng nói) và **ESP32 WROOM-32** (bộ điều khiển thiết bị nhà). Giao tiếp giữa hai thiết bị thông qua giao thức **MCP (Model Context Protocol)** qua Internet — không cần Home Assistant, không cần server riêng.

**繁體中文：**  
本專案讓您的家透過聲音變成智慧家居——不需要觸碰任何按鈕。您說「開電風扇」，AI Xiaozhi 就會理解並在瞬間指揮 ESP32 閉合電風扇的繼電器。

系統由兩個硬體裝置組成：**Xiaozhi ESP32-S3**（語音 AI 模組）與 **ESP32 WROOM-32**（家電控制器）。兩裝置透過網際網路上的 **MCP（模型情境協定）** 通訊——無需 Home Assistant，無需自架伺服器。

-----

## 🏗️ Kiến trúc hệ thống / 系統架構

```
[Người dùng nói / 使用者說話]
         │
         ▼
[Xiaozhi ESP32-S3]          ← Phần cứng 1 / 硬體一：語音 AI Bot
  - ESP32-S3 (16MB Flash, 8MB PSRAM)
  - Màn hình TFT ST7789 2.8" (240×320) / TFT 顯示螢幕
  - Mic + Loa / 麥克風 + 喇叭
         │
         │ WebSocket (WSS) — OPUS audio
         ▼
[Xiaozhi Cloud Server]      ← Tầng AI đám mây / 雲端 AI 層
  - ASR (nhận dạng giọng nói / 語音辨識)
  - LLM (Qwen / DeepSeek — phân tích ý định / 意圖分析)
  - TTS (tổng hợp giọng nói / 語音合成)
  - MCP Server trung gian / MCP 中介伺服器
         │
         │ WebSocket MCP (wss://api.xiaozhi.me/mcp/?token=...)
         ▼
[ESP32 WROOM-32]            ← Phần cứng 2 / 硬體二：家電控制器
  - Đăng ký 8 MCP Tools / 註冊 8 個 MCP 工具
  - Điều khiển GPIO vật lý / 實體 GPIO 控制
         │
         ├── DHT11         → Nhiệt độ & Độ ẩm / 溫度與濕度
         ├── OLED SSD1306  → Hiển thị thông tin / 資訊顯示
         ├── LED cảnh báo  → Nhấp nháy quá nhiệt / 過熱閃爍警告
         ├── Relay Quạt    → GPIO 25 / 繼電器：電風扇
         ├── Relay Đèn PK  → GPIO 4  / 繼電器：客廳燈
         ├── Relay Đèn PN  → GPIO 18 / 繼電器：臥室燈
         └── Relay Đèn PB  → GPIO 19 / 繼電器：廚房燈
```

### Luồng lệnh thực tế / 實際指令流程

```
1. Nói "Bật quạt" / 說「開電風扇」
2. Audio → Xiaozhi Cloud → LLM gọi tool fan_control {"state":"on"}
          音訊 → 雲端 → LLM 呼叫工具
3. MCP Server chuyển tiếp đến ESP32 / MCP 伺服器轉發至 ESP32
4. ESP32 kéo GPIO 25 HIGH → Relay đóng → Quạt bật
          ESP32 拉高 GPIO 25 → 繼電器閉合 → 風扇啟動
5. ESP32 trả về {"success":true,"state":"on"}
6. Xiaozhi phản hồi bằng giọng "Đã bật quạt" / Xiaozhi 語音回應「電風扇已開啟」
```

-----

## ✨ Tính năng nổi bật / 主要功能特色

|# |Tính năng / 功能                                   |Mô tả / 說明                                                |
|--|-------------------------------------------------|----------------------------------------------------------|
|1 |🎤 Điều khiển 8 thiết bị bằng giọng nói / 8 設備語音控制|Bật/tắt từng thiết bị hoặc tất cả cùng lúc / 逐一或同時開關      |
|2 |🌡️ Cảm biến DHT11 real-time                       |Đọc mỗi 2 giây, hiển thị OLED / 每 2 秒讀取，顯示於 OLED          |
|3 |🔔 Cảnh báo quá nhiệt / 過熱警告                      |LED nhấp nháy non-blocking khi T° > ngưỡng / 非阻塞式 LED 閃爍  |
|4 |🎯 Ngưỡng tùy chỉnh bằng giọng nói / 語音設定閾值       |Nói “đặt ngưỡng 35 độ”, lưu NVS / 說「設定閾值 35 度」，NVS 儲存     |
|5 |📊 Truy vấn trạng thái / 狀態查詢                     |Nói “đèn nào đang bật?” → Xiaozhi trả lời đúng / 語音詢問設備狀態 |
|6 |🖥️ OLED 4 dòng / 4 行顯示                            |Nhiệt độ, độ ẩm, ngưỡng cảnh báo, trạng thái / 溫度、濕度、閾值、狀態|
|7 |🔄 Tự khởi động lại khi mất WiFi / WiFi 斷線自動重啟    |Timeout 15 giây / 15 秒逾時                                  |
|8 |💾 NVS lưu cài đặt / NVS 設定儲存                     |Giữ ngưỡng nhiệt sau mất điện / 斷電後保留閾值設定                 |
|9 |⚡ Dual-core safe                                 |`volatile` cho biến xuyên core / 跨核心變數使用 `volatile`       |
|10|🧠 Memory safe                                    |`StaticJsonDocument`, tránh heap fragmentation / 避免堆積碎片化  |

-----

## 🛠️ Phần cứng cần thiết / 所需硬體

### Thiết bị 1 — Xiaozhi ESP32-S3 / 硬體一

|Thành phần / 元件   |Thông số / 規格                                                         |
|------------------|----------------------------------------------------------------------|
|Vi xử lý / 微控制器   |ESP32-S3 (16MB Flash, 8MB PSRAM)                                      |
|Màn hình / 螢幕     |ST7789 v1.1 TFT 2.8” — 240×RGB×320                                    |
|Âm thanh / 音訊     |Microphone số + Loa Class-D / 數位麥克風 + D 類擴大器喇叭                        |
|Kết nối / 連線      |Wi-Fi 2.4GHz                                                          |
|Firmware          |[xiaozhi-esp32](https://github.com/78/xiaozhi-esp32) (MIT)            |
|Dịch vụ AI / AI 服務|[xiaozhi.me](https://xiaozhi.me) (Qwen model, miễn phí cá nhân / 個人免費)|

### Thiết bị 2 — ESP32 WROOM-32 / 硬體二

|Module / 模組                   |GPIO       |Vai trò / 功能                          |
|------------------------------|-----------|--------------------------------------|
|DHT11 (DATA)                  |GPIO 23    |Đọc nhiệt độ & độ ẩm / 讀取溫濕度          |
|OLED SSD1306 (SDA)            |GPIO 21    |I2C — hiển thị / I2C 顯示               |
|OLED SSD1306 (SCL)            |GPIO 22    |I2C — hiển thị / I2C 顯示               |
|LED cảnh báo / 警告 LED         |GPIO 5     |Nhấp nháy khi T° > ngưỡng / T° 超過閾值時閃爍|
|Relay Quạt / 電風扇繼電器           |**GPIO 25**|Bật/tắt quạt / 開關電風扇                  |
|Relay Đèn phòng khách / 客廳燈繼電器|GPIO 4     |Bật/tắt đèn / 開關燈                     |
|Relay Đèn phòng ngủ / 臥室燈繼電器  |GPIO 18    |Bật/tắt đèn / 開關燈                     |
|Relay Đèn phòng bếp / 廚房燈繼電器  |GPIO 19    |Bật/tắt đèn / 開關燈                     |


> ⚠️ **Lưu ý / 注意：** GPIO 25 được chọn thay vì GPIO 2 vì GPIO 2 trên ESP32-WROOM-32 có pull-down nội bộ dùng để boot — nếu relay kéo HIGH lúc flash sẽ không upload firmware được.  
> GPIO 25 取代 GPIO 2，原因是 ESP32-WROOM-32 的 GPIO 2 有內建下拉電阻用於開機程序，若繼電器在燒錄時拉高將導致韌體無法上傳。

-----

## 🔧 8 MCP Tools đã đăng ký / 已註冊的 8 個 MCP 工具

|#|Tool name                        |Mô tả / 說明                  |Input / 輸入                 |Output / 輸出                           |
|-|---------------------------------|----------------------------|---------------------------|--------------------------------------|
|1|`temperature_and_humidity_values`|Đọc DHT11 / 讀取 DHT11        |*(none)*                   |`{temperature, humidity}`             |
|2|`set_temperature_warning`        |Đặt ngưỡng cảnh báo / 設定警告閾值|`max_temperature` (20–60°C)|`{success, new_limit}`                |
|3|`fan_control`                    |Điều khiển quạt / 控制電風扇     |`state: "on"/"off"`        |`{success, state}`                    |
|4|`living_room_lights_control`     |Đèn phòng khách / 客廳燈       |`state: "on"/"off"`        |`{success, state}`                    |
|5|`bedroom_lights_control`         |Đèn phòng ngủ / 臥室燈         |`state: "on"/"off"`        |`{success, state}`                    |
|6|`kitchen_lights_control`         |Đèn phòng bếp / 廚房燈         |`state: "on"/"off"`        |`{success, state}`                    |
|7|`all_devices_control`            |Tất cả thiết bị / 所有設備      |`state: "on"/"off"`        |`{success, state}`                    |
|8|`get_device_status`              |Trạng thái thiết bị / 查詢設備狀態|*(none)*                   |`{fan, living_room, bedroom, kitchen}`|

-----

## 📦 Thư viện cần cài / 所需函式庫

|Thư viện / 函式庫        |Nguồn / 來源                                                               |Ghi chú / 備注                                    |
|----------------------|-------------------------------------------------------------------------|------------------------------------------------|
|`WebSocketMCP`        |[toddpan/xiaozhi-esp32-mcp](https://github.com/toddpan/xiaozhi-esp32-mcp)|Copy `.h` + `.cpp` vào thư mục sketch / 複製至草稿資料夾|
|`DHT sensor library`  |Adafruit — Arduino Library Manager                                       |                                                |
|`Adafruit GFX Library`|Adafruit — Arduino Library Manager                                       |                                                |
|`Adafruit SSD1306`    |Adafruit — Arduino Library Manager                                       |                                                |
|`ArduinoJson`         |Benoit Blanchon — Arduino Library Manager                                |Version 6.x                                     |
|`Preferences`         |Built-in ESP32 Arduino core                                              |Không cần cài thêm / 內建，無需安裝                    |

-----

## 🚀 Hướng dẫn cài đặt / 安裝指南

### Bước 1 / 步驟一：Cài thư viện / 安裝函式庫

1. Mở Arduino IDE → **Sketch → Include Library → Manage Libraries**  
   開啟 Arduino IDE → **草稿碼 → 匯入程式庫 → 管理程式庫**
1. Tìm và cài: `DHT sensor library`, `Adafruit GFX`, `Adafruit SSD1306`, `ArduinoJson`  
   搜尋並安裝以上四個函式庫
1. Tải `WebSocketMCP` từ [toddpan/xiaozhi-esp32-mcp](https://github.com/toddpan/xiaozhi-esp32-mcp), copy file `.h` và `.cpp` vào cùng thư mục với file `.ino`  
   從連結下載後，將 `.h` 與 `.cpp` 複製至 `.ino` 同一資料夾

### Bước 2 / 步驟二：Lấy MCP Token / 取得 MCP Token

1. Đăng ký tài khoản tại [xiaozhi.me](https://xiaozhi.me)  
   至 xiaozhi.me 註冊帳號
1. Vào **Agent → Dịch vụ tuỳ chỉnh → Lấy điểm cuối MCP**  
   進入 **Agent → 自訂服務 → 取得 MCP 端點**
1. Copy token từ URL dạng: `wss://api.xiaozhi.me/mcp/?token=YOUR_TOKEN`  
   複製 URL 中的 token

### Bước 3 / 步驟三：Sửa cấu hình / 修改設定

Mở `ESP32_FULL_MCP_XIAOZHI_v4.ino`, sửa 3 dòng sau / 開啟 `.ino` 檔，修改以下 3 行：

```cpp
const char* ssid        = "TÊN_WIFI_CỦA_BẠN";   // Tên WiFi / WiFi 名稱
const char* password    = "MẬT_KHẨU_WIFI";        // Mật khẩu / WiFi 密碼
const char* mcpEndpoint = "wss://api.xiaozhi.me/mcp/?token=TOKEN_CỦA_BẠN";
```

### Bước 4 / 步驟四：Nạp code / 燒錄程式

1. Chọn board: **ESP32 Dev Module** trong Arduino IDE  
   選擇開發板：Arduino IDE 中選 **ESP32 Dev Module**
1. Chọn đúng cổng COM / 選擇正確的 COM 埠
1. Nhấn Upload / 點擊上傳

### Bước 5 / 步驟五：Kết nối phần cứng và test / 連接硬體並測試

Sau khi upload, mở Serial Monitor (115200 baud) để xem IP của ESP32.  
上傳後，開啟序列埠監控器（115200 baud）查看 ESP32 的 IP 位址。

-----

## 🗂️ Cấu trúc thư mục / 專案目錄結構

```
📁 smart-home-xiaozhi-mcp/
│
├── ESP32_FULL_MCP_XIAOZHI_v4.ino   ← File code chính / 主程式碼
├── WebSocketMCP.h                   ← Thư viện MCP / MCP 函式庫標頭檔
├── WebSocketMCP.cpp                 ← Thư viện MCP / MCP 函式庫原始碼
│
├── README.md                        ← File này / 本文件
├── HARDWARE.md                      ← Sơ đồ kết nối / 接線圖與零件清單
├── INSTALL.md                       ← Hướng dẫn chi tiết / 詳細安裝說明
├── CHANGELOG.md                     ← Lịch sử phiên bản / 版本更新紀錄
└── .gitignore
```

-----

## 📋 Lịch sử phiên bản / 版本更新紀錄

Xem chi tiết tại [CHANGELOG.md](./CHANGELOG.md)  
詳情請見 [CHANGELOG.md](./CHANGELOG.md)

|Phiên bản / 版本|Thay đổi chính / 主要更新                                                    |
|--------------|-------------------------------------------------------------------------|
|v1.0          |Code gốc, 7 MCP tools cơ bản / 基礎版本，7 個 MCP 工具                           |
|v2.0          |Sửa GPIO 25, thêm NVS, Tool 8, timeout WiFi / 修正 GPIO，新增 NVS、工具 8、WiFi 逾時|
|v3.0          |Xóa portMUX (nguy cơ watchdog), thêm `volatile` / 移除 portMUX，新增 volatile |
|**v4.0**      |**`warningTemp` → `volatile float`, validation 20–60°C**                 |

-----

## 🔗 Tài nguyên & Liên kết / 相關資源與連結

|Tài nguyên / 資源                 |URL                                                                                 |
|--------------------------------|------------------------------------------------------------------------------------|
|Firmware Xiaozhi ESP32-S3       |[github.com/78/xiaozhi-esp32](https://github.com/78/xiaozhi-esp32)                  |
|Thư viện WebSocketMCP / MCP 函式庫 |[github.com/toddpan/xiaozhi-esp32-mcp](https://github.com/toddpan/xiaozhi-esp32-mcp)|
|Xiaozhi Cloud Console           |[xiaozhi.me](https://xiaozhi.me)                                                    |
|Tài liệu MCP Endpoint / MCP 端點說明|[Feishu Wiki](https://my.feishu.cn/wiki/W14Kw1s1uieoKjkP8N0c1VVvn8d)                |
|Cộng đồng Xiaozhi / Xiaozhi 社群  |[16302.com](https://16302.com)                                                      |

-----

## 📜 Giấy phép / 授權條款

Dự án này sử dụng nền tảng Xiaozhi được phát hành theo giấy phép **MIT License**.  
本專案使用之 Xiaozhi 平台採用 **MIT License** 開放原始碼授權。

-----

## 🙏 Lời cảm ơn / 致謝

- [xiaozhi-esp32](https://github.com/78/xiaozhi-esp32) — Nền tảng AI giọng nói tuyệt vời / 出色的語音 AI 平台
- [toddpan/xiaozhi-esp32-mcp](https://github.com/toddpan/xiaozhi-esp32-mcp) — Thư viện MCP cho ESP32 / ESP32 MCP 函式庫
- Adafruit — Các thư viện cảm biến và màn hình chất lượng cao / 高品質感測器與顯示器函式庫