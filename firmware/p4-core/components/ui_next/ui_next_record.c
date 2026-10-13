#include <stdio.h>
#include <string.h>

#include "ui_next.h"
#include "ui_next_chrome.h"
#include "ui_next_util.h"

/* Record page (screen 09): big timer, running waveform with a red write head, two
 * level meters, and the panel with STOP / MARKER, file facts and recent files.
 * Landscape puts the capture column (1.3) beside the panel; portrait stacks them. */

#define GAP6 6
#define NFACT 4

static const char *const FACT[NFACT] = {"SOURCE", "FORMAT", "RATE", "SIZE"};

static struct {
    ui_next_layout_t lay;
    const ui_next_palette_t *pal;
    const ui_next_fonts_t *f;
    bool day, landscape, wide, files;
    int row_h, pad, gap, fs;
    lv_obj_t *root, *status_dot, *status, *free_chip, *timer, *wave, *meter[2], *db[2], *value[NFACT], *stop;
    lv_obj_t *file_name[UI_NEXT_REC_FILES], *file_size[UI_NEXT_REC_FILES];
    int meter_x[2], meter_w;
    ui_next_record_model_t m;
} R;

static void wave_cb(lv_event_t *e)
{
    lv_layer_t *l = lv_event_get_layer(e);
    lv_area_t a;
    lv_obj_get_coords(lv_event_get_target_obj(e), &a);
    const int w = lv_area_get_width(&a);
    if (!R.m.wave[0]) return;
    const int head = (int)(R.m.head01 * (float)w);
    ui_next_chrome_draw_profile(l, &a, R.m.wave, UI_NEXT_REC_COLS, R.pal);
    /* Unwritten part of the window, then the red write head. */
    ui_next_fill_rect(l, a.x1 + head, a.y1, a.x2, a.y2, R.pal->well, LV_OPA_COVER);
    ui_next_fill_rect(l, a.x1 + head, a.y1, a.x1 + head + 1, a.y2, R.pal->bad, LV_OPA_COVER);
}

static void meter_cb(lv_event_t *e)
{
    const int ch = (int)(intptr_t)lv_event_get_user_data(e);
    lv_layer_t *l = lv_event_get_layer(e);
    lv_area_t a;
    lv_obj_get_coords(lv_event_get_target_obj(e), &a);
    const int w = lv_area_get_width(&a);
    /* linear-gradient: teal to 72 %, amber to 90 %, red after. */
    for (int px = 0; px < w; px++) {
        const int pct = px * 100 / w;
        const uint32_t c = pct < 72 ? R.pal->wave_mid : pct < 90 ? R.pal->warn : R.pal->bad;
        const uint32_t teal = R.day ? 0x3FD6C0 : c;
        ui_next_fill_rect(l, a.x1 + px, a.y1, a.x1 + px, a.y2, pct < 72 ? teal : c, LV_OPA_COVER);
    }
    const int lit = (int)(R.m.level[ch] * (float)w);
    ui_next_fill_rect(l, a.x1 + lit, a.y1, a.x2, a.y2, R.pal->well, LV_OPA_COVER);
    const int pk = a.x1 + (int)(R.m.peak[ch] * (float)w);
    ui_next_fill_rect(l, pk, a.y1, pk + 2, a.y2, R.pal->text, LV_OPA_COVER);
}

