#include <WiFi.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

// DHT sensor setup
#define DHTPIN 4 // GPIO pin connected to DHT11
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// Network credentials for Access Point mode
const char* ssid = "ESP32_DHT11";
const char* password = "12345678"; // Minimum 8 characters

WiFiServer server(80);

void setup() {
  Serial.begin(115200);
  dht.begin();

  // Set up Access Point
  Serial.println("Setting up Access Point...");
  WiFi.softAP(ssid, password);

  // Start the server
  server.begin();
  Serial.println("Access Point started.");
  Serial.print("IP Address: ");
  Serial.println(WiFi.softAPIP());
}

void loop() {
  WiFiClient client = server.available();
  if (client) {
    String request = client.readStringUntil('\r');
    client.flush();
    
    // Serve the HTML page
    if (request.indexOf("GET / ") >= 0) {
      String html = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Temperature and Humidity Monitor</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: center;
            height: 100vh;
            margin: 0;
            background: linear-gradient(135deg, #2c3e50, #4b79a1);
            animation: bgAnimation 10s infinite alternate;
            color: #fff;
        }

        @keyframes bgAnimation {
            0% {
                background: linear-gradient(135deg, #2c3e50, #4b79a1);
            }
            50% {
                background: linear-gradient(135deg, #1f2833, #3a3d40);
            }
            100% {
                background: linear-gradient(135deg, #0f2027, #203a43);
            }
        }

        h1 {
            color: #fff;
            font-size: 2.5rem;
            text-shadow: 2px 2px 5px rgba(0, 0, 0, 0.5);
        }

        .container {
            display: flex;
            justify-content: space-around;
            width: 80%;
            gap: 20px;
        }

        .gauge-wrapper {
            display: flex;
            flex-direction: column;
            align-items: center;
        }

        .gauge {
            width: 150px;
            height: 300px;
            position: relative;
            border-radius: 25px;
            overflow: hidden;
            box-shadow: 0 0 15px rgba(255, 255, 255, 0.3);
            border: 3px solid rgba(255, 255, 255, 0.5);
            background: rgba(255, 255, 255, 0.1);
            backdrop-filter: blur(10px);
        }

        .thermometer,
        .humidity {
            position: absolute;
            bottom: 0;
            width: 100%;
            transition: height 1s, background 1s;
        }

        .thermometer {
            background: linear-gradient(to top, #ff3333, #ffcccc);
        }

        .humidity {
            background: linear-gradient(to top, #3399ff, #cce5ff);
        }

        .label, .emoji {
            text-align: center;
            margin-top: 10px;
            font-weight: bold;
            color: #fff;
            text-shadow: 1px 1px 3px rgba(0, 0, 0, 0.5);
        }

        .emoji {
            font-size: 3rem;
            margin-top: 10px;
            transition: transform 0.5s, color 0.5s;
        }

        .glow {
            animation: pulseGlow 2s infinite;
        }

        @keyframes pulseGlow {
            0%, 100% {
                box-shadow: 0 0 15px rgba(255, 255, 255, 0.5);
            }
            50% {
                box-shadow: 0 0 25px rgba(255, 255, 255, 0.8);
            }
        }
    </style>
</head>
<body>
    <h1>Temperature and Humidity Monitor</h1>
    <div class="container">
        <div class="gauge-wrapper">
            <div class="gauge glow">
                <div id="thermometer" class="thermometer"></div>
            </div>
            <div class="label">Temperature: <span id="tempValue">0</span>°C</div>
            <div class="emoji" id="tempEmoji">🌡</div>
        </div>
        <div class="gauge-wrapper">
            <div class="gauge glow">
                <div id="humidity" class="humidity"></div>
            </div>
            <div class="label">Humidity: <span id="humValue">0</span>%</div>
            <div class="emoji" id="humEmoji">💧</div>
        </div>
    </div>
    <script>
        async function fetchSensorData() {
            const response = await fetch('/data');
            const data = await response.json();

            updateTemperature(data.temperature);
            updateHumidity(data.humidity);
        }

        function updateTemperature(temp) {
            const thermometer = document.getElementById('thermometer');
            const tempValue = document.getElementById('tempValue');
            const tempEmoji = document.getElementById('tempEmoji');

            const height = Math.min(Math.max(temp / 50 * 100, 0), 100); // Clamp 0-100%
            thermometer.style.height = `${height}%`;
            tempValue.textContent = temp;

            tempEmoji.textContent = temp > 35 ? "🔥" : temp < 15 ? "🥶" : "🌞";
        }

        function updateHumidity(hum) {
            const humidity = document.getElementById('humidity');
            const humValue = document.getElementById('humValue');
            const humEmoji = document.getElementById('humEmoji');

            const height = Math.min(Math.max(hum, 0), 100); // Clamp 0-100%
            humidity.style.height = `${height}%`;
            humValue.textContent = hum;

            humEmoji.textContent = hum > 70 ? "💦" : hum < 30 ? "🌱" : "💧";
        }

        setInterval(fetchSensorData, 2000);
    </script>
</body>
</html>
)rawliteral";
      client.println("HTTP/1.1 200 OK");
      client.println("Content-Type: text/html");
      client.println("Connection: close");
      client.println();
      client.print(html);
    }

    // Serve sensor data as JSON
    if (request.indexOf("GET /data") >= 0) {
      float temperature = dht.readTemperature();
      float humidity = dht.readHumidity();

      if (isnan(temperature) || isnan(humidity)) {
        temperature = 0.0;
        humidity = 0.0;
      }

      String json = "{";
      json += "\"temperature\": " + String(temperature) + ",";
      json += "\"humidity\": " + String(humidity);
      json += "}";

      client.println("HTTP/1.1 200 OK");
      client.println("Content-Type: application/json");
      client.println("Connection: close");
      client.println();
      client.print(json);
    }
    delay(100);
    client.stop();
  }
}
