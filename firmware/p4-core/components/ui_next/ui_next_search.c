#include <stdio.h>
#include <string.h>

#include "ui_next.h"
#include "ui_next_chrome.h"
#include "ui_next_util.h"

/* Search page (screen 06): query field, result list with the matched letters in
 * amber, touch keyboard, dock. The page is two panels: list and keyboard side by
 * side in landscape (1 : 1.4), stacked in portrait (keyboard 4.9 rows high). */

#define MAX_ROWS 10
#define KEY_GAP 6

typedef struct {
    lv_obj_t *rs, *bar, *art, *pre, *hit, *post, *artist, *bpm, *key;
} row_t;

static struct {
    ui_next_layout_t lay;
    const ui_next_palette_t *pal;
    const ui_next_fonts_t *f;
    bool day, landscape, wide;
    int row_h, pad, gap, fs;
    lv_obj_t *root, *query, *status;
    row_t rows[MAX_ROWS];
    int n_rows;
} S;

static uint32_t key_face(bool wide) { return wide ? (S.day ? 0xD5DCE5 : 0x2D3644) : S.pal->chip; }

static lv_obj_t *key(lv_obj_t *parent, int x, int y, int w, int h, const char *t, int kind /* 0 key, 1 wide, 2 go */)
{
    lv_obj_t *k = ui_next_box(parent, x, y, w, h, kind == 2 ? S.pal->accent : key_face(kind == 1));
    lv_obj_set_style_radius(k, 8, 0);
    lv_obj_set_style_border_width(k, 1, 0);
    lv_obj_set_style_border_color(k, ui_next_col(kind == 2 ? S.pal->accent : S.pal->border_hi), 0);
    lv_obj_t *l = ui_next_label(k, t, kind == 0 ? S.f->med : S.f->tagf, kind == 2 ? S.pal->on_accent : S.pal->text);
    if (kind != 0) lv_obj_set_style_text_letter_space(l, 1, 0);
    lv_obj_center(l);
    return k;
}

/* One row of keys: weights in tenths, outer padding in percent of the width. */
static void key_row(lv_obj_t *kb, int y, int w, int h, const char *const *labels, const int *weight,
                    const uint8_t *kind, int n, int pad_pct)
{
    const int margin = w * pad_pct / 100, inner = w - 2 * margin;
    int total = 0;
    for (int i = 0; i < n; i++) total += weight[i];
    const int avail = inner - (n - 1) * KEY_GAP;
    int before = 0;
    for (int i = 0; i < n; i++) {
        const int x0 = avail * before / total + i * KEY_GAP;
        before += weight[i];
        const int x1 = avail * before / total + i * KEY_GAP;
        key(kb, margin + x0, y, x1 - x0, h, labels[i], kind[i]);
    }
}

static void build_keyboard(int x, int y, int w, int h)
{
    lv_obj_t *panel = ui_next_box(S.root, x, y, w, h, S.pal->panel);
    lv_obj_set_style_radius(panel, 12, 0);
    lv_obj_set_style_border_width(panel, 1, 0);
    lv_obj_set_style_border_color(panel, ui_next_col(S.pal->border), 0);
    const int iw = w - 2 * S.pad - 2, ih = h - 2 * S.pad - 2;
    lv_obj_t *kb = ui_next_box(panel, S.pad - 1, S.pad - 1, iw, ih, S.pal->panel);
    const int rh = (ih - 3 * KEY_GAP) / 4;
    static const char *const R1[] = {"Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P"};
    static const char *const R2[] = {"A", "S", "D", "F", "G", "H", "J", "K", "L"};
    static const char *const R3[] = {"SHIFT", "Z", "X", "C", "V", "B", "N", "M", "DEL"};
    static const char *const R4[] = {"123", "SPACE", "SEARCH"};
    static const int W1[] = {10, 10, 10, 10, 10, 10, 10, 10, 10, 10}, W3[] = {15, 10, 10, 10, 10, 10, 10, 10, 15},
                     W4[] = {15, 50, 24};
    static const uint8_t K0[10] = {0}, K3[] = {1, 0, 0, 0, 0, 0, 0, 0, 1}, K4[] = {1, 1, 2};
    key_row(kb, 0, iw, rh, R1, W1, K0, 10, 0);
    key_row(kb, rh + KEY_GAP, iw, rh, R2, W1, K0, 9, 5);
    key_row(kb, 2 * (rh + KEY_GAP), iw, rh, R3, W3, K3, 9, 0);
    key_row(kb, 3 * (rh + KEY_GAP), iw, ih - 3 * (rh + KEY_GAP), R4, W4, K4, 3, 0);
}

