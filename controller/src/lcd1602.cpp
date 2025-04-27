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
    lcd->print(message); // Kein clear() und setCursor(), nur direkte Ausgabe
}

void lcd1602::printTwoLines(String line1, String line2)
{
    lcd->clear();
    lcd->setCursor(0, 0);
    lcd->print(line1.substring(0, 16)); // Auf 16 Zeichen begrenzen
    lcd->setCursor(0, 1);
    lcd->print(line2.substring(0, 16)); // Auf 16 Zeichen begrenzen
}

void lcd1602::setupLCD()
{
    Wire.begin(21, 22); // SDA = Pin 21, SCL = Pin 22 (ESP32 Standard)
    lcd->init();
    lcd->backlight();
    lcd->clear();
    lcd->setCursor(0, 0);
    lcd->print("Ready!");
    delay(1000);
    lcd->clear();
}

void lcd1602::displayDefaultMessage()
{
    lcd->clear();
    lcd->setCursor(0, 0);
    lcd->print("Enter Combination:");
}