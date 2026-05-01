#pragma once
#include <lvgl.h>
#include "alarm_mgr.h"
#include "weather_api.h"
#include "calendar_api.h"

void ui_init(AlarmManager *alarm, WeatherData *weather);
void ui_update_time(uint8_t h, uint8_t m, uint8_t s, uint8_t wday,
                    uint8_t day, uint8_t mon, uint16_t year);
void ui_update_weather(const WeatherData &wd);
void ui_update_calendar(const CalEvent *events, int count);
void ui_show_alarm_ring(AlarmManager *alarm);
void ui_hide_alarm_ring();
void ui_clock_set_mode(bool digital);

#define SCREEN_CLOCK     0
#define SCREEN_WEATHER   1
#define SCREEN_CALENDAR  2
#define SCREEN_SETTINGS  3
void ui_show_screen(int idx);
extern int g_current_screen;
void ui_show_screen_titled(int idx, int direction);
void ui_weather_scroll(int dy);
void ui_settings_scroll(int dy);
void ui_settings_set_ip(const char *ip);
