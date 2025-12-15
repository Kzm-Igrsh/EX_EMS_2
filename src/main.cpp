#include <M5Unified.h>

const int PWM0_PIN_C = 13;
const int PWM1_PIN_C = 14;

const int PWM0_PIN_D = 3;
const int PWM1_PIN_D = 1;

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
int userChopWeak = 0;       // Weak初期値を0に変更
int userChopStrong = 0;     // Strong初期値を0に変更
bool testingWeak = false;   // Weakテスト中フラグ
bool testingStrong = false; // Strongテスト中フラグ

// メイン動作用変数
enum PortPos { PORT_C, PORT_D, PORT_E };
PortPos currentPort = PORT_C;
bool stimActive = false;
int chopValue = 50;

// ========== 関数の前方宣言 ==========
void drawUI();
void drawSetupUI();

int dutyFromPercent(int v) {
  return map(v, 0, 100, 0, (1 << PWM_RES) - 1);
}

void stopAllStimulus() {
  ledcWrite(PWM_CH_C_CTRL, 0);
  ledcWrite(PWM_CH_C_CHOP, 0);
  ledcWrite(PWM_CH_D_CTRL, 0);
  ledcWrite(PWM_CH_D_CHOP, 0);
  ledcWrite(PWM_CH_E_CTRL, 0);
  ledcWrite(PWM_CH_E_CHOP, 0);
  stimActive = false;
  testingWeak = false;
  testingStrong = false;
}

// Port Cにのみ刺激を出力
void applyToPortC(int chop) {
  stopAllStimulus();
  
  int ctrlDuty = dutyFromPercent(CTRL_VALUE);
  int chopDuty = dutyFromPercent(chop);
  
  ledcWrite(PWM_CH_C_CTRL, ctrlDuty);
  ledcWrite(PWM_CH_C_CHOP, chopDuty);
}

void applyStimulus() {
  stopAllStimulus();

  int ctrlDuty = dutyFromPercent(CTRL_VALUE);
  int chopDuty = dutyFromPercent(chopValue);

  if (currentPort == PORT_C) {
    ledcWrite(PWM_CH_C_CTRL, ctrlDuty);
    ledcWrite(PWM_CH_C_CHOP, chopDuty);
  } else if (currentPort == PORT_D) {
    ledcWrite(PWM_CH_D_CTRL, ctrlDuty);
    ledcWrite(PWM_CH_D_CHOP, chopDuty);
  } else {
    ledcWrite(PWM_CH_E_CTRL, ctrlDuty);
    ledcWrite(PWM_CH_E_CHOP, chopDuty);
  }

  stimActive = true;
}

// ========== セットアップ画面（横並びレイアウト） ==========
void drawSetupUI() {
  M5.Display.clear(BLACK);
  M5.Display.setTextColor(WHITE);
  M5.Display.setTextSize(2);
  M5.Display.setCursor(20, 10);
  M5.Display.printf("Setup Port C G%d,%d", PWM0_PIN_C, PWM1_PIN_C);

  // Weak調整エリア
  M5.Display.setTextSize(2);
  M5.Display.setCursor(10, 45);
  M5.Display.println("Weak:");

  // Weak調整ボタン（-/+）
  M5.Display.fillRect(10, 70, 40, 30, RED);
  M5.Display.setTextColor(WHITE);
  M5.Display.setTextSize(2);
  M5.Display.setCursor(20, 75);
  M5.Display.println("-");

  M5.Display.fillRect(60, 70, 40, 30, GREEN);
  M5.Display.setCursor(70, 75);
  M5.Display.println("+");

  // Weak数値表示
  M5.Display.setTextSize(3);
  M5.Display.setCursor(110, 73);
  M5.Display.printf("%3d", userChopWeak);

  // Weakテストボタン（右側に配置）
  M5.Display.fillRect(180, 70, 130, 30, testingWeak ? ORANGE : BLUE);
  M5.Display.setTextColor(BLACK);
  M5.Display.setTextSize(2);
  M5.Display.setCursor(188, 75);
  M5.Display.println(testingWeak ? "Testing" : "Test Weak");

  // Strong調整エリア
  M5.Display.setTextColor(WHITE);
  M5.Display.setTextSize(2);
  M5.Display.setCursor(10, 115);
  M5.Display.println("Strong:");

  // Strong調整ボタン（-/+）
  M5.Display.fillRect(10, 140, 40, 30, RED);
  M5.Display.setTextSize(2);
  M5.Display.setCursor(20, 145);
  M5.Display.println("-");

  M5.Display.fillRect(60, 140, 40, 30, GREEN);
  M5.Display.setCursor(70, 145);
  M5.Display.println("+");

  // Strong数値表示
  M5.Display.setTextSize(3);
  M5.Display.setCursor(110, 143);
  M5.Display.printf("%3d", userChopStrong);

  // Strongテストボタン（右側に配置）
  M5.Display.fillRect(180, 140, 130, 30, testingStrong ? ORANGE : BLUE);
  M5.Display.setTextColor(BLACK);
  M5.Display.setTextSize(2);
  M5.Display.setCursor(180, 145);
  M5.Display.println(testingStrong ? "Testing" : "Test Strong");

  // DONEボタン（十分な間隔を確保）
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

  // Weak調整（-/+ボタン）
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

  // Weakテストボタン
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

  // Strong調整（-/+ボタン）
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

  // Strongテストボタン
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

  // DONEボタン
  if (y >= 200 && y <= 235 && x >= 85 && x <= 235) {
    stopAllStimulus();
    setupComplete = true;
    chopValue = userChopWeak;  // 初期値をWeakに設定
    drawUI();
    return;
  }

  if (redraw) drawSetupUI();
}

