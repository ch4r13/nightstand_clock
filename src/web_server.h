#pragma once
#include "runtime_config.h"
#include "alarm_mgr.h"

void web_server_begin(RuntimeConfig *cfg, AlarmManager *alarm);
void web_server_handle();
