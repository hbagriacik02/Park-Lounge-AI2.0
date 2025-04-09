#include "led.h"

LED::LED(int pin) : _pin(pin), _state(false) {}

void LED::begin()
{
    pinMode(_pin, OUTPUT);
    digitalWrite(_pin, LOW);
}

void LED::on()
{
    _state = true;
    digitalWrite(_pin, HIGH);
}

void LED::off()
{
    _state = false;
    digitalWrite(_pin, LOW);
}

void LED::toggle()
{
    _state = !_state;
    digitalWrite(_pin, _state ? HIGH : LOW);
}

bool LED::isOn()
{
    return _state;
}
