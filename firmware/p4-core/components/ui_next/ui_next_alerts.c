#include <string.h>

#include "ui_next.h"
#include "ui_next_util.h"

/* Alerts overlay (screen 13). A dim layer over the page, up to three toasts at
 * the top right (the third only above the S class) and one modal dialog at 60 %
 * of the height. Pure drawing: the UI task owns it, the audio task never waits
 * for it. */

#define DIM_OPA 189 /* rgba(5,7,10,.74) */

static struct {
    ui_next_layout_t lay;
    const ui_next_palette_t *pal;
    const ui_next_fonts_t *f;
    bool day;
    int w, h, pad, gap, fs;
    lv_obj_t *root, *dim;
    lv_obj_t *toast[UI_NEXT_TOASTS], *toast_bar[UI_NEXT_TOASTS], *toast_title[UI_NEXT_TOASTS], *toast_text[UI_NEXT_TOASTS];
    lv_obj_t *modal, *m_title, *m_body, *m_btn[2], *m_btn_label[2];
    int toasts_shown;
} A;

static void shadow(lv_obj_t *o, int width, int ofs_y, lv_opa_t opa)
{
    lv_obj_set_style_shadow_width(o, width, 0);
    lv_obj_set_style_shadow_offset_y(o, ofs_y, 0);
    lv_obj_set_style_shadow_color(o, lv_color_black(), 0);
    lv_obj_set_style_shadow_opa(o, opa, 0);
}

/* Drive removed icon, from the board path in a 24 unit box. */
static void usb_icon_cb(lv_event_t *e)
{
    lv_layer_t *layer = lv_event_get_layer(e);
    lv_area_t a;
    lv_obj_get_coords(lv_event_get_target_obj(e), &a);
    const float k = (float)lv_area_get_width(&a) / 24.0f, x = (float)a.x1, y = (float)a.y1;
    static const int8_t S[][4] = {
        {8, 3, 16, 3}, {16, 3, 16, 9}, {16, 9, 8, 9}, {8, 9, 8, 3},
        {10, 9, 10, 13}, {14, 9, 14, 13},
        {7, 13, 17, 13}, {17, 13, 17, 21}, {17, 21, 7, 21}, {7, 21, 7, 13},
    };
    for (unsigned i = 0; i < sizeof S / sizeof S[0]; i++)
        ui_next_seg(layer, x + S[i][0] * k, y + S[i][1] * k, x + S[i][2] * k, y + S[i][3] * k, 2, A.pal->bad,
                    LV_OPA_COVER);
}

static uint32_t kind_color(ui_next_toast_kind_t k)
{
    return k == UI_NEXT_TOAST_DONE ? 0x3FD6C0 : k == UI_NEXT_TOAST_CARE ? 0xFFC15A : A.pal->bad;
}

static void build_toasts(void)
{
    const int tw = A.fs * 22 < A.w - 2 * A.pad ? A.fs * 22 : A.w - 2 * A.pad;
    const int th = A.f->row->line_height + A.f->small->line_height + A.pad * 8 / 5 + 2;
    const int x = A.w - A.pad - tw, y0 = A.lay.tok.top_bar_px + A.gap;
    A.toasts_shown = A.lay.cls == UI_NEXT_CLASS_S ? 2 : UI_NEXT_TOASTS;
    for (int i = 0; i < UI_NEXT_TOASTS; i++) {
        lv_obj_t *t = ui_next_box(A.root, x, y0 + i * (th + A.gap), tw, th, A.pal->chip);
        lv_obj_set_style_radius(t, 10, 0);
        lv_obj_set_style_clip_corner(t, true, 0);
        lv_obj_set_style_border_width(t, 1, 0);
        lv_obj_set_style_border_color(t, ui_next_col(A.pal->border_hi), 0);
        shadow(t, 24, 8, 128);
        A.toast_bar[i] = ui_next_box(t, 0, 0, 5, th, A.pal->ok);
        A.toast_title[i] = ui_next_label(t, "", A.f->row, A.pal->text);
        A.toast_text[i] = ui_next_label(t, "", A.f->small, A.pal->muted);
        const int tx = 5 + A.gap, ty = A.pad * 4 / 5;
        lv_obj_set_pos(A.toast_title[i], tx, ty);
        lv_obj_set_pos(A.toast_text[i], tx, ty + A.f->row->line_height);
        lv_obj_set_width(A.toast_title[i], tw - tx - A.pad);
        lv_obj_set_width(A.toast_text[i], tw - tx - A.pad);
        lv_label_set_long_mode(A.toast_title[i], LV_LABEL_LONG_MODE_DOTS);
        lv_label_set_long_mode(A.toast_text[i], LV_LABEL_LONG_MODE_DOTS);
        A.toast[i] = t;
    }
}

