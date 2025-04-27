#ifndef LCD1602_H
#define LCD1602_H

#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

class lcd1602
{
private:
    LiquidCrystal_I2C *lcd;

public:
    lcd1602();
    void setupLCD();
    void clear();
    void setCursor(int x, int y);
    void print(String message);
    void printTwoLines(String line1, String line2); // Neue Methode für zweizeilige Ausgabe
    void displayDefaultMessage();
};

#endif