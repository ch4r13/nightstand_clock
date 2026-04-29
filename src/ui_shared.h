#pragma once
#include <lvgl.h>
#include "alarm_mgr.h"
#include "weather_api.h"

// Colour palette
#define C_BG        lv_color_hex(0x0D1B2A)
#define C_RING      lv_color_hex(0x1E3A5F)
#define C_TICK_MAJ  lv_color_hex(0xF0A500)
#define C_TICK_MIN  lv_color_hex(0x405070)
#define C_HAND_H    lv_color_hex(0xFFFFFF)
#define C_HAND_M    lv_color_hex(0xDDDDDD)
#define C_HAND_S    lv_color_hex(0xFF3B30)
#define C_CENTER    lv_color_hex(0xF0A500)
#define C_DATE      lv_color_hex(0x8AB4D8)
#define C_ACCENT    lv_color_hex(0xF0A500)
#define C_WHITE     lv_color_hex(0xFFFFFF)
#define C_DIM       lv_color_hex(0x607080)
#define C_ALARM_BG  lv_color_hex(0xFF3B30)

// Shared globals - defined in ui_screens.cpp
extern AlarmManager *g_alarm;
extern WeatherData  *g_weather;

extern lv_obj_t *scr_clock;
extern lv_obj_t *scr_weather;
extern lv_obj_t *scr_settings;
extern lv_obj_t *scr_ring;

// Clock widgets
extern lv_obj_t *lbl_time;
extern lv_obj_t *lbl_date;
extern lv_obj_t *lbl_alarm_icon;
extern lv_obj_t *clock_face;
extern uint8_t   c_h, c_m, c_s;

// Weather widgets
extern lv_obj_t *lbl_wicon;
extern lv_obj_t *lbl_wtemp;
extern lv_obj_t *lbl_wdesc;
extern lv_obj_t *lbl_wtomorrow;
extern lv_obj_t *lbl_whumidity;

// Settings widgets
extern lv_obj_t *roller_h;
extern lv_obj_t *roller_m;
extern lv_obj_t *sw_alarm;

// Internal builders
void build_clock_screen();
void build_weather_screen();
void build_settings_screen();
void add_nav_bar(lv_obj_t *scr, int active_idx);