static void build_modal(void)
{
    const int mw = A.fs * 27 < A.w * 88 / 100 ? A.fs * 27 : A.w * 88 / 100;
    const int pad = A.pad * 3 / 2;
    const int iw = mw - 2 * pad - 2;
    const int icon = A.lay.tok.row_px * 8 / 10;
    const int bh = A.lay.tok.row_px * 85 / 100;
    A.modal = ui_next_box(A.root, 0, 0, mw, LV_SIZE_CONTENT, A.pal->panel);
    lv_obj_set_style_radius(A.modal, 16, 0);
    lv_obj_set_style_border_width(A.modal, 1, 0);
    lv_obj_set_style_border_color(A.modal, ui_next_col(A.pal->border_hi), 0);
    lv_obj_set_style_pad_all(A.modal, pad, 0);
    lv_obj_set_flex_flow(A.modal, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(A.modal, A.gap, 0);
    shadow(A.modal, 48, 16, 153);

    lv_obj_t *ring = ui_next_box(A.modal, 0, 0, icon, icon, A.day ? 0xFBE3E3 : 0x2A1A1D);
    lv_obj_set_style_radius(ring, LV_RADIUS_CIRCLE, 0);
    lv_obj_t *ic = lv_obj_create(ring);
    lv_obj_remove_style_all(ic);
    lv_obj_remove_flag(ic, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_size(ic, 26, 26);
    lv_obj_center(ic);
    lv_obj_add_event_cb(ic, usb_icon_cb, LV_EVENT_DRAW_MAIN, NULL);

    A.m_title = ui_next_label(A.modal, "", A.f->head, A.pal->text);
    A.m_body = ui_next_label(A.modal, "", A.f->txt, A.pal->muted);
    lv_obj_set_width(A.m_body, iw);
    lv_label_set_long_mode(A.m_body, LV_LABEL_LONG_MODE_WRAP);
    lv_obj_set_style_text_line_space(A.m_body, A.fs * 14 / 10 - A.f->txt->line_height, 0);

    lv_obj_t *row = lv_obj_create(A.modal);
    lv_obj_remove_style_all(row);
    lv_obj_remove_flag(row, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_size(row, iw, bh);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_column(row, A.gap, 0);
    for (int i = 0; i < 2; i++) {
        const bool on = i == 0;
        lv_obj_t *b = ui_next_box(row, 0, 0, 0, bh, on ? A.pal->accent : A.pal->chip);
        lv_obj_set_flex_grow(b, 1);
        lv_obj_set_style_radius(b, 10, 0);
        lv_obj_set_style_border_width(b, 1, 0);
        lv_obj_set_style_border_color(b, ui_next_col(on ? A.pal->accent : A.pal->border_hi), 0);
        A.m_btn_label[i] = ui_next_label(b, "", A.f->row, on ? A.pal->on_accent : A.pal->text);
        lv_obj_center(A.m_btn_label[i]);
        A.m_btn[i] = b;
    }
}

lv_obj_t *ui_next_alerts_create(lv_obj_t *parent, uint16_t w, uint16_t h, bool day)
{
    memset(&A, 0, sizeof A);
    if (!ui_next_layout_compute(w, h, &A.lay)) return NULL;
    A.day = day;
    A.pal = ui_next_palette(day);
    A.f = ui_next_fonts_for(A.lay.cls);
    A.w = w;
    A.h = h;
    A.pad = A.lay.tok.pad_px;
    A.gap = A.lay.tok.gap_px;
    A.fs = A.lay.tok.text_px;

    A.root = lv_obj_create(parent);
    lv_obj_remove_style_all(A.root);
    lv_obj_remove_flag(A.root, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(A.root, w, h);
    A.dim = ui_next_box(A.root, 0, 0, w, h, A.pal->frame);
    lv_obj_set_style_bg_opa(A.dim, DIM_OPA, 0);
    build_toasts();
    build_modal();
    return A.root;
}

void ui_next_alerts_update(const ui_next_alerts_model_t *m)
{
    if (!A.root) return;
    for (int i = 0; i < UI_NEXT_TOASTS; i++) {
        const bool show = i < m->n_toasts && i < A.toasts_shown;
        if (!show) {
            lv_obj_add_flag(A.toast[i], LV_OBJ_FLAG_HIDDEN);
            continue;
        }
        lv_obj_remove_flag(A.toast[i], LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_style_bg_color(A.toast_bar[i], ui_next_col(kind_color(m->toast[i].kind)), 0);
        lv_label_set_text(A.toast_title[i], m->toast[i].title);
        lv_label_set_text(A.toast_text[i], m->toast[i].text);
    }
    if (!m->modal) {
        lv_obj_add_flag(A.modal, LV_OBJ_FLAG_HIDDEN);
        return;
    }
    lv_obj_remove_flag(A.modal, LV_OBJ_FLAG_HIDDEN);
    lv_label_set_text(A.m_title, m->title);
    lv_label_set_text(A.m_body, m->body);
    lv_label_set_text(A.m_btn_label[0], m->primary);
    lv_label_set_text(A.m_btn_label[1], m->secondary);
    lv_obj_update_layout(A.modal);
    lv_obj_set_pos(A.modal, (A.w - lv_obj_get_width(A.modal)) / 2, A.h * 6 / 10 - lv_obj_get_height(A.modal) / 2);
}
