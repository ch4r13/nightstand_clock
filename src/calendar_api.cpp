#include "calendar_api.h"
#include "serial_compat.h"
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>

// ─────────────────────────────────────────────────────────────
// Helpers
// ─────────────────────────────────────────────────────────────

// Returns offset = local_now - utc_now (seconds east of UTC).
static long tz_offset_secs() {
    time_t now = time(nullptr);
    struct tm *gm = gmtime(&now);
    time_t utc = mktime(gm);   // mktime treats its input as local, giving wrong result
    // gmtime returns UTC fields; mktime re-interprets them as local → difference gives offset
    return (long)(now - utc);
}

// Parse iCal datetime string to time_t (local).
// Accepts: YYYYMMDD  or  YYYYMMDDTHHmmss  or  YYYYMMDDTHHmmssZ
static time_t parse_dt(const char *s, bool &all_day_out) {
    if (!s || strlen(s) < 8) return 0;

    struct tm t = {};
    all_day_out = false;

    // Date portion
    char tmp[5];
    tmp[4] = '\0';

    memcpy(tmp, s, 4);   t.tm_year = atoi(tmp) - 1900;
    memcpy(tmp, s+4, 2); tmp[2]='\0'; t.tm_mon  = atoi(tmp) - 1;
    memcpy(tmp, s+6, 2); tmp[2]='\0'; t.tm_mday = atoi(tmp);

    if (strlen(s) == 8) {
        // DATE-only – all-day event
        all_day_out = true;
        t.tm_hour = 0; t.tm_min = 0; t.tm_sec = 0;
        t.tm_isdst = -1;
        return mktime(&t);
    }

    // Time portion (after 'T')
    const char *tp = s + 9;
    memcpy(tmp, tp,   2); tmp[2]='\0'; t.tm_hour = atoi(tmp);
    memcpy(tmp, tp+2, 2); tmp[2]='\0'; t.tm_min  = atoi(tmp);
    memcpy(tmp, tp+4, 2); tmp[2]='\0'; t.tm_sec  = atoi(tmp);
    t.tm_isdst = -1;

    bool is_utc = (s[strlen(s)-1] == 'Z');
    time_t result = mktime(&t);   // treated as local
    if (is_utc) {
        // mktime assumed local; add offset to convert UTC→local
        result += tz_offset_secs();
    }
    return result;
}

// ─────────────────────────────────────────────────────────────
// Unfold iCal line continuations (RFC 5545 §3.1)
// A CRLF followed immediately by a space or tab is a fold; remove it.
// ─────────────────────────────────────────────────────────────
static String unfold(const String &raw) {
    String out;
    out.reserve(raw.length());
    for (int i = 0; i < (int)raw.length(); i++) {
        if (raw[i] == '\r' && i+1 < (int)raw.length() && raw[i+1] == '\n' &&
            i+2 < (int)raw.length() && (raw[i+2] == ' ' || raw[i+2] == '\t')) {
            i += 2;  // skip CRLF + whitespace
            continue;
        }
        out += raw[i];
    }
    return out;
}

// ─────────────────────────────────────────────────────────────
// Extract value after ":" on a property line, stripping TZID= prefix lines.
// e.g. "DTSTART;TZID=Europe/Prague:20250101T080000" → "20250101T080000"
// ─────────────────────────────────────────────────────────────
static const char *prop_value(const char *line) {
    const char *colon = strchr(line, ':');
    if (!colon) return nullptr;
    return colon + 1;
}

// ─────────────────────────────────────────────────────────────
// Strip Czech/Slovak diacritics: replace UTF-8 multi-byte sequences
// for accented Latin characters with their plain ASCII equivalent.
// Any unrecognised multi-byte byte is dropped; ASCII passes through.
// ─────────────────────────────────────────────────────────────
static void strip_diacritics(const char *src, char *dst, size_t max) {
    size_t di = 0;
    const unsigned char *s = (const unsigned char *)src;
    while (*s && di + 1 < max) {
        if (*s < 0x80) {
            dst[di++] = (char)*s++;
            continue;
        }
        // 2-byte sequences only (Czech fits in U+0080..U+017F → 0xC2..0xC5 lead)
        if ((*s & 0xE0) == 0xC0 && s[1]) {
            unsigned b0 = *s, b1 = s[1];
            char rep = '?';
            if (b0 == 0xC3) {
                switch (b1) {
                    case 0x81: rep='A'; break;  // Á
                    case 0x89: rep='E'; break;  // É
                    case 0x8D: rep='I'; break;  // Í
                    case 0x93: rep='O'; break;  // Ó
                    case 0x9A: rep='U'; break;  // Ú
                    case 0x9D: rep='Y'; break;  // Ý
                    case 0xA1: rep='a'; break;  // á
                    case 0xA9: rep='e'; break;  // é
                    case 0xAD: rep='i'; break;  // í
                    case 0xB3: rep='o'; break;  // ó
                    case 0xBA: rep='u'; break;  // ú
                    case 0xBD: rep='y'; break;  // ý
                    default:   rep= 0;  break;  // drop others from C3 block
                }
            } else if (b0 == 0xC4) {
                switch (b1) {
                    case 0x8C: rep='C'; break;  // Č
                    case 0x8D: rep='c'; break;  // č
                    case 0x8E: rep='D'; break;  // Ď
                    case 0x8F: rep='d'; break;  // ď
                    case 0x9A: rep='E'; break;  // Ě
                    case 0x9B: rep='e'; break;  // ě
                    default:   rep= 0;  break;
                }
            } else if (b0 == 0xC5) {
                switch (b1) {
                    case 0x87: rep='N'; break;  // Ň
                    case 0x88: rep='n'; break;  // ň
                    case 0x98: rep='R'; break;  // Ř
                    case 0x99: rep='r'; break;  // ř
                    case 0xA0: rep='S'; break;  // Š
                    case 0xA1: rep='s'; break;  // š
                    case 0xA4: rep='T'; break;  // Ť
                    case 0xA5: rep='t'; break;  // ť
                    case 0xAE: rep='U'; break;  // Ů
                    case 0xAF: rep='u'; break;  // ů
                    case 0xBD: rep='Z'; break;  // Ž
                    case 0xBE: rep='z'; break;  // ž
                    default:   rep= 0;  break;
                }
            }
            if (rep) dst[di++] = rep;
            s += 2;
            continue;
        }
        // 3/4-byte sequence: skip entirely
        if ((*s & 0xF0) == 0xE0)      { s += 3; continue; }
        if ((*s & 0xF8) == 0xF0)      { s += 4; continue; }
        s++;  // unexpected byte, skip
    }
    dst[di] = '\0';
}

