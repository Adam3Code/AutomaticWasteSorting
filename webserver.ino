#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WiFiMulti.h>   // Include the Wi-Fi-Multi library
#include <ESP8266WebServer.h>   // Include the WebServer library
#include <ESP8266mDNS.h>        // Include the mDNS library
// #include <FS.h>        // Removed
// #include <LittleFS.h>  // Removed

ESP8266WiFiMulti wifiMulti;
// Create an instance of the server
ESP8266WebServer server(80);

const int led = D2;

// --- New Counters ---
int foodCount = 3; // Example starting value
int nonFoodCount = 6; // Example starting value
const int MAX_BIN_CAPACITY = 10; // Defin

float temperature = 23.5; // Simulated temperature value (°C)
int audioLevel = 62;      // Simulated audio level (dB)

// const char* IMAGE_FILE_PATH = "/latest_waste.jpg"; // Removed
// File fsUploadFile; // Removed

void handleRoot();  
void handleLED();  
void handleNotFound();
void handleResult(); // Forward declare if not already
void handleSensorData(); // Declare the new handler
void handleEmptyBin(); // Declare handler for emptying bins
// void handleFileUpload(); // Removed
// void handleServeImage(); // Removed
// void formatLittleFS();   // Removed

void setup() {
  Serial.begin(115200);
  delay(1000);

  // LittleFS.begin() and related removed
  // Serial.println("LittleFS mounted successfully."); // Removed

  server.on("/", HTTP_GET, handleRoot);
  server.on("/result", HTTP_POST, handleResult);
  server.on("/sensordata", HTTP_POST, handleSensorData);
  server.on("/empty", HTTP_GET, handleEmptyBin);
  // server.on(IMAGE_FILE_PATH, HTTP_GET, handleServeImage); // Removed
  // server.on("/uploadimage", ..., handleFileUpload); // Removed
  server.onNotFound(handleNotFound);

  pinMode(led, OUTPUT);
  digitalWrite(led,1);
  
  // Connect to WiFi network
  Serial.println();

  wifiMulti.addAP("Majstor Bob", "ratat123");  
  
  Serial.println();
  Serial.print("Connecting ...");
  //WiFi.begin(ssid, password);
 
  while (wifiMulti.run() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected to ");
  Serial.println(WiFi.SSID());
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("iot")) {              // Start the mDNS responder for esp8266.local
    Serial.println("mDNS responder started");
  } else {
    Serial.println("Error setting up MDNS responder!");
  }

  server.begin();
  Serial.println("Server started"); 
}

void loop() {
  // Check if a client has connected
  server.handleClient();


}

