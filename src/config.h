#pragma once
#include "serial_compat.h"

// =============================================================
//  NIGHTSTAND ALARM CLOCK - user configuration
//  Edit this file before flashing!
// =============================================================

// ---- WiFi ----
#define WIFI_SSID      "your_wifi_ssid"
#define WIFI_PASSWORD  "your_wifi_password"
#define WIFI_TIMEOUT_MS 15000

// ---- OpenWeather API (https://openweathermap.org/api) ----
// Free plan: "Current Weather" + "5 day / 3 hour Forecast"
#define OW_API_KEY  "your_openweather_api_key"
#define OW_CITY     "Prague"
#define OW_COUNTRY  "CZ"
#define OW_UNITS    "metric"
#define OW_LANG     "cs"
#define OW_UPDATE_INTERVAL_MS  (15UL * 60UL * 1000UL)

// ---- NTP ----
#define NTP_SERVER1   "pool.ntp.org"
#define NTP_SERVER2   "time.nist.gov"
#define NTP_TZ        "CET-1CEST,M3.5.0,M10.5.0/3"
#define NTP_SYNC_INTERVAL_MS  (60UL * 60UL * 1000UL)

// ---- Hardware pins ----
#define PIN_TOUCH_SDA   4
#define PIN_TOUCH_SCL   5
#define PIN_TOUCH_INT   0
#define PIN_TOUCH_RST   1
#define PIN_BUZZER      8
#define PIN_BACKLIGHT   3

// ---- Alarm / buzzer ----
#define BUZZER_LEDC_CHANNEL  0
#define BUZZER_LEDC_RES      10
#define ALARM_FREQ_HZ        1500
#define ALARM_PATTERN_ON_MS  400
#define ALARM_PATTERN_OFF_MS 200
#define ALARM_SNOOZE_MIN     5
#define ALARM_AUTO_STOP_MIN  3

// ---- Display ----
#define SCREEN_WIDTH   240
#define SCREEN_HEIGHT  240
#define LVGL_BUF_LINES 20

// ---- Calendar (iCal / .ics) ----
#define CALENDAR_URL                ""   // paste your .ics URL here
#define CALENDAR_UPDATE_INTERVAL_MS (15UL * 60UL * 1000UL)
