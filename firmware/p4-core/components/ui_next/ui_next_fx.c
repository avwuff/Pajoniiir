#include <stdio.h>
#include <string.h>

#include "ui_next.h"
#include "ui_next_chrome.h"
#include "ui_next_util.h"

/* FX page (screen 07): three effect slots, an XY pad and the echo panel.
 * Landscape puts the slot column beside the panel; portrait stacks them (1 : 1.2). */

#define GAP6 6

static const char *const BEATS[4] = {"1/4", "1/2", "1", "2"};

typedef struct {
    lv_obj_t *track, *knob, *name, *fill, *text;
} slot_t;

static struct {
    ui_next_layout_t lay;
    const ui_next_palette_t *pal;
    const ui_next_fonts_t *f;
    bool day, landscape;
    int row_h, pad, gap, fs, xy_x, xy_y, xy_w, xy_h, bar_w[UI_NEXT_FX_SLOTS];
    lv_obj_t *root, *deck_btn[2], *xy, *panel_title, *beat[4], *wet_v, *fb_v, *wet_fill, *fb_fill, *hold;
    lv_obj_t *wet_track, *fb_track, *xy_lx, *xy_ly;
    slot_t slot[UI_NEXT_FX_SLOTS];
    ui_next_fx_model_t m;
} X;

static uint32_t off_fill(void) { return X.day ? 0x8A94A3 : 0x6B7482; }

static void xy_draw_cb(lv_event_t *e)
{
    lv_layer_t *l = lv_event_get_layer(e);
    lv_area_t a;
    lv_obj_get_coords(lv_event_get_target_obj(e), &a);
    const int w = lv_area_get_width(&a), h = lv_area_get_height(&a);
    const uint32_t grid = X.day ? 0xE3E8EE : 0x1C232D;
    for (int i = 0; i < 4; i++) {
        ui_next_fill_rect(l, a.x1, a.y1 + h * i / 4, a.x2, a.y1 + h * i / 4, grid, LV_OPA_COVER);
        ui_next_fill_rect(l, a.x1 + w * i / 4, a.y1, a.x1 + w * i / 4, a.y2, grid, LV_OPA_COVER);
    }
    const int px = a.x1 + (int)(X.m.xy_x * (float)w), py = a.y1 + (int)(X.m.xy_y * (float)h);
    ui_next_fill_rect(l, a.x1, py, a.x2, py, X.pal->accent, 128);
    ui_next_fill_rect(l, px, a.y1, px, a.y2, X.pal->accent, 128);
    lv_draw_rect_dsc_t d;
    lv_draw_rect_dsc_init(&d);
    d.radius = LV_RADIUS_CIRCLE;
    d.bg_color = ui_next_col(X.pal->accent);
    d.bg_opa = 64; /* halo, rgba(255,193,90,.25) */
    lv_area_t halo = {px - 16, py - 16, px + 16, py + 16};
    lv_draw_rect(l, &d, &halo);
    d.bg_opa = LV_OPA_COVER;
    lv_area_t puck = {px - 11, py - 11, px + 10, py + 10};
    lv_draw_rect(l, &d, &puck);
}

