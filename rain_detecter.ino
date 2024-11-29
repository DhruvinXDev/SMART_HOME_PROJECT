#include <WiFi.h>

// Set the Access Point credentials
const char* apSSID = "ESP32_Rain_AP";  // Access Point SSID
const char* apPassword = "12345678";   // Access Point Password

// Define rain sensor pin (change pin as per your setup)
const int rainSensorPin = 34; // Example pin number
int rainStatus = 0;

// Create a Wi-Fi server on port 80
WiFiServer server(80);

void setup() {
  // Start serial communication
  Serial.begin(115200);

  // Set ESP32 as an Access Point
  WiFi.softAP(apSSID, apPassword);
  Serial.println("Access Point Started");

  // Print the IP address of the ESP32
  Serial.print("Access Point IP Address: ");
  Serial.println(WiFi.softAPIP());

  // Start the server
  server.begin();

  // Set rain sensor pin as input
  pinMode(rainSensorPin, INPUT);
}

void loop() {
  WiFiClient client = server.available();  // Listen for incoming clients
  
  if (client) {
    String request = "";
    
    // Read the request from the client
    while (client.available()) {
      char c = client.read();
      request += c;
    }

    // Print the request to the Serial monitor
    Serial.println(request);

    // Serve the HTML page
    if (request.indexOf("GET / ") != -1) {
      String html = "<!DOCTYPE html><html lang='en'><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1.0'><title>Weather Animation</title><style>body {margin: 0; height: 100vh; display: flex; justify-content: center; align-items: center; font-family: Arial, sans-serif; overflow: hidden; background: linear-gradient(45deg, #ff9a9e, #fad0c4, #fbc2eb, #a6c1ee); background-size: 400% 400%; animation: backgroundShift 10s infinite;} @keyframes backgroundShift { 0% { background-position: 0% 50%; } 25% { background-position: 50% 50%; } 50% { background-position: 100% 50%; } 75% { background-position: 50% 100%; } 100% { background-position: 0% 50%; }} .square {position: relative; width: 350px; height: 350px; border-radius: 15px; overflow: hidden; display: flex; justify-content: center; align-items: center; transition: background-color 1s ease; border: 2px solid transparent; animation: glowingBorder 1.5s infinite alternate;} .square.sunny { background-color: #ffeb99; } .square.rainy { background-color: #4a6572; } .sun { position: absolute; top: 10%; left: 50%; width: 100px; height: 100px; background: radial-gradient(circle, yellow, orange); border-radius: 50%; box-shadow: 0 0 50px rgba(255, 223, 0, 0.8); animation: sunPulse 5s infinite alternate;} @keyframes sunPulse { 0% { transform: scale(1); } 100% { transform: scale(1.1); }} .rain { position: absolute; width: 100%; height: 100%; visibility: hidden; } .rain-drop { position: absolute; width: 2px; height: 15px; background: blue; animation: fall 0.7s linear infinite; border-radius: 2px;} @keyframes fall { 0% { top: -10%; opacity: 0; } 50% { opacity: 1; } 100% { top: 110%; opacity: 0; }} .status-message { position: absolute; bottom: 10px; width: 100%; text-align: center; font-size: 1.2em; font-weight: bold; color: #000;} .toggle-btn { padding: 10px 20px; background: #007bff; color: white; border: none; border-radius: 5px; cursor: pointer;} .toggle-btn:hover { background: #0056b3;} @keyframes glowingBorder { 0% { border-color: #0b3d91; box-shadow: 0 0 5px #0b3d91, 0 0 10px #0b3d91; } 50% { border-color: #1a56db; box-shadow: 0 0 5px #1a56db, 0 0 10px #1a56db; } 100% { border-color: #0b3d91; box-shadow: 0 0 5px #0b3d91, 0 0 10px #0b3d91; }} </style></head><body>";
      html += "<div class='square' id='square'>";
      html += "<div class='sun' id='sun'></div>";
      html += "<div class='rain' id='rain'>";
      html += "<div class='rain-drop' style='left: 10%; animation-delay: 0s;'></div>";
      html += "<div class='rain-drop' style='left: 30%; animation-delay: 0.2s;'></div>";
      html += "<div class='rain-drop' style='left: 50%; animation-delay: 0.4s;'></div>";
      html += "<div class='rain-drop' style='left: 70%; animation-delay: 0.6s;'></div>";
      html += "<div class='rain-drop' style='left: 90%; animation-delay: 0.8s;'></div>";
      html += "</div>";
      html += "<div class='status-message' id='statusMessage'>Sunny Weather</div>";
      html += "</div>";
      html += "<script>setInterval(function(){ fetch('/weather').then(response => response.text()).then(data => { document.getElementById('statusMessage').innerText = data; if(data == 'Rainy Weather'){ document.getElementById('square').classList.add('rainy'); document.getElementById('square').classList.remove('sunny'); document.getElementById('sun').style.visibility = 'hidden'; document.getElementById('rain').style.visibility = 'visible'; } else { document.getElementById('square').classList.add('sunny'); document.getElementById('square').classList.remove('rainy'); document.getElementById('sun').style.visibility = 'visible'; document.getElementById('rain').style.visibility = 'hidden'; } }); }, 1000);</script></body></html>";
      client.print("HTTP/1.1 200 OK\r\n");
      client.print("Content-Type: text/html\r\n");
      client.print("Connection: close\r\n\r\n");
      client.print(html);
    }

    // Handle /weather request
    if (request.indexOf("GET /weather") != -1) {
      rainStatus = digitalRead(rainSensorPin);
      String weather = (rainStatus == HIGH) ? "Rainy Weather" : "Sunny Weather";
      client.print("HTTP/1.1 200 OK\r\n");
      client.print("Content-Type: text/plain\r\n");
      client.print("Connection: close\r\n\r\n");
      client.print(weather);
    }

    // Close the connection after serving the request
    client.stop();
  }
}
