#include "servo.h"

ServoControl::ServoControl(int servoPin)
    : _servoPin(servoPin), _pos(0) {}

void ServoControl::begin()
{
    _myServo.attach(_servoPin);
    _myServo.write(_pos);
}

void ServoControl::setPosition(int pos)
{
    _pos = pos;
    _myServo.write(_pos);
}