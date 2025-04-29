#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "wifi_config.h"
#include "servo.h"
#include "lcd1602.h"
#include "ir_sensor.h"
#include "button.h"
#include "led.h"

// Verwendete Pins
// Servo: Pin 13
// LCD: SDA = Pin 21, SCL = Pin 22
// IR-Sensor: Pin 27
// Rote LED: Pin 14
// Grüne LED: Pin 25
// Button: Pin 26

// Wi-Fi und MQTT Zugangsdaten
#ifndef MQTT_BROKER_IP
#error "MQTT_BROKER_IP must be defined in platformio.ini"
#endif
#ifndef MQTT_BROKER_USER
#error "MQTT_BROKER_USER must be defined in platformio.ini"
#endif
#ifndef MQTT_BROKER_PASSWORD
#error "MQTT_BROKER_PASSWORD must be defined in platformio.ini"
#endif
const char *mqtt_server = MQTT_BROKER_IP;
const int mqtt_port = 1883;
const char *mqtt_user = MQTT_BROKER_USER;
const char *mqtt_password = MQTT_BROKER_PASSWORD;

// MQTT Themen
const char *MQTT_TOPIC_TRIGGER = "parkhaus/camera/trigger";
const char *MQTT_TOPIC_RESPONSE = "parkhaus/camera/response";
const char *MQTT_TOPIC_STATUS = "parkhaus/status";
const char *MQTT_TOPIC_SETUP = "parkhaus/setup";

// MQTT Client
WiFiClient espClient;
PubSubClient client(espClient);

// Zustände
typedef enum
{
    STATE_IDLE,
    STATE_SCANNING,
    STATE_APPROVED,
    STATE_DENIED,
    STATE_ERROR,
    STATE_MANUAL_OPEN,
    STATE_CLOSING,
    STATE_FULL
} ParkhausState;

// Übergänge
typedef enum
{
    NO_EVENT,
    CAR_DETECTED,
    SCAN_SUCCESS,
    SCAN_FAILED,
    PLATE_APPROVED,
    PLATE_DENIED,
    BUTTON_PRESSED,
    TIMEOUT_EXPIRED,
    MQTT_TIMEOUT
} Transition;

// Funktionsdeklaration für stateMachine
void stateMachine(Transition &event);

// Hardware-Objekte
lcd1602 lcd;
ServoControl servo(13);
IRSensor irSensor(27);
LED redLED(14);
LED greenLED(25);
Button button(26);

// Zustandsmaschine
ParkhausState currentState = STATE_IDLE;
ParkhausState lastState = STATE_IDLE;
bool stateChanged = false;

// Zeitsteuerung
unsigned long stateEntryTime = 0;
const unsigned long WAIT_TIME = 5000;
const unsigned long SCAN_TIMEOUT = 5000;
const unsigned long MQTT_TOTAL_TIMEOUT = 15000;

// Parkplatz-Zähler
const int MAX_PARKING_SPACES = 5;
int usedParkingSpaces = 0;

// Scanning-Versuche
int scanningAttempts = 0;
String lastPlate = "";

// Flag, um IR-Sensor-Erkennung zu steuern
bool irDetectionEnabled = true;

// Zeitstempel für den letzten Reconnect-Versuch
unsigned long lastWiFiReconnectAttempt = 0;
const unsigned long WIFI_RECONNECT_INTERVAL = 5000; // 5 Sekunden zwischen Reconnect-Versuchen

// Funktion zur Anzeige der Uhrzeit mit NTP über ESP32
void updateTimeDisplay()
{
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo))
    {
        Serial.println("Fehler: Zeit konnte nicht abgerufen werden");
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("No Time");
        return;
    }
    char timeStr[9];
    strftime(timeStr, sizeof(timeStr), "%H:%M:%S", &timeinfo);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(timeStr);
    Serial.println("LCD: Time updated");
}

