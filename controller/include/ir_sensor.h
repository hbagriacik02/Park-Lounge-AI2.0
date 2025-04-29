#ifndef IR_SENSOR_H
#define IR_SENSOR_H

#include <Arduino.h>

class IRSensor
{
public:
    IRSensor(int pin);
    void begin();
    bool isObjectDetected();

private:
    int _pin;
};

#endif
