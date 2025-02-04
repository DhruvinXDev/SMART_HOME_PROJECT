#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>

// WiFi credentials for Access Point
const char* ap_ssid = "smarthome";  // Your AP SSID
const char* ap_password = "12345678";  // Your AP password

// Pin Definitions
#define WATER_TRIG_PIN 5    
#define WATER_ECHO_PIN 18
#define LED_PIN 2           
#define DOOR_TRIG_PIN 13    
#define DOOR_ECHO_PIN 12
#define FIRE_SENSOR_PIN 34  
#define GAS_SENSOR_PIN 35   
#define SMOKE_SENSOR_PIN 32 
#define RAIN_SENSOR_PIN 33  
#define PIR_SENSOR_PIN 25   
#define SOIL_SENSOR_PIN 26  
#define DHT_PIN 4          
#define PARKING_TRIG_PIN 14 
#define PARKING_ECHO_PIN 27

WebServer server(80);
DHT dht(DHT_PIN, DHT11);

// Your existing HTML code
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Smart Home Automation</title>
    <style>
        /* General Styles */
        body {
            font-family: Arial, sans-serif;
            margin: 0;
            padding: 0;
            text-align: center;
            background-size: 400% 400%;
            animation: gradientBG 15s ease infinite;
            background: linear-gradient(
                -45deg, 
                #1a2a6c,
                #b21f1f,
                #fdbb2d,
                #2c3e50,
                #3498db
            );
        }

        @keyframes gradientBG {
            0% {
                background-position: 0% 50%;
            }
            50% {
                background-position: 100% 50%;
            }
            100% {
                background-position: 0% 50%;
            }
        }

        /* Header */
        header {
            background: rgba(52, 152, 219, 0.8);
            padding: 15px;
            font-size: 26px;
            font-weight: bold;
            text-transform: uppercase;
            letter-spacing: 2px;
            color: #fff;
            backdrop-filter: blur(5px);
            box-shadow: 0 2px 10px rgba(0, 0, 0, 0.2);
        }

        /* Container */
        .container {
            display: flex;
            flex-wrap: wrap;
            justify-content: center;
            gap: 20px;
            padding: 20px;
        }

        /* Card Styling */
        .card {
            padding: 20px;
            width: 300px;
            border-radius: 15px;
            box-shadow: 0 8px 25px rgba(0, 0, 0, 0.2);
            transition: all 0.4s ease;
            color: #fff;
            font-size: 18px;
            position: relative;
            background: rgba(255, 255, 255, 0.1);
            backdrop-filter: blur(10px);
            border: 1px solid rgba(255, 255, 255, 0.2);
        }

        .card:hover {
            transform: translateY(-10px);
            box-shadow: 0 15px 35px rgba(0, 0, 0, 0.3);
            background: rgba(255, 255, 255, 0.15);
        }

        .card h2 {
            color: #fff;
            margin-bottom: 15px;
            text-shadow: 0 2px 5px rgba(0, 0, 0, 0.2);
        }

        /* Buttons */
        .btn {
            padding: 12px 24px;
            border: none;
            cursor: pointer;
            font-size: 16px;
            font-weight: bold;
            border-radius: 25px;
            margin-top: 15px;
            text-transform: uppercase;
            transition: 0.3s;
            background: rgba(52, 152, 219, 0.8);
            color: #fff;
            box-shadow: 0 4px 15px rgba(0, 0, 0, 0.2);
        }

        .btn:hover {
            background: rgba(52, 152, 219, 1);
            transform: translateY(-2px);
            box-shadow: 0 6px 20px rgba(0, 0, 0, 0.3);
        }

        /* Red Alert */
        .alert {
            background: rgba(231, 76, 60, 0.9) !important;
            color: white !important;
            animation: pulse 2s infinite;
        }

        @keyframes pulse {
            0% { transform: scale(1); }
            50% { transform: scale(1.02); }
            100% { transform: scale(1); }
        }

        /* LED Flicker Animation */
        @keyframes flicker {
            0% { opacity: 1; }
            50% { opacity: 0.5; }
            100% { opacity: 1; }
        }

        .led-on {
            animation: flicker 0.5s infinite;
            background: rgba(241, 196, 15, 0.8) !important;
        }

        /* Smoke Animation */
        .smoke {
            position: absolute;
            bottom: 10px;
            left: 50%;
            width: 40px;
            height: 60px;
            background: rgba(255, 255, 255, 0.5);
            border-radius: 50%;
            opacity: 0;
            animation: smoke-rise 3s linear infinite;
        }

        @keyframes smoke-rise {
            0% { transform: translateY(0); opacity: 0.8; }
            50% { opacity: 0.4; }
            100% { transform: translateY(-100px); opacity: 0; }
        }

        /* Responsive */
        @media (max-width: 768px) {
            .container {
                flex-direction: column;
                align-items: center;
            }
        }
    </style>
