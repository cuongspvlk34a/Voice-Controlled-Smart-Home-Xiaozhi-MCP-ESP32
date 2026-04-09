# 🔌 Sơ đồ kết nối & Danh sách linh kiện
# 🔌 接線圖與零件清單

---

## 📦 Danh sách linh kiện đầy đủ / 完整零件清單

### Thiết bị 1 — Xiaozhi ESP32-S3 (Mua nguyên bộ) / 硬體一（整套購買）

| Linh kiện / 零件 | Số lượng / 數量 | Ghi chú / 備注 |
|---|---|---|
| Board Xiaozhi ESP32-S3 | 1 | Có sẵn màn hình TFT + mic + loa / 含 TFT 螢幕、麥克風、喇叭 |

> 💡 Mua board Xiaozhi ESP32-S3 tích hợp sẵn — không cần tự lắp màn hình.  
> 購買 Xiaozhi ESP32-S3 整合板——無需自行組裝螢幕。

---

### Thiết bị 2 — ESP32 WROOM-32 + Linh kiện ngoại vi / 硬體二 + 周邊元件

| Linh kiện / 零件 | Số lượng / 數量 | Model / 型號 | Ghi chú / 備注 |
|---|---|---|---|
| ESP32 WROOM-32 DevKit | 1 | ESP32-WROOM-32 | Board phát triển 38 chân / 38 腳開發板 |
| Cảm biến nhiệt độ-độ ẩm / 溫濕度感測器 | 1 | DHT11 | Module có điện trở pull-up sẵn / 含上拉電阻模組 |
| Màn hình OLED / OLED 顯示器 | 1 | SSD1306 0.96" I2C | Địa chỉ I2C: 0x3C |
| LED đơn / 單顆 LED | 1 | LED 5mm đỏ hoặc vàng / 紅色或黃色 | Cảnh báo quá nhiệt / 過熱警告 |
| Điện trở / 電阻 | 1 | 220Ω | Nối tiếp với LED / LED 串聯電阻 |
| Module Relay 1 kênh / 單通道繼電器模組 | 4 | 5V optocoupler relay | Kích HIGH để đóng / 高電平觸發閉合 |
| Dây jumper / 跳線 | ~20 | Male-to-Male, Male-to-Female | |
| Breadboard | 1 | 830 lỗ / 830 孔 | Tùy chọn / 選用 |
| Nguồn 5V / 5V 電源 | 1 | Adapter 5V 2A | Cấp nguồn cho relay / 為繼電器供電 |

---

## 🗺️ Sơ đồ kết nối ESP32 WROOM-32 / ESP32 WROOM-32 接線圖

### DHT11 → ESP32
```
DHT11           ESP32 WROOM-32
─────────────────────────────
VCC    ────────  3.3V
GND    ────────  GND
DATA   ────────  GPIO 23
```

### OLED SSD1306 → ESP32
```
OLED SSD1306    ESP32 WROOM-32
───────────────────────────────
VCC    ────────  3.3V
GND    ────────  GND
SDA    ────────  GPIO 21
SCL    ────────  GPIO 22
```
> Địa chỉ I2C: `0x3C` / I2C 位址：`0x3C`

### LED cảnh báo → ESP32 / 警告 LED → ESP32
```
LED (Anode +)  ──── 220Ω ──── GPIO 5
LED (Cathode -) ──── GND
```

### Relay Quạt → ESP32 / 電風扇繼電器 → ESP32
```
Relay Module    ESP32 WROOM-32
───────────────────────────────
VCC    ────────  5V (nguồn ngoài / 外部電源)
GND    ────────  GND (chung / 共地)
IN     ────────  GPIO 25
```

### Relay Đèn phòng khách → ESP32 / 客廳燈繼電器 → ESP32
```
Relay Module    ESP32 WROOM-32
───────────────────────────────
VCC    ────────  5V
GND    ────────  GND
IN     ────────  GPIO 4
```

### Relay Đèn phòng ngủ → ESP32 / 臥室燈繼電器 → ESP32
```
Relay Module    ESP32 WROOM-32
───────────────────────────────
VCC    ────────  5V
GND    ────────  GND
IN     ────────  GPIO 18
```

