#include "ui_next_util.h"

#include "ui_next_fonts.h"

const ui_next_fonts_t *ui_next_fonts_for(ui_next_class_t cls)
{
    static const ui_next_fonts_t F[] = {
        [UI_NEXT_CLASS_S] = {&ui_next_roboto_reg_14, &ui_next_roboto_med_16, &ui_next_cousine_bold_10,
                             &ui_next_cousine_reg_12, &ui_next_cousine_reg_20, &ui_next_cousine_reg_16,
                             &ui_next_roboto_reg_12, &ui_next_roboto_med_28,
                             &ui_next_roboto_med_14, &ui_next_roboto_reg_12, &ui_next_cousine_reg_14, &ui_next_roboto_med_12,
                             &ui_next_roboto_med_18, &ui_next_roboto_med_20, &ui_next_cousine_timer_48, &ui_next_cousine_reg_10, &ui_next_cousine_reg_14},
        [UI_NEXT_CLASS_M] = {&ui_next_roboto_reg_16, &ui_next_roboto_med_18, &ui_next_cousine_bold_10,
                             &ui_next_cousine_reg_12, &ui_next_cousine_reg_20, &ui_next_cousine_reg_16,
                             &ui_next_roboto_med_14, &ui_next_roboto_med_30,
                             &ui_next_roboto_med_16, &ui_next_roboto_reg_12, &ui_next_cousine_reg_14, &ui_next_roboto_med_12,
                             &ui_next_roboto_med_20, &ui_next_roboto_med_22, &ui_next_cousine_timer_52, &ui_next_cousine_reg_10, &ui_next_cousine_reg_16},
        [UI_NEXT_CLASS_L] = {&ui_next_roboto_reg_18, &ui_next_roboto_med_18, &ui_next_cousine_bold_12,
                             &ui_next_cousine_reg_14, &ui_next_cousine_reg_20, &ui_next_cousine_reg_20,
                             &ui_next_roboto_med_16, &ui_next_roboto_med_34,
                             &ui_next_roboto_med_18, &ui_next_roboto_reg_14, &ui_next_cousine_reg_16, &ui_next_roboto_med_14,
                             &ui_next_roboto_med_22, &ui_next_roboto_med_24, &ui_next_cousine_timer_58, &ui_next_cousine_reg_12, &ui_next_cousine_reg_16},
    };
    return &F[cls];
}

lv_obj_t *ui_next_box(lv_obj_t *parent, int x, int y, int w, int h, uint32_t rgb)
{
    lv_obj_t *o = lv_obj_create(parent);
    lv_obj_remove_style_all(o);
    lv_obj_remove_flag(o, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_pos(o, x, y);
    lv_obj_set_size(o, w, h);
    lv_obj_set_style_bg_color(o, lv_color_hex(rgb), 0);
    lv_obj_set_style_bg_opa(o, LV_OPA_COVER, 0);
    return o;
}

lv_obj_t *ui_next_label(lv_obj_t *parent, const char *s, const lv_font_t *f, uint32_t rgb)
{
    lv_obj_t *l = lv_label_create(parent);
    lv_label_set_text(l, s);
    lv_obj_set_style_text_font(l, f, 0);
    lv_obj_set_style_text_color(l, lv_color_hex(rgb), 0);
    return l;
}

void ui_next_fill_rect(lv_layer_t *layer, int x1, int y1, int x2, int y2, uint32_t rgb, lv_opa_t opa)
{
    if (x2 < x1 || y2 < y1) return;
    lv_draw_rect_dsc_t d;
    lv_draw_rect_dsc_init(&d);
    d.bg_color = lv_color_hex(rgb);
    d.bg_opa = opa;
    lv_area_t a = {x1, y1, x2, y2};
    lv_draw_rect(layer, &d, &a);
}

void ui_next_seg(lv_layer_t *layer, float x1, float y1, float x2, float y2, int w, uint32_t rgb, lv_opa_t opa)
{
    lv_draw_line_dsc_t d;
    lv_draw_line_dsc_init(&d);
    d.p1.x = (lv_value_precise_t)x1;
    d.p1.y = (lv_value_precise_t)y1;
    d.p2.x = (lv_value_precise_t)x2;
    d.p2.y = (lv_value_precise_t)y2;
    d.width = w;
    d.color = lv_color_hex(rgb);
    d.opa = opa;
    d.round_start = d.round_end = 1;
    lv_draw_line(layer, &d);
}

void ui_next_text_at(lv_layer_t *layer, const char *s, int x, int y, const lv_font_t *f, uint32_t rgb)
{
    lv_draw_label_dsc_t d;
    lv_draw_label_dsc_init(&d);
    d.text = s;
    d.font = f;
    d.color = lv_color_hex(rgb);
    lv_area_t a = {x, y, x + 60, y + 20};
    lv_draw_label(layer, &d, &a);
}
