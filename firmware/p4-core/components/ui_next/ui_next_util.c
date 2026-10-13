#include "ui_next_util.h"

#include <math.h>

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

void ui_next_text_down(lv_layer_t *layer, const char *s, int x, int y, const lv_font_t *f, uint32_t rgb, lv_opa_t opa)
{
    /* Each letter is turned 90 degrees clockwise about its own top left and set below the
     * previous one; the glyph height then extends to the left of the pen position. */
    lv_draw_letter_dsc_t d;
    lv_draw_letter_dsc_init(&d);
    d.font = f;
    d.color = lv_color_hex(rgb);
    d.opa = opa;
    d.rotation = 900;
    d.pivot.x = 0;
    d.pivot.y = 0;
    int pen = y;
    for (const char *c = s; *c; c++) {
        d.unicode = (uint32_t)(unsigned char)*c;
        lv_point_t p = {x + f->line_height, pen};
        lv_draw_letter(layer, &d, &p);
        pen += lv_font_get_glyph_width(f, d.unicode, (uint32_t)(unsigned char)c[1]);
    }
}

typedef struct {
    lv_layer_t *layer;
    uint32_t rgb;
    float len, dash, period;
} dash_t;

static void dash_pt(dash_t *d, float x, float y)
{
    if (fmodf(d->len, d->period) < d->dash) ui_next_fill_rect(d->layer, (int)(x + .5f), (int)(y + .5f), (int)(x + .5f), (int)(y + .5f), d->rgb, LV_OPA_COVER);
    d->len += 1.0f;
}

void ui_next_dashed_round(lv_layer_t *layer, const lv_area_t *a, int radius, uint32_t rgb, int dash, int gap)
{
    dash_t d = {layer, rgb, 0.0f, (float)dash, (float)(dash + gap)};
    const float x1 = (float)a->x1 + .5f, y1 = (float)a->y1 + .5f, x2 = (float)a->x2 + .5f, y2 = (float)a->y2 + .5f;
    float r = (float)radius;
    const float half = fminf(x2 - x1, y2 - y1) / 2.0f;
    if (r > half) r = half;
    const float pi = 3.14159265f;
    /* Clockwise from the top left corner: top edge, corner arc, right edge, ... */
    for (float x = x1 + r; x < x2 - r; x += 1.0f) dash_pt(&d, x, y1);
    for (float t = 0; t < r * pi / 2; t += 1.0f) dash_pt(&d, x2 - r + r * sinf(t / r), y1 + r - r * cosf(t / r));
    for (float y = y1 + r; y < y2 - r; y += 1.0f) dash_pt(&d, x2, y);
    for (float t = 0; t < r * pi / 2; t += 1.0f) dash_pt(&d, x2 - r + r * cosf(t / r), y2 - r + r * sinf(t / r));
    for (float x = x2 - r; x > x1 + r; x -= 1.0f) dash_pt(&d, x, y2);
    for (float t = 0; t < r * pi / 2; t += 1.0f) dash_pt(&d, x1 + r - r * sinf(t / r), y2 - r + r * cosf(t / r));
    for (float y = y2 - r; y > y1 + r; y -= 1.0f) dash_pt(&d, x1, y);
    for (float t = 0; t < r * pi / 2; t += 1.0f) dash_pt(&d, x1 + r - r * cosf(t / r), y1 + r - r * sinf(t / r));
}

void ui_next_cover(lv_obj_t *obj, lv_grad_dsc_t *g, uint32_t c0, uint32_t c1)
{
    const lv_color_t cols[2] = {lv_color_hex(c0), lv_color_hex(c1)};
    const lv_opa_t opas[2] = {LV_OPA_COVER, LV_OPA_COVER};
    const uint8_t fracs[2] = {0, 255};
    lv_grad_init_stops(g, cols, opas, fracs, 2);
    lv_grad_linear_init(g, 0, 0, lv_pct(100), lv_pct(100), LV_GRAD_EXTEND_PAD);
    lv_obj_set_style_bg_grad(obj, g, 0);
}
