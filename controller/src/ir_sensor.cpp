#include "ir_sensor.h"

IRSensor::IRSensor(int pin)
{
    _pin = pin;
}

void IRSensor::begin()
{
    pinMode(_pin, INPUT);
}

bool IRSensor::isObjectDetected()
{
    return digitalRead(_pin) == LOW; // LOW = Objekt erkannt
}