#include <Arduino.h>
#include <WiFi.h>
#include <TFT_eSPI.h>
#include <lvgl.h>
#include <time.h>

#include "config.h"
#include "touch_cst816.h"
#include "weather_api.h"
#include "calendar_api.h"
#include "alarm_mgr.h"
#include "ui_screens.h"
#include "ui_shared.h"
#include "runtime_config.h"
#include "web_server.h"

static TFT_eSPI     tft;
static CST816Touch  touch(PIN_TOUCH_SDA, PIN_TOUCH_SCL,
                          PIN_TOUCH_INT, PIN_TOUCH_RST);
static AlarmManager alarmMgr(PIN_BUZZER);
static WeatherData  weather = {};
static RuntimeConfig rt_cfg;

static lv_color_t         lvbuf1[SCREEN_WIDTH * LVGL_BUF_LINES];
static lv_color_t         lvbuf2[SCREEN_WIDTH * LVGL_BUF_LINES];
static lv_disp_draw_buf_t lvbuf;
static lv_disp_drv_t      lvdisp;
static lv_indev_drv_t     lvindev;

static void lv_flush_cb(lv_disp_drv_t *drv, const lv_area_t *area,
                        lv_color_t *px_map) {
    uint32_t w = area->x2 - area->x1 + 1;
    uint32_t h = area->y2 - area->y1 + 1;
    tft.startWrite();
    tft.setAddrWindow(area->x1, area->y1, w, h);
    tft.pushColors((uint16_t *)px_map, w * h, false);
    tft.endWrite();
    lv_disp_flush_ready(drv);
}

static void lv_touch_cb(lv_indev_drv_t *drv, lv_indev_data_t *data) {
    (void)drv;
    TouchPoint pt;
    touch.read(pt);

    // Swipe navigation: only fire on actual touch, reset on finger lift,
    // and block entirely while the alarm ring overlay is shown.
    static bool gesture_handled = false;
    static bool was_pressed = false;
    if (!pt.pressed && was_pressed) gesture_handled = false;  // reset on lift
    was_pressed = pt.pressed;
    if (scr_ring == nullptr &&
        pt.pressed && !gesture_handled && pt.gesture != CST816Gesture::None) {
        gesture_handled = true;
        if (pt.gesture == CST816Gesture::SwipeLeft)
            ui_show_screen_titled((g_current_screen + 1) % 4, +1);
        else if (pt.gesture == CST816Gesture::SwipeRight)
            ui_show_screen_titled((g_current_screen + 3) % 4, -1);
        else if ((pt.gesture == CST816Gesture::SwipeUp ||
                  pt.gesture == CST816Gesture::SwipeDown)) {
            int dy = (pt.gesture == CST816Gesture::SwipeUp) ? 60 : -60;
            if (g_current_screen == SCREEN_CLOCK)
                ui_clock_set_mode(!g_clock_digital);
            else if (g_current_screen == SCREEN_WEATHER)
                ui_weather_scroll(dy);
            else if (g_current_screen == SCREEN_SETTINGS)
                ui_settings_scroll(dy);
        }
    }

    if (pt.pressed && pt.gesture == CST816Gesture::None) {
        data->point.x = pt.x;
        data->point.y = pt.y;
        data->state   = LV_INDEV_STATE_PRESSED;
    } else {
        data->state = LV_INDEV_STATE_RELEASED;
    }
}

static void wifi_connect() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    uint32_t t0 = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - t0 < WIFI_TIMEOUT_MS)
        delay(250);
    if (WiFi.status() == WL_CONNECTED)
        Serial.printf("[WiFi] %s\n", WiFi.localIP().toString().c_str());
    else
        Serial.println("[WiFi] offline mode");
}

static void ntp_sync() {
    if (WiFi.status() != WL_CONNECTED) return;
    configTzTime(NTP_TZ, rt_cfg.ntp_server[0] ? rt_cfg.ntp_server : NTP_SERVER1, NTP_SERVER2);
    struct tm ti;
    uint32_t t0 = millis();
    while (!getLocalTime(&ti, 1000) && millis() - t0 < 10000) delay(500);
}

static void draw_splash() {
    tft.fillScreen(TFT_BLACK);
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("Budik",        120,  96, 4);
    tft.drawString("Pripojuji...", 120, 136, 2);
}

static CalEvent  cal_events[CALENDAR_MAX_EVENTS];
static int       cal_count         = 0;
static uint32_t  last_calendar_ms  = 0;
static uint32_t  last_cal_refresh_ms = 0;

static uint32_t last_sec_ms     = 0;
static uint32_t last_weather_ms = 0;
static uint32_t last_ntp_ms     = 0;
static bool     alarm_was_ringing = false;

