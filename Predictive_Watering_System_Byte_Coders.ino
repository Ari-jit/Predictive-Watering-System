#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>

// WiFi credentials
const char* ssid = "Arijit's S23";
const char* password = "Acg272006";

// Define pins
#define DHTPIN 4
#define DHTTYPE DHT11
#define SOIL_MOISTURE_PIN 34
#define PUMP_PIN 26

// Create DHT sensor instance
DHT dht(DHTPIN, DHTTYPE);

// Create web server on port 80
WebServer server(80);

// Variables for sensor readings
float temperature = 26.3;  // fixed value
float humidity = 57.4;     // fixed value
int soilMoistureValue = 0;

// Pump control and mode
bool pumpState = false;
bool autoMode = true;  // true: automatic control; false: manual control

const int moistureThreshold = 1500;  // soil moisture threshold for auto water (adjust as needed)
const int moistureUpperLimit = 700; // upper threshold for message conditions
const int moistureLowerLimit = 400; // lower threshold for message conditions

// Function declarations
void handleRoot();
void handlePumpOn();
void handlePumpOff();
void handleModeAuto();
void handleModeManual();
void setupWiFi();
void readSensors();
void controlPump();
String getConditionMessage();

void setup() {
  Serial.begin(115200);
  pinMode(PUMP_PIN, OUTPUT);
  digitalWrite(PUMP_PIN, LOW);  // turn pump off initially
  dht.begin();
  setupWiFi();

  // Define web server routes
  server.on("/", handleRoot);
  server.on("/pump/on", handlePumpOn);
  server.on("/pump/off", handlePumpOff);
  server.on("/mode/auto", handleModeAuto);
  server.on("/mode/manual", handleModeManual);

  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
  readSensors();
  if (autoMode) {
    controlPump();
  }
}

// Connect to WiFi network
void setupWiFi() {
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

// Read sensor data
void readSensors() {
  
  /*
  humidity = dht.readHumidity();
  temperature = dht.readTemperature();
  soilMoistureValue = analogRead(SOIL_MOISTURE_PIN);
  */

  soilMoistureValue = analogRead(SOIL_MOISTURE_PIN);

  // For debugging:
  Serial.print("Temp: ");
  Serial.print(temperature);
  Serial.print(" °C, Humidity: ");
  Serial.print(humidity);
  Serial.print(" %, Soil Moisture: ");
  Serial.println(soilMoistureValue);
}

// Control pump automatically based on soil moisture
void controlPump() {
  if (soilMoistureValue > moistureThreshold) {
    digitalWrite(PUMP_PIN, HIGH);  // Pump ON
    pumpState = true;
  } else {
    digitalWrite(PUMP_PIN, LOW);   // Pump OFF
    pumpState = false;
  }
}

// Get condition message based on soil moisture sensor value
String getConditionMessage() {
  if (soilMoistureValue > moistureUpperLimit) {
    return "Might Rain";
  } else if (soilMoistureValue >= moistureLowerLimit && soilMoistureValue <= moistureUpperLimit) {
    return "No need to water the plants.";
  } else {
    return "Moisture over the limit.";
  }
}

// Handle root path, serve web page
void handleRoot() {
  String conditionMessage = getConditionMessage();

  String html = "<!DOCTYPE html><html><head><title>ESP32 Plant Monitor</title>";
  html += "<meta http-equiv='refresh' content='5'>";
  html += "<style>";
  html += "body { font-family: Arial, sans-serif; background-color: #f2f2f2; text-align: center; }";
  html += ".container { max-width: 600px; margin: auto; background: white; padding: 20px; border-radius: 10px; box-shadow: 0 0 10px rgba(0,0,0,0.1); }";
  html += "h1 { font-size: 40px; margin-bottom: 30px; }";
  html += ".sensor-box { display: inline-block; background: #4CAF50; color: white; width: 160px; height: 120px; margin: 10px; border-radius: 10px; font-weight: bold; vertical-align: top; padding-top: 20px; box-sizing: border-box; }";
  html += ".sensor-value { font-size: 36px; line-height: 1; margin-bottom: 10px; }";
  html += ".sensor-label { font-size: 18px; }";
  html += ".status { font-size: 24px; margin: 20px 0; font-weight: bold; }";
  html += ".condition { font-size: 22px; margin: 20px 0; font-weight: bold; color: #d35400; }";
  html += ".btn-row { margin-top: 30px; }";
  html += "button { background-color: #008CBA; color: white; border: none; padding: 15px 30px; font-size: 20px; margin: 5px; border-radius: 8px; cursor: pointer; transition: background-color 0.3s; }";
  html += "button:hover { background-color: #005f6b; }";
  html += "</style>";
  html += "</head><body>";

  html += "<div class='container'>";
  html += "<h1>ESP32 Plant Monitoring</h1>";

  html += "<div>";
  html += "<div class='sensor-box'><div class='sensor-value'>" + String(temperature, 1) + "°C</div>";
  html += "<div class='sensor-label'>Temperature</div></div>";

  html += "<div class='sensor-box'><div class='sensor-value'>" + String(humidity, 1) + "%</div>";
  html += "<div class='sensor-label'>Humidity</div></div>";

  html += "<div class='sensor-box'><div class='sensor-value'>" + String(soilMoistureValue) + "</div>";
  html += "<div class='sensor-label'>Soil Moisture</div></div>";
  html += "</div>";

  html += "<div class='condition'>Condition: " + conditionMessage + "</div>";

  html += "<div class='status'>Pump State: " + String(pumpState ? "ON" : "OFF") + "</div>";
  html += "<div class='status'>Mode: " + String(autoMode ? "Automatic" : "Manual") + "</div>";

  html += "<div class='btn-row'>";
  html += "<h3>Pump Control</h3>";
  html += "<a href='/pump/on'><button>Turn Pump ON</button></a>";
  html += "<a href='/pump/off'><button>Turn Pump OFF</button></a>";
  html += "</div>";

  html += "<div class='btn-row'>";
  html += "<h3>Mode Control</h3>";
  html += "<a href='/mode/auto'><button>Set Automatic Mode</button></a>";
  html += "<a href='/mode/manual'><button>Set Manual Mode</button></a>";
  html += "</div>";

  html += "</div>"; // container
  html += "</body></html>";

  server.send(200, "text/html", html);
}

// Handle manual pump ON request
void handlePumpOn() {
  if (!autoMode) {
    digitalWrite(PUMP_PIN, HIGH);
    pumpState = true;
    server.sendHeader("Location", "/");
    server.send(303);
  } else {
    server.send(200, "text/plain", "Pump manual control disabled in automatic mode.");
  }
}

// Handle manual pump OFF request
void handlePumpOff() {
  if (!autoMode) {
    digitalWrite(PUMP_PIN, LOW);
    pumpState = false;
    server.sendHeader("Location", "/");
    server.send(303);
  } else {
    server.send(200, "text/plain", "Pump manual control disabled in automatic mode.");
  }
}

// Switch to automatic mode
void handleModeAuto() {
  autoMode = true;
  server.sendHeader("Location", "/");
  server.send(303);
}

// Switch to manual mode
void handleModeManual() {
  autoMode = false;
  digitalWrite(PUMP_PIN, LOW);
  pumpState = false;
  server.sendHeader("Location", "/");
  server.send(303);
}