static void build_row(lv_obj_t *list, int i, int y, int w)
{
    row_t *r = &S.rows[i];
    r->rs = ui_next_box(list, 0, y, w, S.row_h * 105 / 100, S.pal->panel);
    lv_obj_set_style_radius(r->rs, 8, 0);
    lv_obj_set_style_clip_corner(r->rs, true, 0);
    lv_obj_t *flow = lv_obj_create(r->rs);
    lv_obj_remove_style_all(flow);
    lv_obj_remove_flag(flow, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_size(flow, w, S.row_h * 105 / 100);
    lv_obj_set_flex_flow(flow, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(flow, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_left(flow, S.gap, 0);
    lv_obj_set_style_pad_right(flow, S.pad, 0);
    lv_obj_set_style_pad_column(flow, S.gap, 0);

    const int as = S.row_h * 78 / 100;
    r->art = ui_next_box(flow, 0, 0, as, as, S.pal->chip);
    lv_obj_set_style_radius(r->art, 6, 0);
    lv_obj_set_style_bg_grad_dir(r->art, LV_GRAD_DIR_VER, 0);

    lv_obj_t *tx = lv_obj_create(flow);
    lv_obj_remove_style_all(tx);
    lv_obj_remove_flag(tx, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_height(tx, LV_SIZE_CONTENT);
    lv_obj_set_flex_grow(tx, 1);
    lv_obj_set_flex_flow(tx, LV_FLEX_FLOW_COLUMN);
    lv_obj_t *nm = lv_obj_create(tx);
    lv_obj_remove_style_all(nm);
    lv_obj_remove_flag(nm, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_size(nm, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(nm, LV_FLEX_FLOW_ROW);
    r->pre = ui_next_label(nm, "", S.f->row, S.pal->text);
    r->hit = ui_next_label(nm, "", S.f->row, S.pal->accent_ink);
    r->post = ui_next_label(nm, "", S.f->row, S.pal->text);
    r->artist = ui_next_label(tx, "", S.f->small, S.pal->muted);

    r->bar = ui_next_box(r->rs, 0, 0, 3, S.row_h * 105 / 100, S.pal->accent);
    r->bpm = ui_next_label(flow, "", S.f->rmono, S.pal->text);
    if (S.wide) {
        r->key = ui_next_label(flow, "", S.f->small, S.pal->text);
        lv_obj_set_style_bg_color(r->key, ui_next_col(S.pal->chip), 0);
        lv_obj_set_style_bg_opa(r->key, LV_OPA_COVER, 0);
        lv_obj_set_style_radius(r->key, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_pad_hor(r->key, S.pad * 8 / 10, 0);
        lv_obj_set_style_pad_ver(r->key, (S.fs * 19 / 10 - S.f->small->line_height) / 2, 0);
    }
}

static void build_top_bar(void)
{
    lv_obj_t *row = ui_next_chrome_topbar(S.root);
    ui_next_chrome_button(row, NULL, UI_NEXT_ICON_CHEVRON, false);

    lv_obj_t *field = lv_obj_create(row);
    lv_obj_remove_style_all(field);
    lv_obj_remove_flag(field, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_height(field, S.lay.tok.top_bar_px - 10);
    lv_obj_set_flex_grow(field, 1);
    lv_obj_set_style_bg_color(field, ui_next_col(S.pal->well), 0);
    lv_obj_set_style_bg_opa(field, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(field, 1, 0);
    lv_obj_set_style_border_color(field, ui_next_col(S.pal->accent), 0);
    lv_obj_set_style_radius(field, 10, 0);
    lv_obj_set_style_pad_hor(field, S.pad, 0);
    lv_obj_set_style_pad_column(field, 8, 0);
    lv_obj_set_flex_flow(field, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(field, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    ui_next_chrome_icon(field, 18, UI_NEXT_ICON_SEARCH);
    S.query = ui_next_label(field, "", S.f->bpm, S.pal->text);
    ui_next_box(field, 0, 0, 2, (S.lay.tok.top_bar_px - 10) * 6 / 10, S.pal->accent);
    if (S.wide) {
        lv_obj_t *sp = lv_obj_create(field);
        lv_obj_remove_style_all(sp);
        lv_obj_set_size(sp, 1, 1);
        lv_obj_set_flex_grow(sp, 1);
        S.status = ui_next_label(field, "", S.f->tiny, S.pal->muted);
        lv_obj_set_style_text_letter_space(S.status, 0, 0);
    }
    if (S.wide) ui_next_chrome_button(row, "CLEAR", -1, false);
}

lv_obj_t *ui_next_search_create(lv_obj_t *parent, uint16_t w, uint16_t h, bool day)
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
    ui_next_chrome_begin(&S.lay, day, UI_NEXT_PAGE_BROWSE);

    S.root = ui_next_box(parent, 0, 0, w, h, S.pal->frame);
    const int x0 = S.lay.dock.x, y0 = S.lay.top_bar.h;
    const int iw = S.lay.dock.w - 2 * S.pad, ih = S.lay.dock.y - y0 - 2 * S.pad;
    int lx = x0 + S.pad, ly = y0 + S.pad, lw, lh;
    if (S.landscape) {
        lw = (iw - S.pad) * 10 / 24;
        lh = ih;
        build_keyboard(lx + lw + S.pad, ly, iw - lw - S.pad, ih);
    } else {
        const int kh = S.row_h * 49 / 10;
        lw = iw;
        lh = ih - kh - S.pad;
        build_keyboard(lx, ly + lh + S.pad, iw, kh);
    }
    lv_obj_t *list = ui_next_box(S.root, lx, ly, lw, lh, S.pal->frame);
    lv_obj_set_style_clip_corner(list, true, 0);
    const int step = S.row_h * 105 / 100 + 6;
    S.n_rows = (lh + 6 + step - 1) / step;
    if (S.n_rows > MAX_ROWS) S.n_rows = MAX_ROWS;
    for (int i = 0; i < S.n_rows; i++) build_row(list, i, i * step, lw);
    lv_obj_t *sb = ui_next_box(list, lw - 3, 0, 3, lh * 38 / 100, S.pal->border_hi);
    lv_obj_set_style_radius(sb, 2, 0);

    ui_next_chrome_dock(S.root, 0, false);
    ui_next_chrome_dock(S.root, 1, false);
    build_top_bar();
    ui_next_chrome_nav(S.root);
    return S.root;
}

void ui_next_search_update(const ui_next_search_model_t *m)
{
    if (!S.root) return;
    lv_label_set_text(S.query, m->query);
    if (S.status) lv_label_set_text(S.status, m->status);
    for (int i = 0; i < S.n_rows; i++) {
        row_t *r = &S.rows[i];
        if (i >= m->n) {
            lv_obj_add_flag(r->rs, LV_OBJ_FLAG_HIDDEN);
            continue;
        }
        const ui_next_search_result_t *s = &m->result[i];
        const bool on = i == m->selected;
        lv_obj_set_style_bg_color(r->rs, ui_next_col(on ? S.pal->sel : S.pal->panel), 0);
        lv_label_set_text(r->pre, s->pre);
        lv_label_set_text(r->hit, s->hit);
        lv_label_set_text(r->post, s->post);
        lv_label_set_text(r->artist, s->artist);
        lv_label_set_text(r->bpm, s->bpm);
        if (r->key) lv_label_set_text(r->key, s->key);
        lv_obj_set_style_bg_color(r->art, ui_next_col(s->art0), 0);
        lv_obj_set_style_bg_grad_color(r->art, ui_next_col(s->art1), 0);
        if (on) lv_obj_remove_flag(r->bar, LV_OBJ_FLAG_HIDDEN);
        else lv_obj_add_flag(r->bar, LV_OBJ_FLAG_HIDDEN);
    }
    ui_next_chrome_dock_set(0, &m->deck[0]);
    ui_next_chrome_dock_set(1, &m->deck[1]);
}
