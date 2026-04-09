# 📋 Lịch sử thay đổi / 版本更新紀錄

Tất cả các thay đổi quan trọng của dự án được ghi lại tại đây.  
本專案的所有重要變更均記錄於此。

Định dạng theo [Keep a Changelog](https://keepachangelog.com/en/1.0.0/)

---

## [v4.0] — Phiên bản ổn định hiện tại / 當前穩定版本

### 🔴 Sửa lỗi nghiêm trọng / 修復嚴重錯誤
- **`warningTemp` khai báo thiếu `volatile`** — biến này được đọc ở core 0 (loop) và ghi ở core 1 (MCP callback), thiếu `volatile` gây undefined behavior tiềm ẩn. Đã sửa thành `volatile float warningTemp`.  
  **`warningTemp` 缺少 `volatile` 宣告** ——此變數在 core 0（loop）讀取、在 core 1（MCP 回呼）寫入，缺少 `volatile` 會導致潛在的未定義行為。已修正為 `volatile float warningTemp`。

### 🟠 Cải tiến / 改進
- **Thêm validation dải nhiệt độ** cho `set_temperature_warning`: chỉ chấp nhận `max_temperature` trong khoảng 20–60°C. Giá trị ngoài dải trả về lỗi có hướng dẫn rõ ràng.  
  **新增溫度範圍驗證** 至 `set_temperature_warning`：僅接受 20–60°C 範圍內的 `max_temperature`。超出範圍將回傳含明確說明的錯誤訊息。

---

## [v3.0] — Review lần 2 / 第二次審查

### 🔴 Sửa lỗi nghiêm trọng / 修復嚴重錯誤
- **Xóa `portMUX` / `portENTER_CRITICAL` / `portEXIT_CRITICAL`** — Các macro này khi dùng sai context (không phải ISR) có thể gây watchdog timeout → ESP32 tự reset liên tục. Đã thay bằng `volatile` keyword cho các biến state.  
  **移除 `portMUX` / `portENTER_CRITICAL` / `portEXIT_CRITICAL`** ——這些 macro 在非 ISR 情境下使用不當，可能導致看門狗逾時 → ESP32 持續重啟。改用 `volatile` 關鍵字宣告狀態變數。

### 🟠 Cải tiến / 改進
- **Thêm `volatile bool`** cho tất cả biến state điều khiển thiết bị (`fanState`, `livingRoomState`, `bedroomState`, `kitchenState`) — đảm bảo dual-core safe khi đọc/ghi từ core khác nhau.  
  **為所有設備狀態控制變數新增 `volatile bool`**（`fanState`、`livingRoomState`、`bedroomState`、`kitchenState`）——確保跨核心讀寫的雙核心安全性。

### 🟡 Tối ưu / 優化
- **Thêm `cachedTemp` và `cachedHumidity`** — tránh gọi `dht.readTemperature()` / `dht.readHumidity()` trực tiếp trong MCP callback (chạy ở core 1). DHT library không thread-safe → dùng giá trị cache được cập nhật mỗi 2s từ loop().  
  **新增 `cachedTemp` 與 `cachedHumidity`** ——避免在 MCP 回呼（在 core 1 執行）中直接呼叫 `dht.readTemperature()` / `dht.readHumidity()`。DHT 函式庫非執行緒安全，改用每 2 秒由 loop() 更新的快取值。

---

## [v2.0] — Review lần 1 / 第一次審查

### 🔴 Sửa lỗi nghiêm trọng / 修復嚴重錯誤
- **Đổi GPIO 2 → GPIO 25** cho relay quạt — GPIO 2 trên ESP32-WROOM-32 có pull-down nội bộ dùng để boot. Relay kéo HIGH khi flashing gây lỗi upload firmware.  
  **將繼電器腳位從 GPIO 2 改為 GPIO 25** ——ESP32-WROOM-32 的 GPIO 2 有開機用的內建下拉電阻，燒錄時繼電器拉高將導致韌體無法上傳。

- **Sửa Schema Tool 1** (`temperature_and_humidity_values`) — xóa trường `required` không cần thiết gây parse error trên một số phiên bản LLM.  
  **修正工具一的 Schema**（`temperature_and_humidity_values`）——刪除不必要的 `required` 欄位，該欄位在某些 LLM 版本上會導致解析錯誤。

### 🟠 Cải tiến / 改進
- **Thêm validation JSON args** — kiểm tra `args.isNull()` và `args.is<JsonObject>()` trước khi truy xuất tham số tool. Tránh crash khi LLM gửi JSON không hợp lệ.  
  **新增 JSON 參數驗證** ——在存取工具參數前先檢查 `args.isNull()` 與 `args.is<JsonObject>()`，避免 LLM 傳送無效 JSON 時發生崩潰。

### 🟡 Tối ưu / 優化
- **`DynamicJsonDocument` → `StaticJsonDocument`** — tránh heap fragmentation trên ESP32. Static allocation an toàn hơn cho thiết bị nhúng chạy liên tục.  
  **`DynamicJsonDocument` 改為 `StaticJsonDocument`** ——避免 ESP32 上的堆積碎片化，靜態分配對持續運行的嵌入式裝置更為安全。

- **Chu kỳ đọc DHT11: 1s → 2s** — DHT11 có thời gian phục hồi tối thiểu 1-2 giây. Đọc quá nhanh trả về NaN. 2s là giá trị an toàn.  
  **DHT11 讀取週期：1 秒改為 2 秒** ——DHT11 最短恢復時間為 1-2 秒，讀取過快會回傳 NaN，2 秒為安全值。

### ➕ Tính năng mới / 新增功能
- **Timeout WiFi 15 giây** + tự restart — thay vì treo vô hạn khi mất WiFi.  
  **WiFi 逾時 15 秒** + 自動重啟——取代 WiFi 中斷時無限等待的行為。

- **OLED Boot Screen** — hiển thị địa chỉ IP trong 3 giây khi khởi động, dễ debug không cần Serial Monitor.  
  **OLED 開機畫面** ——開機時顯示 IP 位址 3 秒，無需序列埠監控器即可除錯。

- **NVS (Non-Volatile Storage)** — lưu `warningTemp` vào flash. Giá trị được giữ nguyên sau khi mất điện / reset.  
  **NVS（非揮發性儲存）** ——將 `warningTemp` 儲存至 flash，斷電或重置後設定值不會遺失。

- **Tool 8: `get_device_status`** — truy vấn trạng thái tất cả thiết bị bằng một lệnh giọng nói.  
  **工具 8：`get_device_status`** ——以單一語音指令查詢所有設備的狀態。

---

## [v1.0] — Code gốc / 初始版本

### ✅ Tính năng ban đầu / 初始功能
- Ghép code DHT11 + OLED từ file ZIP với code điều khiển relay từ file .txt  
  將 ZIP 檔中的 DHT11 + OLED 程式碼與 .txt 檔中的繼電器控制程式碼合併

- 7 MCP tools cơ bản:  
  7 個基礎 MCP 工具：
  - `temperature_and_humidity_values`
  - `set_temperature_warning`
  - `fan_control`
  - `living_room_lights_control`
  - `bedroom_lights_control`
  - `kitchen_lights_control`
  - `all_devices_control`

- Kết nối WebSocket MCP đến Xiaozhi Cloud  
  透過 WebSocket MCP 連接至 Xiaozhi 雲端

- Điều khiển 4 relay (quạt + 3 đèn) qua GPIO  
  透過 GPIO 控制 4 個繼電器（電風扇 + 3 盞燈）

- Hiển thị DHT11 trên OLED  
  在 OLED 上顯示 DHT11 數據
