#include "servo.h"

ServoControl::ServoControl(int servoPin)
    : _servoPin(servoPin), _pos(0) {}

void ServoControl::begin()
{
    _myServo.attach(_servoPin); // Servo wird an den angegebenen Pin angeschlossen
    _myServo.write(_pos);       // Startet mit Position 0° (verriegelt)
}

void ServoControl::setPosition(int pos)
{
    _pos = pos;
    _myServo.write(_pos); // Bewegt den Servo auf die angegebene Position
}