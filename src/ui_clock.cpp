#include "ui_shared.h"
#include "ui_screens.h"
#include "config.h"
#include <math.h>

static void draw_hand(lv_draw_ctx_t *ctx, int cx, int cy,
                      float angle_deg, int r_inner, int r_outer,
                      lv_color_t col, uint16_t width)
{
    float rad = angle_deg * (float)M_PI / 180.0f;
    lv_point_t p1 = {(lv_coord_t)(cx + sinf(rad) * r_inner),
                     (lv_coord_t)(cy - cosf(rad) * r_inner)};
    lv_point_t p2 = {(lv_coord_t)(cx + sinf(rad) * r_outer),
                     (lv_coord_t)(cy - cosf(rad) * r_outer)};
    lv_draw_line_dsc_t dsc;
    lv_draw_line_dsc_init(&dsc);
    dsc.color = col; dsc.width = width;
    dsc.round_start = 1; dsc.round_end = 1;
    lv_draw_line(ctx, &dsc, &p1, &p2);
}

static void clock_face_draw_cb(lv_event_t *e) {
    if (lv_event_get_code(e) != LV_EVENT_DRAW_POST_END) return;
    lv_draw_ctx_t *ctx = lv_event_get_draw_ctx(e);
    const int cx = 120, cy = 120, R = 108;

    for (int i = 0; i < 60; i++) {
        float angle = i * 6.0f;
        bool  major = (i % 5 == 0);
        int   r_in  = major ? R - 14 : R - 7;
        float rad   = angle * (float)M_PI / 180.0f;
        lv_point_t p1 = {(lv_coord_t)(cx + sinf(rad) * (R - 2)),
                         (lv_coord_t)(cy - cosf(rad) * (R - 2))};
        lv_point_t p2 = {(lv_coord_t)(cx + sinf(rad) * r_in),
                         (lv_coord_t)(cy - cosf(rad) * r_in)};
        lv_draw_line_dsc_t dsc;
        lv_draw_line_dsc_init(&dsc);
        dsc.color = major ? C_TICK_MAJ : C_TICK_MIN;
        dsc.width = major ? 3 : 1;
        lv_draw_line(ctx, &dsc, &p1, &p2);
    }

    static const char  *nums[] = {"12","3","6","9"};
    static const float  nAng[] = {0, 90, 180, 270};
    lv_draw_label_dsc_t ldsc;
    lv_draw_label_dsc_init(&ldsc);
    ldsc.color = C_WHITE;
    ldsc.font  = &lv_font_montserrat_16;
    ldsc.align = LV_TEXT_ALIGN_CENTER;
    for (int i = 0; i < 4; i++) {
        float a = nAng[i] * (float)M_PI / 180.0f;
        int nx = cx + (int)(sinf(a) * (R - 26));
        int ny = cy - (int)(cosf(a) * (R - 26));
        lv_area_t area = {(lv_coord_t)(nx-14),(lv_coord_t)(ny-10),
                          (lv_coord_t)(nx+14),(lv_coord_t)(ny+10)};
        lv_draw_label(ctx, &ldsc, &area, nums[i], nullptr);
    }

    draw_hand(ctx, cx, cy, (c_h%12)*30.0f + c_m*0.5f, -12, R-38, C_HAND_H, 6);
    draw_hand(ctx, cx, cy, c_m*6.0f + c_s*0.1f,        -14, R-20, C_HAND_M, 4);
    draw_hand(ctx, cx, cy, c_s*6.0f,                    -20, R-16, C_HAND_S, 2);

    lv_draw_rect_dsc_t cap;
    lv_draw_rect_dsc_init(&cap);
    cap.bg_color = C_CENTER; cap.radius = LV_RADIUS_CIRCLE; cap.bg_opa = LV_OPA_COVER;
    lv_area_t ca = {(lv_coord_t)(cx-6),(lv_coord_t)(cy-6),
                    (lv_coord_t)(cx+6),(lv_coord_t)(cy+6)};
    lv_draw_rect(ctx, &cap, &ca);
}

void build_clock_screen() {
    scr_clock = lv_obj_create(nullptr);
    lv_obj_set_style_bg_color(scr_clock, C_BG, 0);
    lv_obj_set_style_bg_opa(scr_clock, LV_OPA_COVER, 0);
    lv_obj_clear_flag(scr_clock, LV_OBJ_FLAG_SCROLLABLE);

    clock_bezel = lv_arc_create(scr_clock);
    lv_obj_set_size(clock_bezel, 226, 226);
    lv_obj_center(clock_bezel);
    lv_arc_set_bg_angles(clock_bezel, 0, 360);
    lv_obj_set_style_arc_color(clock_bezel, lv_color_hex(0x1E3A5F), LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(clock_bezel, 4, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(clock_bezel, lv_color_hex(0x1E3A5F), LV_PART_MAIN);
    lv_obj_set_style_arc_width(clock_bezel, 4, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(clock_bezel, LV_OPA_TRANSP, 0);
    lv_obj_remove_style(clock_bezel, nullptr, LV_PART_KNOB);
    lv_obj_clear_flag(clock_bezel, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_t *sec_ring = lv_arc_create(scr_clock);
    lv_obj_set_size(sec_ring, 234, 234);
    lv_obj_center(sec_ring);
    lv_arc_set_range(sec_ring, 0, 59);
    lv_arc_set_bg_angles(sec_ring, 0, 360);
    lv_arc_set_rotation(sec_ring, 270);
    lv_obj_set_style_arc_color(sec_ring, lv_color_hex(0xFF3B30), LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(sec_ring, 3, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(sec_ring, lv_color_hex(0x1A2A3A), LV_PART_MAIN);
    lv_obj_set_style_arc_width(sec_ring, 3, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(sec_ring, LV_OPA_TRANSP, 0);
    lv_obj_remove_style(sec_ring, nullptr, LV_PART_KNOB);
    lv_obj_clear_flag(sec_ring, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_user_data(scr_clock, sec_ring);

    clock_face = lv_obj_create(scr_clock);
    lv_obj_set_size(clock_face, 220, 220);
    lv_obj_center(clock_face);
    lv_obj_set_style_bg_opa(clock_face, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(clock_face, 0, 0);
    lv_obj_clear_flag(clock_face, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_event_cb(clock_face, clock_face_draw_cb, LV_EVENT_DRAW_POST_END, nullptr);

    lbl_time = lv_label_create(scr_clock);
    lv_obj_set_style_text_font(lbl_time, &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_color(lbl_time, C_WHITE, 0);
    lv_obj_align(lbl_time, LV_ALIGN_CENTER, 0, 30);
    lv_label_set_text(lbl_time, "00:00");

    lbl_date = lv_label_create(scr_clock);
    lv_obj_set_style_text_font(lbl_date, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(lbl_date, C_DATE, 0);
    lv_obj_align(lbl_date, LV_ALIGN_CENTER, 0, 52);
    lv_label_set_text(lbl_date, "Po 1. Led 2025");

    lbl_alarm_icon = lv_label_create(scr_clock);
    lv_label_set_text(lbl_alarm_icon, LV_SYMBOL_BELL);
    lv_obj_set_style_text_color(lbl_alarm_icon, C_ACCENT, 0);
    lv_obj_set_style_text_font(lbl_alarm_icon, &lv_font_montserrat_16, 0);
    lv_obj_align(lbl_alarm_icon, LV_ALIGN_CENTER, 0, -60);
    lv_obj_add_flag(lbl_alarm_icon, LV_OBJ_FLAG_HIDDEN);

}
