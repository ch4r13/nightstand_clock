#include <Arduino.h>
#include <WiFi.h>
#include <TFT_eSPI.h>
#include <lvgl.h>
#include <time.h>

#include "config.h"
#include "touch_cst816.h"
#include "weather_api.h"
#include "alarm_mgr.h"
#include "ui_screens.h"

static TFT_eSPI     tft;
static CST816Touch  touch(PIN_TOUCH_SDA, PIN_TOUCH_SCL,
                          PIN_TOUCH_INT, PIN_TOUCH_RST);
static AlarmManager alarmMgr(PIN_BUZZER);
static WeatherData  weather = {};

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
    tft.pushColors((uint16_t *)px_map, w * h, true);
    tft.endWrite();
    lv_disp_flush_ready(drv);
}

static void lv_touch_cb(lv_indev_drv_t *drv, lv_indev_data_t *data) {
    (void)drv;
    TouchPoint pt;
    if (touch.read(pt) && pt.pressed) {
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
    configTzTime(NTP_TZ, NTP_SERVER1, NTP_SERVER2);
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

static uint32_t last_sec_ms     = 0;
static uint32_t last_weather_ms = 0;
static uint32_t last_ntp_ms     = 0;
static bool     alarm_was_ringing = false;

void setup() {
    Serial.begin(115200);
    pinMode(PIN_BACKLIGHT, OUTPUT);
    digitalWrite(PIN_BACKLIGHT, HIGH);

    tft.init();
    tft.setRotation(0);
    draw_splash();

    if (!touch.begin())
        Serial.println("[Touch] not found");

    alarmMgr.begin();
    wifi_connect();
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

    struct tm ti;
    if (getLocalTime(&ti, 100))
        ui_update_time(ti.tm_hour, ti.tm_min, ti.tm_sec,
                       ti.tm_wday, ti.tm_mday, ti.tm_mon + 1,
                       ti.tm_year + 1900);

    if (WiFi.status() == WL_CONNECTED) {
        weather_fetch(weather);
        ui_update_weather(weather);
        last_weather_ms = millis();
    }
}

void loop() {
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
