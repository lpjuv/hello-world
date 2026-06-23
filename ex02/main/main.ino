// 使用millis()非阻塞实现1Hz LED闪烁(周期1000ms，亮500ms、灭500ms)
const int ledPin = 13;
unsigned long prevTime = 0;
unsigned long interval = 300; // 500ms翻转一次，整体1Hz
bool ledState = LOW;

void setup() {
  pinMode(ledPin, OUTPUT);
}

void loop() {
  unsigned long curTime = millis();
  // 间隔达到500ms执行翻转
  if (curTime - prevTime >= interval) {
    prevTime = curTime;
    ledState = !ledState;
    digitalWrite(ledPin, ledState);
  }
}

