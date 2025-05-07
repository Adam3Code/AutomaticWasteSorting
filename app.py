import requests
import ollama
import time
import serial

ESP32_CAM_IP = '192.168.216.51'
ESP32_CAM_URL = f'http://{ESP32_CAM_IP}/capture'
IMAGE_PATH = 'captured.jpg'

ESP8266_SERVER_IP = 'http://192.168.216.41'
ESP8266_RESULT_URL = f'{ESP8266_SERVER_IP}/result'
ESP8266_SENSORDATA_URL = f'{ESP8266_SERVER_IP}/sensordata'

SERIAL_PORT = 'COM11'
BAUD_RATE = 9600

print("[*] Starting main loop...")

try:
    with serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1) as arduino:
        time.sleep(2)  # Allow Arduino to reset

        while True:
            # 1. Wait for "trigger" OR process ambient sensor data
            print("[*] Waiting for Arduino trigger or processing ambient data...")
            trigger_received = False
            while not trigger_received:
                if arduino.in_waiting > 0:
                    line = arduino.readline().decode().strip()
                    if line: # Ensure line is not empty
                        print(f"[Serial] Received: {line}")
                        if line == "trigger":
                            trigger_received = True
                        elif line.startswith("AMBIENT_TEMP:"):
                            try:
                                parts = line.split(',')
                                temp_part = parts[0].split(':')[1]
                                audio_part = parts[1].split(':')[1]
                                temp_value = float(temp_part)
                                audio_value = int(audio_part)
                                print(f"[Ambient Data] Parsed Temp: {temp_value}°C, Audio: {audio_value} dB")
                                
                                sensor_payload = {'temperature': str(temp_value), 'audio': str(audio_value)}
                                try:
                                    response = requests.post(ESP8266_SENSORDATA_URL, data=sensor_payload, timeout=3)
                                    if response.status_code == 200:
                                        print(f"[+] Sent ambient data to ESP8266: {response.text}")
                                    else:
                                        print(f"[!] Failed to send ambient data to ESP8266, status: {response.status_code}, {response.text}")
                                except requests.exceptions.RequestException as e:
                                    print(f"[!] Error sending ambient data to ESP8266: {e}")
                            except Exception as e:
                                print(f"[!] Error parsing ambient sensor data from Arduino: {e}")
                
                if trigger_received: # Check if trigger was found in this iteration
                    break # Exit the while not trigger_received loop
                
                time.sleep(0.1) # Polling interval if no data or no trigger

            # If trigger was received, continue with the rest of the cycle
            # This part is reached ONLY if trigger_received is True

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

            # 6. Wait for Arduino to send sensor data and then "Done" before restarting loop
            print("[*] Waiting for Arduino to send post-classification sensor data and 'Done'...")
            sensor_data_received = False
            temp_value = None
            audio_value = None

            # Timeout for waiting for Done and post-classification data
            done_receive_timeout = time.time() + 10 # Wait for 10 seconds for Done and data

            while time.time() < done_receive_timeout:
                if arduino.in_waiting > 0:
                    arduino_line = arduino.readline().decode().strip()
                    if arduino_line:
                        print(f"[Serial] Received (post-classification): {arduino_line}")

                        if arduino_line.startswith("DATA_TEMP:"):
                            try:
                                # Example: DATA_TEMP:23.5,AUDIO:60
                                parts = arduino_line.split(',') 
                                temp_part = parts[0].split(':')[1] # Get value after "DATA_TEMP:"
                                audio_part = parts[1].split(':')[1] # Get value after "AUDIO:"
                                temp_value = float(temp_part)
                                audio_value = int(audio_part)
                                sensor_data_received = True
                                print(f"[Post-Class. Data] Parsed Temp: {temp_value}°C, Audio: {audio_value} dB")
                            except Exception as e:
                                print(f"[!] Error parsing post-classification sensor data from Arduino: {e}")
                                sensor_data_received = False
                                temp_value = None
                                audio_value = None
                        
                        elif arduino_line == "Done":
                            if sensor_data_received and temp_value is not None and audio_value is not None:
                                print(f"[*] Sending post-classification sensor data to ESP8266 at {ESP8266_SENSORDATA_URL}...")
                                sensor_payload = {'temperature': str(temp_value), 'audio': str(audio_value)}
                                try:
                                    response = requests.post(ESP8266_SENSORDATA_URL, data=sensor_payload, timeout=5)
                                    if response.status_code == 200:
                                        print(f"[+] ESP8266 post-classification sensor data response: {response.text}")
                                    else:
                                        print(f"[!] Failed to send post-class. sensor data to ESP8266, status: {response.status_code}, {response.text}")
                                except requests.exceptions.RequestException as e:
                                    print(f"[!] Error sending post-class. sensor data to ESP8266: {e}")
                            elif not sensor_data_received:
                                print("[!] 'Done' received from Arduino, but post-classification sensor data was not received or parsed correctly. Skipping ESP8266 update.")
                            else: 
                                print("[!] 'Done' received, sensor data parsed but values are invalid. Skipping ESP8266 update.")
                            
                            print("[*] Restarting cycle...\n")
                            break # Exit the "Done" waiting loop
                time.sleep(0.1)
            else: # This else corresponds to the while time.time() < done_receive_timeout
                print("[!] Timed out waiting for 'Done' from Arduino. Restarting cycle...")

except serial.SerialException as e:
    print(f"[!] Serial Error: {e}")
except KeyboardInterrupt:
    print("[*] Program interrupted by user. Exiting.")
finally:
    print("[*] Cleaning up...")