// ========== メイン画面（コンパクト版） ==========
void drawUI() {
  M5.Display.clear(BLACK);

  M5.Display.setTextColor(WHITE);
  M5.Display.setTextSize(2);
  M5.Display.setCursor(60, 5);
  M5.Display.println("EMS C/D/E");

  M5.Display.setTextSize(1);
  M5.Display.setCursor(5, 25);
  M5.Display.printf("C:G%d,%d D:G%d,%d E:G%d,%d",
                    PWM0_PIN_C, PWM1_PIN_C,
                    PWM0_PIN_D, PWM1_PIN_D,
                    PWM0_PIN_E, PWM1_PIN_E);

  // ポート選択ボタン
  M5.Display.setTextSize(1);
  M5.Display.setCursor(10, 42);
  M5.Display.println("Port:");

  int ySel = 55;
  int hSel = 35;
  int wSel = 75;

  M5.Display.fillRect(10, ySel, wSel, hSel, (currentPort == PORT_C) ? GREEN : DARKGREY);
  M5.Display.setTextColor(BLACK);
  M5.Display.setTextSize(2);
  M5.Display.setCursor(38, ySel + 8);
  M5.Display.println("C");

  M5.Display.fillRect(120, ySel, wSel, hSel, (currentPort == PORT_D) ? GREEN : DARKGREY);
  M5.Display.setCursor(148, ySel + 8);
  M5.Display.println("D");

  M5.Display.fillRect(230, ySel, wSel, hSel, (currentPort == PORT_E) ? GREEN : DARKGREY);
  M5.Display.setCursor(258, ySel + 8);
  M5.Display.println("E");

  // Chop調整
  M5.Display.setTextColor(WHITE);
  M5.Display.setTextSize(1);
  M5.Display.setCursor(10, 95);
  M5.Display.println("Chop:");

  int yChopBtn = 108;
  int hChopBtn = 30;
  int wChopBtn = 60;

  M5.Display.fillRect(10, yChopBtn, wChopBtn, hChopBtn, DARKGREY);
  M5.Display.setTextColor(BLACK);
  M5.Display.setTextSize(2);
  M5.Display.setCursor(30, yChopBtn + 5);
  M5.Display.println("-");

  M5.Display.fillRect(250, yChopBtn, wChopBtn, hChopBtn, DARKGREY);
  M5.Display.setCursor(270, yChopBtn + 5);
  M5.Display.println("+");

  M5.Display.setTextColor(WHITE);
  M5.Display.setTextSize(3);
  M5.Display.setCursor(135, yChopBtn + 2);
  M5.Display.printf("%3d", chopValue);

  // Output ON/OFF
  M5.Display.setTextSize(1);
  M5.Display.setTextColor(WHITE);
  M5.Display.setCursor(10, 145);
  M5.Display.println("Output:");

  int yOut = 158;
  int hOut = 28;
  int wOut = 115;

  M5.Display.fillRect(10, yOut, wOut, hOut, stimActive ? DARKGREY : BLUE);
  M5.Display.setTextColor(BLACK);
  M5.Display.setTextSize(2);
  M5.Display.setCursor(45, yOut + 5);
  M5.Display.println("ON");

  M5.Display.fillRect(195, yOut, wOut, hOut, stimActive ? RED : DARKGREY);
  M5.Display.setCursor(225, yOut + 5);
  M5.Display.println("OFF");

  // ステータス表示
  M5.Display.setTextSize(1);
  M5.Display.setTextColor(WHITE);
  M5.Display.setCursor(5, 195);
  M5.Display.printf("Ctrl=%d%% Chop=%d%%", CTRL_VALUE, chopValue);
  
  M5.Display.setCursor(5, 210);
  M5.Display.printf("Status: %s", stimActive ? "ON" : "OFF");
  
  M5.Display.setCursor(5, 225);
  const char* portName = (currentPort == PORT_C) ? "C" : (currentPort == PORT_D) ? "D" : "E";
  M5.Display.printf("Active Port: %s", portName);
}

