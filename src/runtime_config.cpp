#include "runtime_config.h"
#include "config.h"
#include <Preferences.h>

void runtime_config_load(RuntimeConfig &cfg) {
    Preferences p;
    p.begin("clock", true);
    strlcpy(cfg.ntp_server,   p.getString("ntp", NTP_SERVER1).c_str(),    RT_NTP_LEN);
    strlcpy(cfg.calendar_url, p.getString("cal", CALENDAR_URL).c_str(),   RT_CAL_LEN);
    p.end();
}

void runtime_config_save(const RuntimeConfig &cfg) {
    Preferences p;
    p.begin("clock", false);
    p.putString("ntp", cfg.ntp_server);
    p.putString("cal", cfg.calendar_url);
    p.end();
}
