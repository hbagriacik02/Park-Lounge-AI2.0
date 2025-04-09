#ifndef LED_H
#define LED_H

#include <Arduino.h>

class LED
{
public:
    LED(int pin);
    void begin();
    void on();
    void off();
    void toggle();
    bool isOn();

private:
    int _pin;
    bool _state;
};

#endif
