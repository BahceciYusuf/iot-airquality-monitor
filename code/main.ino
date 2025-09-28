/**********************
 * IoT Air Quality Monitor - ESP8266 (Wemos D1 mini)
 * DHT11 -> D1 (GPIO5)
 * MQ-135 -> A0 (analog)
 **********************/

 #include "secrets.h"            // WIFI_SSID, WIFI_PASSWORD, SMTP_HOST, SMTP_PORT, SENDER_EMAIL, SENDER_PASS, RECEIVER_EMAIL
 #include <Arduino.h>
 #include <ESP8266WiFi.h>
 #include <DHT.h>
 #include <ESP_Mail_Client.h>
 
 /* --------- PIN & SENSOR --------- */
 #define DHTPIN   D1
 #define DHTTYPE  DHT11
 DHT dht(DHTPIN, DHTTYPE);
 
 /* --------- THRESHOLDS (isteğe göre düzenleyin) --------- */
 int   KRITIK_HAVA_KALITESI = 200;   // MQ-135 ham değer eşiği (kalibrasyon sonrası güncelleyin)
 float KRITIK_SICAKLIK      = 30.0;  // °C
 float KRITIK_NEM           = 70.0;  // %RH
 
 /* --------- TIMING --------- */
 const unsigned long EMAIL_BEKLEME_SURESI = 300000UL; // 5 dakika (ms)
 const unsigned long SICAKLIK_OKUMA_ARALIGI = 1200UL; // DHT için min ~1s
 const unsigned long MQ_OKUMA_ARALIGI       = 400UL;
 
 unsigned long sonEmailZamani   = 0;
 unsigned long sonDHTOkumaMs    = 0;
 unsigned long sonMQOkumaMs     = 0;
 unsigned long bootMs           = 0;   // warm-up timer
 
 /* --------- STATE --------- */
 float  sonSicaklik = NAN;
 float  sonNem      = NAN;
 int    sonMQ       = 0;
 
 /* --------- SMTP --------- */
 SMTPSession smtp;
 
 /* Yardımcı: Wi-Fi durumunu koru */
 void ensureWiFi() {
   if (WiFi.status() == WL_CONNECTED) return;
 
   Serial.print("[WiFi] Baglaniyor ");
   WiFi.mode(WIFI_STA);
   WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
 
   unsigned long t0 = millis();
   while (WiFi.status() != WL_CONNECTED && millis() - t0 < 15000UL) {
     delay(500);
     Serial.print(".");
   }
   Serial.println(WiFi.status() == WL_CONNECTED ? " OK" : " FAIL");
 }
 
 /* Yardımcı: DHT okumalarını sağlamlaştır (NaN'a karşı 2. deneme) */
 float readDHT_T() {
   float t = dht.readTemperature();         // °C
   if (isnan(t)) { delay(1500); t = dht.readTemperature(); }
   return t;
 }
 float readDHT_H() {
   float h = dht.readHumidity();            // %RH
   if (isnan(h)) { delay(1500); h = dht.readHumidity(); }
   return h;
 }
 
 /* Yardımcı: Warm-up tamam mı? (MQ-135 için) */
 bool warmedUp() {
   return (millis() - bootMs) > 60000UL;    // 60 sn
 }
 
 /* E-posta gönderimi */
 void emailGonder(float temp, float hum, int airQuality) {
   if (millis() - sonEmailZamani < EMAIL_BEKLEME_SURESI) {
     Serial.println("[MAIL] Rate-limit: Bekleniyor.");
     return;
   }
 
   Session_Config config;
   config.server.host_name = SMTP_HOST;
   config.server.port      = SMTP_PORT;
   config.login.email      = SENDER_EMAIL;
   config.login.password   = SENDER_PASS;
 
   // STARTTLS (587) için:
   config.secure.startTLS  = true;
   // SSL (465) kullanacaksanız:
   // config.secure.startTLS = false;
 
   // (Opsiyonel) Saat eşitleme - bazı SMTP sunucuları için gerekli olabilir
   config.time.ntp_server  = "pool.ntp.org";
 
   Serial.print("[MAIL] Sunucuya baglaniyor...");
   if (!smtp.connect(&config)) {
     Serial.printf(" HATA: %s\n", smtp.errorReason().c_str());
     return;
   }
   Serial.println(" OK");
 
   SMTP_Message message;
   message.sender.name  = "Air Quality Monitor";
   message.sender.email = SENDER_EMAIL;
   message.subject      = "Uyari: Esik Deger Asildi";
   message.addRecipient("Alici", RECEIVER_EMAIL);
 
   String body;
   body += "MQ-135: " + String(airQuality) + "\n";
   body += "Sicaklik: " + String(temp, 1) + " C\n";
   body += "Nem: " + String(hum, 1) + " %\n";
   message.text.content = body.c_str();
 
   if (!MailClient.sendMail(&smtp, &message)) {
     Serial.printf("[MAIL] Gonderilemedi: %s\n", smtp.errorReason().c_str());
   } else {
     Serial.println("[MAIL] Gonderildi.");
     sonEmailZamani = millis();
   }
 
   smtp.closeSession();
 }
 
 void setup() {
   Serial.begin(115200);
   delay(200);
   Serial.println("\n[BOOT] IoT Air Quality Monitor");
 
   dht.begin();
 
   ensureWiFi();
   bootMs = millis();
 
   Serial.println("[INFO] DHT11 -> D1 (GPIO5), MQ-135 -> A0");
   Serial.println("[INFO] Warm-up: 60-120 sn arasi MQ-135 degerleri kararsiz olabilir.");
 }
 
 void loop() {
   ensureWiFi();
 
   unsigned long now = millis();
 
   /* MQ-135 okuma */
   if (now - sonMQOkumaMs >= MQ_OKUMA_ARALIGI) {
     sonMQOkumaMs = now;
     int mq = analogRead(A0);  // 0..1023
     if (mq >= 1023) {
       Serial.println("[MQ] A0 saturasyon! Cikis >3.2V, bolucu/besleme kontrol edin.");
     } else {
       sonMQ = mq;
       Serial.print("[MQ] ");
       Serial.println(sonMQ);
     }
   }
 
   /* DHT okuma */
   if (now - sonDHTOkumaMs >= SICAKLIK_OKUMA_ARALIGI) {
     sonDHTOkumaMs = now;
     float t = readDHT_T();
     float h = readDHT_H();
     if (!isnan(t) && !isnan(h)) {
       sonSicaklik = t;
       sonNem      = h;
       Serial.print("[DHT] T: "); Serial.print(sonSicaklik, 1); Serial.print(" C, H: ");
       Serial.print(sonNem, 1); Serial.println(" %");
     } else {
       Serial.println("[DHT] NaN okuma! Pin/tip/pull-up/zamanlama kontrol.");
     }
   }
 
   /* Alarm kontrol (warm-up sonrasi) */
   if (warmedUp()) {
     bool kritik = false;
     String sebep;
 
     if (sonMQ >= 1023) {
       kritik = true;
       sebep += "MQ-135 saturasyon; ";
     } else if (sonMQ > KRITIK_HAVA_KALITESI) {
       kritik = true;
       sebep += "Hava kalitesi esigi; ";
     }
     if (isfinite(sonSicaklik) && sonSicaklik > KRITIK_SICAKLIK) {
       kritik = true;
       sebep += "Sicaklik esigi; ";
     }
     if (isfinite(sonNem) && sonNem > KRITIK_NEM) {
       kritik = true;
       sebep += "Nem esigi; ";
     }
 
     if (kritik) {
       Serial.print("[ALARM] Tetiklendi -> "); Serial.println(sebep);
       emailGonder(sonSicaklik, sonNem, sonMQ);
     }
   } else {
     // İlk dakika bilgi mesajı
     if ((now - bootMs) < 5000UL && (now - bootMs) > 0) {
       // sadece ilk saniyelerde ek bilgi gösterelim
       Serial.println("[INFO] Warm-up devam ediyor...");
     }
   }
 
   delay(50);
 }