void handleRoot() {
  int foodFullness = min((foodCount * 100) / MAX_BIN_CAPACITY, 100);
  int nonFoodFullness = min((nonFoodCount * 100) / MAX_BIN_CAPACITY, 100);

  String html = String("<!DOCTYPE html>");
  html += "<html lang='en'>";
  html += "<head>";
  html += "<meta charset='UTF-8'>";
  html += "<meta http-equiv='refresh' content='5'>";
  html += "<title>Automatic Waste Sorting</title>";
  html += "<style>";
  html += "body { background-color: #1e1e1e; color: #e0e0e0; font-family: 'Segoe UI', Tahoma, sans-serif; display: flex; flex-direction: column; align-items: center; padding-top: 30px; }";
  html += "h1 { color: #4CAF50; margin-bottom: 20px; }";
  html += ".main-container { display: flex; flex-direction: column; align-items: center; gap: 30px; }";
  html += ".bins-container { display: flex; justify-content: center; gap: 50px; }";
  html += ".bin { background-color: #333; border: 2px solid #555; border-radius: 10px; padding: 20px; width: 200px; text-align: center; position: relative; height: 300px; display: flex; flex-direction: column; justify-content: flex-end; }";
  html += ".bin h2 { margin-top: 0; margin-bottom: 15px; color: #4CAF50; }";
  html += ".bin-label { position: absolute; top: 10px; left: 0; right: 0; z-index: 2; }";
  html += ".fill-level { position: absolute; bottom: 0; left: 0; width: 100%; z-index: 1; transition: height 0.5s ease-in-out; }";
  html += ".bin.food .fill-level { background-color: #8B4513; }";
  html += ".bin.non-food .fill-level { background-color: #FFA500; }";
  html += ".counter { font-size: 1.4em; font-weight: bold; z-index: 2; position: relative; margin-top: 10px; background: rgba(0,0,0,0.5); padding: 5px; border-radius: 5px; }";
  html += ".warning { color: red; font-weight: bold; z-index: 2; position: relative; margin-top: 10px; }";
  html += ".empty-button { display: none; margin-top: 10px; padding: 8px; color: white; border: none; border-radius: 5px; cursor: pointer; z-index:3; }";
  html += ".food .empty-button { background-color: #4CAF50; }";
  html += ".non-food .empty-button { background-color: #FFA500; }";
  // Image container style removed
  html += ".sensor-box { padding: 20px; background-color: #2a2a2a; border-radius: 10px; text-align: center; width: 300px; }"; // Ensure this style is present if image container is removed
  html += "</style>";
  html += "</head>";
  html += "<body>";
  html += "<h1>Waste Management Status</h1>";
  html += "<div class='main-container'>";
  // Image container div and img tag removed
  html += "<div class='bins-container'>";
  html += "<div class='bin food'>";
  html += "<div class='bin-label'><h2>Food Waste</h2></div>";
  html += "<div class='fill-level' id='foodFill'></div>";
  html += "<div class='counter'>Items: <span id='foodCountDisplay'>?</span></div>";
  html += "<div id='foodWarning' class='warning' style='display: none;'>⚠️ Bin Full!</div>";
  html += "<button id='emptyFoodButton' class='empty-button' onclick='emptyBin(\"food\")'>Empty Food Bin</button>";
  html += "</div>";
  html += "<div class='bin non-food'>";
  html += "<div class='bin-label'><h2>General Waste</h2></div>";
  html += "<div class='fill-level' id='nonFoodFill'></div>";
  html += "<div class='counter'>Items: <span id='nonFoodCountDisplay'>?</span></div>";
  html += "<div id='nonFoodWarning' class='warning' style='display: none;'>⚠️ Bin Full!</div>";
  html += "<button id='emptyNonFoodButton' class='empty-button' onclick='emptyBin(\"nonfood\")'>Empty General Bin</button>";
  html += "</div>";
  html += "</div>";
  html += "<div class='sensor-box'>";
  html += "<h2>Environment</h2>";
  html += "<p>🌡️ Temperature: <strong>" + String(temperature, 1) + "°C</strong></p>";
  html += "<p>🔊 Noise Level: <strong>" + String(audioLevel) + " (analog)</strong></p>";
  html += "</div>";
  html += "</div>";
  html += "<script>";
  html += "let foodItems = " + String(foodCount) + ";";
  html += "let nonFoodItems = " + String(nonFoodCount) + ";";
  html += "let maxCapacity = " + String(MAX_BIN_CAPACITY) + ";";
  html += "let foodFullnessPercent = Math.min((foodItems / maxCapacity) * 100, 100);";
  html += "let nonFoodFullnessPercent = Math.min((nonFoodItems / maxCapacity) * 100, 100);";
  html += "document.getElementById('foodCountDisplay').innerText = foodItems;";
  html += "document.getElementById('nonFoodCountDisplay').innerText = nonFoodItems;";
  html += "document.getElementById('foodFill').style.height = foodFullnessPercent + '%';";
  html += "document.getElementById('nonFoodFill').style.height = nonFoodFullnessPercent + '%';";
  html += "if (foodItems >= maxCapacity) { document.getElementById('foodWarning').style.display = 'block'; document.getElementById('emptyFoodButton').style.display = 'block'; } else { document.getElementById('foodWarning').style.display = 'none'; document.getElementById('emptyFoodButton').style.display = 'none'; }";
  html += "if (nonFoodItems >= maxCapacity) { document.getElementById('nonFoodWarning').style.display = 'block'; document.getElementById('emptyNonFoodButton').style.display = 'block'; } else { document.getElementById('nonFoodWarning').style.display = 'none'; document.getElementById('emptyNonFoodButton').style.display = 'none'; }";
  html += "function emptyBin(binType) { if (confirm('Are you sure you want to mark the ' + binType + ' bin as empty?')) { window.location.href = '/empty?bin=' + binType; } }";
  html += "</script>";
  html += "</body>";
  html += "</html>";

  server.send(200, "text/html", html);
}

void handleLED() {
  digitalWrite(led,!digitalRead(led));
  server.sendHeader("Location","/");
  server.send(303);
}

void handleNotFound(){
  server.send(404, "text/plain", "404: Not found");
}

void handleResult() {
  if (server.hasArg("classification")) {
    String classification = server.arg("classification");
    classification.toLowerCase();

    Serial.print("Received classification: ");
    Serial.println(classification);

    if (classification == "food") {
      foodCount++;
    } else if (classification == "non-food") {
      nonFoodCount++;
    } else {
      server.send(400, "text/plain", "Invalid classification value");
      return;
    }
    server.send(200, "text/plain", "Classification received");
  } else {
    server.send(400, "text/plain", "Missing classification");
  }
}

void handleSensorData() {
  if (server.hasArg("temperature") && server.hasArg("audio")) {
    String tempArg = server.arg("temperature");
    String audioArg = server.arg("audio");

    temperature = tempArg.toFloat();
    audioLevel = audioArg.toInt();

    Serial.print("Received sensor data: Temp=");
    Serial.print(temperature);
    Serial.print("C, Audio=");
    Serial.println(audioLevel);

    server.send(200, "text/plain", "Sensor data received");
  } else {
    server.send(400, "text/plain", "Missing temperature or audio data arguments");
  } }

void handleEmptyBin() {
  String binToEmpty = server.arg("bin");
  if (binToEmpty == "food") {
    foodCount = 0;
    Serial.println("Food bin counter reset.");
  } else if (binToEmpty == "nonfood" || binToEmpty == "non-food") {
    nonFoodCount = 0;
    Serial.println("Non-food bin counter reset.");
  } else {
    server.send(400, "text/plain", "Invalid bin type specified for emptying.");
    return;
  }
  server.sendHeader("Location", "/", true);
  server.send(302, "text/plain", "");
}

// handleFileUpload, handleServeImage, and formatLittleFS functions are now removed.
