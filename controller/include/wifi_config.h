#ifndef WIFI_CONFIG_H
#define WIFI_CONFIG_H

// Wi-Fi Zugangsdaten werden über Build Flags definiert
#ifndef WIFI_SSID
#error "WIFI_SSID must be defined in platformio.ini"
#endif

#ifndef WIFI_PASSWORD
#error "WIFI_PASSWORD must be defined in platformio.ini"
#endif

#endif