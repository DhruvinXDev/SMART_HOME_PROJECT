#include <WiFi.h>
#include <WebServer.h>

// Access Point credentials
const char* ssid = "ESP32_AP"; // Access Point Name
const char* password = "12345678"; // Access Point Password

// Pin Definitions
const int pirPin = 14; // PIR Sensor pin
const int ledPin = 16; // LED pin

bool motionDetected = false;

// Create a WebServer object on port 80
WebServer server(80);

// Generate the webpage with animated status
void handleRoot() {
  String html = R"rawliteral(
    <!DOCTYPE html>
    <html lang="en">
    <head>
      <meta charset="UTF-8">
      <meta name="viewport" content="width=device-width, initial-scale=1.0">
      <title>ESP32 Motion Detector</title>
      <style>
        body {
          font-family: Arial, sans-serif;
          background: #282828;
          color: white;
          text-align: center;
          display: flex;
          justify-content: center;
          align-items: center;
          height: 100vh;
          margin: 0;
          flex-direction: column;
          transition: background 1s ease;
        }
        h1 {
          font-size: 3em;
          animation: slideIn 2s ease-out;
        }
        @keyframes slideIn {
          0% { opacity: 0; transform: translateY(-50px); }
          100% { opacity: 1; transform: translateY(0); }
        }
        .status {
          font-size: 2em;
          font-weight: bold;
          margin: 30px;
          animation: blink 2s infinite alternate;
        }
        .motion {
          color: #32ff7e;
          animation: pulseGreen 1s infinite alternate;
        }
        .nomotion {
          color: #ff4d4d;
          animation: pulseRed 1s infinite alternate;
        }
        @keyframes blink {
          0% { opacity: 1; }
          50% { opacity: 0.7; }
          100% { opacity: 1; }
        }
        @keyframes pulseGreen {
          0% { transform: scale(1); color: #32ff7e; }
          100% { transform: scale(1.2); color: #28d498; }
        }
        @keyframes pulseRed {
          0% { transform: scale(1); color: #ff4d4d; }
          100% { transform: scale(1.2); color: #f44336; }
        }
        .led {
          margin-top: 40px;
          width: 100px;
          height: 100px;
          border-radius: 50%;
          background-color: grey;
          box-shadow: 0 0 10px rgba(0, 0, 0, 0.7);
          transition: background-color 0.5s ease;
        }
        .motion .led {
          background-color: #32ff7e;
          box-shadow: 0 0 20px #32ff7e;
        }
        .nomotion .led {
          background-color: #ff4d4d;
          box-shadow: 0 0 20px #ff4d4d;
        }
      </style>
      <script>
        // Auto-refresh the page every second
        setTimeout(function(){
          window.location.reload();
        }, 1000);
      </script>
    </head>
    <body>
      <h1>ESP32 Motion Detector</h1>
      <div class="status %STATUS_CLASS%">
        %STATUS_TEXT%
      </div>
      <div class="led %STATUS_CLASS%"></div>
      <p>Refreshing every second...</p>
    </body>
    </html>
  )rawliteral";

  // Update the dynamic content
  if (motionDetected) {
    html.replace("%STATUS_CLASS%", "motion");
    html.replace("%STATUS_TEXT%", "Motion Detected!");
  } else {
    html.replace("%STATUS_CLASS%", "nomotion");
    html.replace("%STATUS_TEXT%", "No Motion");
  }

  server.send(200, "text/html", html);
}

void setup() {
  // Initialize serial communication
  Serial.begin(115200);

  // Set up pins for PIR and LED
  pinMode(pirPin, INPUT);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  // Set up Access Point
  Serial.println("Setting up Access Point...");
  WiFi.softAP(ssid, password);
  Serial.println("Access Point Created!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.softAPIP());

  // Set up root URL handler
  server.on("/", handleRoot);

  // Start the web server
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  // Handle client requests
  server.handleClient();

  // Read the PIR sensor value
  motionDetected = digitalRead(pirPin);

  // Control the LED based on the PIR sensor status
  if (motionDetected) {
    digitalWrite(ledPin, HIGH); // Turn on LED
    Serial.println("Motion Detected!");
  } else {
    digitalWrite(ledPin, LOW); // Turn off LED
    Serial.println("No Motion.");
  }

  delay(100); // Delay to reduce unnecessary reads
}