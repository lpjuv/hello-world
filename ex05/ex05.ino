/*
 * 实验5：多档位触摸调速呼吸灯（ESP32版）
 * 引脚：触摸GPIO4，LED GPIO15（外接，避开板载LED）
 */

#define TOUCH_PIN 4
#define LED_PIN 2   // ⬅️ 改成 GPIO15

// ----- 触摸相关变量 -----
int threshold = 200;
int touchValue;
int lastTouchValue = 0;

// ----- 呼吸灯相关变量 -----
int brightness = 0;
int step = 1;
bool increasing = true;

// ----- 档位控制 -----
int speedLevel = 1;
const int maxLevel = 3;

// PWM设置
const int freq = 5000;
const int resolution = 8;

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  ledcAttach(LED_PIN, freq, resolution);
  ledcWrite(LED_PIN, 0);
}

int getStep() {
  switch(speedLevel) {
    case 1: return 1;
    case 2: return 3;
    case 3: return 7;
    default: return 1;
  }
}

int getDelay() {
  switch(speedLevel) {
    case 1: return 15;
    case 2: return 8;
    case 3: return 3;
    default: return 15;
  }
}

void handleTouch() {
  touchValue = touchRead(TOUCH_PIN);
  
  if (touchValue < threshold && lastTouchValue >= threshold) {
    speedLevel++;
    if (speedLevel > maxLevel) {
      speedLevel = 1;
    }
    Serial.print("=== 切换到第 ");
    Serial.print(speedLevel);
    Serial.println(" 档 ===");
    delay(300);
  }
  
  lastTouchValue = touchValue;
}

void updateBreathing() {
  int currentStep = getStep();
  int currentDelay = getDelay();
  
  if (increasing) {
    brightness += currentStep;
    if (brightness >= 255) {
      brightness = 255;
      increasing = false;
    }
  } else {
    brightness -= currentStep;
    if (brightness <= 0) {
      brightness = 0;
      increasing = true;
    }
  }
  
  ledcWrite(LED_PIN, brightness);
  delay(currentDelay);
}

void loop() {
  handleTouch();
  updateBreathing();
}