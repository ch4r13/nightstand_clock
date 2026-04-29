#include "alarm_mgr.h"
#include "config.h"
#include <Preferences.h>

static Preferences prefs;

AlarmManager::AlarmManager(uint8_t buzzer_pin) : _pin(buzzer_pin) {}

void AlarmManager::begin() {
    prefs.begin("alarm", true);
    _cfg.hour    = prefs.getUChar("h",  7);
    _cfg.minute  = prefs.getUChar("m",  0);
    _cfg.enabled = prefs.getBool("en", false);
    prefs.end();

    ledcAttach(_pin, ALARM_FREQ_HZ, BUZZER_LEDC_RES);
    buzz_off();
}

void AlarmManager::save() {
    prefs.begin("alarm", false);
    prefs.putUChar("h",  _cfg.hour);
    prefs.putUChar("m",  _cfg.minute);
    prefs.putBool("en",  _cfg.enabled);
    prefs.end();
}

void AlarmManager::set(uint8_t h, uint8_t m, bool enabled) {
    _cfg.hour    = h;
    _cfg.minute  = m;
    _cfg.enabled = enabled;
    save();
    _triggered_today = false;
}

void AlarmManager::tick(uint8_t cur_h, uint8_t cur_m, uint8_t cur_s) {
    if (!_cfg.enabled) return;

    if (cur_h == 0 && cur_m == 0 && cur_s == 0) {
        _triggered_today = false;
        _snooze_active   = false;
    }

    if (_snooze_active && cur_h == _snooze_h && cur_m == _snooze_m && cur_s == 0) {
        _snooze_active   = false;
        _triggered_today = false;
    }

    if (!_triggered_today && !_ringing &&
        cur_h == _cfg.hour && cur_m == _cfg.minute && cur_s == 0) {
        _ringing         = true;
        _triggered_today = true;
        _ring_start_ms   = millis();
    }

    if (_ringing) {
        if (millis() - _ring_start_ms > (uint32_t)ALARM_AUTO_STOP_MIN * 60000UL) {
            stop();
            return;
        }
        pattern_tick();
    }
}

void AlarmManager::snooze() {
    if (!_ringing) return;
    stop();
    uint16_t total = _cfg.hour * 60 + _cfg.minute + ALARM_SNOOZE_MIN;
    total %= 1440;
    _snooze_h      = total / 60;
    _snooze_m      = total % 60;
    _snooze_active   = true;
    _triggered_today = true;
}

void AlarmManager::stop() {
    _ringing = false;
    buzz_off();
}

void AlarmManager::buzz_on() {
    ledcWrite(_pin, (1 << (BUZZER_LEDC_RES - 1)));
}

void AlarmManager::buzz_off() {
    ledcWrite(_pin, 0);
}

void AlarmManager::pattern_tick() {
    uint32_t now    = millis();
    uint32_t period = _buzz_state ? ALARM_PATTERN_ON_MS : ALARM_PATTERN_OFF_MS;
    if (now - _last_pattern_ms >= period) {
        _buzz_state = !_buzz_state;
        _buzz_state ? buzz_on() : buzz_off();
        _last_pattern_ms = now;
    }
}
