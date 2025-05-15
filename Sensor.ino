#include <AccelStepper.h>
#include <Servo.h>
#include <LiquidCrystal_74HC595.h>

// Stepper Motor 1
#define MP1 8
#define MP2 11
#define MP3 12
#define MP4 13

#define trigPin 10
#define echoPin 9
#define MotorInterfaceType 8

// LCD via 74HC595 Shift Register
#define DP 4
#define STCP 5
#define SHCP 6
#define RS 1 // Shift register output pins
#define E 2
#define D4 3
#define D5 4
#define D6 5
#define D7 6

const int thermistorPin = A1;
const int soundPin = A0;

LiquidCrystal_74HC595 lcd(DP, SHCP, STCP, RS, E, D4, D5, D6, D7);
AccelStepper stepper1 = AccelStepper(MotorInterfaceType, MP4, MP3, MP2, MP1);
Servo myServo;

const int runTime = 20000;
unsigned long startTime = millis();
bool triggered = false;

// New variables for periodic ambient data sending
unsigned long previousAmbientMillis = 0;
const long ambientInterval = 5000;

void setup()
{
  stepper1.setMaxSpeed(800);
  stepper1.setSpeed(800);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(soundPin, INPUT);

  myServo.attach(3);
  myServo.write(50);

  lcd.begin(16, 2);
  Serial.begin(9600);
}

void loop()
{
  // Periodic Ambient Sensor Data Sending
  unsigned long currentMillisLoop = millis();
  if (currentMillisLoop - previousAmbientMillis >= ambientInterval)
  {
    previousAmbientMillis = currentMillisLoop;

    float ambientTemp = readTempC();
    int ambientAudio = readAudioLevel();

    Serial.print("AMBIENT_TEMP:");
    Serial.print(ambientTemp);
    Serial.print(",AUDIO:");
    Serial.println(ambientAudio);
  }

  // Detect trash within range
  float distance = getDistance();

  if (distance < 8 && !triggered)
  {
    delay(2000);
    // send signal serially to python to indicate object detection
    Serial.println("trigger");
    triggered = true;
  }

  // await response from python client
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
        myServo.write(70);
      }
    }
    else
    {
      while (millis() - startTime < runTime)
      {
        stepper1.runSpeed();
        myServo.write(30);
      }
    }

    // reset servo hand
    myServo.write(50);

    // Read sensor values
    float currentTemp = readTempC();
    int currentAudio = readAudioLevel();

    // Send sensor data to Python script
    Serial.print("DATA_TEMP:");
    Serial.print(currentTemp);
    Serial.print(",AUDIO:");
    Serial.println(currentAudio);

    Serial.println("Done");
    triggered = false;
  }
}

// get distance from ultrasonic sensor
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

// prints the item on the LCD
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
// Read temperature from thermistor
float readTempC()
{
  int analogValue = analogRead(thermistorPin);

  // Using  10k resistor
  double resistance = 10000.0 * (1023.0 / (float)analogValue - 1.0);
  double lnR = log(resistance);

  // Steinhart-Hart equation coefficients
  const double A = 0.001129148;
  const double B = 0.000234125;
  const double C = 0.0000000876741;

  double tempK = 1.0 / (A + (B * lnR) + (C * lnR * lnR * lnR));

  // Convert Kelvin to Celsius
  return (float)(tempK - 273.15);
}

// Reads the audio level using the readSound() function.
int readAudioLevel()
{
  float soundAnalogAverage = readSound();
  return (int)soundAnalogAverage;
}

// Function to compute and return average sound level from SY-M213
float readSound()
{
  long sum = 0;
  for (int i = 0; i < 10; i++)
  {
    sum += analogRead(soundPin);
    delay(5);
  }
  return sum / 10.0;
}
