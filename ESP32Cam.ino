#include "esp_camera.h"
#include <WiFi.h>

// Select camera model
#define CAMERA_MODEL_AI_THINKER
#include "camera_pins.h"

// Replace with your network credentials
const char *ssid = "";
const char *password = "";

// http server initialized on port 80
WiFiServer server(80);

void startCamera()
{
    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sscb_sda = SIOD_GPIO_NUM;
    config.pin_sscb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_JPEG;
    // Smaller resolution for faster transfer
    config.frame_size = FRAMESIZE_QVGA;
    config.jpeg_quality = 10;
    config.fb_count = 1;

    // Try to init camera
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK)
    {
        Serial.printf("Camera init failed: 0x%x\n", err);
        return;
    }
}

void setup()
{
    Serial.begin(115200);
    pinMode(4, OUTPUT);
    digitalWrite(4, LOW);

    // Connect to WiFi network
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    // Print the IP address assigned
    Serial.println("\nWiFi connected!");
    Serial.print("Camera ready at: http://");
    Serial.println(WiFi.localIP());

    // Start the Camera and HTTP server
    startCamera();
    server.begin();
}

void loop()
{
    WiFiClient client = server.available();
    if (!client)
        return;

    Serial.println("Client connected");

    // Wait until client sends data
    while (!client.available())
        delay(1);

    String request = client.readStringUntil('\r');
    client.readStringUntil('\n');
    Serial.print("Request: ");
    Serial.println(request);

    // Check if the HTTP request is for the /capture endpoint
    if (request.indexOf("GET /capture") != -1)
    {
        // turn on flash
        digitalWrite(4, HIGH);
        delay(200);

        // Warm up dummy capture
        camera_fb_t *dummy = esp_camera_fb_get();
        if (dummy)
            esp_camera_fb_return(dummy);

        delay(100);

        // Capture the image from camera
        camera_fb_t *fb = esp_camera_fb_get();

        // turn off flash after capture
        digitalWrite(4, LOW);

        // If the image capture failed, send a 500 Internal Server Error
        if (!fb)
        {
            Serial.println("Camera capture failed");
            client.println("HTTP/1.1 500 Internal Server Error");
            client.println("Content-Type: text/plain");
            client.println("Connection: close");
            client.println();
            client.println("Camera capture failed");
            return;
        }

        // Send a successful HTTP response with the image
        client.println("HTTP/1.1 200 OK");
        client.println("Content-Type: image/jpeg");
        client.println("Content-Length: " + String(fb->len));
        client.println("Connection: close");
        client.println();

        // Write the image data directly to the client
        client.write(fb->buf, fb->len);
        esp_camera_fb_return(fb);

        Serial.println("Image sent");
    }
    else
    {
        // If the request is not /capture, respond with 404 Not Found
        client.println("HTTP/1.1 404 Not Found");
        client.println("Content-Type: text/plain");
        client.println("Connection: close");
        client.println();
        client.println("Use /capture to get an image.");
    }
    // Close the connection to the client
    client.stop();
    Serial.println("Client disconnected");
}
