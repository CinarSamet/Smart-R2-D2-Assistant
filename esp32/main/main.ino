#include <WiFi.h>
#include <HTTPClient.h>
#include "driver/i2s.h"
#include "RobotFace.h"
#include <WiFiManager.h>
#include "r2d2_ses.h"
#include <ArduinoOTA.h>
// wifi
const char* serverUrl = "http://192.168.1.106:5001/upload";

// OLED 
RobotFace face(11, 12, 9, 8, 10);

// I2S MIC
#define MIC_I2S_PORT I2S_NUM_0
#define MIC_SCK 5
#define MIC_WS 4
#define MIC_SD 6

// I2S SPK
#define SPK_I2S_PORT I2S_NUM_1
#define SPK_BCLK 15
#define SPK_LRC 16
#define SPK_DIN 7

#define SAMPLE_RATE 16000
#define BUF_LEN 512
#define RECORD_SECONDS 4

// -------- DOKUNMATIK SENSÖR --------
#define TOUCH_PIN 13 // <-- DİKKAT: Pin 1 yerine 13'ü kullandık. Sensör kablosunu 13'e tak!

// IDLE (Bekleme) durumunu ekledik
enum State { IDLE, RECORDING, UPLOADING, ERROR_STATE };
State state = IDLE;

int16_t micBuf[BUF_LEN];
uint8_t* audioBuf = NULL;
size_t recorded = 0;
const int totalBytes = SAMPLE_RATE * RECORD_SECONDS * 2;

HTTPClient http;

void playPROGMEM(const unsigned char* audio_data, size_t data_size) {
  if(data_size <= 44) return; 
  
  int offset = 44;
  int bytes_left = data_size - 44;
  const int CHUNK_SIZE = 1024; 
  
  while(bytes_left > 0) {
    int to_write = (bytes_left > CHUNK_SIZE) ? CHUNK_SIZE : bytes_left;
    size_t bw;
    
    i2s_write(SPK_I2S_PORT, audio_data + offset, to_write, &bw, portMAX_DELAY);
    
    offset += to_write;
    bytes_left -= to_write;
    
    vTaskDelay(1); 
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(TOUCH_PIN, INPUT);
  
  face.begin();
  face.setMode(MODE_NORMAL);
   // SPEAKER CONFIG
  i2s_config_t spk_cfg = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
    .sample_rate = SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 4,
    .dma_buf_len = BUF_LEN,
    .use_apll = true,
    .tx_desc_auto_clear = true,
    .fixed_mclk = 0
  };
  i2s_pin_config_t spk_pins = { .bck_io_num=SPK_BCLK, .ws_io_num=SPK_LRC, .data_out_num=SPK_DIN, .data_in_num=-1 };
  i2s_driver_install(SPK_I2S_PORT, &spk_cfg, 0, NULL);
  i2s_set_pin(SPK_I2S_PORT, &spk_pins);
  i2s_set_clk(SPK_I2S_PORT, SAMPLE_RATE, I2S_BITS_PER_SAMPLE_16BIT, I2S_CHANNEL_STEREO);

  playPROGMEM(r2d2_ses_4, sizeof(r2d2_ses_4)); 
  delay(500);

  face.setStatus("WIFI BEKLIYOR");

  face.setStatus("WIFI BEKLIYOR");
  Serial.println("Kayitli WiFi araniyor veya Kurulum Modu baslatiliyor...");

  WiFiManager wm;
  // Wifi isim ve şifre burada belirlenir
  bool res = wm.autoConnect("R2D2_Kurulum", "12345678");

  if(!res) {
    Serial.println("Baglanti kurulamadi, sistem yeniden baslatiliyor...");
    face.setStatus("BAGLANTI HATASI");
    delay(3000);
    ESP.restart(); 
  }

  Serial.println("WiFi Baglandi!");
  face.setStatus("BELLEK TESTI");
  face.update();

  audioBuf = (uint8_t*) malloc(totalBytes);
  if (audioBuf == NULL) {
    Serial.println("KRITIK HATA: Yeterli RAM yok!");
    face.setStatus("RAM HATASI");
    state = ERROR_STATE;
    return;
  }

  // MIC CONFIG
  i2s_config_t mic_cfg = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 4,
    .dma_buf_len = BUF_LEN,
    .use_apll = false,
    .tx_desc_auto_clear = false,
    .fixed_mclk = 0
  };
  i2s_pin_config_t mic_pins = { .bck_io_num=MIC_SCK, .ws_io_num=MIC_WS, .data_out_num=-1, .data_in_num=MIC_SD };
  i2s_driver_install(MIC_I2S_PORT, &mic_cfg, 0, NULL);
  i2s_set_pin(MIC_I2S_PORT, &mic_pins);

 

  face.setStatus("DOKUN BANA");
  Serial.println("Sistem hazir, dokunulmasi bekleniyor.");
  
  i2s_stop(MIC_I2S_PORT);
  //arduino isim ve şifre 
  ArduinoOTA.setHostname("R2D2-Robot");
  ArduinoOTA.setPassword("admin123");   
  
  ArduinoOTA.onStart([]() { Serial.println("Guncelleme basliyor..."); });
  ArduinoOTA.onEnd([]() { Serial.println("\nBitti!"); });
  
  ArduinoOTA.begin();
}

