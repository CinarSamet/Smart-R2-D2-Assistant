#ifndef ROBOTFACE_H
#define ROBOTFACE_H

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

#define WHITE SH110X_WHITE
#define BLACK SH110X_BLACK

enum EyeMode { MODE_NORMAL, MODE_ANGRY, MODE_HAPPY, MODE_SLEEP, MODE_SCAN };

class RobotFace {
  private:
    Adafruit_SH1106G* display;
    int _mosi, _clk, _dc, _res, _cs;
    
    // Göz Konumları (Referans koddaki gibi)
    int leftEyeX = 34;
    int rightEyeX = 94;
    int eyeY = 26;      // Margin için biraz yukarıda
    int eyeSize = 20;
    int pupilSize = 8;
    
    EyeMode currentMode = MODE_NORMAL;
    EyeMode defaultMode = MODE_NORMAL;
    
    String currentStatus = "";
    
    // Zamanlayıcılar
    unsigned long lastUpdate = 0;
    unsigned long lastBlink = 0;
    unsigned long lastMove = 0;
    unsigned long moveInterval = 1000; // İlk hareket bekleme süresi
    
    // Göz Hedefleri
    int currentLookX = 0;
    int currentLookY = 0;
    
    bool isBlinking = false;

    // Süreli İfade Değişkenleri
    bool isTimedExpression = false;
    unsigned long expressionStartTime = 0;
    unsigned long expressionDuration = 0;

  public:
    RobotFace(int mosi, int clk, int dc, int res, int cs) {
      _mosi = mosi; _clk = clk; _dc = dc; _res = res; _cs = cs;
      display = new Adafruit_SH1106G(128, 64, _mosi, _clk, _dc, _res, _cs);
    }

    void begin() {
      display->begin(0, true);
      display->clearDisplay();
      playStartupAnimation();
      setStatus("SISTEM HAZIR");
      update();
    }

    // --- İFADE GÖSTERME FONKSİYONU ---
    void showExpression(EyeMode mode, int duration_ms, String text = "") {
      currentMode = mode;
      isTimedExpression = true;
      expressionDuration = duration_ms;
      expressionStartTime = millis();
      
      if (text != "") setStatus(text);
      else updateStatusText(mode);
      
      draw();
    }

    void setMode(EyeMode mode) {
      currentMode = mode;
      defaultMode = mode;
      isTimedExpression = false;
      updateStatusText(mode);
    }

    void setStatus(String text) {
      currentStatus = text;
    }

    // --- ANA GÜNCELLEME DÖNGÜSÜ ---
    void update() {
      unsigned long currentMillis = millis();

      // Süreli mod kontrolü
      if (isTimedExpression && (currentMillis - expressionStartTime > expressionDuration)) {
        isTimedExpression = false;
        currentMode = defaultMode;
        updateStatusText(currentMode);
      }

      // FPS Limiti (Gereksiz işlemci yormasın)
      if (currentMillis - lastUpdate < 33) return;
      lastUpdate = currentMillis;

      // --- GÖZ KIRPMA MANTIĞI (Referans koda sadık) ---
      if (currentMode == MODE_NORMAL && !isBlinking) {
        // Her döngüde çok düşük ihtimalle göz kırp (Referanstaki random(0,10)<3 mantığına benzer)
        if (random(0, 1000) < 15) {
          isBlinking = true;
          lastBlink = currentMillis;
        }
      }
      
      // Göz kırpma süresi (150ms kapalı kalsın)
      if (isBlinking && currentMillis - lastBlink > 150) isBlinking = false;

      // Göz Hareketlerini Hesapla
      handleEyeMovement(currentMillis);
      
      // Çiz
      draw();
    }

  private:
    void updateStatusText(EyeMode mode) {
      switch(mode) {
        case MODE_ANGRY: setStatus("DIKKAT!"); break;
        case MODE_HAPPY: setStatus("MUTLU"); break;
        case MODE_SCAN:  setStatus("TARANIYOR..."); break;
        case MODE_SLEEP: setStatus("UYKU MODU"); break;
        default:         setStatus("SISTEM HAZIR"); break;
      }
    }

    // --- HIZ AYARI BURADA YAPILDI ---
    void handleEyeMovement(unsigned long currentMillis) {
      
      if (currentMode == MODE_NORMAL) {
        // Referans Koddaki Mantık: delay(random(500, 1500));
        // Yani: Son hareketten bu yana "moveInterval" kadar zaman geçti mi?
        
        if (currentMillis - lastMove > moveInterval) {
            // EĞER SÜRE DOLDUYSA YENİ KONUM SEÇ
            
            // Referans koddaki değerler: random(-10, 10) ve random(-8, 8)
            currentLookX = random(-10, 10);
            currentLookY = random(-8, 8);
            
            // Bir sonraki hareket için rastgele bekleme süresi belirle
            // Referans koddaki: delay(random(500, 1500))
            moveInterval = random(500, 1500);
            
            lastMove = currentMillis; // Zamanlayıcıyı sıfırla
        }
        // EĞER SÜRE DOLMADIYSA: Hiçbir şey yapma, gözler son pozisyonda sabit kalsın.
      }
      else if (currentMode == MODE_ANGRY) {
         // Kızgın modda titreme (Referans kodundaki jitter)
         currentLookX = random(-2, 3);
         currentLookY = random(-2, 3);
      }
    }

