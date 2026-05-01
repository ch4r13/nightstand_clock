#include "ui_shared.h"
#include "ui_screens.h"
#include "calendar_api.h"
#include "config.h"
#include <time.h>

// ─────────────────────────────────────────────────────────────
// Calendar screen widgets (definitions live in ui_screens.cpp)
// ─────────────────────────────────────────────────────────────

void build_calendar_screen() {
    scr_calendar = lv_obj_create(nullptr);
    lv_obj_set_style_bg_color(scr_calendar, lv_color_hex(0x0A1628), 0);
    lv_obj_set_style_bg_opa(scr_calendar, LV_OPA_COVER, 0);
    lv_obj_clear_flag(scr_calendar, LV_OBJ_FLAG_SCROLLABLE);

    // Title
    lv_obj_t *title = lv_label_create(scr_calendar);
    lv_label_set_text(title, "Kalendar");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(title, C_ACCENT, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 14);

    // Date label
    lbl_cal_date = lv_label_create(scr_calendar);
    lv_obj_set_style_text_font(lbl_cal_date, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(lbl_cal_date, C_DATE, 0);
    lv_obj_align(lbl_cal_date, LV_ALIGN_TOP_MID, 0, 30);
    lv_label_set_text(lbl_cal_date, "");

    // Scrollable event list container
    cal_list = lv_obj_create(scr_calendar);
    lv_obj_set_size(cal_list, 180, 150);
    lv_obj_align(cal_list, LV_ALIGN_TOP_MID, 0, 48);
    lv_obj_set_style_bg_opa(cal_list, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(cal_list, 0, 0);
    lv_obj_set_style_pad_all(cal_list, 0, 0);
    // Keep scrollable so long lists can be scrolled
    lv_obj_set_scroll_dir(cal_list, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(cal_list, LV_SCROLLBAR_MODE_OFF);
}

void ui_update_calendar(const CalEvent *events, int count) {
    if (!scr_calendar) return;

    // Czech date strings (mirrors ui_screens.cpp)
    static const char *wday_cs[]  = {"Ne","Po","Ut","St","Ct","Pa","So"};
    static const char *month_cs[] = {"Led","Unor","Bre","Dub","Kve","Cer",
                                     "Cvc","Srp","Zar","Rij","Lis","Pro"};

    // Update date label with today
    time_t now = time(nullptr);
    struct tm ti;
    localtime_r(&now, &ti);
    if (lbl_cal_date) {
        char dbuf[32];
        snprintf(dbuf, sizeof(dbuf), "%s %d. %s %d",
                 wday_cs[ti.tm_wday % 7],
                 ti.tm_mday,
                 month_cs[(ti.tm_mon) % 12],
                 ti.tm_year + 1900);
        lv_label_set_text(lbl_cal_date, dbuf);
    }

    if (!cal_list) return;

    // Midnight boundaries for today/tomorrow
    struct tm tod = ti;
    tod.tm_hour = 0; tod.tm_min = 0; tod.tm_sec = 0; tod.tm_isdst = -1;
    time_t today_midnight    = mktime(&tod);
    time_t tomorrow_midnight = today_midnight + 86400;

    lv_obj_clean(cal_list);

    if (count == 0) {
        lv_obj_t *lbl = lv_label_create(cal_list);
        lv_label_set_text(lbl, "Zadne udalosti");
        lv_obj_set_style_text_font(lbl, &lv_font_montserrat_12, 0);
        lv_obj_set_style_text_color(lbl, C_DIM, 0);
        lv_obj_set_width(lbl, 178);
        lv_obj_set_style_text_align(lbl, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_pos(lbl, 0, 50);
        lv_obj_scroll_to_y(cal_list, 0, LV_ANIM_OFF);
        return;
    }

    int row          = 0;
    bool sep_placed  = false;

    for (int i = 0; i < count; i++) {
        const CalEvent &ev = events[i];

        // Determine status
        bool is_past    = (ev.end   <= now);
        bool is_current = (ev.start <= now && ev.end > now);
        // future = !is_past && !is_current

        // Insert separator before first future event
        if (!sep_placed && !is_past && !is_current) {
            sep_placed = true;
            lv_obj_t *sep = lv_label_create(cal_list);
            lv_label_set_text(sep, "--- NYNI ---");
            lv_obj_set_style_text_font(sep, &lv_font_montserrat_12, 0);
            lv_obj_set_style_text_color(sep, C_ACCENT, 0);
            lv_obj_set_width(sep, 178);
            lv_obj_set_style_text_align(sep, LV_TEXT_ALIGN_CENTER, 0);
            lv_label_set_long_mode(sep, LV_LABEL_LONG_CLIP);
            lv_obj_set_pos(sep, 0, row * 20);
            row++;
        }

        // Format time range
        char timebuf[20];
        if (ev.all_day) {
            snprintf(timebuf, sizeof(timebuf), "celodenne  ");
        } else {
            struct tm ts, te;
            localtime_r(&ev.start, &ts);
            localtime_r(&ev.end,   &te);
            snprintf(timebuf, sizeof(timebuf), "%02d:%02d-%02d:%02d ",
                     ts.tm_hour, ts.tm_min, te.tm_hour, te.tm_min);
        }

        // Prefix character
        const char *prefix = is_past ? "- " : (is_current ? "> " : "  ");

        char text[80];
        snprintf(text, sizeof(text), "%s%s%s", prefix, timebuf, ev.summary);

        lv_obj_t *lbl = lv_label_create(cal_list);
        lv_label_set_text(lbl, text);
        lv_obj_set_style_text_font(lbl, &lv_font_montserrat_12, 0);
        lv_obj_set_width(lbl, 178);
        lv_label_set_long_mode(lbl, LV_LABEL_LONG_CLIP);
        lv_obj_set_pos(lbl, 0, row * 20);

        lv_color_t col = is_past ? C_DIM : (is_current ? C_ACCENT : C_WHITE);
        lv_obj_set_style_text_color(lbl, col, 0);

        row++;
    }

    lv_obj_scroll_to_y(cal_list, 0, LV_ANIM_OFF);
}