### Relay Đèn phòng bếp → ESP32 / 廚房燈繼電器 → ESP32
```
Relay Module    ESP32 WROOM-32
───────────────────────────────
VCC    ────────  5V
GND    ────────  GND
IN     ────────  GPIO 19
```

---

## 📊 Bảng tổng hợp chân GPIO / GPIO 腳位彙整表

| GPIO | Chức năng / 功能 | Module | Ghi chú / 備注 |
|---|---|---|---|
| GPIO 21 | SDA (I2C) | OLED SSD1306 | I2C mặc định ESP32 / ESP32 預設 I2C |
| GPIO 22 | SCL (I2C) | OLED SSD1306 | I2C mặc định ESP32 / ESP32 預設 I2C |
| GPIO 23 | DATA | DHT11 | Cảm biến nhiệt độ / 溫度感測器 |
| GPIO 5  | OUTPUT | LED đỏ / 紅色 LED | Cảnh báo quá nhiệt / 過熱警告 |
| GPIO 25 | OUTPUT | Relay Quạt / 電風扇 | ⚠️ Không dùng GPIO 2 / 勿使用 GPIO 2 |
| GPIO 4  | OUTPUT | Relay Đèn PK / 客廳 | |
| GPIO 18 | OUTPUT | Relay Đèn PN / 臥室 | |
| GPIO 19 | OUTPUT | Relay Đèn PB / 廚房 | |

---

## ⚠️ Lưu ý quan trọng khi đấu dây / 重要接線注意事項

### 1. Tại sao dùng GPIO 25 thay GPIO 2? / 為何使用 GPIO 25 而非 GPIO 2？

ESP32-WROOM-32 có **GPIO 2 với pull-down nội bộ** dùng trong quá trình boot (flashing mode). Nếu relay kéo chân này lên HIGH khi đang nạp firmware → ESP32 sẽ **không vào được chế độ flash** và upload thất bại.  

GPIO 25 không có ràng buộc boot, an toàn để dùng với relay.

ESP32-WROOM-32 的 **GPIO 2 有內建下拉電阻**，用於開機程序（燒錄模式）。若繼電器在燒錄韌體時將此腳拉高，ESP32 將**無法進入燒錄模式**，導致上傳失敗。

GPIO 25 無開機限制，可安全用於繼電器控制。

### 2. Nguồn cho relay / 繼電器電源

Các module relay 1 kênh với optocoupler cần nguồn **5V**. Không dùng 3.3V vì cuộn hút relay cần đủ điện áp để đóng.  
單通道光耦繼電器模組需要 **5V** 電源。請勿使用 3.3V，因繼電器線圈需要足夠電壓才能閉合。

### 3. Chung GND / 共地

Luôn nối GND của relay với GND của ESP32 và GND của nguồn ngoài thành một mạch chung.  
繼電器的 GND、ESP32 的 GND 與外部電源的 GND 必須連接在一起，形成共地迴路。

### 4. Loại relay / 繼電器類型

Dùng relay có optocoupler (cách ly quang). Kích HIGH (IN = 3.3V từ GPIO) → relay đóng.  
使用有光耦隔離的繼電器模組。高電平觸發（IN = GPIO 輸出 3.3V）→ 繼電器閉合。

---

## 🧮 Chi phí ước tính / 預估費用

| Linh kiện / 零件 | Giá ước tính (VND) / 預估價格 (NTD) |
|---|---|
| Xiaozhi ESP32-S3 board | ~500,000 VND / ~700 NTD |
| ESP32 WROOM-32 DevKit | ~80,000 VND / ~100 NTD |
| DHT11 module | ~15,000 VND / ~30 NTD |
| OLED SSD1306 0.96" | ~30,000 VND / ~60 NTD |
| 4× Relay module | ~80,000 VND / ~120 NTD |
| LED + Điện trở / LED + 電阻 | ~5,000 VND / ~10 NTD |
| Dây jumper, Breadboard / 跳線、麵包板 | ~30,000 VND / ~50 NTD |
| **Tổng / 總計** | **~740,000 VND / ~1,070 NTD** |

> Giá tham khảo, có thể thay đổi tùy nguồn mua / 價格僅供參考，實際因購買來源而異
