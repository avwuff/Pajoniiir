#include <stdio.h>
#include <string.h>

#include "ui_next.h"
#include "ui_next_chrome.h"
#include "ui_next_util.h"

/* Grid edit page (screen 08): the zoomed waveform with the beat grid, the
 * overview with the zoomed window, and four rows of buttons. The page is one
 * column in both orientations; the waveform takes what the rest leaves. */

#define ROWS 4

static const char *const ROW_LABEL[ROWS] = {"SHIFT", "SPACING", "TEMPO", "GRID"};
static const char *const ROW_BTN[ROWS][4] = {
    {"-10 MS", "-1 MS", "+1 MS", "+10 MS"}, {"-10 MS", "-1 MS", "+1 MS", "+10 MS"},
    {"/2", "x2", "TAP", "RESET"}, {"SET BEAT 1", "LOCK", "", ""},
};

static struct {
    ui_next_layout_t lay;
    const ui_next_palette_t *pal;
    const ui_next_fonts_t *f;
    bool day, wide;
    int row_h, pad, gap, fs;
    lv_obj_t *root, *wave, *ovs, *win, *tempo, *drift, *hint, *beat_lbl[4], *btn[ROWS][4];
    ui_next_gridedit_model_t m;
} G;

static uint32_t amber_text(void) { return G.pal->accent_ink; }

static void wave_cb(lv_event_t *e)
{
    lv_layer_t *l = lv_event_get_layer(e);
    lv_area_t a;
    lv_obj_get_coords(lv_event_get_target_obj(e), &a);
    const int w = lv_area_get_width(&a);
    if (!G.m.wave[0]) return;
    ui_next_chrome_draw_profile(l, &a, G.m.wave, UI_NEXT_GRID_COLS, G.pal);
    /* Beat lines: white at 35 %, the first beat of each bar in amber and 2 px wide. */
    for (int c = G.m.first_beat % 9; c < UI_NEXT_GRID_COLS; c += 9) {
        const int x = a.x1 + c * w / UI_NEXT_GRID_COLS;
        const bool bar = (c - G.m.first_beat) % 36 == 0;
        if (bar) ui_next_fill_rect(l, x, a.y1, x + 1, a.y2, G.pal->accent, LV_OPA_COVER);
        else ui_next_fill_rect(l, x, a.y1, x, a.y2, G.pal->text, 89);
    }
    const int px = a.x1 + w / 2;
    ui_next_fill_rect(l, px, a.y1, px + 1, a.y2, G.pal->text, LV_OPA_COVER);
}

static void ovs_cb(lv_event_t *e)
{
    lv_area_t a;
    lv_obj_get_coords(lv_event_get_target_obj(e), &a);
    ui_next_chrome_draw_overview_ex(lv_event_get_layer(e), &a, G.m.env, -1.0f, false, false, G.pal->head, G.pal);
}

