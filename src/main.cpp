#include <M5Unified.h>

// ==== 関数プロトタイプ ====
void drawSetupUI();
void drawPatternUI();
void handleSetupTouch();
void handlePatternTouch();
void updatePattern();
void stopAllStimulus();
void applyToPortC(int chop);
void applyPattern(const char* pattern);
void sendSerialState(String state);
int dutyFromPercent(int v);



const int PWM0_PIN_C = 13;
const int PWM1_PIN_C = 14;

// 【修正】GPIO 1と3はUSBシリアル通信に使われるため、PWMには使用できません。
// 代わりに Port A (GPIO 32, 33) など、空いているピンを使用してください。
const int PWM0_PIN_D = 33; // 元: 3
const int PWM1_PIN_D = 32; // 元: 1

const int PWM0_PIN_E = 27;
const int PWM1_PIN_E = 19;

const int PWM_CH_C_CTRL = 0;
const int PWM_CH_C_CHOP = 8;
const int PWM_CH_D_CTRL = 1;
const int PWM_CH_D_CHOP = 9;
const int PWM_CH_E_CTRL = 2;
const int PWM_CH_E_CHOP = 10;

const int PWM_CTRL_FREQ = 50;
const int PWM_CHOP_FREQ = 1000;
const int PWM_RES = 10;

const int CTRL_VALUE = 20;

// セットアップ用変数
bool setupComplete = false;
int userChopWeak = 0;
int userChopStrong = 0;
bool testingWeak = false;
bool testingStrong = false;

// パターン実行用
bool patternRunning = false;
int currentPatternStep = 0;
unsigned long patternStepStartTime = 0;
bool inInterval = false;

// シリアル送信用（前回状態）
String lastSerialState = "";

// 20パターン
const char* patternSequence[20] = {
  "C-weak", "C-strong",
  "E-weak", "D-strong", 
  "C-weak", "E-strong", 
  "D-weak",
  "D-weak", "D-weak",
  "E-strong", "E-strong",
  "C-strong",
  "E-weak", "E-strong",
  "D-weak",
  "C-weak","C-weak",
  "D-strong",
  "C-strong"
};

const int patternDurations[20] = {
  3200,2700,3400,2500,3100,
  2900,3500,2600,3300,2800,
  3100,2700,3400,2900,3200,
  2600,3500,2800,3000,3300
};

const int patternIntervals[20] = {
  400,250,150,350,50,
  450,200,300,0,100,
  250,400,150,300,100,
  450,50,350,200,0
};

const int totalTrials = 20;

// ========= 関数 =========

int dutyFromPercent(int v) {
  return map(v, 0, 100, 0, (1 << PWM_RES) - 1);
}

// 変化があったときのみ送信
void sendSerialState(String state) {
  if (state != lastSerialState) {
    Serial.println(state);
    lastSerialState = state;
  }
}

// 刺激停止
void stopAllStimulus() {
  ledcWrite(PWM_CH_C_CTRL, 0);
  ledcWrite(PWM_CH_C_CHOP, 0);
  ledcWrite(PWM_CH_D_CTRL, 0);
  ledcWrite(PWM_CH_D_CHOP, 0);
  ledcWrite(PWM_CH_E_CTRL, 0);
  ledcWrite(PWM_CH_E_CHOP, 0);
  testingWeak = false;
  testingStrong = false;

  // ★停止時に none,none を送信
  sendSerialState("none,none");
}

// PortC のテスト出力
void applyToPortC(int chop) {
  stopAllStimulus();

  if (chop == 0) return;

  int ctrlDuty = dutyFromPercent(CTRL_VALUE);
  int chopDuty = dutyFromPercent(chop);

  // 強弱は現在値から判定できるが "portC,weak/strong" 必要ならここで送信
  sendSerialState(String("portC,") + (chop==userChopStrong ? "strong" : "weak"));

  ledcWrite(PWM_CH_C_CTRL, ctrlDuty);
  ledcWrite(PWM_CH_C_CHOP, chopDuty);
}

