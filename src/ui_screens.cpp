#include "ui_screens.h"
#include "ui_shared.h"
#include "config.h"

// ── Shared globals ──────────────────────────────────────────
AlarmManager *g_alarm   = nullptr;
WeatherData  *g_weather = nullptr;

lv_obj_t *scr_clock    = nullptr;
lv_obj_t *scr_weather  = nullptr;
lv_obj_t *scr_calendar = nullptr;
lv_obj_t *scr_settings = nullptr;
lv_obj_t *scr_ring     = nullptr;

lv_obj_t *lbl_time       = nullptr;
lv_obj_t *lbl_date       = nullptr;
lv_obj_t *lbl_alarm_icon = nullptr;
lv_obj_t *clock_face     = nullptr;
uint8_t   c_h = 0, c_m = 0, c_s = 0;

lv_obj_t *lbl_wicon     = nullptr;
lv_obj_t *lbl_wtemp     = nullptr;
lv_obj_t *lbl_wdesc     = nullptr;
lv_obj_t *lbl_wtomorrow = nullptr;
lv_obj_t *lbl_whumidity = nullptr;
char      g_wicon_code[8] = "03d";

lv_obj_t *roller_h = nullptr;
lv_obj_t *roller_m = nullptr;
lv_obj_t *sw_alarm = nullptr;

lv_obj_t *cal_list     = nullptr;
lv_obj_t *lbl_cal_date = nullptr;
lv_obj_t *clock_bezel  = nullptr;
bool      g_clock_digital = false;

int g_current_screen = SCREEN_CLOCK;

// ── Title overlay (on lv_layer_top) ─────────────────────────
static lv_obj_t *s_title_bg  = nullptr;
static lv_obj_t *s_title_lbl = nullptr;