// Funktion zur Aktualisierung der LEDs basierend auf freien Parkplätzen
void updateLEDs()
{
    Serial.println("DEBUG: Inside updateLEDs");
    if (usedParkingSpaces < MAX_PARKING_SPACES)
    {
        greenLED.on();
        redLED.off();
        Serial.println("LEDs: Green ON, Red OFF");
    }
    else
    {
        greenLED.off();
        redLED.on();
        Serial.println("LEDs: Green OFF, Red ON");
    }
    Serial.println("DEBUG: Exiting updateLEDs");
}

// MQTT Nachricht senden
void sendMQTTMessage(const char *status, int occupiedSpaces, const char *topic, bool includePlate = false, const char *plate = nullptr, bool approved = false)
{
    Serial.println("DEBUG: Inside sendMQTTMessage");
    StaticJsonDocument<200> doc;

    // Für Nachrichten mit "status" (z. B. "full", "failed", "test_setup")
    if (status != nullptr)
    {
        doc["status"] = status;
        if (strcmp(status, "space_freed") == 0)
        {
            doc["remaining_spaces"] = occupiedSpaces;
        }
        else
        {
            doc["blocked_space"] = occupiedSpaces;
        }
    }
    // Für Rückmeldungen nach KI-Antwort (PLATE_APPROVED, PLATE_DENIED)
    else
    {
        doc["free_space"] = MAX_PARKING_SPACES - occupiedSpaces;
        doc["blocked_space"] = occupiedSpaces;
    }

    char jsonBuffer[256];
    serializeJson(doc, jsonBuffer);
    client.publish(topic, jsonBuffer);
    Serial.print("MQTT Nachricht gesendet an Topic ");
    Serial.print(topic);
    Serial.print(": ");
    Serial.println(jsonBuffer);
    Serial.println("DEBUG: Exiting sendMQTTMessage");
}

// MQTT Callback-Funktion
void callback(char *topic, byte *payload, unsigned int length)
{
    Serial.print("MQTT Raw Payload: ");
    for (unsigned int i = 0; i < length; i++)
    {
        Serial.print((char)payload[i]);
    }
    Serial.println();

    String message;
    for (unsigned int i = 0; i < length; i++)
    {
        message += (char)payload[i];
    }
    Serial.print("MQTT Nachricht empfangen: ");
    Serial.println(message);

    if (strcmp(topic, MQTT_TOPIC_RESPONSE) == 0)
    {
        StaticJsonDocument<200> doc;
        DeserializationError error = deserializeJson(doc, message);
        if (error)
        {
            Serial.print("JSON Parsing Fehler: ");
            Serial.println(error.c_str());
            return;
        }

        const char *status = doc["status"];
        if (status == nullptr)
        {
            Serial.println("Fehler: 'status' fehlt in der MQTT-Nachricht");
            return;
        }

        if (strcmp(status, "success") == 0)
        {
            lastPlate = doc["plate"].as<String>();
            if (lastPlate.isEmpty())
            {
                Serial.println("Fehler: 'plate' fehlt oder ist leer");
                return;
            }
            bool approved = doc["approved"];
            if (approved)
            {
                Transition event = PLATE_APPROVED;
                stateMachine(event);
            }
            else
            {
                Transition event = PLATE_DENIED;
                stateMachine(event);
            }
        }
        else if (strcmp(status, "failed") == 0)
        {
            Transition event = SCAN_FAILED;
            stateMachine(event);
        }
    }
}

// MQTT Verbindung herstellen
void reconnect()
{
    while (!client.connected())
    {
        // Stelle sicher, dass WiFi verbunden ist, bevor MQTT versucht wird
        if (WiFi.status() != WL_CONNECTED)
        {
            Serial.println("Kann MQTT nicht verbinden: WiFi nicht verbunden!");
            return;
        }

        Serial.print("Verbinde mit MQTT Broker...");
        String clientId = "ESP32Client-";
        clientId += String(random(0xffff), HEX);
        if (client.connect(clientId.c_str(), mqtt_user, mqtt_password))
        {
            Serial.println("verbunden");
            client.subscribe(MQTT_TOPIC_RESPONSE);
        }
        else
        {
            Serial.print("fehlgeschlagen, rc=");
            Serial.print(client.state());
            Serial.println(" Versuche in 5 Sekunden erneut");
            delay(5000);
        }
    }
}

