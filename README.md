# Nightstand Alarm Clock
**ESP32-C3-MINI-1U · GC9A01 240x240 round IPS · CST816S touch · Piezo buzzer**

Budik pro nocni stolek s fancy UI, WiFi, NTP a predpovedi pocasi (OpenWeather API).

## Nastaveni pred flashovanim

Otevri `src/config.h` a vypln:

```c
#define WIFI_SSID      "nazev_site"
#define WIFI_PASSWORD  "heslo_site"
#define OW_API_KEY     "klic_z_openweathermap.org"
#define OW_CITY        "Prague"
#define OW_COUNTRY     "CZ"
#define NTP_TZ         "CET-1CEST,M3.5.0,M10.5.0/3"
```

## Kompilace

```bash
pio run --target upload
pio device monitor
```