void handleTouch() {
  auto t = M5.Touch.getDetail();
  if (!t.wasPressed()) return;

  int x = t.x;
  int y = t.y;
  bool redraw = false;

  // ポート選択
  int ySel = 55;
  int hSel = 35;

  if (y >= ySel && y <= ySel + hSel) {
    if (x >= 10 && x <= 85)        { currentPort = PORT_C; redraw = true; }
    else if (x >= 120 && x <= 195) { currentPort = PORT_D; redraw = true; }
    else if (x >= 230 && x <= 305) { currentPort = PORT_E; redraw = true; }
  }

  // Chop調整
  int yChopBtn = 108;
  int hChopBtn = 30;

  if (y >= yChopBtn && y <= yChopBtn + hChopBtn) {
    if (x >= 10 && x <= 70) {
      chopValue -= 5;
      if (chopValue < 0) chopValue = 0;
      if (stimActive) applyStimulus();
      redraw = true;
    } else if (x >= 250 && x <= 310) {
      chopValue += 5;
      if (chopValue > 100) chopValue = 100;
      if (stimActive) applyStimulus();
      redraw = true;
    }
  }

  // Output ON/OFF
  int yOut = 158;
  int hOut = 28;

  if (y >= yOut && y <= yOut + hOut) {
    if (x >= 10 && x <= 125) {
      applyStimulus();
      redraw = true;
    } else if (x >= 195 && x <= 310) {
      stopAllStimulus();
      redraw = true;
    }
  }

  if (redraw) drawUI();
}

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);
  
  M5.Display.setBrightness(255);

  ledcSetup(PWM_CH_C_CTRL, PWM_CTRL_FREQ, PWM_RES);
  ledcAttachPin(PWM0_PIN_C, PWM_CH_C_CTRL);
  ledcWrite(PWM_CH_C_CTRL, 0);

  ledcSetup(PWM_CH_D_CTRL, PWM_CTRL_FREQ, PWM_RES);
  ledcAttachPin(PWM0_PIN_D, PWM_CH_D_CTRL);
  ledcWrite(PWM_CH_D_CTRL, 0);

  ledcSetup(PWM_CH_E_CTRL, PWM_CTRL_FREQ, PWM_RES);
  ledcAttachPin(PWM0_PIN_E, PWM_CH_E_CTRL);
  ledcWrite(PWM_CH_E_CTRL, 0);

  ledcSetup(PWM_CH_C_CHOP, PWM_CHOP_FREQ, PWM_RES);
  ledcAttachPin(PWM1_PIN_C, PWM_CH_C_CHOP);
  ledcWrite(PWM_CH_C_CHOP, 0);

  ledcSetup(PWM_CH_D_CHOP, PWM_CHOP_FREQ, PWM_RES);
  ledcAttachPin(PWM1_PIN_D, PWM_CH_D_CHOP);
  ledcWrite(PWM_CH_D_CHOP, 0);

  ledcSetup(PWM_CH_E_CHOP, PWM_CHOP_FREQ, PWM_RES);
  ledcAttachPin(PWM1_PIN_E, PWM_CH_E_CHOP);
  ledcWrite(PWM_CH_E_CHOP, 0);

  // セットアップ画面から開始
  drawSetupUI();
}

void loop() {
  M5.update();
  
  if (!setupComplete) {
    handleSetupTouch();
  } else {
    handleTouch();
  }
  
  delay(10);
}
