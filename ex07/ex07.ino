#include <WiFi.h>

// WiFi配置
const char* ssid     = "Xiaomi 14 Pro";
const char* password = "9az62tyqayj59p6";

#define LED_PIN 2
int ledBright = 0;

WiFiServer server(80);

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  analogWrite(LED_PIN, ledBright);

  // 连接WiFi
  Serial.print("连接WiFi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi连接成功！");
  Serial.print("控制网页地址：http://");
  Serial.println(WiFi.localIP());
  server.begin();
}

void loop() {
  WiFiClient client = server.available();
  if (!client) return;

  String req = client.readStringUntil('\r');
  Serial.println(req);
  client.flush();

  // 解析滑动条传过来的亮度数值
  if (req.indexOf("/?bright=") != -1) {
    int pos = req.indexOf("bright=");
    String valStr = req.substring(pos + 7, req.indexOf(" ", pos));
    ledBright = valStr.toInt();
    ledBright = constrain(ledBright, 0, 255);
    analogWrite(LED_PIN, ledBright);
  }

  // 网页HTML：滑动条调光
  String html = R"HTML(
<!DOCTYPE html>
<html lang="zh-CN">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>ESP32 无极调光器</title>
<style>
body {
  text-align: center;
  font-family: 'Microsoft YaHei', Arial, sans-serif;
  background: #0a0a1a;
  color: #fff;
  min-height: 100vh;
  display: flex;
  justify-content: center;
  align-items: center;
  margin: 0;
}
.card {
  background: linear-gradient(145deg, #141428, #1a1a35);
  padding: 50px 60px;
  border-radius: 30px;
  box-shadow: 0 20px 60px rgba(0,0,0,0.8);
  max-width: 450px;
  width: 90%;
}
h2 {
  font-size: 1.6rem;
  font-weight: 300;
  letter-spacing: 3px;
  color: #8888cc;
  margin-bottom: 10px;
}
.sub {
  color: #555599;
  font-size: 0.85rem;
  letter-spacing: 2px;
  margin-bottom: 30px;
}
.brightness-display {
  font-size: 4.5rem;
  font-weight: 700;
  background: linear-gradient(135deg, #ff6b6b, #ffd93d);
  -webkit-background-clip: text;
  -webkit-text-fill-color: transparent;
  margin: 10px 0;
}
.brightness-label {
  color: #555599;
  font-size: 0.8rem;
  letter-spacing: 2px;
}
.slider-container {
  margin: 30px 0 10px 0;
}
input[type="range"] {
  width: 100%;
  height: 6px;
  -webkit-appearance: none;
  background: linear-gradient(to right, #ff6b6b, #ffd93d, #6bcbff);
  border-radius: 10px;
  outline: none;
  transition: 0.3s;
}
input[type="range"]::-webkit-slider-thumb {
  -webkit-appearance: none;
  width: 28px;
  height: 28px;
  background: radial-gradient(circle, #fff, #ddd);
  border-radius: 50%;
  cursor: pointer;
  box-shadow: 0 0 30px rgba(255, 215, 0, 0.4);
  transition: 0.2s;
}
input[type="range"]::-webkit-slider-thumb:hover {
  transform: scale(1.15);
  box-shadow: 0 0 50px rgba(255, 215, 0, 0.7);
}
input[type="range"]::-moz-range-thumb {
  width: 28px;
  height: 28px;
  background: radial-gradient(circle, #fff, #ddd);
  border: none;
  border-radius: 50%;
  cursor: pointer;
}
.value-row {
  display: flex;
  justify-content: space-between;
  color: #444488;
  font-size: 0.75rem;
  padding: 0 4px;
  margin-top: 4px;
}
.footer {
  margin-top: 25px;
  color: #333366;
  font-size: 0.7rem;
  letter-spacing: 1px;
}
.footer em {
  color: #6666aa;
  font-style: normal;
}
</style>
</head>
<body>
<div class="card">
  <h2>💡 无极调光</h2>
  <div class="sub">拖动滑块实时调节亮度</div>

  <div class="brightness-display" id="brightnessValue">0</div>
  <div class="brightness-label">PWM 亮度值</div>

  <div class="slider-container">
    <input type="range" id="brightnessSlider" min="0" max="255" value="0">
    <div class="value-row">
      <span>0 (灭)</span>
      <span>255 (最亮)</span>
    </div>
  </div>

  <div class="footer">⚡ 实时响应 · <em>ESP32 PWM 调光</em></div>
</div>

<script>
const slider = document.getElementById('brightnessSlider');
const display = document.getElementById('brightnessValue');

// 滑块值变化时触发
slider.addEventListener('input', function() {
  const value = this.value;
  display.textContent = value;
  
  // 通过 fetch 发送 GET 请求
  fetch(`/?bright=${value}`)
    .then(response => {
      if (!response.ok) {
        console.error('请求失败');
      }
    })
    .catch(error => {
      console.error('错误:', error);
    });
});
</script>
</body>
</html>
)HTML";

  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html; charset=UTF-8");
  client.println("Connection: close");
  client.println();
  client.println(html);

  delay(1);
  client.stop();
}