// ─────────────────────────────────────────────────────────────
// Compare-function for qsort (sort CalEvent by start time)
// ─────────────────────────────────────────────────────────────
static int cmp_event(const void *a, const void *b) {
    const CalEvent *ea = (const CalEvent *)a;
    const CalEvent *eb = (const CalEvent *)b;
    if (ea->start < eb->start) return -1;
    if (ea->start > eb->start) return  1;
    return 0;
}

// ─────────────────────────────────────────────────────────────
// Public API
// ─────────────────────────────────────────────────────────────
bool calendar_fetch(const char *url, CalEvent *events, int max_events, int &count) {
    count = 0;
    if (!url || url[0] == '\0') return false;

    HTTPClient http;
    WiFiClientSecure *secClient = nullptr;

    bool is_https = (strncmp(url, "https://", 8) == 0);
    if (is_https) {
        secClient = new WiFiClientSecure();
        secClient->setInsecure();
        http.begin(*secClient, url);
    } else {
        http.begin(url);
    }

    http.setTimeout(10000);
    int code = http.GET();
    if (code != 200) {
        Serial.printf("[Cal] HTTP %d\n", code);
        http.end();
        if (secClient) delete secClient;
        return false;
    }

    String body = http.getString();
    http.end();
    if (secClient) delete secClient;

    body = unfold(body);

    // ── Parse VEVENT blocks ──────────────────────────────────
    // Midnight of today (local) and tomorrow
    time_t now = time(nullptr);
    struct tm today_tm;
    localtime_r(&now, &today_tm);
    today_tm.tm_hour = 0; today_tm.tm_min = 0; today_tm.tm_sec = 0; today_tm.tm_isdst = -1;
    time_t today_midnight    = mktime(&today_tm);
    time_t tomorrow_midnight = today_midnight + 86400;

    int    idx       = 0;
    bool   in_vevent = false;

    // Temporary event accumulator
    CalEvent ev = {};
    bool     got_start = false, got_end = false;

    // Walk line-by-line
    int pos = 0;
    int len = (int)body.length();
    while (pos < len) {
        // Find end of line (\n or \r\n)
        int eol = pos;
        while (eol < len && body[eol] != '\n') eol++;
        // Line is body[pos..eol-1] (strip trailing \r)
        int line_end = eol;
        if (line_end > pos && body[line_end-1] == '\r') line_end--;

        char line[256];
        int  ll = line_end - pos;
        if (ll > 255) ll = 255;
        memcpy(line, body.c_str() + pos, ll);
        line[ll] = '\0';

        pos = eol + 1;

        if (strcmp(line, "BEGIN:VEVENT") == 0) {
            in_vevent = true;
            ev = {};
            got_start = got_end = false;
        } else if (strcmp(line, "END:VEVENT") == 0 && in_vevent) {
            in_vevent = false;
            if (got_start) {
                // For all-day events without DTEND, end = start + 1 day
                if (!got_end) {
                    ev.end = ev.start + 86400;
                }
                // Filter: overlaps today
                if (ev.start < tomorrow_midnight && ev.end > today_midnight) {
                    if (idx < max_events) {
                        events[idx++] = ev;
                    }
                }
            }
        } else if (in_vevent) {
            if (strncmp(line, "DTSTART", 7) == 0) {
                const char *val = prop_value(line);
                if (val) {
                    bool ad = false;
                    ev.start   = parse_dt(val, ad);
                    ev.all_day = ad;
                    got_start  = true;
                }
            } else if (strncmp(line, "DTEND", 5) == 0) {
                const char *val = prop_value(line);
                if (val) {
                    bool ad = false;
                    ev.end  = parse_dt(val, ad);
                    got_end = true;
                }
            } else if (strncmp(line, "SUMMARY", 7) == 0) {
                const char *val = prop_value(line);
                if (val) {
                    strip_diacritics(val, ev.summary, CALENDAR_SUMMARY_LEN);
                }
            }
        }
    }

    count = idx;
    qsort(events, count, sizeof(CalEvent), cmp_event);

    Serial.printf("[Cal] fetched %d events\n", count);
    return true;
}
