#include <AccelStepper.h>

#define MP1 8
#define MP2 11
#define MP3 12
#define MP4 13

#define trigPin 10
#define echoPin 9
#define MotorInterfaceType 8

float distance;

AccelStepper stepper = AccelStepper(MotorInterfaceType ,MP1,MP2,MP3,MP4);

const int runTime = 10000;

unsigned long startTime = millis();

void setup()
{
  stepper.setMaxSpeed(800);
  stepper.setSpeed(800);
  pinMode(trigPin, OUTPUT); // SETTING OUTPUT PIN
  pinMode(echoPin, INPUT); // SETTING INPUT PIN
  Serial.begin(9600); // INITIALISING THE COMMUNICATION
}

void loop() {
  // Measure distance once
  distance = getDistance();
      
  Serial.print("Distance: ");

  Serial.println(distance);
  if (distance < 20) {
   startTime = millis();
    while(millis() - startTime < runTime) {
      stepper.runSpeed();   // Keep motor running
    }
  } 

  
}

// Function to measure distance
float getDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH);
  return duration * 0.0343 / 2;  // Convert to cm
}
