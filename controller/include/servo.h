#ifndef SERVO_H
#define SERVO_H

#include <Arduino.h>
#include <ESP32Servo.h>

class ServoControl
{
public:
    ServoControl(int servoPin);
    void begin();
    void setPosition(int pos);

private:
    int _servoPin;
    Servo _myServo;
    int _pos;
};

#endif