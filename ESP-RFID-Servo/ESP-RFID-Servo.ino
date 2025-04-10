#include <Servo.h>
#include <SPI.h>
#include <MFRC522.h>

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>

//pins til RFID fra højre: 3.3V, D4, GND, none, D6, D7, D5 og D8
//servo pins: den røde er til 3.3V, den orange er til D1 og den brune er til GND

#define RST_PIN D4     
#define SS_PIN  D8     

MFRC522 mfrc522(SS_PIN, RST_PIN);


const char* ssid = "Frederiks iPhone";
const char* password = "Frederik100";

Servo myservo;

WiFiClient client;
HTTPClient http;

ESP8266WebServer server(80);

int skraldeTæller = 0;


byte allowedUID[4] = {0xB2, 0x6A, 0x74, 0x54};

  void handleRoot() {
  String html = "<!DOCTYPE html>"
                "<html lang='da'>"
                "<head>"
                  "<meta charset='UTF-8'>"
                  "<meta http-equiv='refresh' content='2'>" // Auto-refresh hver 5. sekund
                  "<title>Recycle</title>"
                  "<style>"
                    "body { background-color: #121212; color: #FFA500; font-family: Arial, sans-serif; text-align: center; padding: 40px; }"
                    "h1 { font-size: 2.5em; }"
                    ".counter { font-size: 2em; margin: 20px; }"
                    ".warning { color: red; font-size: 1.5em; margin-top: 20px; }"
                  "</style>"
                "</head>"
                "<body>"
                  "<h1>Trashcan status</h1>"
                  "<div class='counter'>Items in trash can: <span id='countDisplay'>?</span></div>"
                  "<div id='warning' class='warning' style='display: none;'>⚠️ Empty trash can!</div>"

                  "<script>";

  // Indsæt den aktuelle skraldeTæller værdi fra Arduino
  html += "let count = " + String(skraldeTæller) + ";";
  
  html += "document.getElementById('countDisplay').innerText = count;";
  html += "if (count >= 10) { document.getElementById('warning').style.display = 'block'; }";

  html += "</script>"
         "</body>"
         "</html>";

  server.send(200, "text/html", html);
}

void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password);

  Serial.print("Forbinder til WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi forbundet!");
    Serial.print("ESP IP: ");
    Serial.println(WiFi.localIP());

    // Opsæt webserverens ruter
    server.on("/", handleRoot);

    server.begin();
    Serial.println("Webserver startet!");
  myservo.attach(D1);  


  myservo.write(0);


  SPI.begin();         
  mfrc522.PCD_Init();  

  Serial.println("Scan et kort");
}

void loop() {

  server.handleClient();
  
  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
    return;
  }


  Serial.print("Card UID:");
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
  }
  Serial.println();


  bool match = true;
  if (mfrc522.uid.size != 4) {
    match = false;
  } else {
    for (byte i = 0; i < 4; i++) {
      if (mfrc522.uid.uidByte[i] != allowedUID[i]) {
        match = false;
        break;
      }
    }
  }

  if (match) {
    Serial.println("Recycle");
    myservo.write(180);
    skraldeTæller += 1;
    delay(2000);
    myservo.write(0);

  } else {
    Serial.println("Not recycle");
  }

}

