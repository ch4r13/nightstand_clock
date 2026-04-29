#include "weather_api.h"
#include "config.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>

static const char OW_CURRENT_URL[] =
    "http://api.openweathermap.org/data/2.5/weather"
    "?q=" OW_CITY "," OW_COUNTRY
    "&appid=" OW_API_KEY
    "&units=" OW_UNITS
    "&lang=" OW_LANG;

static const char OW_FORECAST_URL[] =
    "http://api.openweathermap.org/data/2.5/forecast"
    "?q=" OW_CITY "," OW_COUNTRY
    "&appid=" OW_API_KEY
    "&units=" OW_UNITS
    "&lang=" OW_LANG
    "&cnt=16";

static String http_get(const char *url) {
    HTTPClient http;
    http.begin(url);
    http.setTimeout(8000);
    int code = http.GET();
    String body;
    if (code == 200) body = http.getString();
    http.end();
    return body;
}

bool weather_fetch(WeatherData &out) {
    out.valid = false;

    String cur = http_get(OW_CURRENT_URL);
    if (cur.isEmpty()) return false;

    {
        JsonDocument doc;
        if (deserializeJson(doc, cur) != DeserializationError::Ok) return false;
        out.temp_current = doc["main"]["temp"] | 0.0f;
        out.humidity     = doc["main"]["humidity"] | 0;
        out.wind_speed   = doc["wind"]["speed"] | 0.0f;
        const char *desc = doc["weather"][0]["description"] | "";
        const char *icon = doc["weather"][0]["icon"] | "01d";
        strlcpy(out.description, desc, sizeof(out.description));
        if (out.description[0] >= 'a' && out.description[0] <= 'z')
            out.description[0] -= 32;
        strlcpy(out.icon, icon, sizeof(out.icon));
    }

    String fc = http_get(OW_FORECAST_URL);
    if (!fc.isEmpty()) {
        JsonDocument doc;
        if (deserializeJson(doc, fc) == DeserializationError::Ok) {
            time_t now_ts; time(&now_ts);
            struct tm *lt = localtime(&now_ts);
            int today_day = lt->tm_yday;
            float tmin = 999, tmax = -999;
            bool found = false;
            for (JsonObject item : doc["list"].as<JsonArray>()) {
                time_t ts = item["dt"] | (time_t)0;
                struct tm *ft = localtime(&ts);
                if (ft->tm_yday == today_day + 1) {
                    float t = item["main"]["temp"] | 0.0f;
                    tmin = min(tmin, t); tmax = max(tmax, t);
                    found = true;
                }
            }
            out.temp_min = found ? tmin : out.temp_current - 3.0f;
            out.temp_max = found ? tmax : out.temp_current + 3.0f;
        }
    }

    out.valid      = true;
    out.updated_at = millis();
    return true;
}

const char* weather_icon_emoji(const char *icon_code) {
    if (!icon_code || icon_code[0] == '\0') return "?";
    switch (icon_code[0]) {
        case '0':
            switch (icon_code[1]) {
                case '1': return (icon_code[2]=='d') ? "SUN" : "MOON";
                case '2': return "PCLD";
                case '3': return "CLD";
                case '4': return "OCLD";
                case '9': return "SHWR";
                default:  return "CLD";
            }
        case '1':
            switch (icon_code[1]) {
                case '0': return "RAIN";
                case '1': return "STRM";
                case '3': return "SNOW";
                default:  return "CLD";
            }
        case '5': return "MIST";
        default:  return "?";
    }
}

uint32_t weather_bg_color(const char *icon_code) {
    if (!icon_code || icon_code[0] == '\0') return 0x0D1B2A;
    switch (icon_code[0]) {
        case '0':
            if (icon_code[1] == '1')
                return (icon_code[2] == 'd') ? 0x1A6BAA : 0x0D1B3A;
            return 0x2A3F5F;
        case '1':
            if (icon_code[1] == '1') return 0x2A1A1A;
            if (icon_code[1] == '3') return 0x1A2A3A;
            return 0x1A2A3A;
        default: return 0x0D1B2A;
    }
}
