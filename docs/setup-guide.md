# IoT Air Quality Monitor – Setup Guide

Bu döküman, projenin adım adım nasıl kurulacağını ayrıntılı şekilde anlatır.  

---

## 1. Donanım Gereksinimleri
- ESP8266 (Wemos D1 Mini)
- DHT11 sıcaklık/nem sensörü
- MQ-135 hava kalitesi sensörü
- Breadboard (Zorunlu değil, stabilizasyon için tercih edebilirsiniz.)
- Jumper kablolar
- USB kablosu

---

## 2. Devre Bağlantıları
- **DHT11:**
  - VCC → 3.3V
  - GND → GND
  - DATA → **D1 (GPIO5)**

- **MQ-135:**
  - VCC → 5V
  - GND → GND
  - AOUT → A0  
  ⚠️ Eğer modülünde 3.3V uyumlu çıkış yoksa, direnç bölücü ile 5V’u 3.3V’a düşürmelisiniz.

📌 Detaylı devre bağlantıları için: [Circuit Diagram](../hardware/circuit-diagram.png)  
📌 Breadboard kurulumu: [Breadboard Setup](../hardware/breadboard-setup.jpg)

---

## 3. Yazılım Gereksinimleri
- Arduino IDE (>=1.8.19) veya PlatformIO
- ESP8266 board paketleri
- Kütüphaneler:
  - DHT sensor library (by Adafruit)
  - ESP Mail Client (by Mobizt)

Kurulum adımları:  
1. Arduino IDE’ye ESP8266 URL’sini ekle:  
[http://arduino.esp8266.com/stable/package_esp8266com_index.json]

2. “Tools > Board > ESP8266” seç.

3. `code/main.ino` dosyasını aç.
---

## 4. Yazılım Konfigürasyonu
Kendi WiFi bilgilerinizi girin:  
```cpp
#define WIFI_SSID "SeninWifiAdin"
#define WIFI_PASSWORD "SeninWifiSifren"

Eğer e-posta bildirimi istiyorsanız, SMTP ayarlarını da düzenleyin.
Gmail kullanıyorsanız Uygulama Parolası kullanmanız gerekir.
