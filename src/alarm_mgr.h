#pragma once
#include <Arduino.h>

struct AlarmConfig {
    uint8_t hour;
    uint8_t minute;
    bool    enabled;
};

class AlarmManager {
public:
    AlarmManager(uint8_t buzzer_pin);
    void begin();
    void save();
    void set(uint8_t h, uint8_t m, bool enabled);
    AlarmConfig get() const { return _cfg; }
    void tick(uint8_t cur_h, uint8_t cur_m, uint8_t cur_s);
    bool is_ringing() const { return _ringing; }
    void snooze();
    void stop();

private:
    uint8_t     _pin;
    AlarmConfig _cfg;
    bool        _ringing         = false;
    bool        _triggered_today = false;
    uint32_t    _ring_start_ms   = 0;
    uint8_t     _snooze_h        = 0;
    uint8_t     _snooze_m        = 0;
    bool        _snooze_active   = false;
    void buzz_on();
    void buzz_off();
    void pattern_tick();
    uint32_t _last_pattern_ms = 0;
    bool     _buzz_state      = false;
};
