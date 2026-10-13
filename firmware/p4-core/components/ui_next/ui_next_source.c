#include <math.h>
#include <stdio.h>
#include <string.h>

#include "ui_next.h"
#include "ui_next_chrome.h"
#include "ui_next_util.h"

/* Source page (screen 04): five sources with a colour edge, icon, storage bar
 * and state, and the detail panel of the selected one. Landscape puts the list
 * (1) beside the panel (1.1); portrait stacks them and the panel takes the rest. */

#define SRC_GAP 6

static const char *const LABEL[6] = {"LINK", "INTERFACE", "IP ADDRESS", "DEVICE NAME", "LIBRARY SHARE", "TEMPO MASTER"};

typedef struct {
    lv_obj_t *row, *edge, *icon, *name, *bar_fill, *dot, *status, *chip;
} src_t;

static struct {
    ui_next_layout_t lay;
    const ui_next_palette_t *pal;
    const ui_next_fonts_t *f;
    bool day, landscape, wide;
    int row_h, pad, gap, fs, src_h;
    lv_obj_t *root, *panel_chip, *summary, *panel_title, *panel_state, *value[6], *pn[4];
    src_t src[UI_NEXT_SOURCES];
    ui_next_source_model_t m;
} S;

static uint32_t off_dot(void) { return S.day ? 0x8A94A3 : 0x6B7482; }

/* Line icons of the board, in a 24 unit box. */
static void arc(lv_layer_t *l, float x, float y, float k, float cy, float r, float half, uint32_t c)
{
    float px = 0, py = 0;
    for (int i = 0; i <= 12; i++) {
        const float a = -half + 2.0f * half * (float)i / 12.0f;
        const float ux = 12.0f + r * sinf(a), uy = cy - r * cosf(a);
        if (i) ui_next_seg(l, x + px * k, y + py * k, x + ux * k, y + uy * k, 2, c, LV_OPA_COVER);
        px = ux;
        py = uy;
    }
}

static void src_icon_cb(lv_event_t *e)
{
    const ui_next_src_icon_t kind = (ui_next_src_icon_t)(intptr_t)lv_event_get_user_data(e);
    lv_layer_t *l = lv_event_get_layer(e);
    lv_area_t a;
    lv_obj_get_coords(lv_event_get_target_obj(e), &a);
    const float k = (float)lv_area_get_width(&a) / 24.0f, x = (float)a.x1, y = (float)a.y1;
    const uint32_t c = S.pal->text;
#define LN(x1, y1, x2, y2) ui_next_seg(l, x + (x1)*k, y + (y1)*k, x + (x2)*k, y + (y2)*k, 2, c, LV_OPA_COVER)
    if (kind == UI_NEXT_SRC_USB) {
        LN(8, 3, 16, 3); LN(16, 3, 16, 9); LN(16, 9, 8, 9); LN(8, 9, 8, 3);
        LN(10, 9, 10, 13); LN(14, 9, 14, 13);
        LN(7, 13, 17, 13); LN(17, 13, 17, 21); LN(17, 21, 7, 21); LN(7, 21, 7, 13);
    } else if (kind == UI_NEXT_SRC_SD) {
        LN(7, 3, 15, 3); LN(15, 3, 18, 6); LN(18, 6, 18, 21); LN(18, 21, 7, 21); LN(7, 21, 7, 3);
        LN(10, 3, 10, 7); LN(13, 3, 13, 7);
    } else if (kind == UI_NEXT_SRC_NET) {
        arc(l, x, y, k, 20.18f, 15.0f, asinf(10.0f / 15.0f), c);
        arc(l, x, y, k, 19.14f, 10.0f, asinf(7.0f / 10.0f), c);
        arc(l, x, y, k, 18.57f, 5.0f, asinf(3.5f / 5.0f), c);
        LN(12, 18.5f, 12.2f, 18.5f);
    } else {
        LN(8, 3, 16, 3); LN(16, 3, 16, 21); LN(16, 21, 8, 21); LN(8, 21, 8, 3);
        LN(11, 18, 13, 18);
    }
#undef LN
}

