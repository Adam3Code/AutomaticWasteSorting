// Exercise13 _partB

#include <LiquidCrystal_74HC595.h>
// Due to the limited number of pins on the Arduino Uno, we decided to use the 74HC595 shift register.  
// This allows us to expand 3 Arduino Uno pins into 8 additional output pins, enabling us to connect all the necessary components for the exercise.  
// By using the LiquidCrystal_74HC595 library (https://github.com/matmunk/LiquidCrystal_74HC595),  
// we were able to easily connect the LCD 1602 module to the 74HC595 shift register and fully utilize its features.
#define DP 7 // Pins on the Arduino Uno
#define STCP 6 // Pins on the Arduino Uno
#define SHCP 5 // Pins on the Arduino Uno
#define RS 1 // Pins on the 74HC595 shift register
#define E 2 // Pins on the 74HC595 shift register
#define D4 3 // Pins on the 74HC595 shift register
#define D5 4 // Pins on the 74HC595 shift register
#define D6 5 // Pins on the 74HC595 shift register
#define D7 6 // Pins on the 74HC595 shift register

LiquidCrystal_74HC595 lcd(DP, SHCP, STCP, RS, E, D4, D5, D6, D7);

// Starting time
float start_time = 1000.00;

// Setup function
void setup() {
    lcd.begin(16, 2);
}

// Loop function
void loop() {
  String p = "Food";
  print_LCD(p);

}

void print_LCD(String item){
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