// Zustandsmaschine
void stateMachine(Transition &event)
{
    static ParkhausState lastPrintedState = STATE_IDLE;
    ParkhausState newState = currentState;

    Serial.println("DEBUG: Entering stateMachine");

    Serial.print("DEBUG: Before state change - currentState = ");
    Serial.print(currentState);
    Serial.print(", lastState = ");
    Serial.println(lastState);

    if (currentState == STATE_IDLE)
    {
        Serial.println("DEBUG: Inside STATE_IDLE");
        irDetectionEnabled = true;
        updateTimeDisplay();
        if (event == BUTTON_PRESSED)
        {
            Serial.println("Transition: BUTTON_PRESSED -> STATE_MANUAL_OPEN");
            newState = STATE_MANUAL_OPEN;
            Serial.println("DEBUG: Executing STATE_IDLE to STATE_MANUAL_OPEN actions");
            updateLEDs();
            event = NO_EVENT;
        }
        else if (event == CAR_DETECTED)
        {
            Serial.println("Transition: CAR_DETECTED -> STATE_SCANNING");
            newState = STATE_SCANNING;
            Serial.println("DEBUG: Executing STATE_IDLE to STATE_SCANNING actions");
            event = NO_EVENT;
        }
    }
    else if (currentState == STATE_SCANNING)
    {
        Serial.println("DEBUG: Inside STATE_SCANNING");
        if (stateChanged)
        {
            if (usedParkingSpaces >= MAX_PARKING_SPACES)
            {
                Serial.println("DEBUG: Before lcd.print in STATE_SCANNING (Parkhaus voll)");
                lcd.clear();
                lcd.setCursor(0, 0);
                lcd.print("Parkhaus voll");
                Serial.println("LCD: Parkhaus voll");
                sendMQTTMessage("full", usedParkingSpaces, MQTT_TOPIC_STATUS);
                scanningAttempts = 0;
                newState = STATE_FULL;
                Serial.println("Transition: Parkhaus voll -> STATE_FULL");
            }
            else
            {
                scanningAttempts++;
                Serial.println("DEBUG: Before lcd.print in STATE_SCANNING (Scanning...)");
                String line1 = "Scanning...    ";
                String line2 = String(scanningAttempts) + " mal          ";
                lcd.printTwoLines(line1, line2);
                Serial.println("LCD: Scanning... | " + String(scanningAttempts) + " mal");

                StaticJsonDocument<200> doc;
                doc["command"] = "scanme";
                doc["free_space"] = MAX_PARKING_SPACES - usedParkingSpaces;
                doc["blocked_space"] = usedParkingSpaces;
                char jsonBuffer[256];
                serializeJson(doc, jsonBuffer);
                client.publish(MQTT_TOPIC_TRIGGER, jsonBuffer);
                Serial.println("Sent scan command to camera");
            }
        }

        if (usedParkingSpaces >= MAX_PARKING_SPACES)
        {
            // Bereits oben behandelt
        }
        else
        {
            if (millis() - stateEntryTime >= MQTT_TOTAL_TIMEOUT)
            {
                Serial.println("MQTT Timeout abgelaufen");
                event = MQTT_TIMEOUT;
            }

            if (scanningAttempts == 3 && millis() - stateEntryTime >= SCAN_TIMEOUT * 3)
            {
                Serial.println("Dritter Scan-Versuch fehlgeschlagen (Timeout)");
                sendMQTTMessage("failed", usedParkingSpaces, MQTT_TOPIC_STATUS);
                newState = STATE_ERROR;
                Serial.println("DEBUG: Executing STATE_SCANNING to STATE_ERROR actions");
                updateLEDs();
                Serial.println("DEBUG: Before lcd.print in STATE_ERROR");
                String line1 = "Kein Kennzeichen";
                String line2 = "erkannt        ";
                lcd.printTwoLines(line1, line2);
                Serial.println("LCD: Kein Kennzeichen erkannt");
            }
            else if (millis() - stateEntryTime >= SCAN_TIMEOUT * scanningAttempts && scanningAttempts < 3)
            {
                sendMQTTMessage("failed", usedParkingSpaces, MQTT_TOPIC_STATUS);
                scanningAttempts++;
                Serial.println("DEBUG: Before lcd.print in STATE_SCANNING (retry)");
                String line1 = "Scanning...    ";
                String line2 = String(scanningAttempts) + " mal          ";
                lcd.printTwoLines(line1, line2);
                Serial.println("LCD: Scanning... | " + String(scanningAttempts) + " mal");

                StaticJsonDocument<200> doc;
                doc["command"] = "scanme";
                doc["free_space"] = MAX_PARKING_SPACES - usedParkingSpaces;
                doc["blocked_space"] = usedParkingSpaces;
                char jsonBuffer[256];
                serializeJson(doc, jsonBuffer);
                client.publish(MQTT_TOPIC_TRIGGER, jsonBuffer);
                Serial.println("Sent scan command to camera (retry)");
            }
        }

        if (event == SCAN_FAILED && scanningAttempts >= 3)
        {
            sendMQTTMessage("failed", usedParkingSpaces, MQTT_TOPIC_STATUS);
            newState = STATE_ERROR;
            Serial.println("DEBUG: Executing STATE_SCANNING to STATE_ERROR actions");
            updateLEDs();
            Serial.println("DEBUG: Before lcd.print in STATE_ERROR");
            String line1 = "Kein Kennzeichen";
            String line2 = "erkannt        ";
            lcd.printTwoLines(line1, line2);
            Serial.println("LCD: Kein Kennzeichen erkannt");
            event = NO_EVENT;
        }
        else if (event == PLATE_APPROVED)
        {
            newState = STATE_APPROVED;
            Serial.println("DEBUG: Executing STATE_SCANNING to STATE_APPROVED actions");
            updateLEDs();
            usedParkingSpaces++;
            sendMQTTMessage(nullptr, usedParkingSpaces, MQTT_TOPIC_STATUS);
            Serial.println("DEBUG: Before lcd.print in STATE_APPROVED");
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print(lastPlate.c_str());
            Serial.println("LCD: " + lastPlate);
            event = NO_EVENT;
        }
        else if (event == PLATE_DENIED)
        {
            newState = STATE_DENIED;
            Serial.println("DEBUG: Executing STATE_SCANNING to STATE_DENIED actions");
            updateLEDs();
            sendMQTTMessage(nullptr, usedParkingSpaces, MQTT_TOPIC_STATUS);
            Serial.println("DEBUG: Before lcd.print in STATE_DENIED");
            String line1 = "Einfahrt nicht ";
            String line2 = "gestattet     ";
            lcd.printTwoLines(line1, line2);
            Serial.println("LCD: Einfahrt nicht gestattet");
            event = NO_EVENT;
        }
        else if (event == MQTT_TIMEOUT)
        {
            newState = STATE_ERROR;
            Serial.println("DEBUG: Executing STATE_SCANNING to STATE_ERROR actions (MQTT_TIMEOUT)");
            sendMQTTMessage("failed", usedParkingSpaces, MQTT_TOPIC_STATUS);
            updateLEDs();
            Serial.println("DEBUG: Before lcd.print in STATE_ERROR");
            String line1 = "Kein Kennzeichen";
            String line2 = "erkannt        ";
            lcd.printTwoLines(line1, line2);
            Serial.println("LCD: Kein Kennzeichen erkannt");
            event = NO_EVENT;
        }
    }
    else if (currentState == STATE_APPROVED)
    {
        Serial.println("DEBUG: Inside STATE_APPROVED");
        if (stateChanged)
        {
            Serial.println("DEBUG: Executing STATE_APPROVED actions");
            servo.setPosition(90);
            Serial.println("Servo: Position 90 (STATE_APPROVED)");
            String line1 = "Willkommen     ";
            String line2 = lastPlate + "          ";
            lcd.printTwoLines(line1, line2);
            Serial.println("LCD: Willkommen | " + lastPlate);
        }
        if (millis() - stateEntryTime >= WAIT_TIME)
        {
            Serial.println("Timeout: Transition to STATE_CLOSING");
            newState = STATE_CLOSING;
            Serial.println("DEBUG: Executing STATE_APPROVED to STATE_CLOSING actions");
            updateLEDs();
            servo.setPosition(0);
            Serial.println("Servo: Position 0 (STATE_CLOSING)");
        }
    }
    else if (currentState == STATE_DENIED)
    {
        Serial.println("DEBUG: Inside STATE_DENIED");
        if (millis() - stateEntryTime >= WAIT_TIME)
        {
            Serial.println("Timeout: Transition to STATE_IDLE");
            newState = STATE_IDLE;
            Serial.println("DEBUG: Executing STATE_DENIED to STATE_IDLE actions");
            updateLEDs();
        }
    }
    else if (currentState == STATE_ERROR)
    {
        Serial.println("DEBUG: Inside STATE_ERROR");
        if (stateChanged)
        {
            Serial.println("DEBUG: Executing STATE_ERROR actions");
            String line1 = "Kein Kennzeichen";
            String line2 = "erkannt        ";
            lcd.printTwoLines(line1, line2);
            Serial.println("LCD: Kein Kennzeichen erkannt");
        }
        if (millis() - stateEntryTime >= WAIT_TIME)
        {
            Serial.println("Timeout: Transition to STATE_IDLE");
            newState = STATE_IDLE;
            Serial.println("DEBUG: Executing STATE_ERROR to STATE_IDLE actions");
            updateLEDs();
        }
    }
    else if (currentState == STATE_MANUAL_OPEN)
    {
        Serial.println("DEBUG: Inside STATE_MANUAL_OPEN");
        if (stateChanged)
        {
            Serial.println("DEBUG: Executing STATE_MANUAL_OPEN actions");
            Serial.print("Free heap: ");
            Serial.println(ESP.getFreeHeap());
            if (usedParkingSpaces > 0)
            {
                Serial.println("DEBUG: Decrementing usedParkingSpaces");
                usedParkingSpaces--;
            }
            Serial.println("DEBUG: Before updateLEDs");
            updateLEDs();
            Serial.println("DEBUG: Before sendMQTTMessage");
            sendMQTTMessage("space_freed", usedParkingSpaces, MQTT_TOPIC_STATUS);
            Serial.println("DEBUG: Before servo.setPosition");
            servo.setPosition(90);
            Serial.println("Servo: Position 90 (STATE_MANUAL_OPEN)");
            Serial.println("DEBUG: Before lcd.print");
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Auf Wiedersehen");
            Serial.println("LCD: Auf Wiedersehen");
        }
        if (millis() - stateEntryTime >= WAIT_TIME)
        {
            Serial.println("Timeout: Transition to STATE_CLOSING");
            newState = STATE_CLOSING;
            Serial.println("DEBUG: Executing STATE_MANUAL_OPEN to STATE_CLOSING actions");
            updateLEDs();
            servo.setPosition(0);
            Serial.println("Servo: Position 0 (STATE_CLOSING)");
        }
    }
    else if (currentState == STATE_CLOSING)
    {
        Serial.println("DEBUG: Inside STATE_CLOSING");
        if (stateChanged)
        {
            Serial.println("DEBUG: Executing STATE_CLOSING actions");
            updateLEDs();
        }
        if (millis() - stateEntryTime >= WAIT_TIME)
        {
            Serial.println("Timeout: Transition to STATE_IDLE");
            newState = STATE_IDLE;
            Serial.println("DEBUG: Executing STATE_CLOSING to STATE_IDLE actions");
            updateLEDs();
        }
    }
    else if (currentState == STATE_FULL)
    {
        Serial.println("DEBUG: Inside STATE_FULL");
        if (millis() - stateEntryTime >= WAIT_TIME)
        {
            Serial.println("Timeout: Transition to STATE_IDLE");
            newState = STATE_IDLE;
            Serial.println("DEBUG: Executing STATE_FULL to STATE_IDLE actions");
            updateLEDs();
        }
    }
    else
    {
        Serial.println("DEBUG: Default case reached");
    }

    if (newState != currentState)
    {
        scanningAttempts = 0;
        stateEntryTime = millis();
        Serial.print("Entering state at time: ");
        Serial.println(stateEntryTime);
        lastState = currentState;
        currentState = newState;
        stateChanged = true;
    }
    else
    {
        stateChanged = false;
    }

    if (currentState != lastPrintedState)
    {
        if (currentState == STATE_IDLE)
            Serial.println("State IDLE");
        else if (currentState == STATE_SCANNING)
            Serial.println("State SCANNING");
        else if (currentState == STATE_APPROVED)
            Serial.println("State APPROVED");
        else if (currentState == STATE_DENIED)
            Serial.println("State DENIED");
        else if (currentState == STATE_ERROR)
            Serial.println("State ERROR");
        else if (currentState == STATE_MANUAL_OPEN)
            Serial.println("State MANUAL_OPEN");
        else if (currentState == STATE_CLOSING)
            Serial.println("State CLOSING");
        else if (currentState == STATE_FULL)
            Serial.println("State FULL");
        lastPrintedState = currentState;
    }

    Serial.println("DEBUG: Exiting stateMachine");
}