static lv_obj_t *chip_dot(lv_obj_t *parent, uint32_t dot_rgb, const char *text, bool mono)
{
    lv_obj_t *c = ui_next_box(parent, 0, 0, 0, R.fs * 19 / 10, R.pal->chip);
    lv_obj_set_width(c, LV_SIZE_CONTENT);
    lv_obj_set_style_radius(c, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_pad_hor(c, R.pad * 8 / 10, 0);
    lv_obj_set_style_pad_column(c, 6, 0);
    lv_obj_set_flex_flow(c, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(c, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    if (dot_rgb) {
        lv_obj_t *d = ui_next_box(c, 0, 0, 8, 8, dot_rgb);
        lv_obj_set_style_radius(d, LV_RADIUS_CIRCLE, 0);
    }
    ui_next_label(c, text, mono ? R.f->mono : R.f->small, R.pal->text);
    return c;
}

static void build_panel(int x, int y, int w, int h)
{
    lv_obj_t *p = ui_next_box(R.root, x, y, w, h, R.pal->panel);
    lv_obj_set_style_radius(p, 12, 0);
    lv_obj_set_style_border_width(p, 1, 0);
    lv_obj_set_style_border_color(p, ui_next_col(R.pal->border), 0);
    const int ix = R.pad - 1, iw = w - 2 * R.pad;
    int cy = R.pad - 1;

    const int bh = R.row_h * 85 / 100, bw = (iw - R.gap) / 2;
    R.stop = ui_next_chrome_action(p, ix, cy, bw, bh, "STOP", false);
    lv_obj_set_style_bg_color(R.stop, ui_next_col(R.pal->bad), 0);
    lv_obj_set_style_border_color(R.stop, ui_next_col(R.pal->bad), 0);
    lv_obj_set_style_text_color(lv_obj_get_child(R.stop, 0), ui_next_col(R.day ? 0x14181F : 0x05070A), 0);
    ui_next_chrome_action(p, ix + bw + R.gap, cy, iw - bw - R.gap, bh, "MARKER", false);
    cy += bh + R.gap;

    const int kw = (iw - 6) / 2, kh = 12 + R.f->micro->line_height + R.f->kv->line_height;
    for (int i = 0; i < NFACT; i++) {
        lv_obj_t *kv = ui_next_box(p, ix + (i % 2) * (kw + 6), cy + (i / 2) * (kh + 6), i % 2 ? iw - kw - 6 : kw, kh, R.pal->nav);
        lv_obj_set_style_radius(kv, 8, 0);
        lv_obj_t *l = ui_next_label(kv, FACT[i], R.f->micro, R.pal->muted);
        lv_obj_set_style_text_letter_space(l, 1, 0);
        lv_obj_set_pos(l, 10, 6);
        R.value[i] = ui_next_label(kv, "", R.f->kv, R.pal->text);
        lv_obj_set_pos(R.value[i], 10, 6 + R.f->micro->line_height);
    }
    cy += 2 * kh + 6 + R.gap;
    if (!R.files) return;

    lv_obj_t *rf = ui_next_label(p, "RECENT FILES", R.f->mono, R.pal->muted);
    lv_obj_set_pos(rf, ix, cy);
    cy += R.f->mono->line_height + R.gap;
    const int fh = 12 + (R.f->mono->line_height > R.f->small->line_height ? R.f->mono->line_height : R.f->small->line_height);
    for (int i = 0; i < UI_NEXT_REC_FILES; i++) {
        lv_obj_t *row = ui_next_box(p, ix, cy, iw, fh, R.pal->nav);
        lv_obj_set_style_radius(row, 8, 0);
        R.file_name[i] = ui_next_label(row, "", R.f->mono, R.pal->text);
        lv_obj_set_pos(R.file_name[i], 10, (fh - R.f->mono->line_height) / 2);
        R.file_size[i] = ui_next_label(row, "", R.f->small, R.pal->muted);
        lv_obj_set_width(R.file_size[i], iw - 20);
        lv_obj_set_style_text_align(R.file_size[i], LV_TEXT_ALIGN_RIGHT, 0);
        lv_obj_set_pos(R.file_size[i], 10, (fh - R.f->small->line_height) / 2);
        cy += fh + R.gap;
    }
}

lv_obj_t *ui_next_record_create(lv_obj_t *parent, uint16_t w, uint16_t h, bool day)
{
    memset(&R, 0, sizeof R);
    if (!ui_next_layout_compute(w, h, &R.lay)) return NULL;
    R.day = day;
    R.pal = ui_next_palette(day);
    R.f = ui_next_fonts_for(R.lay.cls);
    R.landscape = R.lay.landscape;
    R.wide = R.landscape && w >= 1000u;
    R.files = R.lay.cls != UI_NEXT_CLASS_S || (!R.landscape && h >= 1000u); /* x2: not on the S class */
    R.files = (w > 800u || h > 800u) && R.lay.cls != UI_NEXT_CLASS_S;
    R.fs = R.lay.tok.text_px;
    R.row_h = R.lay.tok.row_px;
    R.pad = R.lay.tok.pad_px;
    R.gap = R.lay.tok.gap_px;
    ui_next_chrome_begin(&R.lay, day, UI_NEXT_PAGE_RECORD);

    R.root = ui_next_box(parent, 0, 0, w, h, R.pal->frame);
    const int x0 = R.lay.dock.x + R.pad, y0 = R.lay.top_bar.h + R.pad;
    const int iw = R.lay.dock.w - 2 * R.pad, ih = R.lay.dock.y - R.lay.top_bar.h - 2 * R.pad;
    int c1w, c1h, px, py, pw, ph;
    if (R.landscape) {
        c1w = (iw - R.pad) * 13 / 23;
        c1h = ih;
        px = x0 + c1w + R.pad;
        py = y0;
        pw = iw - c1w - R.pad;
        ph = ih;
    } else {
        c1w = iw;
        c1h = (ih - R.pad) / 2;
        px = x0;
        py = y0 + c1h + R.pad;
        pw = iw;
        ph = ih - c1h - R.pad;
    }

    /* Capture column: timer, waveform (rest, at least 60), two meters, 6 px apart. */
    const int tlh = R.f->timer->line_height * 11 / 10 > R.f->timer->line_height ? R.f->timer->line_height * 11 / 10 : R.f->timer->line_height;
    const int mh = R.row_h * 3 / 10 > R.f->mono->line_height ? R.row_h * 3 / 10 : R.f->mono->line_height;
    int wave_h = c1h - tlh - 2 * mh - 3 * GAP6;
    if (wave_h < 60) wave_h = 60;
    R.timer = ui_next_label(R.root, "", R.f->timer, R.pal->text);
    lv_obj_set_pos(R.timer, x0, y0);
    R.wave = ui_next_box(R.root, x0, y0 + tlh + GAP6, c1w, wave_h, R.pal->well);
    lv_obj_set_style_radius(R.wave, 4, 0);
    lv_obj_set_style_clip_corner(R.wave, true, 0);
    lv_obj_add_event_cb(R.wave, wave_cb, LV_EVENT_DRAW_MAIN, NULL);

    const int lw = R.f->mono->line_height * 12 / 10;
    const int dbw = R.wide ? R.f->mono->line_height * 4 : 0;
    R.meter_w = c1w - lw - R.gap - (dbw ? dbw + R.gap : 0);
    for (int ch = 0; ch < 2; ch++) {
        const int my = y0 + tlh + GAP6 + wave_h + GAP6 + ch * (mh + GAP6);
        lv_obj_t *lab = ui_next_label(R.root, ch ? "R" : "L", R.f->mono, R.pal->text);
        lv_obj_set_pos(lab, x0, my + (mh - R.f->mono->line_height) / 2);
        const int bh = R.row_h * 3 / 10;
        R.meter[ch] = ui_next_box(R.root, x0 + lw + R.gap, my + (mh - bh) / 2, R.meter_w, bh, R.pal->well);
        lv_obj_set_style_radius(R.meter[ch], 4, 0);
        lv_obj_set_style_clip_corner(R.meter[ch], true, 0);
        lv_obj_add_event_cb(R.meter[ch], meter_cb, LV_EVENT_DRAW_MAIN, (void *)(intptr_t)ch);
        if (dbw) {
            R.db[ch] = ui_next_label(R.root, "", R.f->mono, R.pal->muted);
            lv_obj_set_width(R.db[ch], dbw);
            lv_obj_set_style_text_align(R.db[ch], LV_TEXT_ALIGN_RIGHT, 0);
            lv_obj_set_pos(R.db[ch], x0 + c1w - dbw, my + (mh - R.f->mono->line_height) / 2);
        }
    }

    build_panel(px, py, pw, ph);
    ui_next_chrome_dock(R.root, 0, false);
    ui_next_chrome_dock(R.root, 1, false);

    lv_obj_t *row = ui_next_chrome_topbar(R.root);
    ui_next_label(row, "Record", R.f->med, R.pal->text);
    lv_obj_t *sp = lv_obj_create(row);
    lv_obj_remove_style_all(sp);
    lv_obj_set_size(sp, 1, 1);
    lv_obj_set_flex_grow(sp, 1);
    R.status = chip_dot(row, R.pal->bad, "", false);
    R.status_dot = lv_obj_get_child(R.status, 0);
    if (R.wide) R.free_chip = chip_dot(row, 0, "", true);
    ui_next_chrome_nav(R.root);
    return R.root;
}

void ui_next_record_update(const ui_next_record_model_t *m)
{
    if (!R.root) return;
    R.m = *m;
    lv_label_set_text(R.timer, m->timer);
    lv_label_set_text(lv_obj_get_child(R.status, 1), m->status);
    lv_obj_set_style_bg_color(R.status_dot, ui_next_col(m->recording ? R.pal->bad : (R.day ? 0x8A94A3 : 0x6B7482)), 0);
    if (R.free_chip) lv_label_set_text(lv_obj_get_child(R.free_chip, 0), m->free_space);
    for (int i = 0; i < NFACT; i++) lv_label_set_text(R.value[i], m->value[i]);
    for (int i = 0; i < UI_NEXT_REC_FILES && R.file_name[i]; i++) {
        lv_label_set_text(R.file_name[i], m->file[i].name);
        lv_label_set_text(R.file_size[i], m->file[i].size);
    }
    for (int ch = 0; ch < 2; ch++) {
        if (R.db[ch]) lv_label_set_text(R.db[ch], m->db[ch]);
        lv_obj_invalidate(R.meter[ch]);
    }
    lv_obj_invalidate(R.wave);
    ui_next_chrome_dock_set(0, &m->deck[0]);
    ui_next_chrome_dock_set(1, &m->deck[1]);
}
