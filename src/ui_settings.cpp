#include "ui_shared.h"
#include "ui_screens.h"
#include "config.h"

static void settings_save_cb(lv_event_t *e) {
    if (!g_alarm) return;
    uint8_t h  = lv_roller_get_selected(roller_h);
    uint8_t m  = lv_roller_get_selected(roller_m);
    bool    en = lv_obj_has_state(sw_alarm, LV_STATE_CHECKED);
    g_alarm->set(h, m, en);
    lv_obj_t *btn = lv_event_get_target(e);
    lv_obj_set_style_bg_color(btn, lv_color_hex(0x27AE60), 0);
    lv_obj_t *lbl = lv_obj_get_child(btn, 0);
    if (lbl) lv_label_set_text(lbl, "Ulozeno!");
}

void build_settings_screen() {
    scr_settings = lv_obj_create(nullptr);
    lv_obj_set_style_bg_color(scr_settings, lv_color_hex(0x0A1628), 0);
    lv_obj_set_style_bg_opa(scr_settings, LV_OPA_COVER, 0);
    lv_obj_clear_flag(scr_settings, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *title = lv_label_create(scr_settings);
    lv_label_set_text(title, LV_SYMBOL_BELL "  Budik");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(title, C_ACCENT, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 14);

    static const char roller_hours[] =
        "00\n01\n02\n03\n04\n05\n06\n07\n08\n09\n10\n11\n"
        "12\n13\n14\n15\n16\n17\n18\n19\n20\n21\n22\n23";

    lv_obj_t *lbl_h = lv_label_create(scr_settings);
    lv_label_set_text(lbl_h, "Hod");
    lv_obj_set_style_text_color(lbl_h, C_DATE, 0);
    lv_obj_set_style_text_font(lbl_h, &lv_font_montserrat_14, 0);
    lv_obj_align(lbl_h, LV_ALIGN_TOP_LEFT, 38, 42);

    roller_h = lv_roller_create(scr_settings);
    lv_roller_set_options(roller_h, roller_hours, LV_ROLLER_MODE_INFINITE);
    lv_roller_set_visible_row_count(roller_h, 2);
    lv_obj_set_width(roller_h, 62);
    lv_obj_align(roller_h, LV_ALIGN_TOP_LEFT, 30, 58);
    lv_obj_set_style_text_font(roller_h, &lv_font_montserrat_24, 0);
    lv_obj_set_style_bg_color(roller_h, lv_color_hex(0x0D1B2A), 0);
    lv_obj_set_style_text_color(roller_h, C_WHITE, 0);
    lv_obj_set_style_text_color(roller_h, C_ACCENT, LV_PART_SELECTED);

    lv_obj_t *colon = lv_label_create(scr_settings);
    lv_label_set_text(colon, ":");
    lv_obj_set_style_text_font(colon, &lv_font_montserrat_28, 0);
    lv_obj_set_style_text_color(colon, C_WHITE, 0);
    lv_obj_align(colon, LV_ALIGN_TOP_MID, 0, 72);

    static const char roller_mins[] =
        "00\n01\n02\n03\n04\n05\n06\n07\n08\n09\n10\n11\n12\n13\n14\n"
        "15\n16\n17\n18\n19\n20\n21\n22\n23\n24\n25\n26\n27\n28\n29\n"
        "30\n31\n32\n33\n34\n35\n36\n37\n38\n39\n40\n41\n42\n43\n44\n"
        "45\n46\n47\n48\n49\n50\n51\n52\n53\n54\n55\n56\n57\n58\n59";

    lv_obj_t *lbl_m = lv_label_create(scr_settings);
    lv_label_set_text(lbl_m, "Min");
    lv_obj_set_style_text_color(lbl_m, C_DATE, 0);
    lv_obj_set_style_text_font(lbl_m, &lv_font_montserrat_14, 0);
    lv_obj_align(lbl_m, LV_ALIGN_TOP_RIGHT, -38, 42);

    roller_m = lv_roller_create(scr_settings);
    lv_roller_set_options(roller_m, roller_mins, LV_ROLLER_MODE_INFINITE);
    lv_roller_set_visible_row_count(roller_m, 2);
    lv_obj_set_width(roller_m, 62);
    lv_obj_align(roller_m, LV_ALIGN_TOP_RIGHT, -30, 58);
    lv_obj_set_style_text_font(roller_m, &lv_font_montserrat_24, 0);
    lv_obj_set_style_bg_color(roller_m, lv_color_hex(0x0D1B2A), 0);
    lv_obj_set_style_text_color(roller_m, C_WHITE, 0);
    lv_obj_set_style_text_color(roller_m, C_ACCENT, LV_PART_SELECTED);

    lv_obj_t *lbl_sw = lv_label_create(scr_settings);
    lv_label_set_text(lbl_sw, "Budik zapnut:");
    lv_obj_set_style_text_font(lbl_sw, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(lbl_sw, C_WHITE, 0);
    lv_obj_align(lbl_sw, LV_ALIGN_TOP_LEFT, 26, 130);

    sw_alarm = lv_switch_create(scr_settings);
    lv_obj_align(sw_alarm, LV_ALIGN_TOP_RIGHT, -26, 126);
    lv_obj_set_style_bg_color(sw_alarm, lv_color_hex(0x27AE60),
                              LV_PART_INDICATOR | LV_STATE_CHECKED);

    lv_obj_t *btn_save = lv_btn_create(scr_settings);
    lv_obj_set_size(btn_save, 140, 34);
    lv_obj_align(btn_save, LV_ALIGN_TOP_MID, 0, 156);
    lv_obj_set_style_bg_color(btn_save, lv_color_hex(0x1E3A5F), 0);
    lv_obj_set_style_radius(btn_save, 17, 0);
    lv_obj_add_event_cb(btn_save, settings_save_cb, LV_EVENT_CLICKED, nullptr);

    lv_obj_t *btn_lbl = lv_label_create(btn_save);
    lv_label_set_text(btn_lbl, LV_SYMBOL_SAVE "  Ulozit");
    lv_obj_set_style_text_color(btn_lbl, C_WHITE, 0);
    lv_obj_center(btn_lbl);

}
