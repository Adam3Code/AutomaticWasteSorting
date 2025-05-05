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

LiquidCrystal_74HC595 lcd(DP, SHCP, STCP, RS, E, D4, D5, D6, D7);
AccelStepper stepper1 = AccelStepper(MotorInterfaceType, MP4, MP3, MP2, MP1);
Servo myServo;

const int runTime = 10000;
unsigned long startTime = millis();
bool triggered = false;

void setup()
{
  stepper1.setMaxSpeed(800);
  stepper1.setSpeed(800);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  myServo.attach(3);
  myServo.write(50);

  lcd.begin(16, 2);
  Serial.begin(9600);
}

void loop()
{
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
    // Serial.println("Done");
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
