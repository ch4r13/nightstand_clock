#include "ui_shared.h"
#include "ui_screens.h"
#include "config.h"

static uint8_t   s_set_h   = 0;
static uint8_t   s_set_m   = 0;
static lv_obj_t *lbl_set_h = nullptr;
static lv_obj_t *lbl_set_m = nullptr;
static lv_obj_t *s_save_btn = nullptr;

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

    // Title ─────────────────────────────────────────────────────
    lv_obj_t *title = lv_label_create(scr_settings);
    lv_label_set_text(title, LV_SYMBOL_BELL "  Budik");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(title, C_ACCENT, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);

    // Column labels (tiny, above the UP buttons) ────────────────
    // Screen center = (120, 120); H-column at x_ofs=-52, M-column at x_ofs=+52
    lv_obj_t *lh = lv_label_create(scr_settings);
    lv_label_set_text(lh, "Hod");
    lv_obj_set_style_text_font(lh, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(lh, C_DATE, 0);
    lv_obj_align(lh, LV_ALIGN_CENTER, -52, -76);

    lv_obj_t *lm = lv_label_create(scr_settings);
    lv_label_set_text(lm, "Min");
    lv_obj_set_style_text_font(lm, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(lm, C_DATE, 0);
    lv_obj_align(lm, LV_ALIGN_CENTER, 52, -76);

    // UP buttons ────────────────────────────────────────────────
    lv_obj_t *bhu = make_btn(scr_settings, LV_SYMBOL_UP, h_inc_cb);
    lv_obj_align(bhu, LV_ALIGN_CENTER, -52, -54);

    lv_obj_t *bmu = make_btn(scr_settings, LV_SYMBOL_UP, m_inc_cb);
    lv_obj_align(bmu, LV_ALIGN_CENTER, 52, -54);

    // Time value labels (font_36, centered in each column) ──────
    lbl_set_h = lv_label_create(scr_settings);
    { char b[4]; snprintf(b,4,"%02d",s_set_h); lv_label_set_text(lbl_set_h,b); }
    lv_obj_set_style_text_font(lbl_set_h, &lv_font_montserrat_36, 0);
    lv_obj_set_style_text_color(lbl_set_h, C_WHITE, 0);
    lv_obj_align(lbl_set_h, LV_ALIGN_CENTER, -52, -8);

    lv_obj_t *colon = lv_label_create(scr_settings);
    lv_label_set_text(colon, ":");
    lv_obj_set_style_text_font(colon, &lv_font_montserrat_36, 0);
    lv_obj_set_style_text_color(colon, C_DIM, 0);
    lv_obj_align(colon, LV_ALIGN_CENTER, 0, -8);

    lbl_set_m = lv_label_create(scr_settings);
    { char b[4]; snprintf(b,4,"%02d",s_set_m); lv_label_set_text(lbl_set_m,b); }
    lv_obj_set_style_text_font(lbl_set_m, &lv_font_montserrat_36, 0);
    lv_obj_set_style_text_color(lbl_set_m, C_WHITE, 0);
    lv_obj_align(lbl_set_m, LV_ALIGN_CENTER, 52, -8);

    // DOWN buttons ──────────────────────────────────────────────
    lv_obj_t *bhd = make_btn(scr_settings, LV_SYMBOL_DOWN, h_dec_cb);
    lv_obj_align(bhd, LV_ALIGN_CENTER, -52, 40);

    lv_obj_t *bmd = make_btn(scr_settings, LV_SYMBOL_DOWN, m_dec_cb);
    lv_obj_align(bmd, LV_ALIGN_CENTER, 52, 40);

    // Enable switch ─────────────────────────────────────────────
    lv_obj_t *lbl_sw = lv_label_create(scr_settings);
    lv_label_set_text(lbl_sw, "Budik:");
    lv_obj_set_style_text_font(lbl_sw, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(lbl_sw, C_WHITE, 0);
    lv_obj_align(lbl_sw, LV_ALIGN_CENTER, -36, 74);

    sw_alarm = lv_switch_create(scr_settings);
    lv_obj_align(sw_alarm, LV_ALIGN_CENTER, 40, 74);
    lv_obj_set_style_bg_color(sw_alarm, lv_color_hex(0x27AE60),
                              LV_PART_INDICATOR | LV_STATE_CHECKED);
    if (g_alarm && g_alarm->get().enabled)
        lv_obj_add_state(sw_alarm, LV_STATE_CHECKED);

    // Save button ───────────────────────────────────────────────
    s_save_btn = lv_btn_create(scr_settings);
    lv_obj_set_size(s_save_btn, 120, 30);
    lv_obj_align(s_save_btn, LV_ALIGN_CENTER, 0, 100);
    lv_obj_set_style_bg_color(s_save_btn, lv_color_hex(0x1E3A5F), 0);
    lv_obj_set_style_radius(s_save_btn, 15, 0);
    lv_obj_add_event_cb(s_save_btn, save_cb, LV_EVENT_CLICKED, nullptr);

    lv_obj_t *btn_lbl = lv_label_create(s_save_btn);
    lv_label_set_text(btn_lbl, LV_SYMBOL_SAVE "  Ulozit");
    lv_obj_set_style_text_font(btn_lbl, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(btn_lbl, C_WHITE, 0);
    lv_obj_center(btn_lbl);
}
