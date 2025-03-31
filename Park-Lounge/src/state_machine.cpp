#include "servo.h"
#include "state_machine.h"
#include <Arduino.h>
#include "lcd1602.h"

ParkhausState currentState = STATE_IDLE; // Startzustand

// *** Funktion zum Aktualisieren des Zustands ***
void stateMachine(Transition event)
{
    switch (currentState)
    {
    case STATE_IDLE:

        break;

    case STATE_SCANNING:

        break;

    case STATE_CHECKING_CSV:

        break;

    case STATE_APPROVED:

        break;

    case STATE_DENIED:

        break;

    case STATE_ERROR:

        break;

    case STATE_MANUAL_OPEN:

        break;

    case STATE_CLOSING:

        break;
    }
}

// *** Funktion zum Abrufen des aktuellen Zustands ***
ParkhausState getCurrentState()
{
    return currentState;
}
