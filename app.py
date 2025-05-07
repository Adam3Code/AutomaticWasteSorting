import requests
import ollama
import time
import serial

ESP32_CAM_IP = '192.168.43.23'
ESP32_CAM_URL = f'http://{ESP32_CAM_IP}/capture'
IMAGE_PATH = 'captured.jpg'


ESP8266_SERVER_IP = 'http://192.168.216.41'
ESP8266_RESULT_URL = f'http://{ESP8266_SERVER_IP}/result'

SERIAL_PORT = 'COM7'
BAUD_RATE = 9600

print("[*] Starting main loop...")

try:
    with serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=2) as arduino:
        time.sleep(2)  # Allow Arduino to reset

        while True:
            # 1. Wait for "trigger"
            print("[*] Waiting for Arduino trigger...")
            while True:
                if arduino.in_waiting:
                    line = arduino.readline().decode().strip()
                    print(f"[Serial] Received: {line}")
                    if line == "trigger":
                        break
                time.sleep(0.1)

            # 2. Capture image
            print("[*] Requesting image from ESP32-CAM...")
            response = requests.get(ESP32_CAM_URL, stream=True)

            if response.status_code == 200:
                with open(IMAGE_PATH, 'wb') as f:
                    for chunk in response.iter_content(chunk_size=1024):
                        f.write(chunk)
                print(f"[+] Image saved to {IMAGE_PATH}")
            else:
                print(f"[!] Failed to get image from ESP32-CAM, status: {response.status_code}")
                continue

            # 3. Send to Ollama
            print("[*] Sending image to Ollama...")
            response = ollama.chat(
                model='llava:7b',
                messages=[{
                    'role': 'user',
                    'content': 'Look at the image. If the object is food, reply with "Food". If it is not food, reply with "Non-Food". Only reply with one of those words.',
                    'images': [IMAGE_PATH]
                }]
            )

            classification = response['message']['content'].strip()
            print(f"[+] Classification: {classification}")
            #4. Send classification to ESP8266
            print("[*] Sending result to ESP8266...")
        
            try:
                response = requests.post(ESP8266_RESULT_URL, data={'classification': classification})
                if response.status_code == 200:
                    print(f"[+] ESP8266 response: {response.text}")
                else:
                    print(f"[!] Failed to send result to ESP8266, status: {response.status_code}")
            except Exception as e:
                print(f"[!] Error sending result to ESP8266: {e}")
            # 5. Send classification to Arduino
            print("[*] Sending result to Arduino...")
            arduino.write((classification + '\n').encode())
            print(f"[+] Sent: {classification}")

            # 6. Wait for "Done" before restarting loop
            print("[*] Waiting for Arduino to say 'Done'...")
            while True:
                if arduino.in_waiting:
                    done_line = arduino.readline().decode().strip()
                    print(f"[Serial] Received: {done_line}")
                    if done_line == "Done":
                        print("[*] Restarting cycle...\n")
                        break
                time.sleep(0.1)

except serial.SerialException as e:
    print(f"[!] Serial Error: {e}")
