#include <WiFi.h>
#include <WebServer.h>

// Access Point credentials
const char* ssid = "ESP32-CarPark";
const char* password = "12345678";

// Ultrasonic sensor pins
#define TRIG_PIN 4
#define ECHO_PIN 5

// Threshold for parking detection (in cm)
#define PARKED_DISTANCE 15

// Create a WebServer object
WebServer server(80);

// Function to measure distance
int measureDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH);
  return duration * 0.034 / 2; // Convert duration to distance (in cm)
}

// HTML Page
const char htmlPage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Advanced Car Parking Animation</title>
  <style>
    body {
      font-family: 'Arial', sans-serif;
      display: flex;
      flex-direction: column;
      align-items: center;
      justify-content: center;
      height: 100vh;
      margin: 0;
      background: linear-gradient(135deg, #d7e9f7, #b0bec5);
    }
    .parking-area {
      position: relative;
      width: 600px;
      height: 400px;
      background: #546e7a;
      border: 5px solid #37474f;
      border-radius: 15px;
      box-shadow: 0 15px 30px rgba(0, 0, 0, 0.3);
      overflow: hidden;
    }
    .port {
      position: absolute;
      top: 50%;
      left: 50%;
      transform: translate(-50%, -50%);
      width: 150px;
      height: 100px;
      background: rgba(0, 150, 255, 0.2);
      border: 3px dashed #0288d1;
      border-radius: 10px;
      animation: glow 2s infinite;
    }
    @keyframes glow {
      0%, 100% {
        box-shadow: 0 0 10px rgba(0, 150, 255, 0.5);
      }
      50% {
        box-shadow: 0 0 20px rgba(0, 150, 255, 0.8);
      }
    }
    .car {
      position: absolute;
      bottom: 20px;
      left: -150px;
      width: 130px;
      height: 70px;
      background: #d50000;
      border-radius: 15px 15px 10px 10px;
      transition: all 2s ease-in-out;
      box-shadow: 0 8px 15px rgba(0, 0, 0, 0.4);
    }
    .car::before, .car::after {
      content: '';
      position: absolute;
      width: 25px;
      height: 25px;
      background: black;
      border-radius: 50%;
      bottom: -10px;
    }
    .car::before {
      left: 15px;
    }
    .car::after {
      right: 15px;
    }
    .car-top {
      position: absolute;
      top: 10px;
      left: 25px;
      width: 80px;
      height: 40px;
      background: #b71c1c;
      border-radius: 5px 5px 0 0;
    }
    .car-window {
      position: absolute;
      top: 8px;
      left: 10px;
      width: 60px;
      height: 20px;
      background: #e0f7fa;
      border-radius: 3px;
    }
    .status {
      margin-top: 20px;
      font-size: 1.5rem;
      font-weight: bold;
      color: #37474f;
      transition: all 0.5s ease-in-out;
      opacity: 1;
    }
  </style>
</head>
<body>
  <div class="parking-area">
    <div class="port"></div>
    <div class="car" id="car">
      <div class="car-top">
        <div class="car-window"></div>
      </div>
    </div>
  </div>
  <div class="status" id="status">Car is ready to park. 🚗</div>

  <script>
    async function fetchStatus() {
      const response = await fetch('/status');
      const data = await response.json();

      const car = document.querySelector('.car');
      const status = document.querySelector('#status');

      if (data.parked) {
        car.style.left = '225px';
        car.style.bottom = '140px';
        status.textContent = 'The car is parked perfectly! ✅';
        status.style.color = '#4caf50';
      } else {
        car.style.left = '-150px';
        car.style.bottom = '20px';
        status.textContent = `Car is not parked. Distance: ${data.distance} cm`;
        status.style.color = '#37474f';
      }
    }

    setInterval(fetchStatus, 1000);
  </script>
</body>
</html>
)rawliteral";

void setup() {
  Serial.begin(115200);

  // Configure ultrasonic sensor pins
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // Start the Access Point
  WiFi.softAP(ssid, password);
  Serial.print("Access Point started. IP: ");
  Serial.println(WiFi.softAPIP());

  // Define REST endpoint for parking status
  server.on("/status", []() {
    int distance = measureDistance();
    bool parked = (distance >= 0 && distance <= PARKED_DISTANCE);
    String json = "{\"distance\":" + String(distance) + ",\"parked\":" + (parked ? "true" : "false") + "}";
    server.send(200, "application/json", json);
  });

  // Serve the HTML page
  server.on("/", []() {
    server.send_P(200, "text/html", htmlPage);
  });

  // Start the server
  server.begin();
}

void loop() {
  server.handleClient();
}
