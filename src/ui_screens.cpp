#include "ui_screens.h"
#include "ui_shared.h"
#include "config.h"

// ── Shared globals ──────────────────────────────────────────
AlarmManager *g_alarm   = nullptr;
WeatherData  *g_weather = nullptr;

lv_obj_t *scr_clock    = nullptr;
lv_obj_t *scr_weather  = nullptr;
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

lv_obj_t *roller_h = nullptr;
lv_obj_t *roller_m = nullptr;
lv_obj_t *sw_alarm = nullptr;

// ── Navigation bar ──────────────────────────────────────────
static void nav_event_cb(lv_event_t *e) {
    ui_show_screen((int)(intptr_t)lv_event_get_user_data(e));
}

void add_nav_bar(lv_obj_t *scr, int active_idx) {
    // Three independent floating buttons so each center lands inside the
    // visible circle of the 240×240 round display.
    // Centers at (44,209), (120,209), (196,209) — all at dist < 120 from (120,120).
    static const char *icons[]  = {LV_SYMBOL_HOME, LV_SYMBOL_WIFI, LV_SYMBOL_BELL};
    static const char *labels[] = {"Cas", "Pocasi", "Budik"};
    static const int   xoffs[]  = {-76, 0, 76};

    for (int i = 0; i < 3; i++) {
        lv_obj_t *btn = lv_btn_create(scr);
        lv_obj_set_size(btn, 64, 38);
        lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, xoffs[i], -12);
        lv_obj_set_style_bg_color(btn, (i==active_idx)
            ? lv_color_hex(0x1E3A5F) : lv_color_hex(0x0A1520), 0);
        lv_obj_set_style_bg_opa(btn, 220, 0);
        lv_obj_set_style_border_width(btn, 0, 0);
        lv_obj_set_style_radius(btn, 8, 0);
        lv_obj_set_style_pad_all(btn, 2, 0);
        lv_obj_add_event_cb(btn, nav_event_cb, LV_EVENT_CLICKED, (void*)(intptr_t)i);

        lv_obj_t *ic = lv_label_create(btn);
        lv_label_set_text(ic, icons[i]);
        lv_obj_set_style_text_color(ic, (i==active_idx) ? C_ACCENT : C_DIM, 0);
        lv_obj_set_style_text_font(ic, &lv_font_montserrat_16, 0);
        lv_obj_align(ic, LV_ALIGN_TOP_MID, 0, 1);

        lv_obj_t *lbl = lv_label_create(btn);
        lv_label_set_text(lbl, labels[i]);
        lv_obj_set_style_text_color(lbl, (i==active_idx) ? C_WHITE : C_DIM, 0);
        lv_obj_set_style_text_font(lbl, &lv_font_montserrat_12, 0);
        lv_obj_align(lbl, LV_ALIGN_BOTTOM_MID, 0, -1);
    }
}

// ── Alarm ring overlay ───────────────────────────────────────
static AlarmManager *g_ring_alarm = nullptr;
static void ring_snooze_cb(lv_event_t *) { if(g_ring_alarm) g_ring_alarm->snooze(); ui_hide_alarm_ring(); }
static void ring_stop_cb(lv_event_t *)   { if(g_ring_alarm) g_ring_alarm->stop();   ui_hide_alarm_ring(); }

void ui_show_alarm_ring(AlarmManager *alarm) {
    g_ring_alarm = alarm;
    if (scr_ring) lv_obj_del(scr_ring);
    scr_ring = lv_obj_create(nullptr);
    lv_obj_set_style_bg_color(scr_ring, lv_color_hex(0x1A0005), 0);
    lv_obj_set_style_bg_opa(scr_ring, LV_OPA_COVER, 0);
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
    lv_label_set_text(stl, LV_SYMBOL_STOP " Stop");
    lv_obj_set_style_text_color(stl, C_WHITE, 0); lv_obj_center(stl);

    lv_scr_load(scr_ring);
}

void ui_hide_alarm_ring() {
    lv_scr_load(scr_clock);
    if (scr_ring) { lv_obj_del(scr_ring); scr_ring = nullptr; }
}

// ── Public API ───────────────────────────────────────────────
void ui_init(AlarmManager *alarm, WeatherData *weather) {
    g_alarm = alarm; g_weather = weather;
    build_clock_screen();
    build_weather_screen();
    build_settings_screen();
    if (alarm) {
        AlarmConfig cfg = alarm->get();
        lv_roller_set_selected(roller_h, cfg.hour,   LV_ANIM_OFF);
        lv_roller_set_selected(roller_m, cfg.minute, LV_ANIM_OFF);
        if (cfg.enabled) lv_obj_add_state(sw_alarm, LV_STATE_CHECKED);
    }
    lv_scr_load(scr_clock);
}

void ui_show_screen(int idx) {
    lv_obj_t *t = nullptr;
    switch(idx) {
        case SCREEN_CLOCK:    t = scr_clock;    break;
        case SCREEN_WEATHER:  t = scr_weather;  break;
        case SCREEN_SETTINGS: t = scr_settings; break;
    }
    if (t) lv_scr_load_anim(t, LV_SCR_LOAD_ANIM_FADE_ON, 200, 0, false);
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
    const char *ic = wd.icon, *sym = LV_SYMBOL_WIFI;
    if      (strncmp(ic,"01",2)==0) sym = LV_SYMBOL_IMAGE;
    else if (strncmp(ic,"09",2)==0||strncmp(ic,"10",2)==0) sym = LV_SYMBOL_DOWNLOAD;
    else if (strncmp(ic,"11",2)==0) sym = LV_SYMBOL_WARNING;
    else if (strncmp(ic,"13",2)==0) sym = LV_SYMBOL_REFRESH;
    if (lbl_wicon) lv_label_set_text(lbl_wicon, sym);
    if (lbl_wtemp) { char b[24]; snprintf(b,sizeof(b),"%.1f C",wd.temp_current); lv_label_set_text(lbl_wtemp,b); }
    if (lbl_wdesc) lv_label_set_text(lbl_wdesc, wd.description);
    if (lbl_wtomorrow) { char b[40]; snprintf(b,sizeof(b),"Zitra: %.0f / %.0f C",wd.temp_min,wd.temp_max); lv_label_set_text(lbl_wtomorrow,b); }
    if (lbl_whumidity) { char b[50]; snprintf(b,sizeof(b),"Vlhkost: %d%%  |  Vitr: %.1f m/s",wd.humidity,wd.wind_speed); lv_label_set_text(lbl_whumidity,b); }
}
