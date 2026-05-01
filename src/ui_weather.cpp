#include "ui_shared.h"
#include "ui_screens.h"
#include <math.h>

static lv_obj_t *s_weather_scroll = nullptr;
static lv_obj_t *s_forecast_lbl[FORECAST_DAYS] = {};

static void weather_icon_draw_cb(lv_event_t *e) {
    if (lv_event_get_code(e) != LV_EVENT_DRAW_POST_END) return;
    lv_draw_ctx_t *ctx = lv_event_get_draw_ctx(e);
    lv_obj_t *obj = lv_event_get_target(e);
    lv_area_t coords;
    lv_obj_get_coords(obj, &coords);
    int cx = (coords.x1 + coords.x2) / 2;
    int cy = (coords.y1 + coords.y2) / 2;

    auto fill_circ = [&](int x, int y, int r, lv_color_t col) {
        lv_draw_rect_dsc_t d; lv_draw_rect_dsc_init(&d);
        d.bg_color = col; d.bg_opa = LV_OPA_COVER;
        d.radius = LV_RADIUS_CIRCLE; d.border_width = 0;
        lv_area_t a = { (lv_coord_t)(x-r), (lv_coord_t)(y-r),
                        (lv_coord_t)(x+r), (lv_coord_t)(y+r) };
        lv_draw_rect(ctx, &d, &a);
    };
    auto fill_rect = [&](int x1, int y1, int x2, int y2, int rad, lv_color_t col) {
        lv_draw_rect_dsc_t d; lv_draw_rect_dsc_init(&d);
        d.bg_color = col; d.bg_opa = LV_OPA_COVER;
        d.radius = rad; d.border_width = 0;
        lv_area_t a = { (lv_coord_t)x1, (lv_coord_t)y1,
                        (lv_coord_t)x2, (lv_coord_t)y2 };
        lv_draw_rect(ctx, &d, &a);
    };
    auto draw_ln = [&](int x1, int y1, int x2, int y2, lv_color_t col, int w) {
        lv_draw_line_dsc_t d; lv_draw_line_dsc_init(&d);
        d.color = col; d.width = w; d.round_start = 1; d.round_end = 1;
        lv_point_t p1 = { (lv_coord_t)x1, (lv_coord_t)y1 };
        lv_point_t p2 = { (lv_coord_t)x2, (lv_coord_t)y2 };
        lv_draw_line(ctx, &d, &p1, &p2);
    };
    // Cloud: 3 bumps + bottom fill, centred at (x, y)
    auto draw_cloud = [&](int x, int y, lv_color_t col) {
        fill_circ(x-10, y,    9, col);
        fill_circ(x,    y-8,  9, col);
        fill_circ(x+10, y-4, 10, col);
        fill_rect(x-19, y-4, x+19, y+10, 5, col);
    };

    const char *code = g_wicon_code;

    if (strncmp(code, "01", 2) == 0) {
        // Clear sky – sun
        lv_color_t sc = lv_color_hex(0xF0A500);
        fill_circ(cx, cy, 12, sc);
        for (int i = 0; i < 8; i++) {
            float r = i * (float)M_PI / 4.0f;
            draw_ln(cx + (int)(sinf(r)*15), cy - (int)(cosf(r)*15),
                    cx + (int)(sinf(r)*22), cy - (int)(cosf(r)*22), sc, 2);
        }
    } else if (strncmp(code, "02", 2) == 0) {
        // Few clouds – small sun + cloud
        lv_color_t sc = lv_color_hex(0xF0A500);
        fill_circ(cx-8, cy-8, 9, sc);
        for (int i = 1; i < 5; i++) {
            float r = i * (float)M_PI / 4.0f;
            draw_ln(cx-8 + (int)(sinf(r)*11), cy-8 - (int)(cosf(r)*11),
                    cx-8 + (int)(sinf(r)*17), cy-8 - (int)(cosf(r)*17), sc, 2);
        }
        draw_cloud(cx+5, cy+6, lv_color_hex(0xCCCCCC));
    } else if (strncmp(code, "03", 2) == 0) {
        // Scattered clouds
        draw_cloud(cx, cy+2, lv_color_hex(0xBBBBBB));
    } else if (strncmp(code, "04", 2) == 0) {
        // Broken clouds – two offset clouds
        draw_cloud(cx+4, cy,   lv_color_hex(0x999999));
        draw_cloud(cx-6, cy+6, lv_color_hex(0xBBBBBB));
    } else if (strncmp(code, "09", 2) == 0) {
        // Shower rain
        draw_cloud(cx, cy-6, lv_color_hex(0x778899));
        lv_color_t rc = lv_color_hex(0x6CB4EE);
        for (int i = -1; i <= 1; i++)
            draw_ln(cx + i*10, cy+8, cx + i*10 - 5, cy+20, rc, 2);
    } else if (strncmp(code, "10", 2) == 0) {
        // Rain
        draw_cloud(cx, cy-6, lv_color_hex(0x8899AA));
        lv_color_t rc = lv_color_hex(0x6CB4EE);
        draw_ln(cx-8,  cy+7,  cx-13, cy+19, rc, 2);
        draw_ln(cx+1,  cy+8,  cx-4,  cy+20, rc, 2);
        draw_ln(cx+10, cy+7,  cx+5,  cy+19, rc, 2);
    } else if (strncmp(code, "11", 2) == 0) {
        // Thunderstorm
        draw_cloud(cx, cy-8, lv_color_hex(0x556677));
        lv_color_t bc = lv_color_hex(0xF5E040);
        draw_ln(cx+4,  cy+4,  cx-2,  cy+12, bc, 3);
        draw_ln(cx-2,  cy+12, cx+4,  cy+12, bc, 3);
        draw_ln(cx+4,  cy+12, cx-4,  cy+22, bc, 3);
    } else if (strncmp(code, "13", 2) == 0) {
        // Snow
        draw_cloud(cx, cy-8, lv_color_hex(0xCCDDEE));
        lv_color_t sc = lv_color_hex(0xDDEEFF);
        for (int i = 0; i < 4; i++) {
            float r = i * (float)M_PI / 4.0f;
            draw_ln(cx - (int)(sinf(r)*9), cy+16 - (int)(cosf(r)*9),
                    cx + (int)(sinf(r)*9), cy+16 + (int)(cosf(r)*9), sc, 2);
        }
    } else {
        // Mist / fog or unknown
        lv_color_t mc = lv_color_hex(0x889AAA);
        for (int i = -2; i <= 2; i++)
            fill_rect(cx-18, cy + i*8 - 1, cx+18, cy + i*8 + 1, 1, mc);
    }
}

