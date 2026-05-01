#include "ui_shared.h"
#include "ui_screens.h"
#include "config.h"

static uint8_t   s_set_h   = 0;
static uint8_t   s_set_m   = 0;
static lv_obj_t *lbl_set_h = nullptr;
static lv_obj_t *lbl_set_m = nullptr;
static lv_obj_t *s_save_btn = nullptr;
static lv_obj_t *s_settings_scroll = nullptr;
static lv_obj_t *lbl_settings_ip = nullptr;

static void update_vals() {
    if (lbl_set_h) { char b[4]; snprintf(b,4,"%02d",s_set_h); lv_label_set_text(lbl_set_h,b); }
    if (lbl_set_m) { char b[4]; snprintf(b,4,"%02d",s_set_m); lv_label_set_text(lbl_set_m,b); }
    // Reset save button after value change
    if (s_save_btn) {
        lv_obj_set_style_bg_color(s_save_btn, lv_color_hex(0x1E3A5F), 0);
        lv_obj_t *l = lv_obj_get_child(s_save_btn, 0);
        if (l) lv_label_set_text(l, LV_SYMBOL_SAVE "  Ulozit");
    }
}

static void h_inc_cb(lv_event_t *) { s_set_h = (s_set_h + 1)  % 24; update_vals(); }
static void h_dec_cb(lv_event_t *) { s_set_h = (s_set_h + 23) % 24; update_vals(); }
static void m_inc_cb(lv_event_t *) { s_set_m = (s_set_m + 1)  % 60; update_vals(); }
static void m_dec_cb(lv_event_t *) { s_set_m = (s_set_m + 59) % 60; update_vals(); }

static void save_cb(lv_event_t *) {
    if (!g_alarm) return;
    g_alarm->set(s_set_h, s_set_m, lv_obj_has_state(sw_alarm, LV_STATE_CHECKED));
    if (s_save_btn) {
        lv_obj_set_style_bg_color(s_save_btn, lv_color_hex(0x27AE60), 0);
        lv_obj_t *l = lv_obj_get_child(s_save_btn, 0);
        if (l) lv_label_set_text(l, LV_SYMBOL_OK " Ulozeno!");
    }
}