lv_obj_t *ui_next_gridedit_create(lv_obj_t *parent, uint16_t w, uint16_t h, bool day)
{
    memset(&G, 0, sizeof G);
    if (!ui_next_layout_compute(w, h, &G.lay)) return NULL;
    G.day = day;
    G.pal = ui_next_palette(day);
    G.f = ui_next_fonts_for(G.lay.cls);
    G.wide = G.lay.landscape && w >= 1000u;
    G.fs = G.lay.tok.text_px;
    G.row_h = G.lay.tok.row_px;
    G.pad = G.lay.tok.pad_px;
    G.gap = G.lay.tok.gap_px;
    ui_next_chrome_begin(&G.lay, day, UI_NEXT_PAGE_PERFORM);

    G.root = ui_next_box(parent, 0, 0, w, h, G.pal->frame);
    const int x0 = G.lay.dock.x + G.pad, y0 = G.lay.top_bar.h + G.pad;
    const int iw = G.lay.dock.w - 2 * G.pad, ih = G.lay.dock.y - G.lay.top_bar.h - 2 * G.pad;
    const int gr_h = G.row_h * 72 / 100, ovs_h = G.row_h * 7 / 10;
    const int gt_h = ROWS * gr_h + (ROWS - 1) * G.gap;
    int gw_h = ih - ovs_h - gt_h - 2 * G.pad;
    if (gw_h < 80) gw_h = 80;

    G.wave = ui_next_box(G.root, x0, y0, iw, gw_h, G.pal->well);
    lv_obj_set_style_radius(G.wave, 8, 0);
    lv_obj_set_style_clip_corner(G.wave, true, 0);
    lv_obj_add_event_cb(G.wave, wave_cb, LV_EVENT_DRAW_MAIN, NULL);
    /* Beat numbers: 1 to 4 at the bar lines, 5 px right of each. */
    for (int i = 0; i < 4; i++) {
        G.beat_lbl[i] = ui_next_label(G.wave, "", G.f->mono, amber_text());
        char n[2] = {(char)('1' + i), 0};
        lv_label_set_text(G.beat_lbl[i], n);
    }
    G.hint = ui_next_label(G.wave, "", G.f->micro, amber_text());
    lv_obj_align(G.hint, LV_ALIGN_BOTTOM_LEFT, 8, -6);
    lv_obj_set_style_text_letter_space(G.hint, 1, 0);

    const int oy = y0 + gw_h + G.pad;
    G.ovs = ui_next_box(G.root, x0, oy, iw, ovs_h, G.pal->well);
    lv_obj_set_style_radius(G.ovs, 4, 0);
    lv_obj_set_style_clip_corner(G.ovs, true, 0);
    lv_obj_add_event_cb(G.ovs, ovs_cb, LV_EVENT_DRAW_MAIN, NULL);
    G.win = ui_next_box(G.ovs, 0, 0, 10, ovs_h, G.pal->accent);
    lv_obj_set_style_bg_opa(G.win, 38, 0); /* rgba(255,193,90,.15) */
    lv_obj_set_style_border_width(G.win, 2, 0);
    lv_obj_set_style_border_color(G.win, ui_next_col(G.pal->accent), 0);

    /* Four rows of buttons under a mono label. */
    const int ly = oy + ovs_h + G.pad, lw = G.fs * 5;
    for (int r = 0; r < ROWS; r++) {
        const int y = ly + r * (gr_h + G.gap);
        lv_obj_t *l = ui_next_label(G.root, ROW_LABEL[r], G.f->mono, G.pal->muted);
        lv_obj_set_pos(l, x0, y + (gr_h - G.f->mono->line_height) / 2);
        const int n = r == 3 ? 2 : 4, bx = x0 + lw + G.gap, bw_total = iw - lw - G.gap;
        const int bw = (bw_total - (n - 1) * G.gap) / n;
        for (int k = 0; k < n; k++)
            G.btn[r][k] = ui_next_chrome_action(G.root, bx + k * (bw + G.gap), y, k == n - 1 ? bw_total - k * (bw + G.gap) : bw,
                                                gr_h, ROW_BTN[r][k], false);
    }

    ui_next_chrome_dock(G.root, 0, false);
    ui_next_chrome_dock(G.root, 1, false);
    lv_obj_t *row = ui_next_chrome_topbar(G.root);
    ui_next_chrome_button(row, G.wide ? "BACK" : NULL, UI_NEXT_ICON_CHEVRON, false);
    ui_next_label(row, "Edit grid", G.f->med, G.pal->text);
    lv_obj_t *sp = lv_obj_create(row);
    lv_obj_remove_style_all(sp);
    lv_obj_set_size(sp, 1, 1);
    lv_obj_set_flex_grow(sp, 1);
    G.tempo = ui_next_chrome_chip(row, "");
    if (G.wide) G.drift = ui_next_chrome_chip(row, "");
    if (G.wide) ui_next_chrome_button(row, "CANCEL", -1, false);
    ui_next_chrome_button(row, "SAVE", -1, true);
    ui_next_chrome_nav(G.root);
    return G.root;
}

void ui_next_gridedit_update(const ui_next_gridedit_model_t *m)
{
    if (!G.root) return;
    G.m = *m;
    lv_label_set_text(lv_obj_get_child(G.tempo, 0), m->tempo);
    if (G.drift) lv_label_set_text(lv_obj_get_child(G.drift, 0), m->drift);
    lv_label_set_text(G.hint, m->hint);
    const int w = lv_obj_get_width(G.wave);
    for (int i = 0; i < 4; i++)
        lv_obj_set_pos(G.beat_lbl[i], (m->first_beat + 36 * i) * w / UI_NEXT_GRID_COLS + 5, 4);
    lv_obj_invalidate(G.wave);
    lv_obj_invalidate(G.ovs);
    const int ow = lv_obj_get_width(G.ovs);
    lv_obj_set_x(G.win, (int)(m->window_x01 * (float)ow));
    lv_obj_set_width(G.win, (int)(m->window_w01 * (float)ow));
    for (int r = 0; r < ROWS; r++) {
        for (int k = 0; k < (r == 3 ? 2 : 4); k++) {
            const bool on = r == m->pressed_row && k == m->pressed_col;
            lv_obj_set_style_bg_color(G.btn[r][k], ui_next_col(on ? G.pal->accent : G.pal->chip), 0);
            lv_obj_set_style_border_color(G.btn[r][k], ui_next_col(on ? G.pal->accent : G.pal->border_hi), 0);
            lv_obj_set_style_text_color(lv_obj_get_child(G.btn[r][k], 0), ui_next_col(on ? G.pal->on_accent : G.pal->text), 0);
        }
    }
    ui_next_chrome_dock_set(0, &m->deck[0]);
    ui_next_chrome_dock_set(1, &m->deck[1]);
}
