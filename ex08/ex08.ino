#include <WiFi.h>
#include <WebServer.h>

// ========== WiFi配置 ==========
const char* ssid     = Xiaomi 14 Pro"";
const char* password = "9az62tyqayj59p6";

// ========== 引脚定义 ==========
#define LED_BUILTIN   2
#define TOUCH_SENSOR  4

// ========== 全局变量 ==========
WebServer webServer(80);
bool systemArmed = false;      // 布防标志
bool alarmTriggered = false;   // 报警触发标志
unsigned long lastTouchTime = 0;
const unsigned long DEBOUNCE_DELAY = 200;  // 防抖延迟

// ========== HTML页面 ==========
const char* PAGE_HTML = R"raw(
<!DOCTYPE html>
<html lang="zh-CN">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>智能安防系统</title>
    <style>
        * { margin: 0; padding: 0; box-sizing: border-box; }
        body {
            font-family: 'Microsoft YaHei', Arial, sans-serif;
            background: linear-gradient(135deg, #0c0c1e, #1a1a3e);
            min-height: 100vh;
            display: flex;
            justify-content: center;
            align-items: center;
        }
        .card {
            background: rgba(255,255,255,0.05);
            backdrop-filter: blur(20px);
            padding: 50px 60px;
            border-radius: 30px;
            border: 1px solid rgba(255,255,255,0.08);
            box-shadow: 0 30px 80px rgba(0,0,0,0.6);
            text-align: center;
            max-width: 450px;
            width: 90%;
        }
        h1 {
            color: #fff;
            font-size: 1.8rem;
            font-weight: 300;
            letter-spacing: 4px;
            margin-bottom: 8px;
        }
        .sub {
            color: #6666aa;
            font-size: 0.85rem;
            letter-spacing: 2px;
            margin-bottom: 35px;
        }
        .btn-group {
            display: flex;
            gap: 20px;
            justify-content: center;
            flex-wrap: wrap;
        }
        .btn {
            padding: 14px 44px;
            font-size: 1.1rem;
            border: none;
            border-radius: 50px;
            cursor: pointer;
            font-weight: 600;
            letter-spacing: 2px;
            transition: all 0.3s ease;
            min-width: 130px;
        }
        .btn-arm {
            background: linear-gradient(135deg, #00c853, #00e676);
            color: #fff;
            box-shadow: 0 8px 25px rgba(0, 200, 83, 0.35);
        }
        .btn-arm:hover { transform: translateY(-3px); box-shadow: 0 12px 35px rgba(0, 200, 83, 0.5); }
        .btn-arm:active { transform: scale(0.95); }
        .btn-disarm {
            background: linear-gradient(135deg, #ff1744, #ff5252);
            color: #fff;
            box-shadow: 0 8px 25px rgba(255, 23, 68, 0.35);
        }
        .btn-disarm:hover { transform: translateY(-3px); box-shadow: 0 12px 35px rgba(255, 23, 68, 0.5); }
        .btn-disarm:active { transform: scale(0.95); }
        .status-box {
            margin-top: 35px;
            padding: 18px;
            background: rgba(0,0,0,0.3);
            border-radius: 15px;
            border: 1px solid rgba(255,255,255,0.05);
        }
        .status-label {
            color: #6666aa;
            font-size: 0.8rem;
            letter-spacing: 2px;
        }
        .status-text {
            font-size: 1.8rem;
            font-weight: 700;
            margin-top: 6px;
            transition: color 0.3s;
        }
        .status-safe { color: #00e676; }
        .status-armed { color: #ffab00; }
        .status-alarm { color: #ff1744; animation: blink 0.5s infinite; }
        @keyframes blink {
            0%, 100% { opacity: 1; }
            50% { opacity: 0.2; }
        }
        .footer {
            margin-top: 20px;
            color: #333366;
            font-size: 0.7rem;
            letter-spacing: 1px;
        }
    </style>
</head>
<body>
    <div class="card">
        <h1>🔐 智能安防</h1>
        <div class="sub">物联网报警系统</div>
        <div class="btn-group">
            <button class="btn btn-arm" onclick="armSystem()">🔒 布防</button>
            <button class="btn btn-disarm" onclick="disarmSystem()">🔓 撤防</button>
        </div>
        <div class="status-box">
            <div class="status-label">当前状态</div>
            <div class="status-text" id="statusDisplay">● 未布防</div>
        </div>
        <div class="footer">触摸传感器触发报警 · LED高频闪烁</div>
    </div>

    <script>
        function armSystem() {
            fetch('/arm')
                .then(() => updateStatus())
                .catch(e => console.error(e));
        }

        function disarmSystem() {
            fetch('/disarm')
                .then(() => updateStatus())
                .catch(e => console.error(e));
        }

        function updateStatus() {
            fetch('/status')
                .then(res => res.text())
                .then(data => {
                    const el = document.getElementById('statusDisplay');
                    el.textContent = data;
                    el.className = 'status-text';
                    if (data.includes('布防')) {
                        el.classList.add('status-armed');
                    } else if (data.includes('报警')) {
                        el.classList.add('status-alarm');
                    } else {
                        el.classList.add('status-safe');
                    }
                })
                .catch(e => console.error(e));
        }

        // 每500ms刷新状态
        setInterval(updateStatus, 500);
        // 页面加载时立即刷新
        window.onload = updateStatus;
    </script>
</body>
</html>
)raw";

// ========== 页面处理 ==========
void sendHomePage() {
    webServer.send(200, "text/html; charset=UTF-8", PAGE_HTML);
}

// ========== 布防处理 ==========
void executeArm() {
    systemArmed = true;
    alarmTriggered = false;  // 布防时重置报警
    digitalWrite(LED_BUILTIN, LOW);
    webServer.send(200, "text/plain", "ARMED");
    Serial.println("[系统] 已布防");
}

// ========== 撤防处理 ==========
void executeDisarm() {
    systemArmed = false;
    alarmTriggered = false;
    digitalWrite(LED_BUILTIN, LOW);
    webServer.send(200, "text/plain", "DISARMED");
    Serial.println("[系统] 已撤防");
}

// ========== 状态查询 ==========
void queryStatus() {
    String statusMsg;
    if (!systemArmed) {
        statusMsg = "○ 未布防";
    } else if (alarmTriggered) {
        statusMsg = "● 报警中！";
    } else {
        statusMsg = "◐ 已布防";
    }
    webServer.send(200, "text/plain; charset=UTF-8", statusMsg);
}

// ========== 初始化 ==========
void setup() {
    // 串口
    Serial.begin(115200);
    delay(100);

    // GPIO
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(TOUCH_SENSOR, INPUT);
    digitalWrite(LED_BUILTIN, LOW);

    // WiFi连接
    Serial.print("[WiFi] 正在连接 " + String(ssid));
    WiFi.begin(ssid, password);
    int tryCount = 0;
    while (WiFi.status() != WL_CONNECTED && tryCount < 30) {
        delay(500);
        Serial.print(".");
        tryCount++;
    }
    Serial.println();

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("[WiFi] ✅ 连接成功");
        Serial.print("[WiFi] 📡 IP地址: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("[WiFi] ❌ 连接失败，请检查WiFi配置");
    }

    // Web路由
    webServer.on("/", sendHomePage);
    webServer.on("/arm", executeArm);
    webServer.on("/disarm", executeDisarm);
    webServer.on("/status", queryStatus);

    webServer.begin();
    Serial.println("[Server] HTTP服务已启动");
}

// ========== 主循环 ==========
void loop() {
    webServer.handleClient();

    // 读取触摸值 (ESP32触摸: 数值越小表示越靠近)
    int touchValue = touchRead(TOUCH_SENSOR);
    bool isTouching = (touchValue < 350);  // 阈值可调整

    // 防抖处理
    if (isTouching) {
        if (millis() - lastTouchTime > DEBOUNCE_DELAY) {
            // 只有在布防状态且未报警时，才触发报警
            if (systemArmed && !alarmTriggered) {
                alarmTriggered = true;
                Serial.println("[警报] 🚨 触发报警！");
            }
            lastTouchTime = millis();
        }
    }

    // 报警闪烁逻辑
    if (alarmTriggered) {
        // 高频闪烁 (亮80ms → 灭80ms)
        digitalWrite(LED_BUILTIN, HIGH);
        delay(80);
        digitalWrite(LED_BUILTIN, LOW);
        delay(80);
    } else {
        // 非报警状态：LED保持熄灭
        digitalWrite(LED_BUILTIN, LOW);
        // 短暂延时避免频繁循环占用CPU
        delay(10);
    }
}