void build_weather_screen() {
    scr_weather = lv_obj_create(nullptr);
    lv_obj_set_style_bg_color(scr_weather, lv_color_hex(0x0A1628), 0);
    lv_obj_set_style_bg_opa(scr_weather, LV_OPA_COVER, 0);
    lv_obj_clear_flag(scr_weather, LV_OBJ_FLAG_SCROLLABLE);

    // Scrollable inner container
    s_weather_scroll = lv_obj_create(scr_weather);
    lv_obj_set_size(s_weather_scroll, 240, 290);
    lv_obj_set_pos(s_weather_scroll, 0, 0);
    lv_obj_set_style_bg_opa(s_weather_scroll, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(s_weather_scroll, 0, 0);
    lv_obj_set_style_pad_all(s_weather_scroll, 0, 0);
    lv_obj_set_scroll_dir(s_weather_scroll, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(s_weather_scroll, LV_SCROLLBAR_MODE_OFF);

    lv_obj_t *title = lv_label_create(s_weather_scroll);
    lv_label_set_text(title, "Pocasi");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(title, C_ACCENT, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 14);

    // Custom-drawn weather icon (52×52)
    lbl_wicon = lv_obj_create(s_weather_scroll);
    lv_obj_set_size(lbl_wicon, 52, 52);
    lv_obj_set_style_bg_opa(lbl_wicon, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(lbl_wicon, 0, 0);
    lv_obj_clear_flag(lbl_wicon, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_align(lbl_wicon, LV_ALIGN_TOP_MID, -50, 38);
    lv_obj_add_event_cb(lbl_wicon, weather_icon_draw_cb, LV_EVENT_DRAW_POST_END, nullptr);

    lbl_wtemp = lv_label_create(s_weather_scroll);
    lv_obj_set_style_text_font(lbl_wtemp, &lv_font_montserrat_28, 0);
    lv_obj_set_style_text_color(lbl_wtemp, C_WHITE, 0);
    lv_obj_align(lbl_wtemp, LV_ALIGN_TOP_MID, 42, 44);
    lv_label_set_text(lbl_wtemp, "-- C");

    lbl_wdesc = lv_label_create(s_weather_scroll);
    lv_obj_set_style_text_font(lbl_wdesc, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(lbl_wdesc, C_DATE, 0);
    lv_obj_align(lbl_wdesc, LV_ALIGN_TOP_MID, 0, 98);
    lv_label_set_text(lbl_wdesc, "Nacitam...");

    lv_obj_t *div = lv_obj_create(s_weather_scroll);
    lv_obj_set_size(div, 150, 1);
    lv_obj_align(div, LV_ALIGN_TOP_MID, 0, 120);
    lv_obj_set_style_bg_color(div, lv_color_hex(0x1E3A5F), 0);
    lv_obj_set_style_border_width(div, 0, 0);

    lbl_wtomorrow = lv_label_create(s_weather_scroll);
    lv_obj_set_style_text_font(lbl_wtomorrow, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(lbl_wtomorrow, C_WHITE, 0);
    lv_obj_align(lbl_wtomorrow, LV_ALIGN_TOP_MID, 0, 126);
    lv_label_set_text(lbl_wtomorrow, "Zitra: -- / -- C");

    lbl_whumidity = lv_label_create(s_weather_scroll);
    lv_obj_set_style_text_font(lbl_whumidity, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(lbl_whumidity, C_DIM, 0);
    lv_obj_align(lbl_whumidity, LV_ALIGN_TOP_MID, 0, 148);
    lv_label_set_text(lbl_whumidity, "Vlhkost: --% | Vitr: -- m/s");

    // Forecast section (below y=168, in the scrollable extension)
    lv_obj_t *div2 = lv_obj_create(s_weather_scroll);
    lv_obj_set_size(div2, 150, 1);
    lv_obj_align(div2, LV_ALIGN_TOP_MID, 0, 168);
    lv_obj_set_style_bg_color(div2, C_RING, 0);
    lv_obj_set_style_border_width(div2, 0, 0);

    lv_obj_t *lbl_fc_title = lv_label_create(s_weather_scroll);
    lv_label_set_text(lbl_fc_title, "Predpoved:");
    lv_obj_set_style_text_font(lbl_fc_title, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(lbl_fc_title, C_ACCENT, 0);
    lv_obj_align(lbl_fc_title, LV_ALIGN_TOP_MID, 0, 172);

    for (int i = 0; i < FORECAST_DAYS; i++) {
        s_forecast_lbl[i] = lv_label_create(s_weather_scroll);
        lv_label_set_text(s_forecast_lbl[i], "...");
        lv_obj_set_style_text_font(s_forecast_lbl[i], &lv_font_montserrat_12, 0);
        lv_obj_set_style_text_color(s_forecast_lbl[i], C_DATE, 0);
        lv_obj_align(s_forecast_lbl[i], LV_ALIGN_TOP_MID, 0, 186 + i * 16);
    }
}

void ui_weather_scroll(int dy) {
    if (s_weather_scroll) lv_obj_scroll_by(s_weather_scroll, 0, dy, LV_ANIM_ON);
}

void weather_update_forecast_labels(const WeatherData &wd) {
    for (int i = 0; i < FORECAST_DAYS && i < wd.forecast_count; i++) {
        if (!s_forecast_lbl[i]) continue;
        char fb[32];
        snprintf(fb, sizeof(fb), "%s  %.0f / %.0f C",
                 wd.forecast[i].label,
                 wd.forecast[i].temp_min,
                 wd.forecast[i].temp_max);
        lv_label_set_text(s_forecast_lbl[i], fb);
    }
}
