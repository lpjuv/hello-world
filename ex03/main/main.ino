const int ledPin = 13;
unsigned long nowTime, lastTime;

// 时长定义：短亮200ms 长亮600ms 间隔150ms 一轮结束停顿1000ms
const unsigned shortOn = 200;
const unsigned longOn = 600;
const unsigned gap = 150;
const unsigned stopGap = 1000;

int state = 0;
bool ledStat = LOW;

void setup() {
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
}

void loop() {
  nowTime = millis();
  if (nowTime - lastTime < (ledStat ? (state==1||state==3||state==5 ? shortOn : longOn) : gap)) return;

  lastTime = nowTime;
  ledStat = !ledStat;
  digitalWrite(ledPin, ledStat);

  if (!ledStat) {
    state++;
    // SOS: 3短(1,2,3) → 3长(4,5,6) → 3短(7,8,9)
    if (state > 9) {
      state = 0;
      lastTime += stopGap; // 整段结束长停顿
    }
  }
}