</head>
<body>
    <header>
        Smart Home Automation 🏠
    </header>

    <div class="container">
        <div class="card" id="water-card">
            <h2>💧 Water Level</h2>
            <p>Status: <span id="water-status">Checking...</span></p>
        </div>

        <div class="card" id="led-card">
            <h2>💡 LED Control</h2>
            <button class="btn" onclick="toggleLED()">Toggle LED</button>
            <p>Status: <span id="led-status">OFF</span></p>
        </div>

        <div class="card" id="door-card">
            <h2>🚪 Door Control</h2>
            <p>Status: <span id="door-status">CLOSED</span></p>
        </div>

        <div class="card" id="fire-card">
            <h2>🔥 Fire Detector</h2>
            <p>Status: <span id="fire-status">No Fire</span></p>
        </div>

        <div class="card" id="gas-card">
            <h2>🛢 Gas Detector</h2>
            <p>Status: <span id="gas-status">Safe</span></p>
        </div>

        <div class="card" id="smoke-card">
            <h2>💨 Smoke Detector</h2>
            <p>Status: <span id="smoke-status">Low</span></p>
        </div>

        <div class="card" id="rain-card">
            <h2>🌧 Rain Detector</h2>
            <p>Status: <span id="rain-status">Dry</span></p>
        </div>

        <div class="card" id="motion-card">
            <h2>🚶‍♂ Motion Detector</h2>
            <p>Status: <span id="motion-status">No Motion</span></p>
        </div>

        <div class="card" id="soil-card">
            <h2>🌱 Soil System</h2>
            <p>Moisture Level: <span id="soil-status">Normal</span></p>
        </div>

        <div class="card" id="temp-card">
            <h2>🌡 Temperature & Humidity</h2>
            <p>Temp: <span id="temp">22</span>°C, Humidity: <span id="humidity">60</span>%</p>
        </div>

        <div class="card" id="parking-card">
            <h2>🚗 Car Parking</h2>
            <p>Status: <span id="parking-status">Available</span></p>
        </div>
    </div>

    <script>
        function toggleLED() {
            fetch('/toggleLED')
                .then(response => response.text())
                .then(state => {
                    document.getElementById("led-status").innerText = state;
                    if (state === "ON") {
                        document.getElementById("led-card").classList.add("led-on");
                    } else {
                        document.getElementById("led-card").classList.remove("led-on");
                    }
                });
        }

        function updateSensorData() {
            fetch('/getData')
                .then(response => response.json())
                .then(data => {
                    document.getElementById("water-status").innerText = data.water;
                    document.getElementById("door-status").innerText = data.door;
                    document.getElementById("fire-status").innerText = data.fire;
                    document.getElementById("gas-status").innerText = data.gas;
                    document.getElementById("smoke-status").innerText = data.smoke;
                    document.getElementById("rain-status").innerText = data.rain;
                    document.getElementById("motion-status").innerText = data.motion;
                    document.getElementById("soil-status").innerText = data.soil;
                    document.getElementById("temp").innerText = data.temperature;
                    document.getElementById("humidity").innerText = data.humidity;
                    document.getElementById("parking-status").innerText = data.parking;

                    // Update alert classes
                    document.getElementById("fire-card").classList.toggle("alert", data.fire !== "No Fire");
                    document.getElementById("gas-card").classList.toggle("alert", data.gas !== "Safe");
                    document.getElementById("smoke-card").classList.toggle("alert", data.smoke === "High");

                    // Add smoke animation if smoke is high
                    if (data.smoke === "High") {
                        let smokeCard = document.getElementById("smoke-card");
                        let smokeElement = document.createElement('div');
                        smokeElement.classList.add('smoke');
                        smokeCard.appendChild(smokeElement);
                        setTimeout(() => smokeElement.remove(), 3000);
                    }
                });
        }

        // Update sensor data every 2 seconds
        setInterval(updateSensorData, 2000);
    </script>
</body>
</html>
)rawliteral";