static void build_slot(int i, int x, int y, int w, int h)
{
    slot_t *s = &X.slot[i];
    lv_obj_t *row = ui_next_box(X.root, x, y, w, h, X.pal->panel);
    lv_obj_set_style_radius(row, 10, 0);
    lv_obj_set_style_border_width(row, 1, 0);
    lv_obj_set_style_border_color(row, ui_next_col(X.pal->border), 0);

    const int tw = X.row_h * 8 / 10, th = X.row_h * 46 / 100, kn = th - 6;
    s->track = ui_next_box(row, X.pad - 1, (h - 2 - th) / 2, tw, th, X.pal->border);
    lv_obj_set_style_radius(s->track, LV_RADIUS_CIRCLE, 0);
    s->knob = ui_next_box(s->track, 3, 3, kn, kn, X.pal->muted);
    lv_obj_set_style_radius(s->knob, LV_RADIUS_CIRCLE, 0);

    const int tx = X.pad - 1 + tw + X.gap;
    s->text = ui_next_label(row, "", X.f->mono, X.pal->text);
    lv_obj_update_layout(s->text);
    s->name = ui_next_label(row, "", X.f->row, X.pal->text);
    const int block = X.f->row->line_height + 4 + 5, by = (h - 2 - block) / 2;
    lv_obj_set_pos(s->name, tx, by);
    /* The value sits at the right edge; reserve the widest text, "HP 65 %". */
    const int vw = X.f->mono->line_height * 6;
    lv_obj_set_pos(s->text, w - 2 - X.pad - vw, (h - 2 - X.f->mono->line_height) / 2);
    lv_obj_set_width(s->text, vw);
    lv_obj_set_style_text_align(s->text, LV_TEXT_ALIGN_RIGHT, 0);
    const int bw = w - 2 - tx - X.pad - vw - X.gap;
    X.bar_w[i] = bw;
    lv_obj_t *bar = ui_next_box(row, tx, by + X.f->row->line_height + 4, bw, 5, X.pal->border);
    lv_obj_set_style_radius(bar, 3, 0);
    s->fill = ui_next_box(bar, 0, 0, 0, 5, X.pal->accent);
    lv_obj_set_style_radius(s->fill, 3, 0);
}

static lv_obj_t *wd_block(lv_obj_t *p, int x, int y, int w, const char *label, lv_obj_t **val, lv_obj_t **fill,
                          lv_obj_t **track)
{
    ui_next_label(p, label, X.f->mono, X.pal->muted);
    lv_obj_set_pos(lv_obj_get_child(p, lv_obj_get_child_count(p) - 1), x, y);
    *val = ui_next_label(p, "", X.f->mono, X.pal->text);
    lv_obj_set_width(*val, w);
    lv_obj_set_pos(*val, x, y);
    lv_obj_set_style_text_align(*val, LV_TEXT_ALIGN_RIGHT, 0);
    const int bh = X.row_h * 34 / 100;
    *track = ui_next_box(p, x, y + X.f->mono->line_height + GAP6, w, bh, X.pal->border);
    lv_obj_set_style_radius(*track, LV_RADIUS_CIRCLE, 0);
    *fill = ui_next_box(*track, 0, 0, 0, bh, X.pal->accent);
    lv_obj_set_style_radius(*fill, LV_RADIUS_CIRCLE, 0);
    return *track;
}

static void build_panel(int x, int y, int w, int h)
{
    lv_obj_t *p = ui_next_box(X.root, x, y, w, h, X.pal->panel);
    lv_obj_set_style_radius(p, 12, 0);
    lv_obj_set_style_border_width(p, 1, 0);
    lv_obj_set_style_border_color(p, ui_next_col(X.pal->border), 0);
    const int ix = X.pad - 1, iw = w - 2 * X.pad, lh = X.f->mono->line_height;
    int cy = X.pad - 1;

    X.panel_title = ui_next_label(p, "", X.f->mono, X.pal->muted);
    lv_obj_set_pos(X.panel_title, ix, cy);
    cy += lh + X.gap;
    lv_obj_t *tl = ui_next_label(p, "TIME (BEATS)", X.f->mono, X.pal->muted);
    lv_obj_set_pos(tl, ix, cy);
    cy += lh + X.gap;

    const int bh = X.lay.tok.top_bar_px - 8, bw = (iw - 3 * GAP6) / 4;
    for (int i = 0; i < 4; i++) {
        X.beat[i] = ui_next_chrome_action(p, ix + i * (bw + GAP6), cy, i == 3 ? iw - 3 * (bw + GAP6) : bw, bh, BEATS[i], false);
    }
    cy += bh + X.gap;

    wd_block(p, ix, cy, iw, "WET / DRY", &X.wet_v, &X.wet_fill, &X.wet_track);
    cy += lh + GAP6 + X.row_h * 34 / 100 + X.gap;
    wd_block(p, ix, cy, iw, "FEEDBACK", &X.fb_v, &X.fb_fill, &X.fb_track);

    const int ah = X.row_h * 85 / 100, aw = (iw - X.gap) / 2, ay = h - 2 - (X.pad - 1) - ah;
    X.hold = ui_next_chrome_action(p, ix, ay, aw, ah, "HOLD", false);
    ui_next_chrome_action(p, ix + aw + X.gap, ay, iw - aw - X.gap, ah, "RESET", false);
}

