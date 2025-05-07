#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WiFiMulti.h>   // Include the Wi-Fi-Multi library
#include <ESP8266WebServer.h>   // Include the WebServer library
#include <ESP8266mDNS.h>        // Include the mDNS library

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

void handleRoot();  
void handleLED();  
void handleNotFound();

void setup() {
  Serial.begin(115200);
  delay(1000);
  server.on("/result", HTTP_POST, handleResult);  // Add this line

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



  server.on("/", HTTP_GET, handleRoot);
  server.onNotFound(handleNotFound);
    
  // Start the server
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

  String html = "<!DOCTYPE html>"
                "<html lang='en'>"
                "<head>"
                "<meta charset='UTF-8'>"
                "<meta http-equiv='refresh' content='5'>"
                "<title>Automatic Waste Sorting</title>"
                "<style>"
                "body { background-color: #1e1e1e; color: #e0e0e0; font-family: 'Segoe UI', Tahoma, sans-serif; display: flex; flex-direction: column; align-items: center; padding-top: 30px; }"
                "h1 { color: #4CAF50; margin-bottom: 20px; }"
                ".bins-container { display: flex; justify-content: center; gap: 50px; }"
                ".bin { background-color: #333; border: 2px solid #555; border-radius: 10px; padding: 20px; width: 200px; text-align: center; position: relative; height: 300px; display: flex; flex-direction: column; justify-content: flex-end; }"
                ".bin h2 { margin-top: 0; margin-bottom: 15px; color: #4CAF50; }"
                ".bin-label { position: absolute; top: 10px; left: 0; right: 0; z-index: 2; }"
                ".fill-level { position: absolute; bottom: 0; left: 0; width: 100%; z-index: 1; transition: height 0.5s ease-in-out; }"
                ".bin.food .fill-level { background-color: #8B4513; }"
                ".bin.non-food .fill-level { background-color: #FFA500; }"
                ".counter { font-size: 1.4em; font-weight: bold; z-index: 2; position: relative; margin-top: 10px; background: rgba(0,0,0,0.5); padding: 5px; border-radius: 5px; }"
                ".warning { color: red; font-weight: bold; z-index: 2; position: relative; margin-top: 10px; }"
                ".sensor-box { margin-top: 30px; padding: 20px; background-color: #2a2a2a; border-radius: 10px; text-align: center; }"
                "</style>"
                "</head>"
                "<body>"
                "<h1>Waste Management Status</h1>"
                "<div class='bins-container'>"
                "<div class='bin food'>"
                "<div class='bin-label'><h2>Food Waste</h2></div>"
                "<div class='fill-level' id='foodFill'></div>"
                "<div class='counter'>Items: <span id='foodCountDisplay'>?</span></div>"
                "<div id='foodWarning' class='warning' style='display: none;'>⚠️ Bin Full!</div>"
                "</div>"
                "<div class='bin non-food'>"
                "<div class='bin-label'><h2>General Waste</h2></div>"
                "<div class='fill-level' id='nonFoodFill'></div>"
                "<div class='counter'>Items: <span id='nonFoodCountDisplay'>?</span></div>"
                "<div id='nonFoodWarning' class='warning' style='display: none;'>⚠️ Bin Full!</div>"
                "</div>"
                "</div>"
                "<div class='sensor-box'>"
                "<h2>Environment</h2>"
                "<p>🌡️ Temperature: <strong>" + String(temperature, 1) + "°C</strong></p>"
                "<p>🔊 Noise Level: <strong>" + String(audioLevel) + " dB</strong></p>"
                "</div>"
                "<script>"
                "let foodItems = " + String(foodCount) + ";"
                "let nonFoodItems = " + String(nonFoodCount) + ";"
                "let maxCapacity = " + String(MAX_BIN_CAPACITY) + ";"
                "let foodFullnessPercent = Math.min((foodItems / maxCapacity) * 100, 100);"
                "let nonFoodFullnessPercent = Math.min((nonFoodItems / maxCapacity) * 100, 100);"
                "document.getElementById('foodCountDisplay').innerText = foodItems;"
                "document.getElementById('nonFoodCountDisplay').innerText = nonFoodItems;"
                "document.getElementById('foodFill').style.height = foodFullnessPercent + '%';"
                "document.getElementById('nonFoodFill').style.height = nonFoodFullnessPercent + '%';"
                "if (foodItems >= maxCapacity) document.getElementById('foodWarning').style.display = 'block';"
                "if (nonFoodItems >= maxCapacity) document.getElementById('nonFoodWarning').style.display = 'block';"
                "</script>"
                "</body>"
                "</html>";

  server.send(200, "text/html", html);
}


void handleLED() {                          // If a POST request is made to URI /LED
  digitalWrite(led,!digitalRead(led));      // Change the state of the LED
  server.sendHeader("Location","/");        // Add a header to respond with a new location for the browser to go to the home page again
  server.send(303);                         // Send it back to the browser with an HTTP status 303 (See Other) to redirect
}

void handleNotFound(){
  server.send(404, "text/plain", "404: Not found"); // Send HTTP status 404 (Not Found) when there's no handler for the URI in the request
}

void handleResult() {
  if (server.hasArg("classification")) {
    String classification = server.arg("classification");
    classification = classification.toLowerCase();

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