// パターン適用処理
void applyPattern(const char* pattern) {
  stopAllStimulus();

  char port = pattern[0];         // C / D / E
  bool isStrong = strstr(pattern, "strong") != NULL;

  int chop = isStrong ? userChopStrong : userChopWeak;

  // ★パターン開始時に送信（変化があれば）
  sendSerialState(String("port") + port + "," + (isStrong ? "strong" : "weak"));

  if (chop == 0) return;

  int ctrlDuty = dutyFromPercent(CTRL_VALUE);
  int chopDuty = dutyFromPercent(chop);

  if (port == 'C') {
    ledcWrite(PWM_CH_C_CTRL, ctrlDuty);
    ledcWrite(PWM_CH_C_CHOP, chopDuty);
  } else if (port == 'D') {
    ledcWrite(PWM_CH_D_CTRL, ctrlDuty);
    ledcWrite(PWM_CH_D_CHOP, chopDuty);
  } else if (port == 'E') {
    ledcWrite(PWM_CH_E_CTRL, ctrlDuty);
    ledcWrite(PWM_CH_E_CHOP, chopDuty);
  }
}

// ========= UI（あなたの元コードから変更なし） =========
// ※UI とタッチ処理は省略せず完全コピーしています。
// 長すぎるので説明を省略していますが、ロジックは一切変更なし。

// ------------------------------
// ここから UI コード（あなたの元コードのまま）
// ------------------------------

void drawSetupUI() {
  M5.Display.clear(BLACK);
  M5.Display.setTextColor(WHITE);
  M5.Display.setTextSize(2);
  M5.Display.setCursor(20, 10);
  M5.Display.printf("Setup Port C G%d,%d", PWM0_PIN_C, PWM1_PIN_C);

  M5.Display.setCursor(10, 45);
  M5.Display.println("Weak:");

  M5.Display.fillRect(10, 70, 40, 30, RED);
  M5.Display.setCursor(20, 75);
  M5.Display.println("-");

  M5.Display.fillRect(60, 70, 40, 30, GREEN);
  M5.Display.setCursor(70, 75);
  M5.Display.println("+");

  M5.Display.setTextSize(3);
  M5.Display.setCursor(110, 73);
  M5.Display.printf("%3d", userChopWeak);

  M5.Display.fillRect(180, 70, 130, 30, testingWeak ? ORANGE : BLUE);
  M5.Display.setTextColor(BLACK);
  M5.Display.setTextSize(2);
  M5.Display.setCursor(188, 75);
  M5.Display.println(testingWeak ? "Testing" : "Test Weak");

  M5.Display.setTextColor(WHITE);
  M5.Display.setTextSize(2);
  M5.Display.setCursor(10, 115);
  M5.Display.println("Strong:");

  M5.Display.fillRect(10, 140, 40, 30, RED);
  M5.Display.setCursor(20, 145);
  M5.Display.println("-");

  M5.Display.fillRect(60, 140, 40, 30, GREEN);
  M5.Display.setCursor(70, 145);
  M5.Display.println("+");

  M5.Display.setTextSize(3);
  M5.Display.setCursor(110, 143);
  M5.Display.printf("%3d", userChopStrong);

  M5.Display.fillRect(180, 140, 130, 30, testingStrong ? ORANGE : BLUE);
  M5.Display.setTextColor(BLACK);
  M5.Display.setTextSize(2);
  M5.Display.setCursor(180, 145);
  M5.Display.println(testingStrong ? "Testing" : "Test Strong");

  M5.Display.fillRect(85, 200, 150, 35, YELLOW);
  M5.Display.setTextColor(BLACK);
  M5.Display.setTextSize(3);
  M5.Display.setCursor(120, 207);
  M5.Display.println("DONE");
}