lv_obj_t *ui_next_fx_create(lv_obj_t *parent, uint16_t w, uint16_t h, bool day)
{
    memset(&X, 0, sizeof X);
    if (!ui_next_layout_compute(w, h, &X.lay)) return NULL;
    X.day = day;
    X.pal = ui_next_palette(day);
    X.f = ui_next_fonts_for(X.lay.cls);
    X.landscape = X.lay.landscape;
    X.fs = X.lay.tok.text_px;
    X.row_h = X.lay.tok.row_px;
    X.pad = X.lay.tok.pad_px;
    X.gap = X.lay.tok.gap_px;
    ui_next_chrome_begin(&X.lay, day, UI_NEXT_PAGE_FX);

    X.root = ui_next_box(parent, 0, 0, w, h, X.pal->frame);
    const int x0 = X.lay.dock.x + X.pad, y0 = X.lay.top_bar.h + X.pad;
    const int iw = X.lay.dock.w - 2 * X.pad, ih = X.lay.dock.y - X.lay.top_bar.h - 2 * X.pad;
    int c1w, c1h, px, py, pw, ph;
    if (X.landscape) {
        c1w = (iw - X.pad) / 2;
        c1h = ih;
        px = x0 + c1w + X.pad;
        py = y0;
        pw = iw - c1w - X.pad;
        ph = ih;
    } else {
        c1w = iw;
        c1h = (ih - X.pad) * 10 / 22;
        px = x0;
        py = y0 + c1h + X.pad;
        pw = iw;
        ph = ih - c1h - X.pad;
    }

    /* Three slots (flex 1, at least 1.05 rows) and the XY pad (flex 2.2, at least 60). */
    const int free_h = c1h - 3 * GAP6, min_row = X.row_h * 105 / 100;
    int rh = free_h * 10 / 52;
    if (rh < min_row) rh = min_row;
    const int xyh = free_h - 3 * rh;
    for (int i = 0; i < UI_NEXT_FX_SLOTS; i++) build_slot(i, x0, y0 + i * (rh + GAP6), c1w, rh);
    X.xy_x = x0;
    X.xy_y = y0 + 3 * (rh + GAP6);
    X.xy_w = c1w;
    X.xy_h = xyh;
    X.xy = ui_next_box(X.root, X.xy_x, X.xy_y, X.xy_w, X.xy_h, X.pal->well);
    lv_obj_set_style_radius(X.xy, 10, 0);
    lv_obj_set_style_clip_corner(X.xy, true, 0);
    lv_obj_set_style_border_width(X.xy, 1, 0);
    lv_obj_set_style_border_color(X.xy, ui_next_col(X.pal->border), 0);
    lv_obj_add_event_cb(X.xy, xy_draw_cb, LV_EVENT_DRAW_MAIN, NULL);
    X.xy_ly = ui_next_label(X.xy, "", X.f->micro, X.pal->muted);
    lv_obj_set_pos(X.xy_ly, 8, 6);
    X.xy_lx = ui_next_label(X.xy, "", X.f->micro, X.pal->muted);
    lv_obj_align(X.xy_lx, LV_ALIGN_BOTTOM_RIGHT, -8, -6);

    build_panel(px, py, pw, ph);
    ui_next_chrome_dock(X.root, 0, false);
    ui_next_chrome_dock(X.root, 1, false);

    lv_obj_t *row = ui_next_chrome_topbar(X.root);
    ui_next_label(row, "FX", X.f->med, X.pal->text);
    lv_obj_t *sp = lv_obj_create(row);
    lv_obj_remove_style_all(sp);
    lv_obj_set_size(sp, 1, 1);
    lv_obj_set_flex_grow(sp, 1);
    X.deck_btn[0] = ui_next_chrome_button(row, "DECK A", -1, true);
    X.deck_btn[1] = ui_next_chrome_button(row, "DECK B", -1, false);
    ui_next_chrome_nav(X.root);
    return X.root;
}

