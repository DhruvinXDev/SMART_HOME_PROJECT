#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

// Wi-Fi credentials for hotspot
const char* ssid = "ESP32_DoorControl";  // Name of the ESP32 hotspot
const char* password = "12345678";       // Password for the hotspot

// Ultrasonic sensor pins
const int TRIG_PIN = 5;  // Trigger pin
const int ECHO_PIN = 18; // Echo pin

// Distance threshold (in cm) to determine if the door is open
const int CLOSE_THRESHOLD = 5;  // Door is closed if distance is less than 5 cm
bool doorOpen = false;

// Create an AsyncWebServer instance
AsyncWebServer server(80);

// HTML Page
const char htmlPage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Ultrasonic Door Control</title>
    <style>
        body {
            margin: 0;
            height: 100vh;
            display: flex;
            justify-content: center;
            align-items: center;
            font-family: Arial, sans-serif;
            background: linear-gradient(120deg, #89f7fe, #66a6ff);
            background-size: 400% 400%;
            animation: gradientAnimation 10s ease infinite;
        }
        @keyframes gradientAnimation {
            0% { background-position: 0% 50%; }
            50% { background-position: 100% 50%; }
            100% { background-position: 0% 50%; }
        }
        .square {
            width: 220px;
            height: 220px;
            background-color: rgba(34, 34, 34, 0.9);
            position: relative;
            display: flex;
            justify-content: center;
            align-items: center;
            border: 5px solid rgba(0, 0, 0, 0.8);
            box-shadow: 0 10px 20px rgba(0, 0, 0, 0.5);
            border-radius: 10px;
        }
        .door {
            width: 90px;
            height: 180px;
            background-color: #8B4513;
            border: 4px solid #5A2E0F;
            position: absolute;
            left: 0;
            transform-origin: left center;
            transition: transform 1s ease-in-out;
            display: flex;
            justify-content: flex-end;
            align-items: center;
        }
        .door-handle {
            width: 10px;
            height: 10px;
            background-color: #FFD700;
            border-radius: 50%;
            margin-right: 10px;
        }
        .status {
            position: absolute;
            top: -50px;
            color: #fff;
            font-size: 24px;
            font-weight: bold;
            text-shadow: 1px 1px 4px rgba(0, 0, 0, 0.8);
        }
    </style>
</head>
<body>
    <div class="square">
        <div class="status" id="status">Door is Closed</div>
        <div class="door" id="door">
            <div class="door-handle"></div>
        </div>
    </div>
    <script>
        const statusText = document.getElementById('status');
        const door = document.getElementById('door');
        async function checkDoorStatus() {
            try {
                // Fetch door status from ESP32
                const response = await fetch('/door-status');
                const { doorOpen } = await response.json();
                // Update door animation and status text
                if (doorOpen) {
                    door.style.transform = 'rotateY(120deg)';
                    statusText.textContent = 'Door is Open';
                } else {
                    door.style.transform = 'rotateY(0deg)';
                    statusText.textContent = 'Door is Closed';
                }
            } catch (error) {
                console.error('Error fetching door status:', error);
            }
        }
        // Poll the ESP32 every 500ms (0.5 seconds)
        setInterval(checkDoorStatus, 500);
    </script>
</body>
</html>
)rawliteral";

// Function to calculate distance using ultrasonic sensor
long measureDistance() {
    // Send a 10us pulse to trigger pin
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);

    // Measure the duration of the echo pulse
    long duration = pulseIn(ECHO_PIN, HIGH);

    // Convert the time into distance in cm
    long distance = duration * 0.034 / 2; // Speed of sound = 0.034 cm/µs
    return distance;
}

void setup() {
    // Initialize serial monitor
    Serial.begin(115200);

    // Set up ultrasonic sensor pins
    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);

    // Set up the ESP32 as a Wi-Fi hotspot
    WiFi.softAP(ssid, password);
    IPAddress IP = WiFi.softAPIP(); // Get the IP address of the ESP32
    Serial.println("Wi-Fi Hotspot Started");
    Serial.print("IP Address: ");
    Serial.println(IP);

    // Serve the HTML page
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send_P(200, "text/html", htmlPage);
    });

    // Serve the door status
    server.on("/door-status", HTTP_GET, [](AsyncWebServerRequest *request) {
        // Update the door state based on ultrasonic sensor reading
        long distance = measureDistance();
        doorOpen = distance >= CLOSE_THRESHOLD; // Door is open if distance is 5 cm or more
        String response = "{\"doorOpen\": " + String(doorOpen ? "true" : "false") + "}";
        request->send(200, "application/json", response);
    });

    // Start server
    server.begin();
}

void loop() {
    // Periodically check the door status and print it to the Serial Monitor
    long distance = measureDistance();
    doorOpen = distance >= CLOSE_THRESHOLD; // Door is open if distance is 5 cm or more

    // Print the current door status for debugging
    Serial.print("Distance: ");
    Serial.print(distance);
    Serial.print(" cm, Door status: ");
    Serial.println(doorOpen ? "Open" : "Closed");

    // The server will automatically send the updated door status to the client when requested

    delay(500); // Wait for half a second before checking again
}