void handleSetupTouch() {
  auto t = M5.Touch.getDetail();
  if (!t.wasPressed()) return;

  int x = t.x;
  int y = t.y;
  bool redraw = false;

  if (y >= 70 && y <= 100) {
    if (x >= 10 && x <= 50) {
      userChopWeak -= 5;
      if (userChopWeak < 0) userChopWeak = 0;
      if (testingWeak) applyToPortC(userChopWeak);
      redraw = true;
    } else if (x >= 60 && x <= 100) {
      userChopWeak += 5;
      if (userChopWeak > 100) userChopWeak = 100;
      if (testingWeak) applyToPortC(userChopWeak);
      redraw = true;
    }
  }

  if (y >= 70 && y <= 100 && x >= 180 && x <= 310) {
    if (testingWeak) {
      stopAllStimulus();
    } else {
      stopAllStimulus();
      testingWeak = true;
      applyToPortC(userChopWeak);
    }
    redraw = true;
  }

  if (y >= 140 && y <= 170) {
    if (x >= 10 && x <= 50) {
      userChopStrong -= 5;
      if (userChopStrong < 0) userChopStrong = 0;
      if (testingStrong) applyToPortC(userChopStrong);
      redraw = true;
    } else if (x >= 60 && x <= 100) {
      userChopStrong += 5;
      if (userChopStrong > 100) userChopStrong = 100;
      if (testingStrong) applyToPortC(userChopStrong);
      redraw = true;
    }
  }

  if (y >= 140 && y <= 170 && x >= 180 && x <= 310) {
    if (testingStrong) {
      stopAllStimulus();
    } else {
      stopAllStimulus();
      testingStrong = true;
      applyToPortC(userChopStrong);
    }
    redraw = true;
  }

  if (y >= 200 && y <= 235 && x >= 85 && x <= 235) {
    stopAllStimulus();
    setupComplete = true;
    drawPatternUI();
    return;
  }

  if (redraw) drawSetupUI();
}

// ========= パターン UI =========
// （同じため省略せず貼る）

void drawPatternUI() {
  M5.Display.clear(BLACK);
  M5.Display.setTextColor(WHITE);
  M5.Display.setTextSize(2);
  M5.Display.setCursor(60, 10);
  M5.Display.println("20 Trials");

  M5.Display.setTextSize(1);
  M5.Display.setCursor(10, 40);
  M5.Display.println("Pattern with consecutive same/diff");
  M5.Display.setCursor(10, 55);
  M5.Display.println("Duration: 2.5-3.5s, Interval: 0-0.5s");

  M5.Display.setTextSize(2);
  M5.Display.setCursor(20, 75);
  M5.Display.printf("Weak:  %d", userChopWeak);
  M5.Display.setCursor(20, 95);
  M5.Display.printf("Strong:%d", userChopStrong);

  if (!patternRunning) {
    M5.Display.fillRect(70, 125, 180, 40, GREEN);
    M5.Display.setTextColor(BLACK);
    M5.Display.setTextSize(3);
    M5.Display.setCursor(100, 135);
    M5.Display.println("START");
  } else {
    M5.Display.fillRect(70, 125, 180, 40, RED);
    M5.Display.setTextColor(BLACK);
    M5.Display.setTextSize(3);
    M5.Display.setCursor(105, 135);
    M5.Display.println("STOP");
  }

  if (patternRunning && currentPatternStep < totalTrials) {
    M5.Display.setTextColor(WHITE);
    M5.Display.setTextSize(2);
    M5.Display.setCursor(15, 175);

    if (inInterval) {
      M5.Display.println("Interval...");
    } else {
      M5.Display.printf("Trial %d/%d: %s",
                        currentPatternStep + 1,
                        totalTrials,
                        patternSequence[currentPatternStep]);
    }

    unsigned long elapsed = millis() - patternStepStartTime;
    unsigned long remaining =
      inInterval
      ? (patternIntervals[currentPatternStep] - elapsed)
      : (patternDurations[currentPatternStep] - elapsed);

    M5.Display.setCursor(15, 200);
    M5.Display.printf("Time: %.1f sec", remaining / 1000.0);

    M5.Display.setTextSize(1);
    M5.Display.setCursor(15, 220);
    M5.Display.printf("Dur:%dms Int:%dms",
                      patternDurations[currentPatternStep],
                      patternIntervals[currentPatternStep]);

  } else if (patternRunning && currentPatternStep >= totalTrials) {
    M5.Display.setTextColor(GREEN);
    M5.Display.setTextSize(3);
    M5.Display.setCursor(60, 185);
    M5.Display.println("Complete!");
  }
}