void setup() {
    Serial.begin(115200);
    delay(500);   // let USB CDC enumerate before first log lines
    pinMode(PIN_BACKLIGHT, OUTPUT);
    digitalWrite(PIN_BACKLIGHT, HIGH);

    tft.init();
    tft.setRotation(0);
    draw_splash();

    Serial.printf("[Touch] begin=%s\n", touch.begin() ? "OK" : "FAIL");

    alarmMgr.begin();
    wifi_connect();
    runtime_config_load(rt_cfg);
    ntp_sync();
    last_ntp_ms = millis();

    lv_init();
    lv_disp_draw_buf_init(&lvbuf, lvbuf1, lvbuf2, SCREEN_WIDTH * LVGL_BUF_LINES);

    lv_disp_drv_init(&lvdisp);
    lvdisp.hor_res   = SCREEN_WIDTH;
    lvdisp.ver_res   = SCREEN_HEIGHT;
    lvdisp.flush_cb  = lv_flush_cb;
    lvdisp.draw_buf  = &lvbuf;
    lv_disp_drv_register(&lvdisp);

    lv_indev_drv_init(&lvindev);
    lvindev.type    = LV_INDEV_TYPE_POINTER;
    lvindev.read_cb = lv_touch_cb;
    lv_indev_drv_register(&lvindev);

    ui_init(&alarmMgr, &weather);

    if (WiFi.status() == WL_CONNECTED) {
        web_server_begin(&rt_cfg, &alarmMgr);
        ui_settings_set_ip(WiFi.localIP().toString().c_str());
    }

    struct tm ti;
    if (getLocalTime(&ti, 100))
        ui_update_time(ti.tm_hour, ti.tm_min, ti.tm_sec,
                       ti.tm_wday, ti.tm_mday, ti.tm_mon + 1,
                       ti.tm_year + 1900);

    if (WiFi.status() == WL_CONNECTED) {
        weather_fetch(weather);
        ui_update_weather(weather);
        last_weather_ms = millis();

        if (rt_cfg.calendar_url[0] != '\0') {
            calendar_fetch(rt_cfg.calendar_url, cal_events, CALENDAR_MAX_EVENTS, cal_count);
            last_calendar_ms = millis();
        }
        ui_update_calendar(cal_events, cal_count);
        last_cal_refresh_ms = millis();
    }
}

void loop() {
    web_server_handle();

    // Advance LVGL's tick so its internal timers (indev poll, anim, etc.) fire.
    // Without this lv_tick_get() always returns 0 and no LVGL timer ever expires.
    static uint32_t lv_tick_prev = 0;
    uint32_t now_ms = millis();
    lv_tick_inc(now_ms - lv_tick_prev);
    lv_tick_prev = now_ms;

    lv_timer_handler();

    uint32_t now = millis();

    if (now - last_sec_ms >= 1000) {
        last_sec_ms = now;
        struct tm ti;
        if (getLocalTime(&ti, 50)) {
            ui_update_time(ti.tm_hour, ti.tm_min, ti.tm_sec,
                           ti.tm_wday, ti.tm_mday, ti.tm_mon + 1,
                           ti.tm_year + 1900);
            alarmMgr.tick(ti.tm_hour, ti.tm_min, ti.tm_sec);
        }
        if (alarmMgr.is_ringing() && !alarm_was_ringing) {
            ui_show_alarm_ring(&alarmMgr);
            alarm_was_ringing = true;
        } else if (!alarmMgr.is_ringing() && alarm_was_ringing) {
            ui_hide_alarm_ring();
            alarm_was_ringing = false;
        }
    }

    // fast buzzer pattern update
    { struct tm ti; if (getLocalTime(&ti, 0)) alarmMgr.tick(ti.tm_hour, ti.tm_min, ti.tm_sec); }

    if (WiFi.status() == WL_CONNECTED && now - last_weather_ms >= OW_UPDATE_INTERVAL_MS) {
        last_weather_ms = now;
        if (weather_fetch(weather)) ui_update_weather(weather);
    }

    if (WiFi.status() == WL_CONNECTED && rt_cfg.calendar_url[0] != '\0' &&
        now - last_calendar_ms >= CALENDAR_UPDATE_INTERVAL_MS) {
        last_calendar_ms = now;
        if (calendar_fetch(rt_cfg.calendar_url, cal_events, CALENDAR_MAX_EVENTS, cal_count)) {
            ui_update_calendar(cal_events, cal_count);
            last_cal_refresh_ms = now;
        }
    }

    // Refresh calendar UI every 60s to update past/current/future status colours
    if (now - last_cal_refresh_ms >= 60000UL && cal_count > 0) {
        last_cal_refresh_ms = now;
        ui_update_calendar(cal_events, cal_count);
    }

    if (WiFi.status() == WL_CONNECTED && now - last_ntp_ms >= NTP_SYNC_INTERVAL_MS) {
        last_ntp_ms = now;
        ntp_sync();
    }

    static uint32_t wifi_chk = 0;
    if (now - wifi_chk >= 30000) {
        wifi_chk = now;
        if (WiFi.status() != WL_CONNECTED) WiFi.reconnect();
    }

    delay(5);
}
