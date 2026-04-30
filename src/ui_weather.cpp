#include "ui_shared.h"
#include "ui_screens.h"

void build_weather_screen() {
    scr_weather = lv_obj_create(nullptr);
    lv_obj_set_style_bg_color(scr_weather, lv_color_hex(0x0A1628), 0);
    lv_obj_set_style_bg_opa(scr_weather, LV_OPA_COVER, 0);
    lv_obj_clear_flag(scr_weather, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *title = lv_label_create(scr_weather);
    lv_label_set_text(title, "Pocasi");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(title, C_ACCENT, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 14);

    // Icon and temperature side-by-side to save vertical space
    lbl_wicon = lv_label_create(scr_weather);
    lv_label_set_text(lbl_wicon, LV_SYMBOL_WIFI);
    lv_obj_set_style_text_font(lbl_wicon, &lv_font_montserrat_36, 0);
    lv_obj_set_style_text_color(lbl_wicon, C_ACCENT, 0);
    lv_obj_align(lbl_wicon, LV_ALIGN_TOP_MID, -50, 38);

    lbl_wtemp = lv_label_create(scr_weather);
    lv_obj_set_style_text_font(lbl_wtemp, &lv_font_montserrat_28, 0);
    lv_obj_set_style_text_color(lbl_wtemp, C_WHITE, 0);
    lv_obj_align(lbl_wtemp, LV_ALIGN_TOP_MID, 42, 44);
    lv_label_set_text(lbl_wtemp, "-- C");

    lbl_wdesc = lv_label_create(scr_weather);
    lv_obj_set_style_text_font(lbl_wdesc, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(lbl_wdesc, C_DATE, 0);
    lv_obj_align(lbl_wdesc, LV_ALIGN_TOP_MID, 0, 88);
    lv_label_set_text(lbl_wdesc, "Nacitam...");

    lv_obj_t *div = lv_obj_create(scr_weather);
    lv_obj_set_size(div, 150, 1);
    lv_obj_align(div, LV_ALIGN_TOP_MID, 0, 110);
    lv_obj_set_style_bg_color(div, lv_color_hex(0x1E3A5F), 0);
    lv_obj_set_style_border_width(div, 0, 0);

    lbl_wtomorrow = lv_label_create(scr_weather);
    lv_obj_set_style_text_font(lbl_wtomorrow, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(lbl_wtomorrow, C_WHITE, 0);
    lv_obj_align(lbl_wtomorrow, LV_ALIGN_TOP_MID, 0, 116);
    lv_label_set_text(lbl_wtomorrow, "Zitra: -- / -- C");

    lbl_whumidity = lv_label_create(scr_weather);
    lv_obj_set_style_text_font(lbl_whumidity, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(lbl_whumidity, C_DIM, 0);
    lv_obj_align(lbl_whumidity, LV_ALIGN_TOP_MID, 0, 136);
    lv_label_set_text(lbl_whumidity, "Vlhkost: --% | Vitr: -- m/s");

    add_nav_bar(scr_weather, SCREEN_WEATHER);
}