void handlePatternTouch() {
  auto t = M5.Touch.getDetail();
  if (!t.wasPressed()) return;

  int x = t.x;
  int y = t.y;

  if (y >= 125 && y <= 165 && x >= 70 && x <= 250) {
    if (!patternRunning) {
      patternRunning = true;
      currentPatternStep = 0;
      patternStepStartTime = millis();
      inInterval = false;

      applyPattern(patternSequence[0]);

    } else {
      patternRunning = false;
      currentPatternStep = 0;
      inInterval = false;
      stopAllStimulus();
    }
    drawPatternUI();
  }
}

void updatePattern() {
  if (!patternRunning || currentPatternStep >= totalTrials) {
    if (currentPatternStep >= totalTrials) {
      patternRunning = false;
      currentPatternStep = 0;
      inInterval = false;
      stopAllStimulus();
      drawPatternUI();
    }
    return;
  }

  unsigned long elapsed = millis() - patternStepStartTime;

  if (!inInterval) {
    if (elapsed >= patternDurations[currentPatternStep]) {
      stopAllStimulus();
      inInterval = true;
      patternStepStartTime = millis();
      drawPatternUI();
    }

  } else {
    if (elapsed >= patternIntervals[currentPatternStep]) {
      currentPatternStep++;
      inInterval = false;

      if (currentPatternStep < totalTrials) {
        patternStepStartTime = millis();
        applyPattern(patternSequence[currentPatternStep]);
        drawPatternUI();
      } else {
        stopAllStimulus();
        drawPatternUI();
      }
    }
  }
}

// ========= setup / loop =========

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);

  Serial.begin(115200);
  M5.Display.setBrightness(255);

  ledcSetup(PWM_CH_C_CTRL, PWM_CTRL_FREQ, PWM_RES);
  ledcAttachPin(PWM0_PIN_C, PWM_CH_C_CTRL);

  ledcSetup(PWM_CH_D_CTRL, PWM_CTRL_FREQ, PWM_RES);
  ledcAttachPin(PWM0_PIN_D, PWM_CH_D_CTRL);

  ledcSetup(PWM_CH_E_CTRL, PWM_CTRL_FREQ, PWM_RES);
  ledcAttachPin(PWM0_PIN_E, PWM_CH_E_CTRL);

  ledcSetup(PWM_CH_C_CHOP, PWM_CHOP_FREQ, PWM_RES);
  ledcAttachPin(PWM1_PIN_C, PWM_CH_C_CHOP);

  ledcSetup(PWM_CH_D_CHOP, PWM_CHOP_FREQ, PWM_RES);
  ledcAttachPin(PWM1_PIN_D, PWM_CH_D_CHOP);

  ledcSetup(PWM_CH_E_CHOP, PWM_CHOP_FREQ, PWM_RES);
  ledcAttachPin(PWM1_PIN_E, PWM_CH_E_CHOP);

  drawSetupUI();

  // 初期状態
  sendSerialState("none,none");
}

void loop() {
  M5.update();

  if (!setupComplete) {
    handleSetupTouch();
  } else {
    handlePatternTouch();
    updatePattern();
  }

  delay(10);
}
