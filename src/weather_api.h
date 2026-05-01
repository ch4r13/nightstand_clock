#pragma once
#include <Arduino.h>

#define FORECAST_DAYS 3
struct DayForecast {
    float temp_min, temp_max;
    char  icon[8];
    char  label[4];   // Czech weekday abbrev, e.g. "Po"
};

struct WeatherData {
    char  description[48];
    char  icon[8];
    float temp_current;
    float temp_min;
    float temp_max;
    int   humidity;
    float wind_speed;
    bool  valid;
    uint32_t updated_at;
    DayForecast forecast[FORECAST_DAYS];
    int         forecast_count;
};

bool         weather_fetch(WeatherData &out);
const char*  weather_icon_emoji(const char *icon_code);
uint32_t     weather_bg_color(const char *icon_code);
