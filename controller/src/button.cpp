#include "button.h"

Button::Button(int pin) : _pin(pin) {}

void Button::begin()
{
    pinMode(_pin, INPUT_PULLUP); // Internen Pull-Up aktivieren
}

bool Button::isPressed()
{
    return digitalRead(_pin) == LOW; // LOW = gedrückt (gegen GND)
}
