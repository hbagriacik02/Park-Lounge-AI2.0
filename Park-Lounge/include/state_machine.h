#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include <Arduino.h>

// Zustände
typedef enum
{
    STATE_IDLE,         // Wartet auf Eingabe (Kennzeichen oder Button)
    STATE_SCANNING,     // Kamera scannt das Kennzeichen
    STATE_CHECKING_CSV, // Kennzeichen wird mit CSV (Whitelist) abgeglichen
    STATE_APPROVED,     // Kennzeichen ist gültig -> Schranke öffnet
    STATE_DENIED,       // Kennzeichen ungültig -> CSV (Blacklist) wird geschrieben
    STATE_ERROR,        // Kennzeichenerkennung nicht möglich
    STATE_MANUAL_OPEN,  // Button gedrückt -> Schranke öffnet
    STATE_CLOSING,      // Schranke schließt
} ParkhausState;

// Übergänge
typedef enum
{
    CAR_DETECTED,      // Kamera erkennt Auto
    PLATE_SCANNED,     // Nummernschild wurde gescannt
    PLATE_NOT_SCANNED, // Nummernschild konnte nicht gescannt werden
    PLATE_VALID,       // Nummernschild ist gültig
    PLATE_INVALID,     // Nummernschild ist ungültig
    BUTTON_PRESSED,    // Manueller Button wurde gedrückt
    WAITING,           // Warten bis Schranke schließt
    DONE               // Übergang zurück zu IDLE
} Transition;

// *** Funktionsprototypen für die Zustandsmaschine ***
void state_machine(Transition event); // Zustandswechsel auslösen
ParkhausState getCurrentState();      // Aktuellen Zustand abrufen

#endif
