#include <WiFi.h>
#include <WebServer.h>

// Define web server on port 80
WebServer server(80);

// Soil moisture sensor pin
#define MOISTURE_SENSOR_PIN 34 // Adjust to your pin
#define MAX_ADC_VALUE 4095     // ESP32 ADC resolution

// Network credentials for Access Point mode
const char* ssid = "ESP32_Soil_Monitor";
const char* password = "12345678";

// Function to get the soil moisture percentage
int getSoilMoisturePercentage() {
  int total = 0;
  const int samples = 10; // Number of samples for averaging

  for (int i = 0; i < samples; i++) {
    total += analogRead(MOISTURE_SENSOR_PIN);
    delay(10); // Short delay between readings
  }
  int averageReading = total / samples;

  // Convert ADC reading to percentage
  int moisturePercentage = map(averageReading, 0, MAX_ADC_VALUE, 100, 0); // Invert to match "dry = 0%, wet = 100%"
  moisturePercentage = constrain(moisturePercentage, 0, 100); // Clamp values to 0-100%
  return moisturePercentage;
}

// Function to create the HTML page
String createHTML() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Soil Water Potential</title>
  <style>
    body {
      display: flex;
      flex-direction: column;
      justify-content: center;
      align-items: center;
      height: 100vh;
      margin: 0;
      background-color: #f4f4f9;
      font-family: Arial, sans-serif;
    }
    h1 {
      margin: 20px 0;
      font-size: 2rem;
      color: #2c3e50;
    }
    .container {
      position: relative;
      width: 200px;
      height: 200px;
      border-radius: 50%;
      border: 5px solid #3498db;
      overflow: hidden;
      background-color: #f4f4f9;
    }
    .ripple-container {
      position: absolute;
      bottom: 0;
      width: 100%;
      height: 100%;
      overflow: hidden;
    }
    .ripple {
      position: absolute;
      width: 100%;
      height: 100%;
      border-radius: 50%;
      background: radial-gradient(circle, rgba(255, 255, 255, 0.5) 0%, rgba(255, 255, 255, 0) 70%);
      animation: ripple-animation 2s infinite linear;
      opacity: 0.6;
    }
    .ripple:nth-child(2) {
      animation-delay: 0.5s;
      opacity: 0.4;
    }
    .ripple:nth-child(3) {
      animation-delay: 1s;
      opacity: 0.3;
    }
    .water {
      position: absolute;
      bottom: 0;
      width: 100%;
      height: 0;
      background: linear-gradient(to top, #4A90E2, #6EC1E4);
      transition: height 1s ease-out;
    }
    .label {
      position: absolute;
      top: 50%;
      left: 50%;
      transform: translate(-50%, -50%);
      font-size: 1.5rem;
      font-weight: bold;
      color: #3498db;
      transition: color 0.3s ease;
    }
    @keyframes ripple-animation {
      0% {
        transform: scale(1);
        opacity: 0.6;
      }
      100% {
        transform: scale(1.5);
        opacity: 0;
      }
    }
  </style>
</head>
<body>
  <h1>Soil Water Potential</h1>
  <div class="container">
    <div class="ripple-container">
      <div class="ripple"></div>
      <div class="ripple"></div>
      <div class="ripple"></div>
    </div>
    <div class="water" id="water"></div>
    <div class="label" id="label">0%</div>
  </div>
  <script>
    async function fetchSoilMoisture() {
      try {
        const response = await fetch('/soil-moisture');
        const data = await response.json();
        return data.moisture;
      } catch (error) {
        console.error('Error fetching soil moisture:', error);
        return null;
      }
    }
    async function updateWaterLevel() {
      const soilMoisture = await fetchSoilMoisture();
      if (soilMoisture !== null) {
        const water = document.getElementById('water');
        const label = document.getElementById('label');
        const height = Math.max(0, Math.min(soilMoisture, 100));
        water.style.height = `${height}%`;
        label.textContent = `${height}%`;
        if (height < 30) label.style.color = 'red';
        else if (height < 70) label.style.color = 'orange';
        else label.style.color = 'green';
      }
    }
    setInterval(updateWaterLevel, 2000);
  </script>
</body>
</html>
)rawliteral";
  return html;
}

// Handle root page request
void handleRoot() {
  String htmlPage = createHTML();
  server.send(200, "text/html", htmlPage);
}

// Handle soil moisture API request
void handleSoilMoistureAPI() {
  int moistureLevel = getSoilMoisturePercentage();
  String jsonResponse = "{\"moisture\":" + String(moistureLevel) + "}";
  server.send(200, "application/json", jsonResponse);
}

void setup() {
  // Initialize serial for debugging
  Serial.begin(115200);

  // Configure soil moisture sensor pin
  pinMode(MOISTURE_SENSOR_PIN, INPUT);

  // Set up Access Point
  WiFi.softAP(ssid, password);
  Serial.println("Access Point started");
  Serial.print("IP Address: ");
  Serial.println(WiFi.softAPIP());  // Print the IP address

  // Start web server
  server.on("/", handleRoot);
  server.on("/soil-moisture", handleSoilMoistureAPI);
  server.begin();
  Serial.println("Web server started");
}

void loop() {
  // Handle client requests
  server.handleClient();
}
