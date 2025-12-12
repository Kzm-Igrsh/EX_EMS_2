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

enum PortPos { PORT_C, PORT_D, PORT_E };

PortPos currentPort = PORT_C;
bool stimActive = false;
int chopValue = 50;

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

void drawUI() {
  M5.Display.clear(BLACK);

  M5.Display.setTextColor(WHITE);
  M5.Display.setTextSize(2);
  M5.Display.setCursor(10, 5);
  M5.Display.println("EMS C / D / E");

  M5.Display.setTextSize(1);
  M5.Display.setCursor(10, 28);
  M5.Display.printf("C:G%d,%d  D:G%d,%d  E:G%d,%d",
                    PWM0_PIN_C, PWM1_PIN_C,
                    PWM0_PIN_D, PWM1_PIN_D,
                    PWM0_PIN_E, PWM1_PIN_E);

  M5.Display.setTextSize(2);
  M5.Display.setCursor(10, 45);
  M5.Display.println("Select");

  int ySel = 70;
  int hSel = 40;
  int wSel = 80;

  M5.Display.fillRect(10, ySel, wSel, hSel, (currentPort == PORT_C) ? GREEN : DARKGREY);
  M5.Display.setTextColor(BLACK);
  M5.Display.setCursor(40, ySel + 10);
  M5.Display.println("C");

  M5.Display.fillRect(120, ySel, wSel, hSel, (currentPort == PORT_D) ? GREEN : DARKGREY);
  M5.Display.setCursor(150, ySel + 10);
  M5.Display.println("D");

  M5.Display.fillRect(230, ySel, wSel, hSel, (currentPort == PORT_E) ? GREEN : DARKGREY);
  M5.Display.setCursor(260, ySel + 10);
  M5.Display.println("E");

  M5.Display.setTextColor(WHITE);
  M5.Display.setTextSize(2);
  M5.Display.setCursor(10, 115);
  M5.Display.println("Chop");

  int yChopBtn = 145;
  int hChopBtn = 35;
  int wChopBtn = 70;

  M5.Display.fillRect(10, yChopBtn, wChopBtn, hChopBtn, DARKGREY);
  M5.Display.setTextColor(BLACK);
  M5.Display.setCursor(35, yChopBtn + 7);
  M5.Display.println("-");

  M5.Display.fillRect(240, yChopBtn, wChopBtn, hChopBtn, DARKGREY);
  M5.Display.setCursor(265, yChopBtn + 7);
  M5.Display.println("+");

  M5.Display.setTextColor(WHITE);
  M5.Display.setTextSize(3);
  M5.Display.setCursor(135, yChopBtn + 3);
  M5.Display.printf("%3d", chopValue);

  M5.Display.setTextSize(2);
  M5.Display.setTextColor(WHITE);
  M5.Display.setCursor(10, 185);
  M5.Display.println("Output");

  int yOut = 205;
  int hOut = 30;
  int wOut = 120;

  M5.Display.fillRect(10, yOut, wOut, hOut, stimActive ? DARKGREY : BLUE);
  M5.Display.setTextColor(BLACK);
  M5.Display.setCursor(45, yOut + 5);
  M5.Display.println("ON");

  M5.Display.fillRect(190, yOut, wOut, hOut, stimActive ? RED : DARKGREY);
  M5.Display.setCursor(220, yOut + 5);
  M5.Display.println("OFF");

  M5.Display.setTextSize(1);
  M5.Display.setTextColor(WHITE);
  M5.Display.setCursor(5, 238);
  M5.Display.printf("Ctrl=%d Chop=%d Status:%s",
                    CTRL_VALUE, chopValue, stimActive ? "ON" : "OFF");
}

void handleTouch() {
  auto t = M5.Touch.getDetail();
  if (!t.wasPressed()) return;

  int x = t.x;
  int y = t.y;

  bool redraw = false;

  int ySel = 70;
  int hSel = 40;

  if (y >= ySel && y <= ySel + hSel) {
    if (x >= 10 && x <= 90)       { currentPort = PORT_C; redraw = true; }
    else if (x >= 120 && x <= 200){ currentPort = PORT_D; redraw = true; }
    else if (x >= 230 && x <= 310){ currentPort = PORT_E; redraw = true; }
  }

  int yChopBtn = 145;
  int hChopBtn = 35;

  if (y >= yChopBtn && y <= yChopBtn + hChopBtn) {
    if (x >= 10 && x <= 80) {
      chopValue -= 5;
      if (chopValue < 0) chopValue = 0;
      if (stimActive) applyStimulus();
      redraw = true;
    } else if (x >= 240 && x <= 310) {
      chopValue += 5;
      if (chopValue > 100) chopValue = 100;
      if (stimActive) applyStimulus();
      redraw = true;
    }
  }

  int yOut = 205;
  int hOut = 30;

  if (y >= yOut && y <= yOut + hOut) {
    if (x >= 10 && x <= 130) {
      applyStimulus();
      redraw = true;
    } else if (x >= 190 && x <= 310) {
      stopAllStimulus();
      redraw = true;
    }
  }

  if (redraw) drawUI();
}

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);

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

  drawUI();
}

void loop() {
  M5.update();
  handleTouch();
  delay(10);
}