void setup() {
    Serial.begin(115200);
    delay(100);
    
    // Initialize pins
    pinMode(LED_PIN, OUTPUT);
    pinMode(WATER_TRIG_PIN, OUTPUT);
    pinMode(WATER_ECHO_PIN, INPUT);
    pinMode(DOOR_TRIG_PIN, OUTPUT);
    pinMode(DOOR_ECHO_PIN, INPUT);
    pinMode(FIRE_SENSOR_PIN, INPUT);
    pinMode(GAS_SENSOR_PIN, INPUT);
    pinMode(SMOKE_SENSOR_PIN, INPUT);
    pinMode(RAIN_SENSOR_PIN, INPUT);
    pinMode(PIR_SENSOR_PIN, INPUT);
    pinMode(SOIL_SENSOR_PIN, INPUT);
    pinMode(PARKING_TRIG_PIN, OUTPUT);
    pinMode(PARKING_ECHO_PIN, INPUT);

    dht.begin();

    // Initialize WiFi Access Point
    WiFi.disconnect();   // Disconnect any previous connections
    WiFi.mode(WIFI_OFF);  // Turn off WiFi
    delay(1000);
    
    WiFi.mode(WIFI_AP);  // Set mode to ACCESS POINT
    delay(100);
    
    // Configure AP
    WiFi.softAP(ap_ssid, ap_password);
    delay(2000); // Give time for AP to start
    
    IPAddress IP = WiFi.softAPIP();
    Serial.println("");
    Serial.println("WiFi AP is started");
    Serial.print("AP SSID: ");
    Serial.println(ap_ssid);
    Serial.print("AP IP address: ");
    Serial.println(IP);
    
    // Print MAC Address
    Serial.print("MAC Address: ");
    Serial.println(WiFi.macAddress());

    // Set up web server routes
    server.on("/", HTTP_GET, []() {
        server.send(200, "text/html", index_html);
    });
    
    server.on("/getData", HTTP_GET, handleGetData);
    server.on("/toggleLED", HTTP_GET, handleToggleLED);
    
    // Start server
    server.begin();
    Serial.println("HTTP server started");
}

void loop() {
    server.handleClient();
    
    // Print connected clients every 10 seconds
    static unsigned long lastPrint = 0;
    if (millis() - lastPrint > 10000) {
        Serial.printf("Stations connected = %d\n", WiFi.softAPgetStationNum());
        lastPrint = millis();
    }
}

float readUltrasonicDistance(int trigPin, int echoPin) {
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    
    long duration = pulseIn(echoPin, HIGH);
    return duration * 0.034 / 2;
}

void handleGetData() {
    float waterLevel = readUltrasonicDistance(WATER_TRIG_PIN, WATER_ECHO_PIN);
    float doorDistance = readUltrasonicDistance(DOOR_TRIG_PIN, DOOR_ECHO_PIN);
    bool fireDetected = analogRead(FIRE_SENSOR_PIN) < 500;
    bool gasDetected = analogRead(GAS_SENSOR_PIN) > 700;
    int smokeLevel = analogRead(SMOKE_SENSOR_PIN);
    bool isRaining = digitalRead(RAIN_SENSOR_PIN) == LOW;
    bool motionDetected = digitalRead(PIR_SENSOR_PIN) == HIGH;
    int soilMoisture = analogRead(SOIL_SENSOR_PIN);
    float temp = dht.readTemperature();
    float humidity = dht.readHumidity();
    float parkingDistance = readUltrasonicDistance(PARKING_TRIG_PIN, PARKING_ECHO_PIN);

    String jsonResponse = "{";
    jsonResponse += "\"water\":\"" + String(waterLevel < 10 ? "High" : "Low") + "\",";
    jsonResponse += "\"door\":\"" + String(doorDistance < 5 ? "OPEN" : "CLOSED") + "\",";
    jsonResponse += "\"fire\":\"" + String(fireDetected ? "🔥 Fire Detected!" : "No Fire") + "\",";
    jsonResponse += "\"gas\":\"" + String(gasDetected ? "⚠ Gas Leak!" : "Safe") + "\",";
    jsonResponse += "\"smoke\":\"" + String(smokeLevel > 700 ? "High" : "Low") + "\",";
    jsonResponse += "\"rain\":\"" + String(isRaining ? "Raining" : "Dry") + "\",";
    jsonResponse += "\"motion\":\"" + String(motionDetected ? "Motion Detected" : "No Motion") + "\",";
    jsonResponse += "\"soil\":\"" + String(soilMoisture < 500 ? "Dry" : "Normal") + "\",";
    jsonResponse += "\"temperature\":" + String(temp) + ",";
    jsonResponse += "\"humidity\":" + String(humidity) + ",";
    jsonResponse += "\"parking\":\"" + String(parkingDistance < 20 ? "Occupied" : "Available") + "\"";
    jsonResponse += "}";

    server.send(200, "application/json", jsonResponse);
}

void handleToggleLED() {
    static bool ledState = false;
    ledState = !ledState;
    digitalWrite(LED_PIN, ledState);
    server.send(200, "text/plain", ledState ? "ON" : "OFF");
}
