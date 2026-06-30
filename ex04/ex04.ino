#define TOUCH_PIN 4
#define LED_PIN 2

int threshold = 200;
int touchValue;
bool ledState = false;        // LED状态
int lastTouchValue = 0;       // 上一次触摸值

void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);  // 初始熄灭
}

void loop() {
  touchValue = touchRead(TOUCH_PIN);
  Serial.print("Touch Value: ");
  Serial.println(touchValue);

  // 检测触摸瞬间：当前触摸（值<阈值）且 上一次未触摸（值>阈值）
  if (touchValue < threshold && lastTouchValue >= threshold) {
    ledState = !ledState;              // 翻转状态
    digitalWrite(LED_PIN, ledState ? HIGH : LOW);
    Serial.println("=== 触摸触发！===");
    delay(300);                         // 防抖延迟
  }

  lastTouchValue = touchValue;          // 更新上一次值
  delay(50);
}