void setup()
{
    Serial.begin(115200);
    Serial.println("Setup beginnt...");

    // Debugging: Werte von WIFI_SSID und WIFI_PASSWORD ausgeben
    Serial.print("WIFI_SSID: '");
    Serial.print(WIFI_SSID);
    Serial.println("'");
    Serial.print("WIFI_PASSWORD: '");
    Serial.print(WIFI_PASSWORD);
    Serial.println("'");

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi verbunden");
    Serial.println(WiFi.localIP());

    configTime(7200, 0, "pool.ntp.org"); // 7200 Sekunden = 2 Stunden Offset für Sommerzeit
    Serial.println("Warte auf NTP-Synchronisation...");
    delay(5000);

    struct tm timeinfo;
    if (!getLocalTime(&timeinfo))
    {
        Serial.println("Fehler: NTP-Synchronisation fehlgeschlagen");
    }
    else
    {
        Serial.println("NTP-Synchronisation erfolgreich");
    }

    client.setServer(mqtt_server, mqtt_port);
    client.setCallback(callback);
    reconnect();
    Serial.println("DEBUG: Testing MQTT in setup");
    sendMQTTMessage("test_setup", 0, MQTT_TOPIC_SETUP);
    delay(1000);

    lcd.setupLCD();
    Serial.println("LCD initialisiert");
    Serial.println("DEBUG: Testing LCD in setup");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Test in Setup");
    Serial.println("LCD: Test in Setup");

    servo.begin();
    Serial.println("Servo initialisiert");
    Serial.println("DEBUG: Testing Servo in setup");
    servo.setPosition(90);
    Serial.println("Servo: Position 90 in setup");
    delay(1000);
    servo.setPosition(0);
    Serial.println("Servo: Position 0 in setup");

    irSensor.begin();
    Serial.println("IR-Sensor initialisiert");
    redLED.begin();
    greenLED.begin();
    button.begin();
    Serial.println("Setup abgeschlossen");

    usedParkingSpaces = 0;
    updateLEDs();
}

void loop()
{
    // Prüfen, ob WiFi verbunden ist, und versuchen, wieder zu verbinden
    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("WiFi nicht verbunden! Versuche Reconnect...");
        // Versuche nur alle WIFI_RECONNECT_INTERVAL Millisekunden einen Reconnect
        if (millis() - lastWiFiReconnectAttempt >= WIFI_RECONNECT_INTERVAL)
        {
            lastWiFiReconnectAttempt = millis();
            WiFi.reconnect();
            Serial.println("WiFi Reconnect ausgeführt");
        }
    }
    else
    {
        // WiFi ist verbunden, prüfe MQTT-Verbindung
        if (!client.connected())
        {
            reconnect();
        }
        client.loop();
    }

    Transition event = NO_EVENT;

    if (button.isPressed())
    {
        Serial.println("Button pressed!");
        event = BUTTON_PRESSED;
    }

    if (irDetectionEnabled && irSensor.isObjectDetected())
    {
        Serial.println("IR Sensor: Object detected!");
        event = CAR_DETECTED;
        irDetectionEnabled = false;
    }

    stateMachine(event);

    delay(100);
}