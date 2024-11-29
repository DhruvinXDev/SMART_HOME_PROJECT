#include <WiFi.h>
#include <WebServer.h>

// Access Point credentials
const char* ssid = "ESP32-LED-AP";
const char* password = "12345678";

// LED pin
#define LED_PIN 14

// Create WebServer instance
WebServer server(80);

// LED state
bool ledState = false;

// HTML content with swapped animation
void handleRoot() {
  String html = R"rawliteral(
    <!DOCTYPE html>
    <html lang="en">
    <head>
      <meta charset="UTF-8">
      <meta name="viewport" content="width=device-width, initial-scale=1.0">
      <title>ESP32 LED Control</title>
      <style>
        body {
          font-family: Arial, sans-serif;
          display: flex;
          flex-direction: column;
          align-items: center;
          justify-content: center;
          height: 100vh;
          margin: 0;
          background: linear-gradient(45deg, #6a6a6a, #3a3a3a);
          color: white;
        }
        h1 {
          margin-bottom: 20px;
        }
        .led {
          width: 50px;
          height: 50px;
          border-radius: 50%;
          background-color: red;
          box-shadow: 0 0 20px red;
          margin-bottom: 20px;
          transition: background-color 0.3s ease, box-shadow 0.3s ease;
        }
        .led.off {
          background-color: #111;
          box-shadow: 0 0 10px black;
        }
        button {
          padding: 10px 20px;
          font-size: 16px;
          border: none;
          border-radius: 5px;
          background-color: #444;
          color: white;
          cursor: pointer;
          transition: background-color 0.3s ease;
        }
        button:hover {
          background-color: #666;
        }
      </style>
    </head>
    <body>
      <h1>ESP32 LED Control</h1>
      <div id="led" class="led"></div>
      <button id="toggleBtn">Turn ON</button>

      <script>
        const led = document.getElementById('led');
        const toggleBtn = document.getElementById('toggleBtn');

        // Function to update the LED state
        function updateLED(state) {
          if (state === "ON") {
            led.classList.add('off'); // LED looks off when state is ON
            toggleBtn.textContent = 'Turn OFF';
          } else {
            led.classList.remove('off'); // LED looks on when state is OFF
            toggleBtn.textContent = 'Turn ON';
          }
        }

        // Event listener for the button
        toggleBtn.addEventListener('click', () => {
          const action = toggleBtn.textContent === 'Turn ON' ? '/led/on' : '/led/off';
          fetch(action)
            .then(() => {
              updateLED(toggleBtn.textContent === 'Turn ON' ? "ON" : "OFF");
            })
            .catch(err => console.error("Error:", err));
        });

        // Fetch initial state on page load
        fetch('/led/state')
          .then(response => response.text())
          .then(updateLED)
          .catch(err => console.error("Error fetching initial state:", err));
      </script>
    </body>
    </html>
  )rawliteral";
  server.send(200, "text/html", html);
}

void handleLEDOn() {
  ledState = true;
  digitalWrite(LED_PIN, HIGH);
  server.send(200, "text/plain", "ON");
}

void handleLEDOff() {
  ledState = false;
  digitalWrite(LED_PIN, LOW);
  server.send(200, "text/plain", "OFF");
}

void handleLEDState() {
  server.send(200, "text/plain", ledState ? "ON" : "OFF");
}

void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);

  // Configure LED pin
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // Create Access Point
  WiFi.softAP(ssid, password);
  Serial.println("Access Point Started");
  Serial.print("AP IP Address: ");
  Serial.println(WiFi.softAPIP());

  // Define web server routes
  server.on("/", handleRoot);
  server.on("/led/on", handleLEDOn);
  server.on("/led/off", handleLEDOff);
  server.on("/led/state", handleLEDState);

  // Start the server
  server.begin();
  Serial.println("Server started");
}

void loop() {
  // Handle client requests
  server.handleClient();
}
