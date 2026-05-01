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
    "&cnt=24";

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
            struct tm lt_tm;
            localtime_r(&now_ts, &lt_tm);
            int today_yday = lt_tm.tm_yday;

            static const char *wday_cs[] = {"Ne","Po","Ut","St","Ct","Pa","So"};
            struct { float tmin, tmax; char icon[8]; int wday; bool set; }
                agg[FORECAST_DAYS] = {};
            for (int i = 0; i < FORECAST_DAYS; i++) { agg[i].tmin = 999; agg[i].tmax = -999; }

            for (JsonObject item : doc["list"].as<JsonArray>()) {
                time_t ts = item["dt"] | (time_t)0;
                struct tm ft_tm;
                localtime_r(&ts, &ft_tm);
                int diff = ft_tm.tm_yday - today_yday;
                if (diff <= 0) diff += 365;
                if (diff < 1 || diff > FORECAST_DAYS) continue;
                int idx = diff - 1;
                float t = item["main"]["temp"] | 0.0f;
                if (t < agg[idx].tmin) agg[idx].tmin = t;
                if (t > agg[idx].tmax) agg[idx].tmax = t;
                if (!agg[idx].set) {
                    const char *ic = item["weather"][0]["icon"] | "01d";
                    strlcpy(agg[idx].icon, ic, 8);
                    agg[idx].wday = ft_tm.tm_wday;
                    agg[idx].set  = true;
                }
            }

            out.forecast_count = 0;
            for (int i = 0; i < FORECAST_DAYS; i++) {
                if (!agg[i].set) break;
                out.forecast[i].temp_min = agg[i].tmin;
                out.forecast[i].temp_max = agg[i].tmax;
                strlcpy(out.forecast[i].icon,  agg[i].icon, 8);
                strlcpy(out.forecast[i].label, wday_cs[agg[i].wday % 7], 4);
                out.forecast_count++;
            }
            // backward compat: use tomorrow data for temp_min/max
            if (out.forecast_count > 0) {
                out.temp_min = out.forecast[0].temp_min;
                out.temp_max = out.forecast[0].temp_max;
            }
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
