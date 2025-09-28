#pragma once
#define WIFI_SSID       "WiFi_ADIN"
#define WIFI_PASSWORD   "WiFi_SIFREN"

#define SMTP_HOST       "smtp.gmail.com"
#define SMTP_PORT       587            // STARTTLS için 587, SSL için 465

#define SENDER_EMAIL    "seninmailin@gmail.com"
#define SENDER_PASS     "uygulama_parolasi"  // Gmail ise App Password kullan
#define RECEIVER_EMAIL  "alarm_gidecek_kisi@mail.com"

// E-posta rate limit (ms). Örn: 10 dakika
#define EMAIL_BEKLEME_SURESI  (10UL * 60UL * 1000UL)