    void draw() {
      display->clearDisplay();
      
      if (currentMode == MODE_SLEEP) {
        drawSleep();
      } else if (isBlinking) {
        drawBlink();
      } else {
        int style = 0;
        if (currentMode == MODE_ANGRY) style = 1;
        if (currentMode == MODE_HAPPY) style = 2;
        
        int offsetX = currentLookX;
        if (currentMode == MODE_SCAN) offsetX = (int)(sin(millis() / 200.0) * 15);

        drawSingleEye(leftEyeX, eyeY, offsetX, currentLookY, style, true);
        drawSingleEye(rightEyeX, eyeY, offsetX, currentLookY, style, false);
      }
      drawStatusBar();
      display->display();
    }

    // Referans koddaki çizim fonksiyonunun aynısı
    void drawSingleEye(int x, int y, int offX, int offY, int style, bool isLeft) {
      display->fillCircle(x, y, eyeSize, WHITE);
      int pSize = (style == 1) ? pupilSize - 3 : pupilSize;
      display->fillCircle(x + offX, y + offY, pSize, BLACK);
      
      if (style != 1) display->fillCircle(x + offX - 3, y + offY - 3, 2, WHITE);

      display->setTextColor(BLACK);
      if (style == 1) { // ANGRY
        if (isLeft) display->fillTriangle(x-eyeSize-5, y-eyeSize-5, x+eyeSize+5, y-eyeSize-5, x+eyeSize, y+5, BLACK);
        else        display->fillTriangle(x-eyeSize-5, y-eyeSize-5, x+eyeSize+5, y-eyeSize-5, x-eyeSize, y+5, BLACK);
      }
      else if (style == 2) { // HAPPY
        display->fillCircle(x, y + eyeSize + 5, eyeSize, BLACK);
      }
    }

    void drawBlink() {
       display->drawLine(leftEyeX - eyeSize, eyeY, leftEyeX + eyeSize, eyeY, WHITE);
       display->drawLine(rightEyeX - eyeSize, eyeY, rightEyeX + eyeSize, eyeY, WHITE);
       drawStatusBar();
    }

    void drawSleep() {
       display->drawLine(leftEyeX-eyeSize, eyeY+5, leftEyeX+eyeSize, eyeY+5, WHITE);
       display->drawLine(rightEyeX-eyeSize, eyeY+5, rightEyeX+eyeSize, eyeY+5, WHITE);
       if ((millis() / 1000) % 2 == 0) {
         display->drawLine(leftEyeX-eyeSize+5, eyeY+6, leftEyeX+eyeSize-5, eyeY+6, WHITE);
         display->drawLine(rightEyeX-eyeSize+5, eyeY+6, rightEyeX+eyeSize-5, eyeY+6, WHITE);
       }
       drawStatusBar();
    }

    void drawStatusBar() {
      if (currentStatus == "") return;
      int boxWidth = currentStatus.length() * 6 + 10;
      int boxX = (128 - boxWidth) / 2;
      int boxY = 64 - 12;
      display->fillRect(boxX, boxY, boxWidth, 12, BLACK);
      display->drawRect(boxX, boxY, boxWidth, 12, WHITE);
      display->setTextColor(WHITE);
      display->setTextSize(1);
      display->setCursor(boxX + 5, boxY + 2);
      display->print(currentStatus);
    }
    
    void playStartupAnimation() {
      int targetBoxSize = 60;
      while (targetBoxSize > 10) {
        display->clearDisplay();
        display->drawRect(0,0, 128, 64, WHITE);
        display->drawFastHLine(0, 32, 128, WHITE);
        display->drawFastVLine(64, 0, 64, WHITE);
        display->setTextSize(1); display->setTextColor(WHITE);
        display->setCursor(4, 4); display->print("CALIBRATING...");
        display->setCursor(90, 54); display->print("[T-16]");

        int tlX = 64 - (targetBoxSize / 2);
        int tlY = 32 - (targetBoxSize / 2);
        display->drawRect(tlX, tlY, targetBoxSize, targetBoxSize, WHITE);
        display->display();
        
        targetBoxSize -= 2;
        delay(30);
      }
      display->fillCircle(64, 32, 8, WHITE);
      display->display();
      delay(200);
      display->invertDisplay(true);
      delay(100);
      display->invertDisplay(false);
      delay(500);
      display->clearDisplay();
    }
};

#endif
