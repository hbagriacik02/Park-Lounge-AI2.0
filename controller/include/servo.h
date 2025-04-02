#ifndef SERVO_H
#define SERVO_H

#include <Arduino.h>
#include <ESP32Servo.h>

class ServoControl
{
public:
    ServoControl(int servoPin); // Nur ein Parameter für den Servo-Pin
    void begin();
    void setPosition(int pos); // Methode zum Setzen der Servo-Position

private:
    int _servoPin;
    Servo _myServo;
    int _pos;
};

#endif