void loop() {
  ArduinoOTA.handle();
  face.update();
  if(state == ERROR_STATE) return;

  if (state == IDLE) {
    if (digitalRead(TOUCH_PIN) == HIGH) { 

      playPROGMEM(r2d2_ses_3, sizeof(r2d2_ses_3));

      Serial.println("Dokunma algilandi! Dinleme basliyor...");

      state = RECORDING;
      recorded = 0;
      memset(audioBuf, 0, totalBytes); 
      face.setStatus("DINLIYORUM");
      
      i2s_start(MIC_I2S_PORT);
      size_t r;
      uint8_t trash[1024];
      i2s_read(MIC_I2S_PORT, trash, sizeof(trash), &r, 100); 
      delay(200); 
    }
  }

  else if(state == RECORDING){
    size_t r=0;
    i2s_read(MIC_I2S_PORT, micBuf, sizeof(micBuf), &r, 100/portTICK_PERIOD_MS);
    if(r==0){ delay(20); return; } 
    
    if(recorded + r <= totalBytes){
      memcpy(audioBuf + recorded, micBuf, r);
      recorded += r;
    } else {
      state = UPLOADING;
      face.setMode(MODE_SCAN);
      face.setStatus("YUKLENIYOR");
      Serial.println("Kayit bitti, sunucuya gonderiliyor...");
      playPROGMEM(r2d2_ses_4, sizeof(r2d2_ses_4));

      i2s_stop(MIC_I2S_PORT); 
    }
  }

  else if(state == UPLOADING){
    http.begin(serverUrl);
    http.addHeader("Content-Type","application/octet-stream");
    
    http.setTimeout(15000); 

    int code = http.POST(audioBuf, totalBytes);
    Serial.print("Sunucu yaniti: "); Serial.println(code);

    if(code == HTTP_CODE_OK){
      face.setMode(MODE_HAPPY);
      face.setStatus("KONUSUYOR");
      i2s_zero_dma_buffer(SPK_I2S_PORT);

      playPROGMEM(r2d2_ses_2, sizeof(r2d2_ses_2));

      WiFiClient* s = http.getStreamPtr();
      
      int total_bytes = http.getSize(); 
      int bytes_downloaded = 0;
      
      const int CHUNK_SIZE = 512; 
      uint8_t netBuf[CHUNK_SIZE];        
      int16_t stereoBuf[CHUNK_SIZE];     
      int leftover_bytes = 0; 

      while(http.connected() || s->available()){
        if(total_bytes > 0 && bytes_downloaded >= total_bytes) break; 

        if(s->available() == 0){ 
          delay(1); 
          continue; 
        }

        int bytes_to_read = CHUNK_SIZE - leftover_bytes;
        if (total_bytes > 0 && (total_bytes - bytes_downloaded) < bytes_to_read) {
            bytes_to_read = total_bytes - bytes_downloaded; 
        }

        int len = s->read(netBuf + leftover_bytes, bytes_to_read);
        
        if(len > 0){
          bytes_downloaded += len;
          int total_len = len + leftover_bytes;
          
          int samples = total_len / 2; 
          leftover_bytes = total_len % 2; 

          int16_t* pcm = (int16_t*)netBuf;

          for(int i = 0; i < samples; i++){
            int16_t current_sample = pcm[i] * 0.8; 
            stereoBuf[i * 2] = current_sample;     
            stereoBuf[i * 2 + 1] = current_sample; 
          }

          size_t bytes_written;
          i2s_write(SPK_I2S_PORT, stereoBuf, samples * 4, &bytes_written, portMAX_DELAY);

          if (leftover_bytes == 1) {
            netBuf[0] = netBuf[total_len - 1];
          }
        }
      }

      size_t written;
      uint8_t silenceBuf[1024] = {0};
      for(int i = 0; i < 4; i++) i2s_write(SPK_I2S_PORT, silenceBuf, sizeof(silenceBuf), &written, portMAX_DELAY);
      i2s_zero_dma_buffer(SPK_I2S_PORT);

      Serial.println("Konuşma bitti.");

    } else {
      Serial.println("HATA: Sunucuya veri gonderilemedi.");
      face.setStatus("SUNUCU HATASI");
      delay(2000);
    }

    http.end();
    
    recorded = 0;
    state = IDLE; 
    face.setMode(MODE_NORMAL);
    face.setStatus("DOKUN BANA");
  }
}