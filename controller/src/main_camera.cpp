#include <WiFi.h>
#include <PubSubClient.h>
#include "esp_camera.h"

// ---- PIN DEFINITIONEN ----
#define IR_SENSOR_PIN 27

// ---- WLAN ZUGANGSDATEN ----
const char *ssid = "DEIN_WLAN_NAME";
const char *password = "DEIN_WLAN_PASSWORT";

// ---- MQTT BROKER ----
const char *mqtt_server = "192.168.1.100"; // IP deines Mosquitto-Brokers
const int mqtt_port = 1883;
const char *mqtt_topic = "parkhaus/erkennung";

WiFiClient espClient;
PubSubClient client(espClient);

// ---- Kamera Setup (AI-THINKER) ----
void initCamera()
{
    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = 5;
    config.pin_d1 = 18;
    config.pin_d2 = 19;
    config.pin_d3 = 21;
    config.pin_d4 = 36;
    config.pin_d5 = 39;
    config.pin_d6 = 34;
    config.pin_d7 = 35;
    config.pin_xclk = 0;
    config.pin_pclk = 22;
    config.pin_vsync = 25;
    config.pin_href = 23;
    config.pin_sscb_sda = 26;
    config.pin_sscb_scl = 27;
    config.pin_pwdn = 32;
    config.pin_reset = -1;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_JPEG;

    if (psramFound())
    {
        config.frame_size = FRAMESIZE_QVGA;
        config.jpeg_quality = 10;
        config.fb_count = 2;
    }
    else
    {
        config.frame_size = FRAMESIZE_QQVGA;
        config.jpeg_quality = 12;
        config.fb_count = 1;
    }

    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK)
    {
        Serial.printf("Kamera konnte nicht initialisiert werden (Fehler 0x%x)\n", err);
        return;
    }
    Serial.println("Kamera bereit.");
}

// ---- WLAN VERBINDUNG ----
void setupWiFi()
{
    delay(100);
    WiFi.begin(ssid, password);
    Serial.print("WLAN wird verbunden ...");
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nVerbunden mit: " + WiFi.SSID());
    Serial.println("IP-Adresse: " + WiFi.localIP().toString());
}

// ---- MQTT VERBINDUNG ----
void reconnectMQTT()
{
    while (!client.connected())
    {
        Serial.print("Verbinde mit MQTT Broker...");
        if (client.connect("ESP32CAMClient"))
        {
            Serial.println(" verbunden.");
        }
        else
        {
            Serial.print(" fehlgeschlagen, rc=");
            Serial.print(client.state());
            Serial.println(" versuche erneut in 5 Sekunden.");
            delay(5000);
        }
    }
}

// ---- SETUP ----
void setup()
{
    Serial.begin(115200);
    pinMode(IR_SENSOR_PIN, INPUT);

    setupWiFi();
    client.setServer(mqtt_server, mqtt_port);
    initCamera();
}

// ---- LOOP ----
void loop()
{
    if (!client.connected())
    {
        reconnectMQTT();
    }
    client.loop();

    // Wenn Auto erkannt (IR aktiv LOW)
    if (digitalRead(IR_SENSOR_PIN) == LOW)
    {
        Serial.println("Auto erkannt - Bildaufnahme!");
        // Hier könnte man ein Bild aufnehmen und später senden
        String message = "Auto erkannt um: " + String(millis()) + " ms";
        client.publish(mqtt_topic, message.c_str());
        delay(2000); // Anti-Bounce Delay
    }

    delay(100);
}
