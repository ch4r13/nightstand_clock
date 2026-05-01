#pragma once
#include <Arduino.h>

#define RT_NTP_LEN  64
#define RT_CAL_LEN  256

struct RuntimeConfig {
    char ntp_server[RT_NTP_LEN];
    char calendar_url[RT_CAL_LEN];
};

void runtime_config_load(RuntimeConfig &cfg);
void runtime_config_save(const RuntimeConfig &cfg);
