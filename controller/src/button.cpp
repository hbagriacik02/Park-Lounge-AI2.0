#include "button.h"

Button::Button(int pin) : _pin(pin), _lastButtonState(LOW), _lastStableState(LOW), _lastDebounceTime(0) {}

void Button::begin()
{
    pinMode(_pin, INPUT_PULLDOWN);
}

bool Button::isPressed()
{
    const unsigned long DEBOUNCE_DELAY = 50; // Debounce-Zeit in Millisekunden
    bool reading = digitalRead(_pin);

    // Prüfe, ob sich der Zustand geändert hat
    if (reading != _lastButtonState)
    {
        _lastDebounceTime = millis();
    }

    // Warte auf das Ende des Prellens
    if ((millis() - _lastDebounceTime) > DEBOUNCE_DELAY)
    {
        // Wenn sich der stabile Zustand geändert hat
        if (reading != _lastStableState)
        {
            _lastStableState = reading;
            if (reading == HIGH) // Button gedrückt
            {
                return true;
            }
        }
    }

    _lastButtonState = reading;
    return _lastStableState == HIGH; // Gibt true zurück, solange der Button gedrückt ist
}