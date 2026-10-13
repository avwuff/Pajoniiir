#include <stdio.h>
#include <string.h>

#include "ui_next.h"
#include "ui_next_chrome.h"
#include "ui_next_util.h"

/* Settings pages (screens 10 to 12). One skeleton: three tabs over a grid of
 * rows. Audio and System use two columns in landscape, Display one column and a
 * colour panel beside (landscape) or below (portrait) the list. Rows that do
 * not fit are cut and fade out at the bottom. */

typedef struct {
    ui_next_setting_kind_t kind;
    lv_obj_t *row, *opt[4], *opt_txt[4], *fill, *knob, *track, *value, *tg, *tg_knob, *dot, *button;
    int track_w, n_opt;
} row_t;

static struct {
    ui_next_layout_t lay;
    const ui_next_palette_t *pal;
    const ui_next_fonts_t *f;
    bool day, landscape, wide, panel_presets, panel_preview;
    int row_h, pad, gap, fs;
    lv_obj_t *root, *tab[3], *swatch[3], *swatch_hex[3], *saved, *preview, *preset[4];
    row_t row[UI_NEXT_SET_MAX];
    int n;
    ui_next_settings_model_t m;
} T;

static const char *const TAB_NAME[3] = {"AUDIO", "DISPLAY", "SYSTEM"};

static uint32_t off_color(void) { return T.day ? 0x8A94A3 : 0x6B7482; }