static void init_title_overlay() {
    lv_obj_t *top = lv_layer_top();

    s_title_bg = lv_obj_create(top);
    lv_obj_set_size(s_title_bg, 150, 44);
    lv_obj_align(s_title_bg, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_color(s_title_bg, lv_color_hex(0x0D1B2A), 0);
    lv_obj_set_style_bg_opa(s_title_bg, 210, 0);
    lv_obj_set_style_border_width(s_title_bg, 0, 0);
    lv_obj_set_style_radius(s_title_bg, 22, 0);
    lv_obj_set_style_opa(s_title_bg, LV_OPA_TRANSP, 0);
    lv_obj_clear_flag(s_title_bg, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(s_title_bg, LV_OBJ_FLAG_SCROLLABLE);

    s_title_lbl = lv_label_create(s_title_bg);
    lv_label_set_text(s_title_lbl, "");
    lv_obj_set_style_text_font(s_title_lbl, &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_color(s_title_lbl, C_WHITE, 0);
    lv_obj_center(s_title_lbl);
}

// ── Alarm ring overlay ───────────────────────────────────────
static AlarmManager *g_ring_alarm = nullptr;
static void ring_snooze_cb(lv_event_t *) { if(g_ring_alarm) g_ring_alarm->snooze(); ui_hide_alarm_ring(); }
static void ring_stop_cb(lv_event_t *)   { if(g_ring_alarm) g_ring_alarm->stop();   ui_hide_alarm_ring(); }

void ui_show_alarm_ring(AlarmManager *alarm) {
    g_ring_alarm = alarm;
    if (scr_ring) { lv_obj_del(scr_ring); scr_ring = nullptr; }
    // Show clock screen behind the overlay
    if (lv_scr_act() != scr_clock) lv_scr_load(scr_clock);
    // Overlay on lv_layer_top so it sits above the clock face
    scr_ring = lv_obj_create(lv_layer_top());
    lv_obj_set_size(scr_ring, 240, 240);
    lv_obj_align(scr_ring, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_style_pad_all(scr_ring, 0, 0);
    lv_obj_set_style_bg_color(scr_ring, lv_color_hex(0x1A0005), 0);
    lv_obj_set_style_bg_opa(scr_ring, 230, 0);
    lv_obj_set_style_border_width(scr_ring, 0, 0);
    lv_obj_set_style_radius(scr_ring, 0, 0);
    lv_obj_clear_flag(scr_ring, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *bell = lv_label_create(scr_ring);
    lv_label_set_text(bell, LV_SYMBOL_BELL);
    lv_obj_set_style_text_font(bell, &lv_font_montserrat_48, 0);
    lv_obj_set_style_text_color(bell, C_ALARM_BG, 0);
    lv_obj_align(bell, LV_ALIGN_TOP_MID, 0, 36);

    lv_anim_t a; lv_anim_init(&a);
    lv_anim_set_var(&a, bell);
    lv_anim_set_values(&a, 180, 220);
    lv_anim_set_time(&a, 400);
    lv_anim_set_playback_time(&a, 400);
    lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
    lv_anim_set_exec_cb(&a, [](void *o, int32_t v){ lv_obj_set_style_text_opa((lv_obj_t*)o,(lv_opa_t)v,0); });
    lv_anim_start(&a);

    lv_obj_t *lbl = lv_label_create(scr_ring);
    lv_label_set_text(lbl, "POSTAVAT!");
    lv_obj_set_style_text_font(lbl, &lv_font_montserrat_36, 0);
    lv_obj_set_style_text_color(lbl, C_WHITE, 0);
    lv_obj_align(lbl, LV_ALIGN_TOP_MID, 0, 96);

    AlarmConfig cfg = alarm->get();
    char tbuf[16]; snprintf(tbuf, sizeof(tbuf), "%02d:%02d", cfg.hour, cfg.minute);
    lv_obj_t *lt = lv_label_create(scr_ring);
    lv_label_set_text(lt, tbuf);
    lv_obj_set_style_text_font(lt, &lv_font_montserrat_28, 0);
    lv_obj_set_style_text_color(lt, C_DATE, 0);
    lv_obj_align(lt, LV_ALIGN_TOP_MID, 0, 140);

    lv_obj_t *bs = lv_btn_create(scr_ring);
    lv_obj_set_size(bs, 100, 44); lv_obj_align(bs, LV_ALIGN_BOTTOM_LEFT, 20, -56);
    lv_obj_set_style_bg_color(bs, lv_color_hex(0x1E3A5F), 0);
    lv_obj_set_style_radius(bs, 22, 0);
    lv_obj_add_event_cb(bs, ring_snooze_cb, LV_EVENT_CLICKED, nullptr);
    lv_obj_t *sl = lv_label_create(bs);
    lv_label_set_text_fmt(sl, LV_SYMBOL_PAUSE " +%dm", ALARM_SNOOZE_MIN);
    lv_obj_set_style_text_color(sl, C_WHITE, 0); lv_obj_center(sl);

    lv_obj_t *bx = lv_btn_create(scr_ring);
    lv_obj_set_size(bx, 100, 44); lv_obj_align(bx, LV_ALIGN_BOTTOM_RIGHT, -20, -56);
    lv_obj_set_style_bg_color(bx, C_ALARM_BG, 0);
    lv_obj_set_style_radius(bx, 22, 0);
    lv_obj_add_event_cb(bx, ring_stop_cb, LV_EVENT_CLICKED, nullptr);
    lv_obj_t *stl = lv_label_create(bx);
    lv_label_set_text(stl, LV_SYMBOL_CLOSE "  Zavrit");
    lv_obj_set_style_text_color(stl, C_WHITE, 0); lv_obj_center(stl);
}

void ui_hide_alarm_ring() {
    // Just remove the overlay; the clock screen is already active behind it.
    if (scr_ring) { lv_obj_del(scr_ring); scr_ring = nullptr; }
}

// ── Public API ───────────────────────────────────────────────
void ui_init(AlarmManager *alarm, WeatherData *weather) {
    g_alarm = alarm; g_weather = weather;
    build_clock_screen();
    build_weather_screen();
    build_calendar_screen();
    build_settings_screen();
    init_title_overlay();
    // sw_alarm and picker values are initialised inside build_settings_screen().
    lv_scr_load(scr_clock);
}

void ui_show_screen(int idx) {
    g_current_screen = idx;
    lv_obj_t *t = nullptr;
    switch(idx) {
        case SCREEN_CLOCK:     t = scr_clock;    break;
        case SCREEN_WEATHER:   t = scr_weather;  break;
        case SCREEN_CALENDAR:  t = scr_calendar; break;
        case SCREEN_SETTINGS:  t = scr_settings; break;
    }
    if (t) lv_scr_load_anim(t, LV_SCR_LOAD_ANIM_FADE_ON, 200, 0, false);
}

void ui_show_screen_titled(int idx, int direction) {
    g_current_screen = idx;
    lv_obj_t *screens[] = { scr_clock, scr_weather, scr_calendar, scr_settings };
    lv_scr_load_anim_t anim_type = (direction >= 0)
        ? LV_SCR_LOAD_ANIM_MOVE_LEFT
        : LV_SCR_LOAD_ANIM_MOVE_RIGHT;
    if (idx >= 0 && idx < 4 && screens[idx])
        lv_scr_load_anim(screens[idx], anim_type, 250, 0, false);

    if (!s_title_bg || idx < 0 || idx >= 4) return;

    static const char *names[] = { "Cas", "Pocasi", "Kalendar", "Budik" };
    lv_label_set_text(s_title_lbl, names[idx]);
    lv_obj_set_style_opa(s_title_bg, LV_OPA_COVER, 0);

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, s_title_bg);
    lv_anim_set_values(&a, LV_OPA_COVER, LV_OPA_TRANSP);
    lv_anim_set_delay(&a, 800);
    lv_anim_set_time(&a, 500);
    lv_anim_set_exec_cb(&a, [](void *obj, int32_t v) {
        lv_obj_set_style_opa((lv_obj_t*)obj, (lv_opa_t)v, 0);
    });
    lv_anim_start(&a);
}

static const char *wday_cs[]  = {"Ne","Po","Ut","St","Ct","Pa","So"};
static const char *month_cs[] = {"Led","Unor","Bre","Dub","Kve","Cer","Cvc","Srp","Zar","Rij","Lis","Pro"};

void ui_update_time(uint8_t h, uint8_t m, uint8_t s,
                    uint8_t wday, uint8_t day, uint8_t mon, uint16_t year) {
    c_h = h; c_m = m; c_s = s;
    if (clock_face) lv_obj_invalidate(clock_face);
    lv_obj_t *sr = (lv_obj_t*)lv_obj_get_user_data(scr_clock);
    if (sr) lv_arc_set_value(sr, s);
    if (lbl_time) { char b[8]; snprintf(b,sizeof(b),"%02d:%02d",h,m); lv_label_set_text(lbl_time,b); }
    if (lbl_date && s==0) {
        char b[32];
        snprintf(b,sizeof(b),"%s %d. %s %d", wday_cs[wday%7], day, month_cs[(mon-1)%12], year);
        lv_label_set_text(lbl_date, b);
    }
    if (lbl_alarm_icon && g_alarm) {
        g_alarm->get().enabled
            ? lv_obj_clear_flag(lbl_alarm_icon, LV_OBJ_FLAG_HIDDEN)
            : lv_obj_add_flag(lbl_alarm_icon,   LV_OBJ_FLAG_HIDDEN);
    }
}

void ui_update_weather(const WeatherData &wd) {
    if (!wd.valid) return;
    strncpy(g_wicon_code, wd.icon, sizeof(g_wicon_code) - 1);
    g_wicon_code[sizeof(g_wicon_code) - 1] = '\0';
    if (lbl_wicon) lv_obj_invalidate(lbl_wicon);
    if (lbl_wtemp) { char b[24]; snprintf(b,sizeof(b),"%.1f C",wd.temp_current); lv_label_set_text(lbl_wtemp,b); }
    if (lbl_wdesc) lv_label_set_text(lbl_wdesc, wd.description);
    if (lbl_wtomorrow) { char b[40]; snprintf(b,sizeof(b),"Zitra: %.0f / %.0f C",wd.temp_min,wd.temp_max); lv_label_set_text(lbl_wtomorrow,b); }
    if (lbl_whumidity) { char b[50]; snprintf(b,sizeof(b),"Vlhkost: %d%%  |  Vitr: %.1f m/s",wd.humidity,wd.wind_speed); lv_label_set_text(lbl_whumidity,b); }
}

void ui_clock_set_mode(bool digital) {
    g_clock_digital = digital;
    lv_obj_t *sec_ring = scr_clock ? (lv_obj_t*)lv_obj_get_user_data(scr_clock) : nullptr;

    if (digital) {
        // Hide analog elements
        if (clock_bezel) lv_obj_add_flag(clock_bezel, LV_OBJ_FLAG_HIDDEN);
        if (clock_face)  lv_obj_add_flag(clock_face,  LV_OBJ_FLAG_HIDDEN);
        if (sec_ring)    lv_obj_add_flag(sec_ring,    LV_OBJ_FLAG_HIDDEN);
        // Enlarge time label
        if (lbl_time) {
            lv_obj_set_style_text_font(lbl_time, &lv_font_montserrat_48, 0);
            lv_obj_align(lbl_time, LV_ALIGN_CENTER, 0, -10);
        }
        // Move date label
        if (lbl_date) {
            lv_obj_set_style_text_font(lbl_date, &lv_font_montserrat_14, 0);
            lv_obj_align(lbl_date, LV_ALIGN_CENTER, 0, 48);
        }
    } else {
        // Show analog elements
        if (clock_bezel) lv_obj_clear_flag(clock_bezel, LV_OBJ_FLAG_HIDDEN);
        if (clock_face)  lv_obj_clear_flag(clock_face,  LV_OBJ_FLAG_HIDDEN);
        if (sec_ring)    lv_obj_clear_flag(sec_ring,    LV_OBJ_FLAG_HIDDEN);
        // Restore time label
        if (lbl_time) {
            lv_obj_set_style_text_font(lbl_time, &lv_font_montserrat_24, 0);
            lv_obj_align(lbl_time, LV_ALIGN_CENTER, 0, 30);
        }
        // Restore date label
        if (lbl_date) {
            lv_obj_set_style_text_font(lbl_date, &lv_font_montserrat_12, 0);
            lv_obj_align(lbl_date, LV_ALIGN_CENTER, 0, 52);
        }
    }
}
