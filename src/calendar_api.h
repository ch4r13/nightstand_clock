#pragma once
#include <Arduino.h>
#include <time.h>

#define CALENDAR_MAX_EVENTS   20
#define CALENDAR_SUMMARY_LEN  48

struct CalEvent {
    time_t start;
    time_t end;
    char   summary[CALENDAR_SUMMARY_LEN];
    bool   all_day;
};

bool calendar_fetch(const char *url, CalEvent *events, int max_events, int &count);
