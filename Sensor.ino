#include <AccelStepper.h>
#include <Servo.h>
#include <LiquidCrystal_74HC595.h>

// Stepper Motor 1 (original)
#define MP1 8
#define MP2 11
#define MP3 12
#define MP4 13

#define trigPin 10
#define echoPin 9
#define MotorInterfaceType 8 // FULL4WIRE

// LCD via 74HC595 Shift Register
#define DP 4 // Arduino pins
#define STCP 5
#define SHCP 6
#define RS 1 // Shift register output pins
#define E 2
#define D4 3
#define D5 4
#define D6 5
#define D7 6

const int thermistorPin = A1;
const int soundPin = A0;  // Analog pin connected to SY-M213 A0

LiquidCrystal_74HC595 lcd(DP, SHCP, STCP, RS, E, D4, D5, D6, D7);
AccelStepper stepper1 = AccelStepper(MotorInterfaceType, MP4, MP3, MP2, MP1);
Servo myServo;

const int runTime = 10000;
unsigned long startTime = millis();
bool triggered = false;

// New variables for periodic ambient data sending
unsigned long previousAmbientMillis = 0;
const long ambientInterval = 5000; // Send ambient data every 5 seconds (5000 ms)

void setup()
{
  stepper1.setMaxSpeed(800);
  stepper1.setSpeed(800);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(soundPin, INPUT); // Initialize sound pin

  myServo.attach(3);
  myServo.write(50);

  lcd.begin(16, 2);
  Serial.begin(9600);
}

void loop()
{
  // --- Periodic Ambient Sensor Data Sending ---
  unsigned long currentMillisLoop = millis(); // Use a different variable name to avoid conflict
  if (currentMillisLoop - previousAmbientMillis >= ambientInterval) {
    previousAmbientMillis = currentMillisLoop;

    float ambientTemp = readTempC();
    int ambientAudio = readAudioLevel(); // Ensure readAudioLevel() is implemented

    Serial.print("AMBIENT_TEMP:");
    Serial.print(ambientTemp);
    Serial.print(",AUDIO:");
    Serial.println(ambientAudio);
  }

  // --- Original Trigger and Classification Logic ---
  float distance = getDistance();

  if (distance < 8 && !triggered)
  {
    delay(2000);
    Serial.println("trigger");
    triggered = true;
  }

  if (Serial.available())
  {
    String input = Serial.readStringUntil('\n');
    input.trim();

    // Display classification on LCD
    print_LCD(input);

    startTime = millis();
    if (input == "Non-Food")
    {
      while (millis() - startTime < runTime)
      {
        stepper1.runSpeed();
        myServo.write(90);
      }
    }
    else
    {
      while (millis() - startTime < runTime)
      {
        stepper1.runSpeed();
        myServo.write(40);
      }
    }

    myServo.write(50);

    // Read sensor values (post-classification, potentially more immediate)
    float currentTemp = readTempC();
    int currentAudio = readAudioLevel();

    // Send data to Python script (post-classification) - using DATA_ prefix
    Serial.print("DATA_TEMP:"); 
    Serial.print(currentTemp);
    Serial.print(",AUDIO:");
    Serial.println(currentAudio);

    Serial.println("Done"); // Now send Done
    triggered = false;
  }
}

float getDistance()
{
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH);
  return duration * 0.0343 / 2;
}

void print_LCD(String item)
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("The item is...");
  lcd.setCursor(0, 1);
  lcd.print(item);
  delay(3500);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Moving on");
  delay(2000);
}

float readTempC() {
  int analogValue = analogRead(thermistorPin);

  // Assuming a 10k thermistor with a 10k pull-up/pull-down resistor.
  // Adjust R_fixed (10000.0) if your fixed resistor is different.
  // analogRead is 0-1023. Using 1023.0 for calculation if analogValue can reach 1023.
  // If your board uses a different ADC range (e.g. 4095), adjust 1023.0 accordingly.
  double resistance = 10000.0 * (1023.0 / (float)analogValue - 1.0);
  double lnR = log(resistance);  // Natural log of resistance

  // Steinhart-Hart equation coefficients (typical for a 10k NTC thermistor)
  // These might need adjustment for your specific thermistor model.
  const double A = 0.001129148;
  const double B = 0.000234125;
  const double C = 0.0000000876741; 

  double tempK = 1.0 / (A + (B * lnR) + (C * lnR * lnR * lnR));

  return (float)(tempK - 273.15);  // Convert Kelvin to Celsius
}

// Reads the audio level using the readSound() function.
int readAudioLevel() {
  // Call readSound which returns an average analog value (0-1023)
  float soundAnalogAverage = readSound();
  
  // For now, we'll cast this float to an int for consistency with previous placeholder.
  // You might want to map this value to a more meaningful scale (e.g., pseudo dB)
  // For example: return map(soundAnalogAverage, 0, 1023, 0, 100);
  return (int)soundAnalogAverage; 
}

// Function to compute and return average sound level from SY-M213
float readSound() {
  long sum = 0;
  for (int i = 0; i < 10; i++) {
    sum += analogRead(soundPin);
    delay(5);  // Small delay between readings
  }
  return sum / 10.0;  // Return as float (average analog reading)
}