static lv_obj_t *chip_dot(lv_obj_t *parent, int x, int y, lv_obj_t **dot, lv_obj_t **text)
{
    lv_obj_t *c = ui_next_box(parent, x, y, 0, S.fs * 19 / 10, S.pal->chip);
    lv_obj_set_width(c, LV_SIZE_CONTENT);
    lv_obj_set_style_radius(c, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_pad_hor(c, S.pad * 8 / 10, 0);
    lv_obj_set_style_pad_column(c, 6, 0);
    lv_obj_set_flex_flow(c, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(c, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    *dot = ui_next_box(c, 0, 0, 8, 8, S.pal->ok);
    lv_obj_set_style_radius(*dot, LV_RADIUS_CIRCLE, 0);
    *text = ui_next_label(c, "", S.f->small, S.pal->text);
    return c;
}

static void build_row(lv_obj_t *list, int i, int y, int w)
{
    src_t *r = &S.src[i];
    r->row = ui_next_box(list, 0, y, w, S.src_h, S.pal->panel);
    lv_obj_set_style_radius(r->row, 10, 0);
    lv_obj_set_style_clip_corner(r->row, true, 0);
    lv_obj_set_style_border_width(r->row, 1, 0);
    lv_obj_set_style_border_color(r->row, ui_next_col(S.pal->border), 0);
    r->edge = ui_next_box(r->row, 0, 0, 5, S.src_h, S.pal->accent);

    const int isz = S.fs * 17 / 10, ix = 5 + S.gap;
    r->icon = ui_next_box(r->row, ix, (S.src_h - 2 - isz) / 2, isz, isz, S.pal->panel);
    lv_obj_set_style_bg_opa(r->icon, LV_OPA_TRANSP, 0);

    /* Chip at the right edge, text block between the icon and the chip. */
    r->chip = chip_dot(r->row, 0, 0, &r->dot, &r->status);
    const int tx = ix + isz + S.gap;
    r->name = ui_next_label(r->row, "", S.f->row, S.pal->text);
    lv_obj_set_pos(r->name, tx, (S.src_h - 2 - (S.f->row->line_height + 9)) / 2);
    r->bar_fill = NULL;
    (void)w;
}

static void place_row_parts(int i, int w)
{
    src_t *r = &S.src[i];
    lv_obj_update_layout(r->chip);
    const int cw = lv_obj_get_width(r->chip), ch = lv_obj_get_height(r->chip);
    lv_obj_set_pos(r->chip, w - 2 - S.pad - cw, (S.src_h - 2 - ch) / 2);
}

/* ---- detail panel ---- */

static int build_panel(int x, int y, int w, int h)
{
    lv_obj_t *p = ui_next_box(S.root, x, y, w, h, S.pal->panel);
    lv_obj_set_style_radius(p, 12, 0);
    lv_obj_set_style_border_width(p, 1, 0);
    lv_obj_set_style_border_color(p, ui_next_col(S.pal->border), 0);
    const int ix = S.pad - 1, iw = w - 2 * S.pad;
    int cy = S.pad - 1;

    S.panel_title = ui_next_label(p, "", S.f->med, S.pal->text);
    lv_obj_set_pos(S.panel_title, ix, cy);
    lv_obj_t *dot, *txt;
    lv_obj_t *chip = chip_dot(p, 0, cy, &dot, &txt);
    S.panel_state = txt;
    lv_obj_set_user_data(chip, dot);
    lv_obj_update_layout(S.panel_title);
    lv_obj_set_x(chip, ix + 10 + 0);
    lv_label_set_text(txt, "DJ LINK ON");
    lv_obj_update_layout(chip);
    const int th = lv_obj_get_height(chip) > S.f->med->line_height ? lv_obj_get_height(chip) : S.f->med->line_height;
    lv_obj_set_y(S.panel_title, cy + (th - S.f->med->line_height) / 2);
    lv_obj_set_y(chip, cy + (th - lv_obj_get_height(chip)) / 2);
    S.panel_chip = chip; /* x is set on update, after the title width is known */
    cy += th + S.gap;

    const int kw = (iw - 6) / 2, kh = 12 + S.f->micro->line_height + S.f->kv->line_height;
    for (int i = 0; i < 6; i++) {
        const int kx = ix + (i % 2) * (kw + 6), ky = cy + (i / 2) * (kh + 6);
        lv_obj_t *kv = ui_next_box(p, kx, ky, i % 2 ? iw - kw - 6 : kw, kh, S.pal->nav);
        lv_obj_set_style_radius(kv, 8, 0);
        lv_obj_t *l = ui_next_label(kv, LABEL[i], S.f->micro, S.pal->muted);
        lv_obj_set_style_text_letter_space(l, 1, 0);
        lv_obj_set_pos(l, 10, 6);
        S.value[i] = ui_next_label(kv, "", S.f->kv, S.pal->text);
        lv_obj_set_pos(S.value[i], 10, 6 + S.f->micro->line_height);
    }
    cy += 3 * kh + 2 * 6 + S.gap;

    lv_obj_t *pl = ui_next_label(p, "PLAYER NUMBER", S.f->mono, S.pal->muted);
    lv_obj_set_pos(pl, ix, cy);
    cy += S.f->mono->line_height + 6;
    const int ph = S.row_h * 62 / 100, pw = (iw - 3 * S.gap) / 4;
    for (int i = 0; i < 4; i++) {
        lv_obj_t *b = ui_next_box(p, ix + i * (pw + S.gap), cy, i == 3 ? iw - 3 * (pw + S.gap) : pw, ph, S.pal->chip);
        lv_obj_set_style_radius(b, 8, 0);
        lv_obj_set_style_border_width(b, 1, 0);
        lv_obj_set_style_border_color(b, ui_next_col(S.pal->border_hi), 0);
        char n[2] = {(char)('1' + i), 0};
        lv_obj_t *t = ui_next_label(b, n, S.f->kv, S.pal->text);
        lv_obj_center(t);
        S.pn[i] = b;
    }
    cy += ph + S.gap;

    const int bh = S.row_h * 85 / 100, bw = (iw - S.gap) / 2;
    ui_next_chrome_action(p, ix, cy, bw, bh, "SCAN", false);
    lv_obj_t *leave = ui_next_chrome_action(p, ix + bw + S.gap, cy, iw - bw - S.gap, bh, "LEAVE LINK", false);
    lv_obj_set_style_bg_color(leave, ui_next_col(S.day ? 0xFBE3E3 : 0x2A1A1D), 0);
    lv_obj_set_style_border_color(leave, ui_next_col(0x8A3F3F), 0);
    lv_obj_set_style_text_color(lv_obj_get_child(leave, 0), ui_next_col(S.day ? 0xB42318 : 0xFF9A9A), 0);
    return cy + bh;
}

static void build_top_bar(void)
{
    lv_obj_t *row = ui_next_chrome_topbar(S.root);
    ui_next_chrome_button(row, S.wide ? "BACK" : NULL, UI_NEXT_ICON_CHEVRON, false);
    ui_next_label(row, "Source", S.f->med, S.pal->text);
    lv_obj_t *sp = lv_obj_create(row);
    lv_obj_remove_style_all(sp);
    lv_obj_set_size(sp, 1, 1);
    lv_obj_set_flex_grow(sp, 1);
    if (S.wide) {
        lv_obj_t *dot, *txt;
        chip_dot(row, 0, 0, &dot, &txt);
        S.summary = txt;
    }
}

lv_obj_t *ui_next_source_create(lv_obj_t *parent, uint16_t w, uint16_t h, bool day)
{
    memset(&S, 0, sizeof S);
    if (!ui_next_layout_compute(w, h, &S.lay)) return NULL;
    S.day = day;
    S.pal = ui_next_palette(day);
    S.f = ui_next_fonts_for(S.lay.cls);
    S.landscape = S.lay.landscape;
    S.wide = S.landscape && w >= 1000u;
    S.fs = S.lay.tok.text_px;
    S.row_h = S.lay.tok.row_px;
    S.pad = S.lay.tok.pad_px;
    S.gap = S.lay.tok.gap_px;
    S.src_h = S.row_h * 11 / 10;
    ui_next_chrome_begin(&S.lay, day, UI_NEXT_PAGE_BROWSE);

    S.root = ui_next_box(parent, 0, 0, w, h, S.pal->frame);
    const int x0 = S.lay.dock.x + S.pad, y0 = S.lay.top_bar.h + S.pad;
    const int iw = S.lay.dock.w - 2 * S.pad, ih = S.lay.dock.y - S.lay.top_bar.h - 2 * S.pad;
    const int list_h = UI_NEXT_SOURCES * S.src_h + (UI_NEXT_SOURCES - 1) * SRC_GAP;
    int lw, px, py, pw, ph;
    if (S.landscape) {
        lw = (iw - S.pad) * 10 / 21;
        px = x0 + lw + S.pad;
        py = y0;
        pw = iw - lw - S.pad;
        ph = ih;
    } else {
        lw = iw;
        px = x0;
        py = y0 + list_h + S.pad;
        pw = iw;
        ph = ih - list_h - S.pad;
    }
    lv_obj_t *list = ui_next_box(S.root, x0, y0, lw, list_h, S.pal->frame);
    lv_obj_set_style_bg_opa(list, LV_OPA_TRANSP, 0);
    for (int i = 0; i < UI_NEXT_SOURCES; i++) build_row(list, i, i * (S.src_h + SRC_GAP), lw);
    build_panel(px, py, pw, ph);

    ui_next_chrome_dock(S.root, 0, false);
    ui_next_chrome_dock(S.root, 1, false);
    build_top_bar();
    ui_next_chrome_nav(S.root);
    return S.root;
}

void ui_next_source_update(const ui_next_source_model_t *m)
{
    if (!S.root) return;
    S.m = *m;
    const int lw = lv_obj_get_width(lv_obj_get_parent(S.src[0].row));
    for (int i = 0; i < UI_NEXT_SOURCES; i++) {
        src_t *r = &S.src[i];
        const ui_next_source_t *s = &m->source[i];
        const bool on = i == m->selected;
        lv_obj_set_style_bg_color(r->row, ui_next_col(on ? S.pal->sel : S.pal->panel), 0);
        lv_obj_set_style_border_color(r->row, ui_next_col(on ? S.pal->accent : S.pal->border), 0);
        lv_obj_set_style_bg_color(r->edge, ui_next_col(s->color), 0);
        lv_obj_remove_event_cb(r->icon, src_icon_cb);
        lv_obj_add_event_cb(r->icon, src_icon_cb, LV_EVENT_DRAW_MAIN, (void *)(intptr_t)s->icon);
        lv_label_set_text(r->name, s->name);
        lv_label_set_text(r->status, s->status);
        lv_obj_set_style_bg_color(r->dot, ui_next_col(s->ready ? S.pal->ok : off_dot()), 0);
        place_row_parts(i, lw);

        /* Storage bar under the name, between the icon and the chip. */
        const int isz = S.fs * 17 / 10, tx = 5 + S.gap + isz + S.gap;
        const int bar_w = lw - 2 - tx - S.pad - S.gap - lv_obj_get_width(r->chip);
        const int by = (S.src_h - 2 - (S.f->row->line_height + 9)) / 2 + S.f->row->line_height + 4;
        if (!r->bar_fill) {
            lv_obj_t *track = ui_next_box(r->row, tx, by, bar_w, 5, S.pal->border);
            lv_obj_set_style_radius(track, 3, 0);
            r->bar_fill = ui_next_box(track, 0, 0, 0, 5, S.day ? S.pal->wave_mid : 0x3FD6C0);
            lv_obj_set_style_radius(r->bar_fill, 3, 0);
        }
        lv_obj_set_width(r->bar_fill, bar_w * s->used_pct / 100);
        if (s->used_pct == 0) lv_obj_add_flag(r->bar_fill, LV_OBJ_FLAG_HIDDEN);
        else lv_obj_remove_flag(r->bar_fill, LV_OBJ_FLAG_HIDDEN);
    }
    lv_label_set_text(S.panel_title, m->panel_title);
    lv_label_set_text(S.panel_state, m->panel_state);
    lv_obj_update_layout(S.panel_title);
    lv_obj_set_x(S.panel_chip, lv_obj_get_x(S.panel_title) + lv_obj_get_width(S.panel_title) + S.gap);
    for (int i = 0; i < 6; i++) lv_label_set_text(S.value[i], m->value[i]);
    for (int i = 0; i < 4; i++) {
        const bool on = m->player == i + 1;
        lv_obj_set_style_bg_color(S.pn[i], ui_next_col(on ? S.pal->accent : S.pal->chip), 0);
        lv_obj_set_style_border_color(S.pn[i], ui_next_col(on ? S.pal->accent : S.pal->border_hi), 0);
        lv_obj_set_style_text_color(lv_obj_get_child(S.pn[i], 0), ui_next_col(on ? S.pal->on_accent : S.pal->text), 0);
    }
    if (S.summary) lv_label_set_text(S.summary, m->summary);
    ui_next_chrome_dock_set(0, &m->deck[0]);
    ui_next_chrome_dock_set(1, &m->deck[1]);
}