static lv_obj_t *flex_row(lv_obj_t *parent, int w, int h, int gap)
{
    lv_obj_t *o = lv_obj_create(parent);
    lv_obj_remove_style_all(o);
    lv_obj_remove_flag(o, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_size(o, w, h);
    lv_obj_set_flex_flow(o, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(o, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(o, gap, 0);
    return o;
}

static void build_row(lv_obj_t *grid, int i, int x, int y, int w, const ui_next_setting_t *s)
{
    row_t *r = &T.row[i];
    r->kind = s->kind;
    const int h = T.row_h * 95 / 100;
    r->row = ui_next_box(grid, x, y, w, h, T.pal->panel);
    lv_obj_set_style_radius(r->row, 10, 0);
    lv_obj_set_style_border_width(r->row, 1, 0);
    lv_obj_set_style_border_color(r->row, ui_next_col(T.pal->border), 0);
    lv_obj_t *flow = flex_row(r->row, w - 2, h - 2, T.gap);
    lv_obj_set_style_pad_hor(flow, T.pad - 1, 0);

    lv_obj_t *lab = ui_next_label(flow, s->label, T.f->row, T.pal->text);
    lv_obj_set_flex_grow(lab, 1);
    lv_label_set_long_mode(lab, LV_LABEL_LONG_MODE_DOTS);
    const int ch = T.row_h * 62 / 100;

    if (s->kind == UI_NEXT_SET_SEG) {
        lv_obj_t *seg = flex_row(flow, LV_SIZE_CONTENT, ch, 0);
        lv_obj_set_style_border_width(seg, 1, 0);
        lv_obj_set_style_border_color(seg, ui_next_col(T.pal->border_hi), 0);
        lv_obj_set_style_radius(seg, 8, 0);
        lv_obj_set_style_clip_corner(seg, true, 0);
        lv_obj_set_style_bg_opa(seg, LV_OPA_COVER, 0);
        lv_obj_set_style_bg_color(seg, ui_next_col(T.pal->chip), 0);
        r->n_opt = s->n_opt;
        for (int k = 0; k < s->n_opt; k++) {
            r->opt[k] = ui_next_box(seg, 0, 0, 10, ch - 2, T.pal->chip);
            if (k) {
                lv_obj_set_style_border_side(r->opt[k], LV_BORDER_SIDE_LEFT, 0);
                lv_obj_set_style_border_width(r->opt[k], 1, 0);
                lv_obj_set_style_border_color(r->opt[k], ui_next_col(T.pal->border_hi), 0);
            }
            r->opt_txt[k] = ui_next_label(r->opt[k], s->opt[k], T.f->tagf, T.pal->text);
            lv_obj_update_layout(r->opt_txt[k]);
            lv_obj_set_width(r->opt[k], lv_obj_get_width(r->opt_txt[k]) + 2 * (T.pad * 9 / 10) + (k ? 1 : 0));
            lv_obj_center(r->opt_txt[k]);
        }
    } else if (s->kind == UI_NEXT_SET_SLIDER) {
        r->track_w = (w - 2) * 34 / 100;
        lv_obj_t *hold = ui_next_box(flow, 0, 0, r->track_w, 20, T.pal->panel);
        lv_obj_set_style_bg_opa(hold, LV_OPA_TRANSP, 0);
        r->track = ui_next_box(hold, 0, 7, r->track_w, 6, T.pal->border);
        lv_obj_set_style_radius(r->track, 3, 0);
        r->fill = ui_next_box(r->track, 0, 0, 0, 6, T.pal->accent);
        lv_obj_set_style_radius(r->fill, 3, 0);
        r->knob = ui_next_box(hold, 0, 0, 20, 20, T.pal->text);
        lv_obj_set_style_radius(r->knob, LV_RADIUS_CIRCLE, 0);
        r->value = ui_next_label(flow, "", T.f->mono, T.pal->text);
        lv_obj_set_width(r->value, T.f->mono->line_height * 34 / 10);
        lv_obj_set_style_text_align(r->value, LV_TEXT_ALIGN_RIGHT, 0);
    } else if (s->kind == UI_NEXT_SET_TOGGLE) {
        const int tw = T.row_h * 8 / 10, th = T.row_h * 46 / 100;
        r->tg = ui_next_box(flow, 0, 0, tw, th, T.pal->border);
        lv_obj_set_style_radius(r->tg, LV_RADIUS_CIRCLE, 0);
        r->tg_knob = ui_next_box(r->tg, 3, 3, th - 6, th - 6, T.pal->muted);
        lv_obj_set_style_radius(r->tg_knob, LV_RADIUS_CIRCLE, 0);
    } else if (s->kind == UI_NEXT_SET_STATUS) {
        r->dot = ui_next_box(flow, 0, 0, 8, 8, T.pal->ok);
        lv_obj_set_style_radius(r->dot, LV_RADIUS_CIRCLE, 0);
        r->value = ui_next_label(flow, "", T.f->mono, T.pal->text);
    } else {
        r->button = ui_next_chrome_action(flow, 0, 0, T.row_h * 22 / 10, ch, s->value, false);
        if (s->danger) {
            lv_obj_set_style_bg_color(r->button, ui_next_col(T.day ? 0xFBE3E3 : 0x2A1A1D), 0);
            lv_obj_set_style_border_color(r->button, ui_next_col(0x8A3F3F), 0);
            lv_obj_set_style_text_color(lv_obj_get_child(r->button, 0), ui_next_col(T.day ? 0xB42318 : 0xFF9A9A), 0);
        }
    }
}

/* Colour panel of the Display tab. */
static void preview_cb(lv_event_t *e)
{
    lv_area_t a;
    lv_obj_get_coords(lv_event_get_target_obj(e), &a);
    if (T.m.preview[0]) ui_next_chrome_draw_profile(lv_event_get_layer(e), &a, T.m.preview, UI_NEXT_REC_COLS, T.pal);
}

static void build_color_panel(int x, int y, int w, int h)
{
    lv_obj_t *p = ui_next_box(T.root, x, y, w, h, T.pal->panel);
    lv_obj_set_style_radius(p, 12, 0);
    lv_obj_set_style_border_width(p, 1, 0);
    lv_obj_set_style_border_color(p, ui_next_col(T.pal->border), 0);
    const int ix = T.pad - 1, iw = w - 2 * T.pad, lh = T.f->mono->line_height;
    int cy = T.pad - 1;
    lv_obj_t *t = ui_next_label(p, "WAVEFORM COLORS", T.f->micro, T.pal->muted);
    lv_obj_set_pos(t, ix, cy);
    cy += lh + T.gap;
    const int sw = T.row_h * 55 / 100;
    for (int i = 0; i < 3; i++) {
        T.swatch[i] = ui_next_box(p, ix, cy, sw, sw, T.pal->wave_low);
        lv_obj_set_style_radius(T.swatch[i], 8, 0);
        lv_obj_t *nm = ui_next_label(p, "", T.f->row, T.pal->text);
        lv_obj_set_pos(nm, ix + sw + T.gap, cy + (sw - T.f->row->line_height) / 2);
        T.swatch_hex[i] = ui_next_label(p, "", T.f->mono, T.pal->muted);
        lv_obj_set_width(T.swatch_hex[i], iw / 3);
        lv_obj_set_style_text_align(T.swatch_hex[i], LV_TEXT_ALIGN_RIGHT, 0);
        lv_obj_set_pos(T.swatch_hex[i], ix + iw - iw / 3, cy + (sw - lh) / 2);
        lv_obj_set_user_data(T.swatch[i], nm);
        cy += sw + T.gap;
    }
    const int bh = T.row_h * 85 / 100;
    const int pv_min = T.row_h * 9 / 10;
    int pv_h = pv_min;
    if (T.landscape) pv_h = h - 2 - (T.pad - 1) - cy - (T.panel_presets ? 2 * bh + T.gap + T.gap : 0);
    if (pv_h < pv_min) pv_h = pv_min;
    if (T.panel_preview) {
        T.preview = ui_next_box(p, ix, cy, iw, pv_h, T.pal->well);
        lv_obj_set_style_radius(T.preview, 4, 0);
        lv_obj_set_style_clip_corner(T.preview, true, 0);
        lv_obj_add_event_cb(T.preview, preview_cb, LV_EVENT_DRAW_MAIN, NULL);
        cy += pv_h + T.gap;
    }
    if (T.panel_presets) {
        const int bw = (iw - T.gap) / 2;
        for (int i = 0; i < 4; i++)
            T.preset[i] = ui_next_chrome_action(p, ix + (i % 2) * (bw + T.gap), cy + (i / 2) * (bh + T.gap),
                                                i % 2 ? iw - bw - T.gap : bw, bh, "", false);
    }
}

static int color_panel_height(void)
{
    const int lh = T.f->mono->line_height, sw = T.row_h * 55 / 100, bh = T.row_h * 85 / 100;
    int h = 2 * T.pad + lh + T.gap + 3 * (sw + T.gap);
    if (T.panel_preview) h += T.row_h * 9 / 10 + T.gap;
    if (T.panel_presets) h += 2 * bh + T.gap;
    return h;
}

lv_obj_t *ui_next_settings_create(lv_obj_t *parent, uint16_t w, uint16_t h, bool day, const ui_next_settings_model_t *m)
{
    memset(&T, 0, sizeof T);
    if (!ui_next_layout_compute(w, h, &T.lay)) return NULL;
    T.day = day;
    T.pal = ui_next_palette(day);
    T.f = ui_next_fonts_for(T.lay.cls);
    T.landscape = T.lay.landscape;
    T.wide = T.landscape && w >= 1000u;
    T.fs = T.lay.tok.text_px;
    T.row_h = T.lay.tok.row_px;
    T.pad = T.lay.tok.pad_px;
    T.gap = T.lay.tok.gap_px;
    /* x2: presets, not on 800x480 nor 480x800. x3: preview, not on 480x800. */
    T.panel_presets = (w > 800u || h > 800u) && !(w == 480u);
    T.panel_presets = T.lay.cls != UI_NEXT_CLASS_S;
    T.panel_preview = !(w == 480u && h == 800u);
    ui_next_chrome_begin(&T.lay, day, UI_NEXT_PAGE_SETTINGS);

    T.root = ui_next_box(parent, 0, 0, w, h, T.pal->frame);
    const int x0 = T.lay.dock.x + T.pad, y0 = T.lay.top_bar.h + T.pad;
    const int iw = T.lay.dock.w - 2 * T.pad;
    const int body_bottom = T.lay.dock.y - T.pad;

    /* Tabs. */
    const int th = T.row_h * 8 / 10, tw = (iw - 2 * T.gap) / 3;
    for (int i = 0; i < 3; i++) {
        T.tab[i] = ui_next_box(T.root, x0 + i * (tw + T.gap), y0, i == 2 ? iw - 2 * (tw + T.gap) : tw, th, T.pal->panel);
        lv_obj_set_style_radius(T.tab[i], 10, 0);
        lv_obj_set_style_border_width(T.tab[i], 1, 0);
        lv_obj_set_style_border_color(T.tab[i], ui_next_col(T.pal->border), 0);
        lv_obj_t *l = ui_next_label(T.tab[i], TAB_NAME[i], T.f->tagf, T.pal->muted);
        lv_obj_set_style_text_letter_space(l, 1, 0);
        lv_obj_center(l);
    }

    /* Body: list column (and the colour panel on Display). */
    const int by = y0 + th + T.pad, bh = body_bottom - by;
    const bool display = m->tab == UI_NEXT_TAB_DISPLAY;
    int lx = x0, ly = by, lw = iw, lh = bh;
    int px = 0, py = 0, pw = 0, ph = 0;
    if (display && T.landscape) {
        lw = (iw - T.pad) * 115 / 215;
        px = x0 + lw + T.pad;
        py = by;
        pw = iw - lw - T.pad;
        ph = bh;
    } else if (display) {
        ph = color_panel_height();
        lh = bh - ph - T.pad;
        px = x0;
        py = by + lh + T.pad;
        pw = iw;
    }
    const int cols = T.landscape && !display ? 2 : 1;
    lv_obj_t *grid = ui_next_box(T.root, lx, ly, lw, lh, T.pal->frame);
    lv_obj_set_style_bg_opa(grid, LV_OPA_TRANSP, 0);
    const int cw = (lw - (cols - 1) * T.gap) / cols, rh = T.row_h * 95 / 100;
    T.n = m->n < UI_NEXT_SET_MAX ? m->n : UI_NEXT_SET_MAX;
    for (int i = 0; i < T.n; i++) {
        const int c = i % cols, rr = i / cols;
        build_row(grid, i, c * (cw + T.gap), rr * (rh + T.gap), c == cols - 1 ? lw - c * (cw + T.gap) : cw, &m->row[i]);
    }
    /* Scroll thumb and the fade at the bottom edge. */
    lv_obj_t *sb = ui_next_box(grid, lw - 3, 0, 3, lh * 38 / 100, T.pal->border_hi);
    lv_obj_set_style_radius(sb, 2, 0);
    static lv_grad_dsc_t grad;
    const lv_color_t cols2[2] = {ui_next_col(T.pal->frame), ui_next_col(T.pal->frame)};
    const lv_opa_t opas[2] = {0, 255};
    const uint8_t fracs[2] = {0, 255};
    lv_grad_init_stops(&grad, cols2, opas, fracs, 2);
    lv_grad_vertical_init(&grad);
    lv_obj_t *fade = lv_obj_create(grid);
    lv_obj_remove_style_all(fade);
    lv_obj_remove_flag(fade, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_size(fade, lw, T.row_h / 2);
    lv_obj_set_pos(fade, 0, lh - T.row_h / 2);
    lv_obj_set_style_bg_grad(fade, &grad, 0);

    if (display) build_color_panel(px, py, pw, ph);

    ui_next_chrome_dock(T.root, 0, false);
    ui_next_chrome_dock(T.root, 1, false);
    lv_obj_t *row = ui_next_chrome_topbar(T.root);
    ui_next_chrome_button(row, T.wide ? "BACK" : NULL, UI_NEXT_ICON_CHEVRON, false);
    ui_next_label(row, "Settings", T.f->med, T.pal->text);
    lv_obj_t *sp = lv_obj_create(row);
    lv_obj_remove_style_all(sp);
    lv_obj_set_size(sp, 1, 1);
    lv_obj_set_flex_grow(sp, 1);
    if (T.wide) T.saved = ui_next_chrome_chip(row, "");
    ui_next_chrome_nav(T.root);
    return T.root;
}

static void style_opt(lv_obj_t *o, lv_obj_t *txt, bool on)
{
    lv_obj_set_style_bg_color(o, ui_next_col(on ? T.pal->accent : T.pal->chip), 0);
    lv_obj_set_style_text_color(txt, ui_next_col(on ? T.pal->on_accent : T.pal->text), 0);
}

void ui_next_settings_update(const ui_next_settings_model_t *m)
{
    if (!T.root) return;
    T.m = *m;
    for (int i = 0; i < 3; i++) {
        const bool on = i == (int)m->tab;
        lv_obj_set_style_bg_color(T.tab[i], ui_next_col(on ? T.pal->accent : T.pal->panel), 0);
        lv_obj_set_style_border_color(T.tab[i], ui_next_col(on ? T.pal->accent : T.pal->border), 0);
        lv_obj_set_style_text_color(lv_obj_get_child(T.tab[i], 0), ui_next_col(on ? T.pal->on_accent : T.pal->muted), 0);
    }
    if (T.saved) lv_label_set_text(lv_obj_get_child(T.saved, 0), m->saved);
    for (int i = 0; i < T.n; i++) {
        row_t *r = &T.row[i];
        const ui_next_setting_t *s = &m->row[i];
        if (r->kind == UI_NEXT_SET_SEG) {
            for (int k = 0; k < r->n_opt; k++) style_opt(r->opt[k], r->opt_txt[k], k == s->sel);
        } else if (r->kind == UI_NEXT_SET_SLIDER) {
            lv_obj_set_width(r->fill, r->track_w * s->pct / 100);
            lv_obj_set_x(r->knob, r->track_w * s->pct / 100 - 10);
            lv_label_set_text(r->value, s->value);
        } else if (r->kind == UI_NEXT_SET_TOGGLE) {
            lv_obj_set_style_bg_color(r->tg, ui_next_col(s->on ? T.pal->accent : T.pal->border), 0);
            lv_obj_set_style_bg_color(r->tg_knob, ui_next_col(s->on ? T.pal->on_accent : T.pal->muted), 0);
            lv_obj_set_x(r->tg_knob, s->on ? lv_obj_get_width(r->tg) - lv_obj_get_width(r->tg_knob) - 3 : 3);
        } else if (r->kind == UI_NEXT_SET_STATUS) {
            lv_obj_set_style_bg_color(r->dot, ui_next_col(s->on ? T.pal->ok : off_color()), 0);
            lv_label_set_text(r->value, s->value);
        }
    }
    for (int i = 0; i < 3 && T.swatch[i]; i++) {
        lv_obj_set_style_bg_color(T.swatch[i], ui_next_col(m->wave_color[i].color), 0);
        const bool on = i == m->color_sel;
        lv_obj_set_style_outline_width(T.swatch[i], on ? 2 : 0, 0);
        lv_obj_set_style_outline_color(T.swatch[i], ui_next_col(T.pal->text), 0);
        lv_obj_set_style_outline_pad(T.swatch[i], on ? 2 : 0, 0);
        lv_label_set_text(lv_obj_get_user_data(T.swatch[i]), m->wave_color[i].name);
        lv_label_set_text(T.swatch_hex[i], m->wave_color[i].hex);
    }
    if (T.preview) lv_obj_invalidate(T.preview);
    for (int i = 0; i < 4 && T.preset[i]; i++) {
        lv_label_set_text(lv_obj_get_child(T.preset[i], 0), m->preset[i]);
        const bool on = i == m->preset_sel;
        lv_obj_set_style_bg_color(T.preset[i], ui_next_col(on ? T.pal->accent : T.pal->chip), 0);
        lv_obj_set_style_border_color(T.preset[i], ui_next_col(on ? T.pal->accent : T.pal->border_hi), 0);
        lv_obj_set_style_text_color(lv_obj_get_child(T.preset[i], 0), ui_next_col(on ? T.pal->on_accent : T.pal->text), 0);
    }
    ui_next_chrome_dock_set(0, &m->deck[0]);
    ui_next_chrome_dock_set(1, &m->deck[1]);
}