// Creates a square tap-button with a symbol label.
// Long-press-repeat fires the same cb for fast scrolling.
static lv_obj_t *make_btn(lv_obj_t *parent, const char *sym, lv_event_cb_t cb) {
    lv_obj_t *btn = lv_btn_create(parent);
    lv_obj_set_size(btn, 56, 34);
    lv_obj_set_style_bg_color(btn, lv_color_hex(0x1E3A5F), 0);
    lv_obj_set_style_radius(btn, 10, 0);
    lv_obj_set_style_pad_all(btn, 0, 0);
    lv_obj_add_event_cb(btn, cb, LV_EVENT_CLICKED,             nullptr);
    lv_obj_add_event_cb(btn, cb, LV_EVENT_LONG_PRESSED_REPEAT, nullptr);
    lv_obj_t *l = lv_label_create(btn);
    lv_label_set_text(l, sym);
    lv_obj_set_style_text_font(l, &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_color(l, C_WHITE, 0);
    lv_obj_center(l);
    return btn;
}

void build_settings_screen() {
    roller_h = nullptr;   // not used in this layout
    roller_m = nullptr;

    // Load saved alarm values before building widgets
    if (g_alarm) { AlarmConfig c = g_alarm->get(); s_set_h = c.hour; s_set_m = c.minute; }

    scr_settings = lv_obj_create(nullptr);
    lv_obj_set_style_bg_color(scr_settings, lv_color_hex(0x0A1628), 0);
    lv_obj_set_style_bg_opa(scr_settings, LV_OPA_COVER, 0);
    lv_obj_clear_flag(scr_settings, LV_OBJ_FLAG_SCROLLABLE);

    // Scrollable inner container (240x290, extends below visible area)
    s_settings_scroll = lv_obj_create(scr_settings);
    lv_obj_set_size(s_settings_scroll, 240, 290);
    lv_obj_set_pos(s_settings_scroll, 0, 0);
    lv_obj_set_style_bg_opa(s_settings_scroll, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(s_settings_scroll, 0, 0);
    lv_obj_set_style_pad_all(s_settings_scroll, 0, 0);
    lv_obj_set_scroll_dir(s_settings_scroll, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(s_settings_scroll, LV_SCROLLBAR_MODE_OFF);

    // Title (y=10)
    lv_obj_t *title = lv_label_create(s_settings_scroll);
    lv_label_set_text(title, LV_SYMBOL_BELL "  Budik");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(title, C_ACCENT, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);

    // Column labels (y=38)
    lv_obj_t *lh = lv_label_create(s_settings_scroll);
    lv_label_set_text(lh, "Hod");
    lv_obj_set_style_text_font(lh, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(lh, C_DATE, 0);
    lv_obj_align(lh, LV_ALIGN_TOP_MID, -52, 38);

    lv_obj_t *lm = lv_label_create(s_settings_scroll);
    lv_label_set_text(lm, "Min");
    lv_obj_set_style_text_font(lm, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(lm, C_DATE, 0);
    lv_obj_align(lm, LV_ALIGN_TOP_MID, 52, 38);

    // UP buttons (y=52)
    lv_obj_t *bhu = make_btn(s_settings_scroll, LV_SYMBOL_UP, h_inc_cb);
    lv_obj_align(bhu, LV_ALIGN_TOP_MID, -52, 52);

    lv_obj_t *bmu = make_btn(s_settings_scroll, LV_SYMBOL_UP, m_inc_cb);
    lv_obj_align(bmu, LV_ALIGN_TOP_MID, 52, 52);

    // Time value labels (font_36, y=90)
    lbl_set_h = lv_label_create(s_settings_scroll);
    { char b[4]; snprintf(b,4,"%02d",s_set_h); lv_label_set_text(lbl_set_h,b); }
    lv_obj_set_style_text_font(lbl_set_h, &lv_font_montserrat_36, 0);
    lv_obj_set_style_text_color(lbl_set_h, C_WHITE, 0);
    lv_obj_align(lbl_set_h, LV_ALIGN_TOP_MID, -52, 90);

    lv_obj_t *colon = lv_label_create(s_settings_scroll);
    lv_label_set_text(colon, ":");
    lv_obj_set_style_text_font(colon, &lv_font_montserrat_36, 0);
    lv_obj_set_style_text_color(colon, C_DIM, 0);
    lv_obj_align(colon, LV_ALIGN_TOP_MID, 0, 90);

    lbl_set_m = lv_label_create(s_settings_scroll);
    { char b[4]; snprintf(b,4,"%02d",s_set_m); lv_label_set_text(lbl_set_m,b); }
    lv_obj_set_style_text_font(lbl_set_m, &lv_font_montserrat_36, 0);
    lv_obj_set_style_text_color(lbl_set_m, C_WHITE, 0);
    lv_obj_align(lbl_set_m, LV_ALIGN_TOP_MID, 52, 90);

    // DOWN buttons (y=138)
    lv_obj_t *bhd = make_btn(s_settings_scroll, LV_SYMBOL_DOWN, h_dec_cb);
    lv_obj_align(bhd, LV_ALIGN_TOP_MID, -52, 138);

    lv_obj_t *bmd = make_btn(s_settings_scroll, LV_SYMBOL_DOWN, m_dec_cb);
    lv_obj_align(bmd, LV_ALIGN_TOP_MID, 52, 138);

    // Enable switch (y=176 label, y=174 switch)
    lv_obj_t *lbl_sw = lv_label_create(s_settings_scroll);
    lv_label_set_text(lbl_sw, "Budik:");
    lv_obj_set_style_text_font(lbl_sw, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(lbl_sw, C_WHITE, 0);
    lv_obj_align(lbl_sw, LV_ALIGN_TOP_MID, -30, 176);

    sw_alarm = lv_switch_create(s_settings_scroll);
    lv_obj_align(sw_alarm, LV_ALIGN_TOP_MID, 44, 174);
    lv_obj_set_style_bg_color(sw_alarm, lv_color_hex(0x27AE60),
                              LV_PART_INDICATOR | LV_STATE_CHECKED);
    if (g_alarm && g_alarm->get().enabled)
        lv_obj_add_state(sw_alarm, LV_STATE_CHECKED);

    // Save button (y=202)
    s_save_btn = lv_btn_create(s_settings_scroll);
    lv_obj_set_size(s_save_btn, 120, 30);
    lv_obj_align(s_save_btn, LV_ALIGN_TOP_MID, 0, 202);
    lv_obj_set_style_bg_color(s_save_btn, lv_color_hex(0x1E3A5F), 0);
    lv_obj_set_style_radius(s_save_btn, 15, 0);
    lv_obj_add_event_cb(s_save_btn, save_cb, LV_EVENT_CLICKED, nullptr);

    lv_obj_t *btn_lbl = lv_label_create(s_save_btn);
    lv_label_set_text(btn_lbl, LV_SYMBOL_SAVE "  Ulozit");
    lv_obj_set_style_text_font(btn_lbl, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(btn_lbl, C_WHITE, 0);
    lv_obj_center(btn_lbl);

    // --- Extra section below 240px visible area ---

    // Thin divider (y=240)
    lv_obj_t *div = lv_obj_create(s_settings_scroll);
    lv_obj_set_size(div, 150, 1);
    lv_obj_align(div, LV_ALIGN_TOP_MID, 0, 240);
    lv_obj_set_style_bg_color(div, C_DIM, 0);
    lv_obj_set_style_border_width(div, 0, 0);

    // IP label (y=250)
    lbl_settings_ip = lv_label_create(s_settings_scroll);
    lv_label_set_text(lbl_settings_ip, "IP: --");
    lv_obj_set_style_text_font(lbl_settings_ip, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(lbl_settings_ip, C_DATE, 0);
    lv_obj_align(lbl_settings_ip, LV_ALIGN_TOP_MID, 0, 250);

    // Web note (y=266)
    lv_obj_t *lbl_web = lv_label_create(s_settings_scroll);
    lv_label_set_text(lbl_web, "Web: :80");
    lv_obj_set_style_text_font(lbl_web, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(lbl_web, C_DIM, 0);
    lv_obj_align(lbl_web, LV_ALIGN_TOP_MID, 0, 266);
}

void ui_settings_set_ip(const char *ip) {
    if (!lbl_settings_ip) return;
    char buf[32]; snprintf(buf, sizeof(buf), "IP: %s", ip);
    lv_label_set_text(lbl_settings_ip, buf);
}

void ui_settings_scroll(int dy) {
    if (s_settings_scroll) lv_obj_scroll_by(s_settings_scroll, 0, dy, LV_ANIM_ON);
}
