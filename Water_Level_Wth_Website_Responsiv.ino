#include <WiFi.h>
#include <WebServer.h>

// Set up the web server on port 80
WebServer server(80);

// Ultrasonic sensor pins
#define TRIG_PIN 5
#define ECHO_PIN 18

// Tank height in meters (5 cm = 0.05 meters)
const float tankHeight = 1;  // 5 cm

long duration;
float distance;
float waterLevel;

// Function to get water level percentage
float getWaterLevelPercentage() {
  // Trigger the ultrasonic sensor
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Read the echo time
  duration = pulseIn(ECHO_PIN, HIGH);

  // Calculate the distance (in meters)
  distance = duration * 0.034 / 2 / 100;  // Convert to meters

  // Calculate the water level
  if (distance >= tankHeight) {
    return 0;  // Tank is empty
  } else {
    waterLevel = tankHeight - distance;
    return (waterLevel / tankHeight) * 100;  // Return percentage
  }
}

// HTML for the web page with enhanced animation and responsive design
String createHTML(float levelPercentage) {
  String html = "<!DOCTYPE html>";
  html += "<html><head><meta http-equiv='refresh' content='1'/>";  // Refresh every 1 second
  html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>"; // Responsive for mobile
  html += "<title>Water Level Monitoring</title>";
  html += "<style>";
  
  // Universal styles
  html += "body { font-family: 'Arial', sans-serif; background: linear-gradient(to right, #141e30, #243b55); color: white; text-align: center; margin: 0; padding: 0; }";
  
  // Container for the water tank and labels
  html += ".container { display: flex; flex-direction: column; justify-content: center; align-items: center; padding: 20px; }";
  
  // CSS for tank design
  html += ".tank { width: 150px; height: 400px; border-radius: 10px; border: 4px solid white; position: relative; background-color: #222; margin: 0 auto; overflow: hidden; }";
  
  // CSS for the water inside the tank
  html += ".water { width: 100%; background: linear-gradient(to bottom, #00e5ff, #0099cc); position: absolute; bottom: 0; height: " + String(levelPercentage) + "%; border-bottom-left-radius: 7px; border-bottom-right-radius: 7px; transition: height 0.3s ease-in-out; animation: shimmer 2s infinite ease-in-out; }";

  // CSS for shimmering effect
  html += "@keyframes shimmer {";
  html += "  0%, 100% { background-position: 0% 50%; }";
  html += "  50% { background-position: 100% 50%; }";
  html += "}";

  // Heading style
  html += "h1 { font-size: 2.5em; margin: 20px 0; text-shadow: 2px 2px #000; }";
  
  // Label style for displaying the percentage
  html += ".label { font-size: 1.8em; font-weight: bold; margin-top: 20px; }";

  // Responsive layout adjustments
  html += "@media only screen and (max-width: 600px) {";
  html += "  .tank { width: 100px; height: 250px; }";  // Smaller tank on mobile
  html += "  h1 { font-size: 2em; }";  // Smaller heading on mobile
  html += "  .label { font-size: 1.5em; }";  // Smaller label text on mobile
  html += "}";

  html += "</style></head><body>";
  
  // Content for the water level page
  html += "<h1>Water Level Monitoring System</h1>";
  html += "<div class='container'>";
  html += "<div class='tank'><div class='water'></div></div>";
  html += "<p class='label'>Water Level: " + String(levelPercentage) + "%</p>";
  html += "</div>";
  
  html += "</body></html>";
  return html;
}

// Handle root page request
void handleRoot() {
  float waterLevelPercent = getWaterLevelPercentage();
  String htmlPage = createHTML(waterLevelPercent);
  server.send(200, "text/html", htmlPage);
}

void setup() {
  // Initialize serial for debugging
  Serial.begin(115200);

  // Set ultrasonic sensor pins
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // Set up ESP32 as an access point
  const char* ssid = "ESP32-WaterLevel";
  const char* password = "12345678";
  WiFi.softAP(ssid, password);

  Serial.println("Access Point started");
  Serial.print("IP Address: ");
  Serial.println(WiFi.softAPIP());  // Print the IP address

  // Start the web server
  server.on("/", handleRoot);
  server.begin();
  Serial.println("Web server started");
}

void loop() {
  // Handle client requests
  server.handleClient();
}
