#include <math.h>
#include <stdio.h>
#include <string.h>

#include "ui_next.h"
#include "ui_next_util.h"

/* Boot page (screen 14). The mark is a ring that holds the three waveform
 * bands and the amber playhead; its band profiles come from the board SVG
 * (viewBox 100, columns 22..78, half height in view units). */

static const float MARK_LOW[57] = {0, 0.6, 1.4, 2.6, 3.9, 5.5, 7.1, 8.7, 10.2, 11.6, 12.7, 13.5, 13.9, 13.9, 13.5, 12.6, 11.3, 9.7, 7.7, 7.1, 9.9, 12.6, 15.2, 17.7, 19.8, 21.6, 22.9, 23.7, 24, 23.7, 22.9, 21.6, 19.8, 17.7, 15.2, 12.6, 9.9, 7.1, 7.7, 9.7, 11.3, 12.6, 13.5, 13.9, 13.9, 13.5, 12.7, 11.6, 10.2, 8.7, 7.1, 5.5, 3.9, 2.6, 1.4, 0.6, 0};
static const float MARK_MID[57] = {0, 0.4, 1.2, 2.2, 3.3, 4.4, 5.3, 5.8, 5.9, 5.5, 4.5, 3.1, 4.5, 6.8, 9, 10.9, 12.3, 13, 13, 12.2, 10.6, 8.4, 5.7, 6.4, 9.4, 12.1, 14.2, 15.5, 16, 15.5, 14.2, 12.1, 9.4, 6.4, 5.7, 8.4, 10.6, 12.2, 13, 13, 12.3, 10.9, 9, 6.8, 4.5, 3.1, 4.5, 5.5, 5.9, 5.8, 5.3, 4.4, 3.3, 2.2, 1.2, 0.4, 0};
static const float MARK_HIGH[57] = {0, 0.3, 0.8, 1.3, 1.7, 1.6, 1, 1.7, 2.9, 3.8, 4.1, 3.5, 2.3, 2.6, 4.5, 5.8, 6.1, 5.4, 3.6, 2.9, 5.3, 7, 7.5, 6.7, 4.7, 2.7, 5.3, 7.3, 8, 7.3, 5.3, 2.7, 4.7, 6.7, 7.5, 7, 5.3, 2.9, 3.6, 5.4, 6.1, 5.8, 4.5, 2.6, 2.3, 3.5, 4.1, 3.8, 2.9, 1.7, 1, 1.6, 1.7, 1.3, 0.8, 0.3, 0};

static struct {
    ui_next_layout_t lay;
    const ui_next_palette_t *pal;
    lv_obj_t *root, *fill, *status, *name, *device, *footer;
    int bar_w;
} B;

static void bg_draw_cb(lv_event_t *e)
{
    /* radial-gradient(circle at 50% 40%, panel, frame 70%), in stepped rings. */
    lv_layer_t *layer = lv_event_get_layer(e);
    lv_area_t a;
    lv_obj_get_coords(lv_event_get_target_obj(e), &a);
    const int w = lv_area_get_width(&a), h = lv_area_get_height(&a);
    const float cx = (float)a.x1 + (float)w * .5f, cy = (float)a.y1 + (float)h * .4f;
    const float dx = (float)w * .5f, dy = (float)h * .6f;
    const float reach = sqrtf(dx * dx + dy * dy) * .7f;
    const uint32_t c0 = B.pal->panel, c1 = B.pal->frame;
    enum { STEPS = 40 };
    for (int i = 0; i < STEPS; i++) {
        const float r = reach * (1.0f - (float)i / (float)STEPS);
        const float t = (float)i / (float)(STEPS - 1);
        const uint32_t cc = lv_color_to_u32(lv_color_mix(ui_next_col(c0), ui_next_col(c1), (uint8_t)(255.0f * t))) & 0xFFFFFF;
        lv_draw_rect_dsc_t d;
        lv_draw_rect_dsc_init(&d);
        d.bg_color = ui_next_col(cc);
        d.radius = LV_RADIUS_CIRCLE;
        lv_area_t c = {(int32_t)(cx - r), (int32_t)(cy - r), (int32_t)(cx + r), (int32_t)(cy + r)};
        lv_draw_rect(layer, &d, &c);
    }
}

