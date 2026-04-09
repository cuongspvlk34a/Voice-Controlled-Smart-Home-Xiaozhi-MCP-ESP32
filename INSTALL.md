# 🛠️ Hướng dẫn cài đặt chi tiết từng bước
# 🛠️ 詳細分步安裝說明

---

## Yêu cầu hệ thống / 系統需求

- Arduino IDE 2.x (khuyến nghị / 建議使用)
- ESP32 Arduino Core ≥ 2.0
- Hệ điều hành: Windows / macOS / Linux
- Kết nối Internet để cài thư viện và lấy token / 需要網路連線安裝函式庫及取得 token

---

## BƯỚC 1: Cài ESP32 Core cho Arduino IDE / 步驟一：安裝 ESP32 Core

1. Mở Arduino IDE → **File → Preferences**  
   開啟 Arduino IDE → **檔案 → 偏好設定**

2. Trong **"Additional boards manager URLs"**, thêm / 在「額外的開發板管理員網址」加入：
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```

3. Vào **Tools → Board → Boards Manager** → Tìm `esp32` → Cài **esp32 by Espressif Systems**  
   進入 **工具 → 開發板 → 開發板管理員** → 搜尋 `esp32` → 安裝 **esp32 by Espressif Systems**

---

## BƯỚC 2: Cài thư viện qua Library Manager / 步驟二：透過程式庫管理員安裝

Mở **Sketch → Include Library → Manage Libraries** (Ctrl+Shift+I)  
開啟 **草稿碼 → 匯入程式庫 → 管理程式庫**（Ctrl+Shift+I）

Tìm và cài lần lượt / 依序搜尋並安裝：

| Tên thư viện / 函式庫名稱 | Tác giả / 作者 | Phiên bản / 版本 |
|---|---|---|
| `DHT sensor library` | Adafruit | Latest |
| `Adafruit GFX Library` | Adafruit | Latest |
| `Adafruit SSD1306` | Adafruit | Latest |
| `ArduinoJson` | Benoit Blanchon | **6.x** (không dùng 7.x) |

> ⚠️ Khi cài `DHT sensor library` nó sẽ hỏi cài thêm `Adafruit Unified Sensor` — nhấn **"Install All"**  
> 安裝 `DHT sensor library` 時會詢問是否一併安裝 `Adafruit Unified Sensor`——點選 **「全部安裝」**

---

## BƯỚC 3: Cài thư viện WebSocketMCP (thủ công) / 步驟三：手動安裝 WebSocketMCP

1. Truy cập: [github.com/toddpan/xiaozhi-esp32-mcp](https://github.com/toddpan/xiaozhi-esp32-mcp)  

2. Nhấn **Code → Download ZIP** để tải về  
   點選 **Code → 下載 ZIP**

3. Giải nén ZIP, tìm các file `WebSocketMCP.h` và `WebSocketMCP.cpp`  
   解壓縮 ZIP，找到 `WebSocketMCP.h` 與 `WebSocketMCP.cpp`

4. Copy 2 file này vào **cùng thư mục** với file `ESP32_FULL_MCP_XIAOZHI_v4.ino`  
   將這 2 個檔案複製至與 `ESP32_FULL_MCP_XIAOZHI_v4.ino` **相同的資料夾**

Cấu trúc thư mục sau khi copy / 複製後的資料夾結構：
```
📁 ESP32_FULL_MCP_XIAOZHI_v4/
├── ESP32_FULL_MCP_XIAOZHI_v4.ino
├── WebSocketMCP.h                  ← Copy vào đây / 複製至此
└── WebSocketMCP.cpp                ← Copy vào đây / 複製至此
```

---

## BƯỚC 4: Lấy MCP Token từ Xiaozhi / 步驟四：從 Xiaozhi 取得 MCP Token

1. Đăng ký / đăng nhập tại [xiaozhi.me](https://xiaozhi.me)  
   在 xiaozhi.me 註冊或登入

2. Tạo một **Agent** mới hoặc chọn agent đã có  
   新建一個 **Agent** 或選擇現有 Agent

3. Vào **Dịch vụ tuỳ chỉnh (Custom Service) → Lấy điểm cuối MCP (Get MCP Endpoint)**  
   進入 **自訂服務 → 取得 MCP 端點**

4. Copy URL có dạng / 複製格式如下的 URL：
   ```
   wss://api.xiaozhi.me/mcp/?token=eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9...
   ```

> 📖 Tài liệu tham khảo chi tiết / 詳細參考文件：  
> [Feishu Wiki — MCP Endpoint Guide](https://my.feishu.cn/wiki/W14Kw1s1uieoKjkP8N0c1VVvn8d)

---

## BƯỚC 5: Cấu hình code / 步驟五：設定程式碼

Mở file `ESP32_FULL_MCP_XIAOZHI_v4.ino` trong Arduino IDE.  
用 Arduino IDE 開啟 `ESP32_FULL_MCP_XIAOZHI_v4.ino`。

Tìm và sửa **3 dòng** ở đầu file / 找到並修改開頭的 **3 行**：

```cpp
// ===== CẤU HÌNH — SỬA 3 DÒNG NÀY / 設定 — 修改這 3 行 =====
const char* ssid        = "TÊN_WIFI_CỦA_BẠN";          // Tên WiFi / WiFi 名稱
const char* password    = "MẬT_KHẨU_WIFI";               // Mật khẩu WiFi / WiFi 密碼  
const char* mcpEndpoint = "wss://api.xiaozhi.me/mcp/?token=TOKEN_CỦA_BẠN";
// ============================================================
```

---

## BƯỚC 6: Chọn board và nạp code / 步驟六：選擇開發板並上傳

1. **Tools → Board → ESP32 Arduino → ESP32 Dev Module**

2. **Tools → Port** → Chọn đúng cổng COM (ví dụ COM3, COM5, /dev/ttyUSB0)  
   選擇正確的 COM 埠（例如 COM3、COM5、/dev/ttyUSB0）

3. Cài đặt thêm nếu cần / 如需要可調整以下設定：
   - Upload Speed: `115200`
   - Flash Size: `4MB (32Mb)`
   - Partition Scheme: `Default 4MB with spiffs`

4. Nhấn nút **Upload** (→) hoặc Ctrl+U  
   按下 **上傳** 按鈕（→）或 Ctrl+U

> ⚠️ Nếu upload thất bại, hãy giữ nút **BOOT** trên ESP32 trong lúc upload bắt đầu, rồi thả ra khi thấy "Connecting..."  
> 若上傳失敗，請在上傳開始時按住 ESP32 的 **BOOT** 鍵，看到「Connecting...」後再放開

---

## BƯỚC 7: Kiểm tra hoạt động / 步驟七：驗證運作

1. Mở **Serial Monitor** (Ctrl+Shift+M), chọn baud rate **115200**  
   開啟**序列埠監控器**（Ctrl+Shift+M），選擇 115200 baud rate

2. Nhấn nút **RESET** trên ESP32  
   按下 ESP32 的 **RESET** 按鈕

3. Bạn sẽ thấy log khởi động / 您將看到開機日誌：
   ```
   Connecting to WiFi...
   Connected! IP: 192.168.x.x
   Connecting to MCP...
   MCP Connected!
   Tool registered: temperature_and_humidity_values
   Tool registered: fan_control
   ... (8 tools)
   Ready!
   ```

4. Màn hình OLED sẽ hiển thị IP trong 3 giây, sau đó hiển thị nhiệt độ/độ ẩm  
   OLED 螢幕將顯示 IP 位址 3 秒，之後顯示溫濕度

---

## BƯỚC 8: Cấu hình Xiaozhi ESP32-S3 / 步驟八：設定 Xiaozhi ESP32-S3

1. Flash firmware Xiaozhi lên ESP32-S3 theo hướng dẫn tại [github.com/78/xiaozhi-esp32](https://github.com/78/xiaozhi-esp32)  
   依照 xiaozhi-esp32 的說明將韌體燒錄至 ESP32-S3

2. Kết nối ESP32-S3 với WiFi và đăng nhập tài khoản xiaozhi.me  
   連接 ESP32-S3 至 WiFi 並登入 xiaozhi.me 帳號

3. Trong xiaozhi.me, cấu hình Agent để dùng cùng MCP token đã cài ở ESP32 WROOM-32  
   在 xiaozhi.me 設定 Agent 使用與 ESP32 WROOM-32 相同的 MCP token

---

## 🧪 Test thử các lệnh giọng nói / 語音指令測試

Sau khi cả hai thiết bị hoạt động, thử nói vào Xiaozhi ESP32-S3:  
兩個裝置都正常運作後，對著 Xiaozhi ESP32-S3 說：

| Lệnh nói / 語音指令 | Kết quả / 預期結果 |
|---|---|
| "Bật quạt" / "開電風扇" | Relay GPIO 25 đóng → Quạt chạy |
| "Tắt quạt" / "關電風扇" | Relay GPIO 25 mở → Quạt dừng |
| "Bật đèn phòng khách" / "開客廳燈" | Relay GPIO 4 đóng |
| "Tắt tất cả đèn" / "關掉所有燈" | GPIO 4, 18, 19 mở |
| "Bật tất cả thiết bị" / "開啟所有設備" | Tất cả relay đóng |
| "Nhiệt độ hiện tại bao nhiêu?" / "現在溫度是幾度？" | Xiaozhi đọc giá trị DHT11 |
| "Đặt ngưỡng cảnh báo 35 độ" / "設定警告閾值 35 度" | `warningTemp` = 35, lưu NVS |
| "Đèn nào đang bật?" / "哪些燈是開著的？" | Xiaozhi báo trạng thái từng đèn |

---

## 🐛 Xử lý sự cố thường gặp / 常見問題排除

### Upload thất bại / 上傳失敗
- Giữ nút BOOT lúc upload / 上傳時按住 BOOT 鍵
- Kiểm tra driver CH340/CP2102 đã cài chưa / 確認已安裝 CH340/CP2102 驅動程式
- Thử giảm Upload Speed xuống 115200 / 嘗試將上傳速度降至 115200

### Không kết nối được WiFi / WiFi 無法連線
- Kiểm tra SSID/password / 確認 SSID 與密碼
- ESP32 chỉ hỗ trợ WiFi 2.4GHz (không hỗ trợ 5GHz) / ESP32 僅支援 2.4GHz WiFi（不支援 5GHz）
- Sau 15 giây không kết nối, ESP32 sẽ tự restart / 15 秒無法連線，ESP32 將自動重啟

### Không kết nối được MCP / MCP 無法連線
- Kiểm tra token còn hạn / 確認 token 仍有效
- Kiểm tra Internet connection / 確認網路連線正常
- Xem log trong Serial Monitor để tìm lỗi cụ thể / 查看序列埠監控器日誌找出具體錯誤

### OLED không hiển thị / OLED 無顯示
- Kiểm tra SDA=21, SCL=22 / 確認 SDA=21、SCL=22
- Kiểm tra địa chỉ I2C: `0x3C` / 確認 I2C 位址為 `0x3C`
- Có thể scan I2C: thêm đoạn scan I2C vào setup() để tìm địa chỉ / 可執行 I2C 掃描找出實際位址

### DHT11 trả về NaN / DHT11 回傳 NaN
- Kiểm tra dây nối GPIO 23 / 確認 GPIO 23 接線
- Đảm bảo dùng module DHT11 có điện trở pull-up / 確保使用含上拉電阻的 DHT11 模組
- DHT11 cần ít nhất 1-2 giây giữa các lần đọc (code đã xử lý) / DHT11 讀取間隔需至少 1-2 秒（程式已處理）

### Relay không kéo / 繼電器不動作
- Kiểm tra nguồn 5V cho relay / 確認繼電器有 5V 電源
- Kiểm tra GND chung / 確認共地連接
- Relay loại kích HIGH: IN = HIGH → relay đóng / 高電平觸發型：IN = HIGH → 繼電器閉合
