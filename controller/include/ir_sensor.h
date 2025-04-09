#ifndef IR_SENSOR_H
#define IR_SENSOR_H

#include <Arduino.h>

class IRSensor
{
public:
    IRSensor(int pin);       // Konstruktor mit Pin
    void begin();            // Pin als Eingang konfigurieren
    bool isObjectDetected(); // Gibt true zurück, wenn Objekt erkannt

private:
    int _pin;
};

#endif