static void mark_draw_cb(lv_event_t *e)
{
    lv_layer_t *layer = lv_event_get_layer(e);
    lv_area_t a;
    lv_obj_get_coords(lv_event_get_target_obj(e), &a);
    const float k = (float)lv_area_get_width(&a) / 100.0f;
    const float ox = (float)a.x1, oy = (float)a.y1;

    lv_draw_rect_dsc_t ring;
    lv_draw_rect_dsc_init(&ring);
    ring.bg_opa = LV_OPA_TRANSP;
    ring.radius = LV_RADIUS_CIRCLE;
    ring.border_width = (int32_t)(3.0f * k + .5f);
    ring.border_color = ui_next_col(B.pal->border_hi);
    lv_area_t r = {(int32_t)(ox + 4 * k), (int32_t)(oy + 4 * k), (int32_t)(ox + 96 * k), (int32_t)(oy + 96 * k)};
    lv_draw_rect(layer, &ring, &r);

    const float *prof[3] = {MARK_LOW, MARK_MID, MARK_HIGH};
    const uint32_t colr[3] = {B.pal->wave_low, B.pal->wave_mid, B.pal->wave_high};
    const int x0 = (int)(ox + 22 * k), x1 = (int)(ox + 78 * k);
    for (int px = x0; px <= x1; px++) {
        const float u = ((float)px - ox) / k - 22.0f; /* 0..56 */
        int i0 = (int)u;
        if (i0 > 55) i0 = 55;
        const float fr = u - (float)i0;
        for (int b = 0; b < 3; b++) {
            const float v = prof[b][i0] + (prof[b][i0 + 1] - prof[b][i0]) * fr;
            int half = (int)(v * k + .5f);
            if (half < 1) half = 1;
            ui_next_fill_rect(layer, px, (int)(oy + 50 * k) - half, px, (int)(oy + 50 * k) + half, colr[b], LV_OPA_COVER);
        }
    }
    /* Amber playhead: x 36, from 20 to 80, 2.5 wide, round caps. */
    ui_next_seg(layer, ox + 36 * k, oy + 21 * k, ox + 36 * k, oy + 79 * k, (int)(2.5f * k + .5f), B.pal->accent, LV_OPA_COVER);
}

lv_obj_t *ui_next_boot_create(lv_obj_t *parent, uint16_t w, uint16_t h, bool day)
{
    memset(&B, 0, sizeof B);
    if (!ui_next_layout_compute(w, h, &B.lay)) return NULL;
    B.pal = ui_next_palette(day);
    const ui_next_tokens_t *t = &B.lay.tok;
    const ui_next_fonts_t *f = ui_next_fonts_for(B.lay.cls);
    static const int MARK[] = {125, 156, 208};
    const int mk = MARK[B.lay.cls];
    const int gap = t->gap_px;

    B.root = ui_next_box(parent, 0, 0, w, h, B.pal->frame);
    lv_obj_add_event_cb(B.root, bg_draw_cb, LV_EVENT_DRAW_MAIN, NULL);

    /* Column centred on the frame: mark, name, device name, bar, status. */
    lv_obj_t *col = lv_obj_create(B.root);
    lv_obj_remove_style_all(col);
    lv_obj_remove_flag(col, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_size(col, w, h);
    lv_obj_set_flex_flow(col, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(col, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(col, gap * 12 / 10, 0);

    lv_obj_t *mark = lv_obj_create(col);
    lv_obj_remove_style_all(mark);
    lv_obj_remove_flag(mark, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_size(mark, mk, mk);
    lv_obj_add_event_cb(mark, mark_draw_cb, LV_EVENT_DRAW_MAIN, NULL);

    /* .bn: letter spacing .3em, and the same margin on the left to keep it centred. */
    static const int SPACE[] = {8, 9, 10};
    B.name = ui_next_label(col, "OPEN DECK", f->title, B.pal->text);
    lv_obj_set_style_text_letter_space(B.name, SPACE[B.lay.cls], 0);
    lv_obj_set_style_pad_left(B.name, SPACE[B.lay.cls], 0);

    B.device = ui_next_label(col, "", f->mono, B.pal->muted);

    /* .bp: width 18 text heights, at most 70 % of the frame, 5 px high. */
    int bw = t->text_px * 18;
    if (bw > w * 70 / 100) bw = w * 70 / 100;
    B.bar_w = bw;
    lv_obj_t *track = ui_next_box(col, 0, 0, bw, 5, B.pal->border);
    lv_obj_set_style_radius(track, 3, 0);
    lv_obj_set_style_margin_top(track, gap * 2 - gap * 12 / 10, 0);
    B.fill = ui_next_box(track, 0, 0, 0, 5, B.pal->accent);
    lv_obj_set_style_radius(B.fill, 3, 0);

    B.status = ui_next_label(col, "", f->txt, B.pal->muted);

    B.footer = ui_next_label(B.root, "", f->mono, B.pal->muted);
    lv_obj_set_style_text_align(B.footer, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(B.footer, w);
    lv_obj_align(B.footer, LV_ALIGN_BOTTOM_MID, 0, -(t->pad_px * 14 / 10));
    return B.root;
}

void ui_next_boot_update(const ui_next_boot_model_t *m)
{
    if (!B.root) return;
    float p = m->progress01 < 0.0f ? 0.0f : m->progress01 > 1.0f ? 1.0f : m->progress01;
    lv_obj_set_width(B.fill, (int32_t)(p * (float)B.bar_w));
    lv_label_set_text(B.device, m->device_name);
    lv_label_set_text(B.status, m->status);
    lv_label_set_text(B.footer, m->footer);
}
