# 🤖 Smart R2-D2 Assistant: Edge-to-Cloud AI Robotics

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](https://opensource.org/licenses/MIT)
[![Hardware License](https://img.shields.io/badge/Hardware-CC%20BY--NC%204.0-lightgrey.svg)](https://creativecommons.org/licenses/by-nc/4.0/)
[![C++](https://img.shields.io/badge/C++-ESP32-00599C?logo=c%2B%2B)](https://isocpp.org/)
[![Python](https://img.shields.io/badge/Python-Flask-3776AB?logo=python)](https://www.python.org/)

Bu proje, ikonik R2-D2 karakterinin 3 boyutlu yazıcı ile üretilmiş özel tasarımıyla, yapay zeka tabanlı gerçek zamanlı bir sesli asistanı bir araya getiren uçtan uca (end-to-end) bir robotik projesidir. Sistem, uç cihazda (ESP32) donanım ve sensör yönetimini sağlarken, ağır işlem gücü gerektiren yapay zeka yükünü yerel bir Python sunucusuna devreder.

## 🌟 Öne Çıkan Özellikler

*   **Gerçek Zamanlı İletişim (Edge-to-Cloud):** ESP32 üzerinden I2S protokolüyle mikrofondan alınan RAW PCM ses verisinin HTTP üzerinden yerel sunucuya aktarımı.
*   **Speech-to-Text (STT):** OpenAI Whisper (Turbo) modeli ile yüksek doğruluklu Türkçe ses tanıma.
*   **Yapay Zeka (LLM):** Google Gemini API entegrasyonu ile bağlama duyarlı, R2-D2 karakterine uygun sohbet ve yanıt üretimi.
*   **Text-to-Speech (TTS):** Edge-TTS altyapısı ile akıcı ve doğal ses sentezi.
*   **Dinamik UI:** `RobotFace` kütüphanesi kullanılarak OLED ekran üzerinde durum tabanlı (Dinliyor, Düşünüyor, Konuşuyor vb.) dinamik yüz animasyonları.
*   **Kablosuz Yönetim:** WiFiManager ile ağ yapılandırması ve ArduinoOTA ile uzaktan kablosuz kod güncelleme desteği.

## 🏗️ Sistem Mimarisi

Sistem **Uç (Edge)** ve **Sunucu (Server)** olmak üzere iki asenkron birimden oluşur:

1.  **ESP32 (Uç Cihaz):** Dokunmatik sensör ile uyandırılır. I2S mikrofondan sesi kaydeder, RAW PCM formatında sunucuya iletir. Animasyonları yönetir ve sunucudan gelen yanıtı I2S amplifikatörü üzerinden çalar.
2.  **Flask Sunucusu (Python):** 
    *   `FFmpeg` ile sesi işlenebilir WAV formatına getirir.
    *   `Whisper` ile sesi metne dönüştürür.
    *   `Gemini LLM` ile metni analiz eder ve yanıt üretir.
    *   `Edge-TTS` ile yanıtı sese çevirip, tekrar RAW PCM'e dönüştürerek cihaza geri gönderir.

## 🛠️ Donanım ve 3B Tasarım

R2-D2'nin dış kasası ve iç şasesi tamamen 3B yazıcılar için optimize edilerek sıfırdan tasarlanmıştır.

👉 **[3B Model ve STL Dosyaları (Thingiverse) - Çok Yakında!]**

**Kullanılan Temel Elektronik Bileşenler:**
*   **Mikrodenetleyici:** ESP32 Development Board
*   **Mikrofon:** INMP441 (I2S MEMS)
*   **Ses Çıkışı:** MAX98357A (I2S Class D Amplifier) + Hoparlör
*   **Ekran:** I2C OLED Ekran
*   **Sensörler:** Kapasitif Dokunmatik Sensör (Uyandırma için)

## 📁 Proje Yapısı

```text
├── server/
│   ├── app.py                 # Flask sunucu ana uygulaması
│   ├── config/
│   │   └── settings.json      # API, TTS ve Sunucu ayarları
│   └── requirements.txt       # Python bağımlılıkları
├── esp32/
│   ├── main.ino               # ESP32 ana kaynak kodu
│   ├── RobotFace.h            # OLED animasyon kütüphanesi
│   └── r2d2_ses.h             # PROGMEM üzerindeki sistem sesleri
├── .gitignore
└── README.md
```

## 🚀 Kurulum ve Kullanım

### 1. Sunucu (Python Backend) Kurulumu
Projenin çalışması için bilgisayarınızda **FFmpeg** kurulu olmalı ve sistem PATH'ine eklenmiş olmalıdır.

1. Depoyu bilgisayarınıza indirin ve klasöre girin:
git clone [https://github.com/CinarSamet/Smart-R2-D2-Assistant.git](https://github.com/CinarSamet/Smart-R2-D2-Assistant.git)
cd Smart-R2-D2-Assistant/server

2. Gerekli Python kütüphanelerini yükleyin:
pip install -r requirements.txt

3. Çevresel değişken olarak Gemini API anahtarınızı ekleyin:
export GEMINI_API_KEY="sizin_api_anahtariniz"

4. Sunucuyu başlatın:
python app.py

### 2. Donanım (ESP32) Kurulumu
1. Arduino IDE'yi açın ve gerekli kütüphaneleri (`WiFiManager`, `ArduinoOTA` vb.) yükleyin.
2. `esp32/main.ino` dosyasındaki `serverUrl` değişkenini sunucunun çalıştığı bilgisayarın yerel IP adresiyle güncelleyin (Örn: `[http://192.168.1.](http://192.168.1.)X:5001/upload`).
3. Kodu ESP32'ye yükleyin. 
4. Cihaz ilk açılışta `R2D2_Kurulum` adında bir Wi-Fi ağı oluşturacaktır. Bu ağa bağlanarak yerel internet bilgilerinizi cihaza tanımlayın.

## 🗺️ Yol Haritası (Roadmap)
- [x] I2S ses boru hattının ESP32 üzerinde kurulması.
- [x] Whisper, Gemini ve TTS entegrasyonu ile kapalı döngü iletişimin sağlanması.
- [x] Donanım State Machine yapısının ve OLED arayüzün oluşturulması.
- [ ] **Dockerization:** Tüm Flask/AI sunucu altyapısının Docker container'ları içine alınarak ortam bağımsız deploy edilebilir hale getirilmesi.
- [ ] **Sensör Füzyonu:** IMU verilerinin ve mesafe sensörlerinin entegre edilerek otonom hareket kabiliyeti eklenmesi.

## 📄 Lisans
Bu depodaki yazılım kodları (ESP32 ve Python) **MIT License** altında lisanslanmıştır. Daha fazla detay için `LICENSE` dosyasına bakabilirsiniz.

Projenin 3B donanım tasarımları (STL dosyaları) **Creative Commons Attribution-NonCommercial 4.0 International (CC BY-NC 4.0)** lisansına tabidir. Ticari amaçlarla çoğaltılması ve satılması yasaktır. Kişisel projelerinizde özgürce kullanabilir ve geliştirebilirsiniz.
