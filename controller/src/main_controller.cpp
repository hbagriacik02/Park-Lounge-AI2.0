#include <Arduino.h>
#include "Servo.h"
#include "lcd1602.h"
#include "ir_sensor.h"
#include "button.h"
#include "led.h"
#include "state_machine.h"

// Verwendete Pins
//  Servo: Pin 13
//  LCD: SDA = Pin 21, SCL = Pin 22
//  IR-Sensor: Pin 27
//  Rote LED: Pin 14
//  Grüne LED: Pin 25
//  Button: Pin 26

lcd1602 lcd;            // Das Display-Objekt
ServoControl servo(13); // Servo an Pin 13
IRSensor irSensor(27);  // IR-Sensor an Pin 27
LED redLED(14);         // Rote LED an Pin 14
LED greenLED(25);       // Grüne LED an Pin 25
Button button(26);      // Button an Pin 26

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200); // Startet serielle Kommunikation für Debugging
  Serial.println("Setup beginnt...");
  lcd.setupLCD(); // Initialisiert das LCD
  Serial.println("LCD initialisiert");
  servo.begin(); // Initialisiert den Servo
  Serial.println("Servo initialisiert");
  irSensor.begin(); // Initialisiert den IR-Sensor
  Serial.println("IR-Sensor initialisiert");
  redLED.begin();
  greenLED.begin();
  button.begin();
}

void loop()
{
  // put your main code here, to run repeatedly:
}
