#include <Arduino.h>
#include "Servo.h"
#include "lcd1602.h"
#include "state_machine.h"

lcd1602 lcd;            // Das Display-Objekt
ServoControl servo(13); // Servo an Pin 13

// put function declarations here:
// int myFunction(int, int);

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200); // Startet serielle Kommunikation für Debugging
  Serial.println("Setup beginnt...");
  lcd.setupLCD(); // Initialisiert das LCD
  Serial.println("LCD initialisiert");
  servo.begin(); // Initialisiert den Servo
  Serial.println("Servo initialisiert");
}

void loop()
{
  // put your main code here, to run repeatedly:
}

// put function definitions here:
// int myFunction(int x, int y) {
//   return x + y;
// }