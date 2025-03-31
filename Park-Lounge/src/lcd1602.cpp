#include "lcd1602.h"
#include <Wire.h>

lcd1602::lcd1602()
{
    lcd = new LiquidCrystal_I2C(0x27, 16, 2); // I2C-Adresse 0x27, 16x2 Display
}

void lcd1602::clear()
{
    lcd->clear();
}

void lcd1602::setCursor(int x, int y)
{
    lcd->setCursor(x, y);
}

void lcd1602::print(String message)
{
    lcd->clear();
    lcd->setCursor(0, 0);
    lcd->print(message);
}

void lcd1602::setupLCD()
{
    Wire.begin(21, 22); // SDA = Pin 21, SCL = Pin 22 (ESP32 Standard)
    lcd->init();        // Initialisiert das LCD
    lcd->backlight();   // Hintergrundbeleuchtung einschalten
    lcd->clear();       // Display leeren
    lcd->setCursor(0, 0);
    lcd->print("Ready!");
    delay(1000); // 1 Sekunde anzeigen
    lcd->clear();
}

void lcd1602::displayDefaultMessage()
{
    lcd->clear();
    lcd->setCursor(0, 0);
    lcd->print("Enter Combination:");
}