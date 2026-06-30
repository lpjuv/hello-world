/*
 * 实验6：警车双闪灯效（双通道PWM交替渐变）
 * 功能：两个LED交替亮暗，平滑渐变
 * 引脚：LED_A → GPIO15，LED_B → GPIO16
 * 
 * 硬件连接：
 * - LED_A 正极 → 220Ω电阻 → GPIO15，负极 → GND
 * - LED_B 正极 → 220Ω电阻 → GPIO16，负极 → GND
 */

// ----- 引脚定义（避开板载LED的GPIO2）-----
const int ledA = 4;    // 红灯（或任意颜色）
const int ledB = 2;    // 蓝灯（或任意颜色）

// ----- PWM设置 -----
const int freq = 5000;
const int resolution = 8;   // 0-255

// ----- 亮度控制变量 -----
int brightness = 0;         // 当前亮度 0-255
int step = 5;               // 亮度变化步长（越小过渡越平滑）
bool increasing = true;     // true=变亮，false=变暗

void setup() {
  // 初始化两个PWM通道
  ledcAttach(ledA, freq, resolution);
  ledcAttach(ledB, freq, resolution);
  
  // 初始状态：LED_A灭，LED_B亮（或都灭）
  ledcWrite(ledA, 0);
  ledcWrite(ledB, 255);
}

void loop() {
  // ----- 更新亮度值（锯齿波：0→255→0→255...）-----
  if (increasing) {
    brightness += step;
    if (brightness >= 255) {
      brightness = 255;
      increasing = false;
    }
  } else {
    brightness -= step;
    if (brightness <= 0) {
      brightness = 0;
      increasing = true;
    }
  }
  
  // ----- 双通道反相输出（核心逻辑）-----
  // LED_A 亮度 = brightness（从0→255→0循环）
  // LED_B 亮度 = 255 - brightness（从255→0→255循环，完全反相）
  ledcWrite(ledA, brightness);
  ledcWrite(ledB, 255 - brightness);
  
  // 延迟控制渐变速度（值越小过渡越平滑越快）
  delay(10);
}