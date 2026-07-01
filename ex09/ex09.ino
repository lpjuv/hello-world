#include <WiFi.h>
#include <WebServer.h>

// ========== WiFi配置 ==========
const char* ssid     = "Xiaomi 14 Pro";
const char* password = "9az62tyqayj59p6";

// ========== 引脚定义 ==========
#define TOUCH_PIN 4      // GPIO4 = T0 触摸通道
#define LED_PIN   2      // 板载LED（可选，用于指示）

// ========== 全局对象 ==========
WebServer server(80);

// 存储最新触摸值（用于调试）
int lastTouchValue = 0;

// ========== HTML页面 ==========
const char* PAGE_HTML = R"raw(
<!DOCTYPE html>
<html lang="zh-CN">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>实时传感器仪表盘</title>
    <style>
        * { margin: 0; padding: 0; box-sizing: border-box; }
        body {
            font-family: 'Microsoft YaHei', Arial, sans-serif;
            background: #0a0a1a;
            min-height: 100vh;
            display: flex;
            justify-content: center;
            align-items: center;
        }
        .dashboard {
            background: linear-gradient(145deg, #141428, #1a1a35);
            padding: 50px 60px;
            border-radius: 30px;
            box-shadow: 0 20px 60px rgba(0,0,0,0.8), inset 0 1px 0 rgba(255,255,255,0.05);
            text-align: center;
            max-width: 500px;
            width: 90%;
        }
        h1 {
            color: #8888cc;
            font-size: 1.6rem;
            font-weight: 300;
            letter-spacing: 4px;
        }
        .subtitle {
            color: #555599;
            font-size: 0.85rem;
            letter-spacing: 2px;
            margin: 6px 0 30px 0;
        }
        .sensor-panel {
            background: #0a0a18;
            border-radius: 20px;
            padding: 35px 20px;
            border: 1px solid #2a2a55;
            position: relative;
            overflow: hidden;
        }
        .sensor-panel::before {
            content: '';
            position: absolute;
            top: -50%;
            left: -50%;
            width: 200%;
            height: 200%;
            background: radial-gradient(circle at center, rgba(100,100,255,0.03), transparent 70%);
            animation: rotateBg 20s linear infinite;
        }
        @keyframes rotateBg {
            0% { transform: rotate(0deg); }
            100% { transform: rotate(360deg); }
        }
        .sensor-number {
            font-size: 5.5rem;
            font-weight: 700;
            position: relative;
            z-index: 1;
            background: linear-gradient(135deg, #6666ff, #aa88ff);
            -webkit-background-clip: text;
            -webkit-text-fill-color: transparent;
            font-variant-numeric: tabular-nums;
            transition: all 0.1s;
            line-height: 1.2;
        }
        .sensor-label {
            color: #555599;
            font-size: 0.8rem;
            letter-spacing: 3px;
            margin-top: 8px;
            position: relative;
            z-index: 1;
        }
        .unit {
            color: #444477;
            font-size: 1rem;
            position: relative;
            z-index: 1;
        }
        .status-bar {
            display: flex;
            justify-content: space-between;
            align-items: center;
            margin-top: 20px;
            padding: 12px 20px;
            background: #0a0a18;
            border-radius: 12px;
            border: 1px solid #1a1a3a;
        }
        .status-dot {
            display: inline-block;
            width: 10px;
            height: 10px;
            border-radius: 50%;
            background: #00ff88;
            animation: pulseDot 1.2s ease-in-out infinite;
            margin-right: 8px;
            vertical-align: middle;
        }
        @keyframes pulseDot {
            0%, 100% { opacity: 1; transform: scale(1); }
            50% { opacity: 0.2; transform: scale(0.7); }
        }
        .status-text {
            color: #555599;
            font-size: 0.8rem;
        }
        .status-text span {
            color: #8888cc;
        }
        .bar-wrapper {
            margin-top: 16px;
            height: 4px;
            background: #1a1a3a;
            border-radius: 4px;
            overflow: hidden;
        }
        .bar-fill {
            height: 100%;
            width: 0%;
            border-radius: 4px;
            transition: width 0.15s ease;
            background: linear-gradient(90deg, #4444cc, #aa88ff);
        }
        .hint {
            margin-top: 18px;
            color: #333366;
            font-size: 0.75rem;
            letter-spacing: 1px;
        }
        .hint em {
            color: #6666aa;
            font-style: normal;
        }
        .touch-indicator {
            display: inline-block;
            padding: 4px 16px;
            border-radius: 20px;
            font-size: 0.7rem;
            letter-spacing: 1px;
            margin-top: 10px;
            background: #1a1a3a;
            color: #555599;
            position: relative;
            z-index: 1;
        }
        .touch-indicator.active {
            background: #ff174422;
            color: #ff5252;
            border: 1px solid #ff174466;
        }
    </style>
</head>
<body>
    <div class="dashboard">
        <h1>📊 传感器仪表盘</h1>
        <div class="subtitle">实时触摸数据监控</div>

        <div class="sensor-panel">
            <div class="sensor-number" id="sensorValue">--</div>
            <div class="sensor-label">触摸传感器数值</div>
            <div class="touch-indicator" id="touchIndicator">👆 等待触摸</div>
        </div>

        <div class="status-bar">
            <div>
                <span class="status-dot"></span>
                <span class="status-text">实时采集</span>
            </div>
            <div class="status-text">
                🕐 <span id="updateTime">--:--:--</span>
            </div>
        </div>

        <div class="bar-wrapper">
            <div class="bar-fill" id="barFill"></div>
        </div>

        <div class="hint">
            💡 用手指靠近 <em>GPIO 4</em> 引脚 · 数值实时变化
        </div>
    </div>

    <script>
        const valueDisplay = document.getElementById('sensorValue');
        const timeDisplay = document.getElementById('updateTime');
        const barFill = document.getElementById('barFill');
        const touchIndicator = document.getElementById('touchIndicator');

        // ========== 修改这里：触摸阈值从50改为150 ==========
        const TOUCH_THRESHOLD = 150;  // 低于此值认为正在触摸
        // ===================================================

        function fetchSensorData() {
            fetch('/data')
                .then(response => response.text())
                .then(data => {
                    const numValue = parseInt(data);
                    if (!isNaN(numValue)) {
                        // 更新大数字
                        valueDisplay.textContent = numValue;

                        // 更新进度条（数值越低 → 进度条越长 → 越靠近）
                        let percent = 0;
                        const maxRef = 200;
                        if (numValue < maxRef) {
                            percent = ((maxRef - numValue) / maxRef) * 100;
                        }
                        if (percent > 100) percent = 100;
                        barFill.style.width = percent + '%';

                        // 根据接近程度改变颜色（使用新阈值150）
                        if (numValue < TOUCH_THRESHOLD) {
                            barFill.style.background = 'linear-gradient(90deg, #ff4466, #ff8844)';
                            touchIndicator.textContent = '✋ 触摸中！';
                            touchIndicator.className = 'touch-indicator active';
                        } else if (numValue < TOUCH_THRESHOLD + 50) {
                            barFill.style.background = 'linear-gradient(90deg, #ff8844, #ffcc44)';
                            touchIndicator.textContent = '👆 靠近中...';
                            touchIndicator.className = 'touch-indicator';
                        } else {
                            barFill.style.background = 'linear-gradient(90deg, #4444cc, #aa88ff)';
                            touchIndicator.textContent = '👆 等待触摸';
                            touchIndicator.className = 'touch-indicator';
                        }

                        // 更新数字颜色（随数值变化）
                        if (numValue < 150) {
                            valueDisplay.style.background = 'linear-gradient(135deg, #ff1744, #ff5252)';
                            valueDisplay.style.webkitBackgroundClip = 'text';
                            valueDisplay.style.webkitTextFillColor = 'transparent';
                        } else if (numValue < 200) {
                            valueDisplay.style.background = 'linear-gradient(135deg, #ffab00, #ffd93d)';
                            valueDisplay.style.webkitBackgroundClip = 'text';
                            valueDisplay.style.webkitTextFillColor = 'transparent';
                        } else {
                            valueDisplay.style.background = 'linear-gradient(135deg, #6666ff, #aa88ff)';
                            valueDisplay.style.webkitBackgroundClip = 'text';
                            valueDisplay.style.webkitTextFillColor = 'transparent';
                        }
                    }
                })
                .catch(error => {
                    console.error('请求失败:', error);
                    valueDisplay.textContent = 'ERR';
                });
        }

        function updateTime() {
            const now = new Date();
            const h = String(now.getHours()).padStart(2, '0');
            const m = String(now.getMinutes()).padStart(2, '0');
            const s = String(now.getSeconds()).padStart(2, '0');
            timeDisplay.textContent = h + ':' + m + ':' + s;
        }

        // 首次加载
        fetchSensorData();
        updateTime();

        // 每 100ms 轮询一次（实时更新）
        setInterval(() => {
            fetchSensorData();
            updateTime();
        }, 100);
    </script>
</body>
</html>
)raw";

// ========== 页面处理 ==========
void handleRoot() {
    server.send(200, "text/html; charset=UTF-8", PAGE_HTML);
}

// ========== 数据接口 ==========
void handleData() {
    int touchValue = touchRead(TOUCH_PIN);
    lastTouchValue = touchValue;
    server.send(200, "text/plain", String(touchValue));
}

// ========== 初始化 ==========
void setup() {
    Serial.begin(115200);
    delay(100);

    // LED初始化（可选）
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);

    // WiFi连接
    Serial.print("[WiFi] 正在连接 ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);

    int retry = 0;
    while (WiFi.status() != WL_CONNECTED && retry < 30) {
        delay(500);
        Serial.print(".");
        retry++;
    }
    Serial.println();

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("[WiFi] ✅ 连接成功");
        Serial.print("[WiFi] 📡 IP地址: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("[WiFi] ❌ 连接失败，请检查配置");
    }

    // 设置路由
    server.on("/", handleRoot);
    server.on("/data", handleData);

    server.begin();
    Serial.println("[Server] HTTP服务已启动");
    Serial.println("[Sensor] 触摸引脚: GPIO 4 (T0)");
    Serial.println("=================================");
}

// ========== 主循环 ==========
void loop() {
    server.handleClient();

    // 可选：用板载LED指示触摸状态
    int touchVal = touchRead(TOUCH_PIN);
    if (touchVal < 150) {   // 同步修改为150
        digitalWrite(LED_PIN, HIGH);  // 触摸时亮起
    } else {
        digitalWrite(LED_PIN, LOW);
    }

    // 短暂延时，避免CPU占用过高
    delay(5);
}