static void style_action(lv_obj_t *b, bool on)
{
    lv_obj_set_style_bg_color(b, ui_next_col(on ? X.pal->accent : X.pal->chip), 0);
    lv_obj_set_style_border_color(b, ui_next_col(on ? X.pal->accent : X.pal->border_hi), 0);
    lv_obj_set_style_text_color(lv_obj_get_child(b, 0), ui_next_col(on ? X.pal->on_accent : X.pal->text), 0);
}

void ui_next_fx_update(const ui_next_fx_model_t *m)
{
    if (!X.root) return;
    X.m = *m;
    for (int i = 0; i < UI_NEXT_FX_SLOTS; i++) {
        slot_t *s = &X.slot[i];
        const ui_next_fx_slot_t *k = &m->slot[i];
        lv_obj_set_style_bg_color(s->track, ui_next_col(k->on ? X.pal->accent : X.pal->border), 0);
        lv_obj_set_style_bg_color(s->knob, ui_next_col(k->on ? X.pal->on_accent : X.pal->muted), 0);
        lv_obj_set_x(s->knob, k->on ? lv_obj_get_width(s->track) - lv_obj_get_width(s->knob) - 3 : 3);
        lv_label_set_text(s->name, k->name);
        lv_label_set_text(s->text, k->text);
        lv_obj_set_style_bg_color(s->fill, ui_next_col(k->on ? X.pal->accent : off_fill()), 0);
        lv_obj_set_width(s->fill, X.bar_w[i] * k->amount / 100);
    }
    lv_label_set_text(X.xy_lx, m->label_x);
    lv_label_set_text(X.xy_ly, m->label_y);
    lv_obj_invalidate(X.xy);
    lv_label_set_text(X.panel_title, m->panel_title);
    for (int i = 0; i < 4; i++) style_action(X.beat[i], i == m->time_beats);
    char b[8];
    snprintf(b, sizeof b, "%u %%", (unsigned)m->wet);
    lv_label_set_text(X.wet_v, b);
    lv_obj_set_width(X.wet_fill, lv_obj_get_width(X.wet_track) * m->wet / 100);
    snprintf(b, sizeof b, "%u %%", (unsigned)m->feedback);
    lv_label_set_text(X.fb_v, b);
    lv_obj_set_width(X.fb_fill, lv_obj_get_width(X.fb_track) * m->feedback / 100);
    style_action(X.hold, m->hold);
    for (int d = 0; d < 2; d++) {
        const bool on = d == m->fx_deck;
        lv_obj_set_style_bg_color(X.deck_btn[d], ui_next_col(on ? X.pal->accent : X.pal->chip), 0);
        lv_obj_set_style_border_color(X.deck_btn[d], ui_next_col(on ? X.pal->accent : X.pal->border_hi), 0);
        lv_obj_set_style_text_color(lv_obj_get_child(X.deck_btn[d], 0), ui_next_col(on ? X.pal->on_accent : X.pal->text), 0);
    }
    ui_next_chrome_dock_set(0, &m->deck[0]);
    ui_next_chrome_dock_set(1, &m->deck[1]);
}
