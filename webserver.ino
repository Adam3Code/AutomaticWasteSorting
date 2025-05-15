#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

ESP8266WiFiMulti wifiMulti;
ESP8266WebServer server(80);

const int led = D2;
// Replace with your network credentials
const char *ssid = "";
const char *password = "";

// Initialize data counters
int foodCount = 3;
int nonFoodCount = 6;
const int MAX_BIN_CAPACITY = 10;
float temperature = 23.5;
int audioLevel = 62;

// forward declarations
void handleRoot();
// --- Counters ---
int foodCount = 6;
int nonFoodCount = 3;
const int MAX_BIN_CAPACITY = 10;

float temperature = 23.5;
int audioLevel = 62;

void handleRoot();
void handleLED();
void handleNotFound();
void handleResult();
void handleSensorData();
void handleEmptyBin();
void handleResult();
void handleSensorData();
void handleEmptyBin();

void setup()
{
  Serial.begin(115200);
  void setup()
  {
    Serial.begin(9600);
    delay(1000);

    server.on("/", HTTP_GET, handleRoot);
    server.on("/result", HTTP_POST, handleResult);
    server.on("/sensordata", HTTP_POST, handleSensorData);
    server.on("/empty", HTTP_GET, handleEmptyBin);

    server.on("/", handleRoot);
    server.on("/sensor-data", handleSensorData);
    server.onNotFound(handleNotFound);

    pinMode(led, OUTPUT);
    digitalWrite(led, 1);

    // Connect to WiFi network
    Serial.println();

    wifiMulti.addAP(ssid, password);

    Serial.println();
    wifiMulti.addAP("Majstor Bob", "ratat123");

    Serial.print("Connecting ...");

    while (wifiMulti.run() != WL_CONNECTED)
    {
      while (wifiMulti.run() != WL_CONNECTED)
      {
        delay(500);
        Serial.print(".");
      }

      Serial.println("");
      Serial.println("WiFi connected to ");
      Serial.println(WiFi.SSID());
      Serial.println("IP address: ");
      Serial.println(WiFi.localIP());

      if (MDNS.begin("iot"))
      {
        if (MDNS.begin("iot"))
        {
          Serial.println("mDNS responder started");
        }
        else
        {
        }
        else
        {
          Serial.println("Error setting up MDNS responder!");
        }

        server.begin();
        Serial.println("Server started");
        Serial.println("Server started");
      }

      void loop()
      {
        // Check if a client has connected
        void loop()
        {
          server.handleClient();
        }

        // serve html for webserver
        void handleRoot()
        {
          int foodFullness = min((foodCount * 100) / MAX_BIN_CAPACITY, 100);
          int nonFoodFullness = min((nonFoodCount * 100) / MAX_BIN_CAPACITY, 100);

          void handleRoot()
          {
            String html = String("<!DOCTYPE html>");
            html += "<html lang='en'><head><meta charset='UTF-8'>";
            html += "<title>Automatic Waste Sorting</title>";
            html += "<script src='https://cdn.jsdelivr.net/npm/chart.js'></script>";
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
            html += ".sensor-box { padding: 20px; background-color: #2a2a2a; border-radius: 10px; text-align: center; width: 300px; }";
            html += "#tempChart { width: 400px; height: 300px; margin-top: 30px; }";
            html += "</style></head><body>";

            html += "<h1>Waste Management Status</h1><div class='main-container'>";
            html += "<div class='bins-container'>";

            html += "<div class='bin food'>";
            html += "<div class='bin-label'><h2>Food Waste</h2></div>";
            html += "<div class='fill-level' id='foodFill'></div>";
            html += "<div class='counter'>Items: <span id='foodCountDisplay'>" + String(foodCount) + "</span></div>";
            html += "<div id='foodWarning' class='warning' style='display: none;'>⚠️ Bin Full!</div>";
            html += "<button id='emptyFoodButton' class='empty-button' onclick='emptyBin(\"food\")'>Empty Food Bin</button>";
            html += "</div>";

            html += "<div class='bin non-food'>";
            html += "<div class='bin-label'><h2>General Waste</h2></div>";
            html += "<div class='fill-level' id='nonFoodFill'></div>";
            html += "<div class='counter'>Items: <span id='nonFoodCountDisplay'>" + String(nonFoodCount) + "</span></div>";
            html += "<div id='nonFoodWarning' class='warning' style='display: none;'>⚠️ Bin Full!</div>";
            html += "<button id='emptyNonFoodButton' class='empty-button' onclick='emptyBin(\"nonfood\")'>Empty General Bin</button>";
            html += "</div>";

            html += "</div>"; // bins-container

            html += "<div class='sensor-box'>";
            html += "<h2>Environment</h2>";
            html += "<p>🌡️ Temperature: <strong>" + String(readTemperature(), 1) + "°C</strong></p>";
            html += "<p>🔊 Noise Level: <strong>" + String(audioLevel) + "</strong></p>";
            html += "</div>";

            html += "<canvas id='tempChart'></canvas>";
            html += "</div>"; // main-container

            html += "<script>";
            html += "let tempData = [];";
            html += "let labels = [];";
            html += "let chart = new Chart(document.getElementById('tempChart'), {";
            html += "type: 'line', data: { labels: labels, datasets: [{ label: 'Temperature (°C)', data: tempData, borderColor: '#FF5733', fill: false }] },";
            html += "options: { responsive: true, scales: { x: { title: { display: true, text: 'Time' } } } } });";

            html += "function updateUI(data) {";
            html += "document.querySelector('p strong').innerText = data.temperature.toFixed(1) + '°C';";
            html += "document.querySelectorAll('strong')[1].innerText = data.audio;";
            html += "document.getElementById('foodCountDisplay').innerText = data.food;";
            html += "document.getElementById('nonFoodCountDisplay').innerText = data.nonfood;";
            html += "let foodPercent = Math.min((data.food / " + String(MAX_BIN_CAPACITY) + ") * 100, 100);";
            html += "let nonFoodPercent = Math.min((data.nonfood / " + String(MAX_BIN_CAPACITY) + ") * 100, 100);";
            html += "document.getElementById('foodFill').style.height = foodPercent + '%';";
            html += "document.getElementById('nonFoodFill').style.height = nonFoodPercent + '%';";
            html += "document.getElementById('foodWarning').style.display = data.food >= " + String(MAX_BIN_CAPACITY) + " ? 'block' : 'none';";
            html += "document.getElementById('nonFoodWarning').style.display = data.nonfood >= " + String(MAX_BIN_CAPACITY) + " ? 'block' : 'none';";
            html += "document.getElementById('emptyFoodButton').style.display = data.food >= " + String(MAX_BIN_CAPACITY) + " ? 'block' : 'none';";
            html += "document.getElementById('emptyNonFoodButton').style.display = data.nonfood >= " + String(MAX_BIN_CAPACITY) + " ? 'block' : 'none';";
            html += "}";

            html += "setInterval(async () => {";
            html += "const res = await fetch('/sensor-data'); const data = await res.json(); updateUI(data);";
            html += "const now = new Date().toLocaleTimeString();";
            html += "if (tempData.length >= 20) { tempData.shift(); labels.shift(); }";
            html += "tempData.push(data.temperature); labels.push(now); chart.update();";
            html += "}, 5000);";

            html += "function emptyBin(binType) { if (confirm('Are you sure you want to mark the ' + binType + ' bin as empty?')) { window.location.href = '/empty?bin=' + binType; } }";
            html += "</script>";

            html += "</body></html>";

            // serve the html string
            server.send(200, "text/html", html);
          }

          void handleNotFound()
          {
            // Rest of the code for handling the result, sensor data, empty bin, etc.

            void handleLED()
            {
              server.sendHeader("Location", "/");
              server.send(303);
            }
            float readTemperature()
            {
              int analogValue = analogRead(A0);

              double resistance = 10000.0 * (1024.0 / analogValue - 1); // Calculate resistance
              double tempK = log(resistance);                           // Natural log of resistance

              // Apply Steinhart-Hart equation for thermistor
              tempK = 1.0 / (0.001129148 + (0.000234125 + (0.0000000876741 * tempK * tempK)) * tempK);

              return (float)(tempK - 273.15); // Convert Kelvin to Celsius
            }

            void handleNotFound()
            {
              server.send(404, "text/plain", "404: Not found");
            }

            void handleResult()
            {
              // handles retrieving the result of the classification
              if (server.hasArg("classification"))
              {
                void handleResult()
                {
                  if (server.hasArg("classification"))
                  {
                    String classification = server.arg("classification");
                    classification.toLowerCase();

                    Serial.print("Received classification: ");
                    Serial.println(classification);

                    if (classification == "food")
                    {
                      if (classification == "food")
                      {
                        foodCount++;
                      }
                      else if (classification == "non-food")
                      {
                      }
                      else if (classification == "non-food")
                      {
                        nonFoodCount++;
                      }
                      else
                      {
                      }
                      else
                      {
                        server.send(400, "text/plain", "Invalid classification value");
                        return;
                      }
                      server.send(200, "text/plain", "Classification received");
                    }
                    else
                    {
                    }
                    else
                    {
                      server.send(400, "text/plain", "Missing classification");
                    }
                  }

                  void handleSensorData()
                  {
                    temperature = readTemperature(); // get fresh value

                    String json = "{";
                    json += "\"temperature\":" + String(temperature, 1) + ",";
                    json += "\"audio\":" + String(audioLevel) + ",";
                    json += "\"food\":" + String(foodCount) + ",";
                    json += "\"nonfood\":" + String(nonFoodCount);
                    json += "}";
                    server.send(200, "application/json", json);
                  }

                  void handleEmptyBin()
                  {
                    // handles the mocking of emptying a full bin
                    void handleEmptyBin()
                    {
                      String binToEmpty = server.arg("bin");
                      if (binToEmpty == "food")
                      {
                        if (binToEmpty == "food")
                        {
                          foodCount = 0;
                          Serial.println("Food bin counter reset.");
                        }
                        else if (binToEmpty == "nonfood" || binToEmpty == "non-food")
                        {
                        }
                        else if (binToEmpty == "nonfood" || binToEmpty == "non-food")
                        {
                          nonFoodCount = 0;
                          Serial.println("Non-food bin counter reset.");
                        }
                        else
                        {
                        }
                        else
                        {
                          server.send(400, "text/plain", "Invalid bin type specified for emptying.");
                          return;
                        }
                        server.sendHeader("Location", "/", true);
                        server.send(302, "text/plain", "");
                      }
