#include <Arduino.h>
#include "servo.h"
#include "lcd1602.h"
#include "ir_sensor.h"
#include "button.h"
#include "led.h"

// Verwendete Pins
//  Servo: Pin 13
//  LCD: SDA = Pin 21, SCL = Pin 22
//  IR-Sensor: Pin 27
//  Rote LED: Pin 14
//  Grüne LED: Pin 25
//  Button: Pin 26

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

// Hardware-Objekte
lcd1602 lcd;            // Das Display-Objekt
ServoControl servo(13); // Servo an Pin 13
IRSensor irSensor(27);  // IR-Sensor an Pin 27
LED redLED(14);         // Rote LED an Pin 14
LED greenLED(25);       // Grüne LED an Pin 25
Button button(26);      // Button an Pin 26

// Zustandsmaschine
ParkhausState currentState = STATE_IDLE; // Startzustand
unsigned long scanningStartTime = 0;
const unsigned long SCAN_TIMEOUT = 3000; // 3 Sekunden Timeout für "Plate not scanned"
const unsigned long WAIT_TIME = 5000;    // 5 Sekunden Wartezeit, bevor Schranke schließt

// Funktion zum Aktualisieren des Zustands
void stateMachine(Transition event)
{
    switch (currentState)
    {
    case STATE_IDLE:
        lcd.print("Bereit");
        redLED.on();
        greenLED.off();
        if (event == BUTTON_PRESSED)
        {
            currentState = STATE_MANUAL_OPEN;
        }
        else if (event == CAR_DETECTED)
        {
            // Vorübergehend direkt zu APPROVED, bis KI implementiert ist
            // Später: currentState = STATE_SCANNING;
            currentState = STATE_APPROVED;
        }
        break;

    case STATE_SCANNING:
        lcd.print("Scanne...");
        scanningStartTime = millis(); // Timer starten
        // Hier würde die KI-Kennzeichenerkennung stattfinden
        // Für jetzt: Direkt zu PLATE_SCANNED simulieren
        if (event == PLATE_SCANNED)
        {
            currentState = STATE_CHECKING_CSV;
        }
        else if (millis() - scanningStartTime > SCAN_TIMEOUT)
        {
            currentState = STATE_ERROR;
        }
        break;

    case STATE_CHECKING_CSV:
        lcd.print("Pruefe...");
        // Hier würde die CSV-Datei geprüft werden
        // Für jetzt: Annahme, dass das Kennzeichen gültig ist
        if (event == PLATE_VALID)
        {
            currentState = STATE_APPROVED;
        }
        else if (event == PLATE_INVALID)
        {
            currentState = STATE_DENIED;
        }
        break;

    case STATE_APPROVED:
        lcd.print("Zufahrt frei");
        servo.setPosition(90); // Schranke öffnen
        greenLED.on();
        redLED.off();
        currentState = STATE_CLOSING;
        break;

    case STATE_DENIED:
        lcd.print("Zufahrt verweigert");
        redLED.on();
        greenLED.off();
        // Hier würde in die Blacklist geschrieben werden
        delay(2000); // 2 Sekunden anzeigen
        currentState = STATE_IDLE;
        break;

    case STATE_ERROR:
        lcd.print("Fehler");
        redLED.on();
        greenLED.off();
        delay(2000); // 2 Sekunden anzeigen
        currentState = STATE_IDLE;
        break;

    case STATE_MANUAL_OPEN:
        lcd.print("Manuell geoeffnet");
        servo.setPosition(90); // Schranke öffnen
        greenLED.on();
        redLED.off();
        currentState = STATE_CLOSING;
        break;

    case STATE_CLOSING:
        lcd.print("Schranke schliesst");
        delay(WAIT_TIME);     // Warten, bis das Auto durchgefahren ist (5 Sekunden)
        servo.setPosition(0); // Schranke schließen
        greenLED.off();
        redLED.on();
        currentState = STATE_IDLE;
        break;
    }
}

void setup()
{
    Serial.begin(115200); // Startet serielle Kommunikation für Debugging
    Serial.println("Setup beginnt...");
    lcd.setupLCD(); // Initialisiert das LCD
    Serial.println("LCD initialisiert");
    servo.begin(); // Initialisiert den Servo
    Serial.println("Servo initialisiert");
    irSensor.begin(); // Initialisiert den IR-Sensor
    Serial.println("IR-Sensor initialisiert");
    redLED.begin();
    greenLED.begin();
    button.begin();
    Serial.println("Setup abgeschlossen");
}

void loop()
{
    // Prüfe Ereignisse
    if (button.isPressed())
    {
        stateMachine(BUTTON_PRESSED);
    }
    else if (irSensor.isObjectDetected())
    {
        stateMachine(CAR_DETECTED);
    }
}