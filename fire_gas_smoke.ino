#include <WiFi.h>

// Define sensor pins for Fire, Gas, and Smoke
#define FIRE_SENSOR_PIN 34  // Fire sensor analog pin
#define GAS_SENSOR_PIN 35   // Gas sensor analog pin
#define SMOKE_SENSOR_PIN 32 // Smoke sensor analog pin

// Wi-Fi credentials
const char* ssid = "ESP32_Fire_Gas_Smoke";
const char* password = "12345678"; // Minimum 8 characters

WiFiServer server(80);

void setup() {
  Serial.begin(115200);

  // Initialize sensor pins
  pinMode(FIRE_SENSOR_PIN, INPUT);
  pinMode(GAS_SENSOR_PIN, INPUT);
  pinMode(SMOKE_SENSOR_PIN, INPUT);

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
    <title>Fire, Gas, and Smoke Detection</title>
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
            0% { background: linear-gradient(135deg, #2c3e50, #4b79a1); }
            50% { background: linear-gradient(135deg, #1f2833, #3a3d40); }
            100% { background: linear-gradient(135deg, #0f2027, #203a43); }
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

        .sensor-wrapper {
            display: flex;
            flex-direction: column;
            align-items: center;
        }

        .sensor-label, .emoji {
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
            0%, 100% { box-shadow: 0 0 15px rgba(255, 255, 255, 0.5); }
            50% { box-shadow: 0 0 25px rgba(255, 255, 255, 0.8); }
        }
    </style>
</head>
<body>
    <h1>Fire, Gas, and Smoke Detection</h1>
    <div class="container">
        <div class="sensor-wrapper">
            <div class="sensor-label">Fire Detection:</div>
            <div class="emoji" id="fireEmoji">🔥</div>
            <div id="fireStatus">Not Detected</div>
        </div>
        <div class="sensor-wrapper">
            <div class="sensor-label">Gas Detection:</div>
            <div class="emoji" id="gasEmoji">💨</div>
            <div id="gasStatus">Not Detected</div>
        </div>
        <div class="sensor-wrapper">
            <div class="sensor-label">Smoke Detection:</div>
            <div class="emoji" id="smokeEmoji">💨</div>
            <div id="smokeStatus">Not Detected</div>
        </div>
    </div>
    <script>
        async function fetchSensorData() {
            const response = await fetch('/data');
            const data = await response.json();

            updateFire(data.fire);
            updateGas(data.gas);
            updateSmoke(data.smoke);
        }

        function updateFire(fire) {
            const fireStatus = document.getElementById('fireStatus');
            const fireEmoji = document.getElementById('fireEmoji');

            if (fire > 500) {
                fireStatus.textContent = "Fire Detected!";
                fireEmoji.textContent = "🔥";
                fireEmoji.style.color = "red";
            } else {
                fireStatus.textContent = "Not Detected";
                fireEmoji.textContent = "🔥";
                fireEmoji.style.color = "white";
            }
        }

        function updateGas(gas) {
            const gasStatus = document.getElementById('gasStatus');
            const gasEmoji = document.getElementById('gasEmoji');

            if (gas > 200) {
                gasStatus.textContent = "Gas Detected!";
                gasEmoji.textContent = "💨";
                gasEmoji.style.color = "yellow";
            } else {
                gasStatus.textContent = "Not Detected";
                gasEmoji.textContent = "💨";
                gasEmoji.style.color = "white";
            }
        }

        function updateSmoke(smoke) {
            const smokeStatus = document.getElementById('smokeStatus');
            const smokeEmoji = document.getElementById('smokeEmoji');

            if (smoke > 300) {
                smokeStatus.textContent = "Smoke Detected!";
                smokeEmoji.textContent = "💨";
                smokeEmoji.style.color = "gray";
            } else {
                smokeStatus.textContent = "Not Detected";
                smokeEmoji.textContent = "💨";
                smokeEmoji.style.color = "white";
            }
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
      int fireValue = analogRead(FIRE_SENSOR_PIN);
      int gasValue = analogRead(GAS_SENSOR_PIN);
      int smokeValue = analogRead(SMOKE_SENSOR_PIN);

      String json = "{";
      json += "\"fire\": " + String(fireValue) + ",";
      json += "\"gas\": " + String(gasValue) + ",";
      json += "\"smoke\": " + String(smokeValue);
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
