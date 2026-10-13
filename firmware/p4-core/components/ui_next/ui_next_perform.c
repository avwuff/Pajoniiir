#include <math.h>
#include <stdio.h>
#include <string.h>

#include "ui_next.h"
#include "ui_next_chrome.h"
#include "ui_next_fonts.h"
#include "ui_next_theme.h"
#include "ui_next_util.h"

/* Perform page (screen 01 and 15). Geometry comes from ui_next_layout, colours
 * from ui_next_theme, numbers from the model. Drawing follows the demo board:
 * three-band waveform, weave lane, VU column, link bar, deck dock. */

#define LINK_CELLS 7u

typedef struct {
    lv_obj_t *cell, *label, *value, *track, *fill, *zero;
    bool solid;
    uint8_t span;
} link_cell_t;

static struct {
    ui_next_layout_t lay;
    const ui_next_palette_t *pal;
    bool day;
    ui_next_perform_model_t model;
    uint16_t mcols;
    bool landscape;
    lv_obj_t *root, *title, *chip_layers, *lane_chip;
    lv_obj_t *wf[2], *lane, *vu[2];
    link_cell_t cells[LINK_CELLS];
    ui_next_page_t page;
    const lv_font_t *f_txt, *f_med, *f_tiny, *f_mono, *f_big, *f_bpm, *f_val, *f_micro;
} P;

/* ---- small helpers ---- */

static uint32_t sev_color(ui_next_sev_t s)
{
    return s == UI_NEXT_SEV_OK ? P.pal->ok : s == UI_NEXT_SEV_WARN ? P.pal->warn : P.pal->bad;
}

/* Window of the zoomed waveform: first envelope column. */
static uint32_t win_start(int d)
{
    uint32_t c = P.model.deck[d].col;
    return c > P.mcols / 2u ? c - P.mcols / 2u : 0u;
}

/* ---- waveform ---- */

/* Section map of the demo track (intro, build, drop, break, outro). CORE-GAP: LB-13. */
static const struct { float a, b; const char *name; uint32_t color; } SEC[] = {
    {0.00f, 0.08f, "INTRO", 0x6C7A96}, {0.08f, 0.26f, "BUILD", 0xFF8A6B}, {0.26f, 0.50f, "DROP", 0xF06BA8},
    {0.50f, 0.60f, "BREAK", 0x5FB3E8}, {0.60f, 0.90f, "DROP", 0xF06BA8},  {0.90f, 1.00f, "OUTRO", 0x6C7A96},
};
#define SEC_N (sizeof SEC / sizeof SEC[0])


static void wf_draw_cb(lv_event_t *e)
{
    const int d = (int)(intptr_t)lv_event_get_user_data(e);
    lv_layer_t *layer = lv_event_get_layer(e);
    lv_area_t a;
    lv_obj_get_coords(lv_event_get_target_obj(e), &a);
    const ui_next_env_t *env = P.model.deck[d].env;
    if (!env) return;

    const bool land = P.landscape;
    const int span = land ? lv_area_get_width(&a) : lv_area_get_height(&a);
    const int cross = land ? lv_area_get_height(&a) : lv_area_get_width(&a);
    const int c0 = (land ? a.y1 : a.x1) + cross / 2;
    const uint32_t start = win_start(d);
    const int m = P.mcols;

    const int ua = land ? a.x1 : a.y1;
    /* Section tints, behind everything: the section colour at 19 %. */
    for (unsigned q = 0; q < SEC_N; q++) {
        const float lo = fmaxf(SEC[q].a * (float)UI_NEXT_ENV_COLUMNS, (float)start);
        const float hi = fminf(SEC[q].b * (float)UI_NEXT_ENV_COLUMNS, (float)(start + (uint32_t)m));
        if (hi <= lo) continue;
        const int u0 = ua + (int)((lo - (float)start) * (float)span / (float)m);
        const int u1 = ua + (int)((hi - (float)start) * (float)span / (float)m) - 1;
        if (land) ui_next_fill_rect(layer, u0, a.y1, u1, a.y2, SEC[q].color, 48);
        else ui_next_fill_rect(layer, a.x1, u0, a.x2, u1, SEC[q].color, 48);
    }
    for (int o = 0; o < m; o++) {
        const uint32_t cl = start + (uint32_t)o;
        if (cl % UI_NEXT_BEAT_COLUMNS) continue;
        const int bar = cl % UI_NEXT_BAR_COLUMNS == 0u;
        const int u = ua + o * span / m;
        if (land) ui_next_fill_rect(layer, u, a.y1, u + bar, a.y2, P.pal->text, bar ? 128 : 40);
        else ui_next_fill_rect(layer, a.x1, u, a.x2, u + bar, P.pal->text, bar ? 128 : 40);
    }
    /* One pixel row at a time, linear between the two nearest columns, as the
     * filled path of the board does. */
    const float amp[3] = {.49f, .38f, .26f};
    const uint32_t colr[3] = {P.pal->wave_low, P.pal->wave_mid, P.pal->wave_high};
    for (int p = 0; p < span; p++) {
        const float pos = (float)p * (float)m / (float)span;
        const uint32_t i0 = start + (uint32_t)pos;
        const uint32_t i1 = i0 + 1u < UI_NEXT_ENV_COLUMNS ? i0 + 1u : i0;
        const float fr = pos - floorf(pos);
        const float v[3] = {env->lo[i0] + (env->lo[i1] - env->lo[i0]) * fr,
                            env->mi[i0] + (env->mi[i1] - env->mi[i0]) * fr,
                            env->hi[i0] + (env->hi[i1] - env->hi[i0]) * fr};
        for (int b = 0; b < 3; b++) {
            int half = (int)(v[b] * amp[b] * (float)cross + .5f);
            if (half < 1) half = 1;
            if (land) ui_next_fill_rect(layer, ua + p, c0 - half, ua + p, c0 + half, colr[b], LV_OPA_COVER);
            else ui_next_fill_rect(layer, c0 - half, ua + p, c0 + half, ua + p, colr[b], LV_OPA_COVER);
        }
    }

    /* Played half dimmed, playhead on top. */
    if (land) {
        ui_next_fill_rect(layer, a.x1, a.y1, a.x1 + span / 2 - 1, a.y2, P.pal->frame, P.pal->shade_opa);
        ui_next_fill_rect(layer, a.x1 + span / 2, a.y1, a.x1 + span / 2 + 1, a.y2, P.pal->head, LV_OPA_COVER);
    } else {
        ui_next_fill_rect(layer, a.x1, a.y1, a.x2, a.y1 + span / 2 - 1, P.pal->frame, P.pal->shade_opa);
        ui_next_fill_rect(layer, a.x1, a.y1 + span / 2, a.x2, a.y1 + span / 2 + 1, P.pal->head, LV_OPA_COVER);
    }

    /* Bar numbers and section names, over the shade. Vertical in portrait. */
    for (int o = 0; o < m; o++) {
        const uint32_t cl = start + (uint32_t)o;
        if (cl % UI_NEXT_BAR_COLUMNS) continue;
        char n[16];
        snprintf(n, sizeof n, "%u", (unsigned)(cl / UI_NEXT_BAR_COLUMNS + 1u));
        const int u0 = ua + o * span / m;
        if (land) ui_next_text_at(layer, n, u0 + 3, d == 0 ? a.y1 + 1 : a.y2 - 14, P.f_tiny, P.pal->text);
        else ui_next_text_down(layer, n, d == 0 ? a.x1 + 2 : a.x2 - 2 - P.f_tiny->line_height, u0 + 2, P.f_tiny, P.pal->text,
                               LV_OPA_COVER);
    }
    for (unsigned q = 0; q < SEC_N; q++) {
        const float lo = fmaxf(SEC[q].a * (float)UI_NEXT_ENV_COLUMNS, (float)start);
        const float hi = fminf(SEC[q].b * (float)UI_NEXT_ENV_COLUMNS, (float)(start + (uint32_t)m));
        if (hi <= lo || (hi - lo) / (float)m <= (land ? .14f : .2f)) continue;
        const int u = ua + (int)((lo - (float)start) * (float)span / (float)m);
        if (land) ui_next_text_at(layer, SEC[q].name, u + 4, a.y1 + 16, P.f_tiny, P.pal->text);
        else ui_next_text_down(layer, SEC[q].name, a.x1 + 16, u + 4, P.f_tiny, P.pal->text, 204);
    }
}

/* ---- weave lane (PB-18) ---- */

static void lane_draw_cb(lv_event_t *e)
{
    lv_layer_t *layer = lv_event_get_layer(e);
    lv_area_t a;
    lv_obj_get_coords(lv_event_get_target_obj(e), &a);
    const ui_next_env_t *ea = P.model.deck[0].env, *eb = P.model.deck[1].env;
    if (!ea || !eb) return;

    const bool land = P.landscape;
    const int span = land ? lv_area_get_width(&a) : lv_area_get_height(&a);
    const int cross = land ? lv_area_get_height(&a) : lv_area_get_width(&a);
    const int m = P.mcols;
    const uint32_t sa = win_start(0), sb = win_start(1);
    const int lw = P.lay.cls == UI_NEXT_CLASS_S ? 3 : P.lay.cls == UI_NEXT_CLASS_M ? 4 : 5;

#define PX(u, v) (land ? (float)a.x1 + (float)(u) * (float)span / (float)m : (float)a.x1 + (float)(v) * (float)cross / 100.0f)
#define PY(u, v) (land ? (float)a.y1 + (float)(v) * (float)cross / 100.0f : (float)a.y1 + (float)(u) * (float)span / (float)m)

    for (int o = 0; o < m; o++) {
        const uint32_t cA = sa + (uint32_t)o;
        if (cA % UI_NEXT_BEAT_COLUMNS) continue;
        int dB = (int)((UI_NEXT_BEAT_COLUMNS - ((sb + (uint32_t)o) % UI_NEXT_BEAT_COLUMNS)) % UI_NEXT_BEAT_COLUMNS);
        if (dB > 4) dB -= 9;
        const int oB = o + dB;
        int dd = -(int)((sb + (uint32_t)o) % UI_NEXT_BAR_COLUMNS);
        if (dd < -18) dd += 36;
        const int oBar = o + dd;
        const bool bar = cA % UI_NEXT_BAR_COLUMNS == 0u;
        const bool clash =
            ui_next_env_smooth(ea->lo, UI_NEXT_ENV_COLUMNS, (int32_t)cA, 2) > .30f &&
            ui_next_env_smooth(eb->lo, UI_NEXT_ENV_COLUMNS, (int32_t)((int32_t)sb + oB), 2) > .30f;

        ui_next_seg(layer, PX(o, 0), PY(o, 0), PX(o, 16), PY(o, 16), 1, P.pal->text, 140);
        ui_next_seg(layer, PX(oB, 84), PY(oB, 84), PX(oB, 100), PY(oB, 100), 1, P.pal->text, 140);
        const int tgt = bar ? oBar : oB;
        if (bar) {
            ui_next_seg(layer, PX(o, 16), PY(o, 16), PX(o, 84), PY(o, 84), 1, P.pal->text, 90);
            /* Cubic from (o,16) to (tgt,84): control points (o,55) and (tgt,45). */
            float px = PX(o, 16), py = PY(o, 16);
            for (int s = 1; s <= 12; s++) {
                float t = (float)s / 12.0f, it = 1.0f - t;
                float x0 = (float)o, x3 = (float)tgt;
                float uu = it * it * it * x0 + 3 * it * it * t * x0 + 3 * it * t * t * x3 + t * t * t * x3;
                float vv = it * it * it * 16 + 3 * it * it * t * 55 + 3 * it * t * t * 45 + t * t * t * 84;
                float nx = PX(uu, vv), ny = PY(uu, vv);
                ui_next_seg(layer, px, py, nx, ny, lw, P.pal->curve, LV_OPA_COVER);
                px = nx;
                py = ny;
            }
        }
        if (clash) {
            const float cx = PX(((float)o + (float)tgt) / 2, 50), cy = PY(((float)o + (float)tgt) / 2, 50);
            ui_next_seg(layer, cx, cy, cx, cy, lw + 3, P.pal->dot, LV_OPA_COVER);
        }
    }
#undef PX
#undef PY

    if (land) {
        ui_next_fill_rect(layer, a.x1, a.y1, a.x1 + span / 2 - 1, a.y2, P.pal->frame, P.pal->shade_opa);
        ui_next_fill_rect(layer, a.x1 + span / 2, a.y1, a.x1 + span / 2 + 1, a.y2, P.pal->head, LV_OPA_COVER);
    } else {
        ui_next_fill_rect(layer, a.x1, a.y1, a.x2, a.y1 + span / 2 - 1, P.pal->frame, P.pal->shade_opa);
        ui_next_fill_rect(layer, a.x1, a.y1 + span / 2, a.x2, a.y1 + span / 2 + 1, P.pal->head, LV_OPA_COVER);
    }
}

/* ---- VU column (PB-19) ---- */

static void vu_draw_cb(lv_event_t *e)
{
    const int d = (int)(intptr_t)lv_event_get_user_data(e);
    lv_layer_t *layer = lv_event_get_layer(e);
    lv_area_t a;
    lv_obj_get_coords(lv_event_get_target_obj(e), &a);
    const int w = lv_area_get_width(&a), h = lv_area_get_height(&a);
    const int bw = (w - 2) / 2;
    const float lev[2] = {P.model.deck[d].vu_l, P.model.deck[d].vu_r};
    const float pk[2] = {P.model.deck[d].peak_l, P.model.deck[d].peak_r};
    const int n = h / 5;
    for (int s = 0; s < 2; s++) {
        const int x = a.x1 + s * (bw + 2);
        for (int k = 0; k < n; k++) {
            const float fr = ((float)k + .5f) / (float)n;
            const uint32_t c = fr < .62f ? P.pal->ok : fr < .84f ? P.pal->warn : P.pal->bad;
            const int y2 = a.y2 - k * 5;
            ui_next_fill_rect(layer, x, y2 - 3, x + bw - 1, y2, c, fr <= lev[s] ? LV_OPA_COVER : 45);
        }
        const int py = a.y2 - (int)(pk[s] * (float)h);
        ui_next_fill_rect(layer, x, py - 1, x + bw - 1, py, P.pal->text, LV_OPA_COVER);
    }
}

/* ---- build ---- */

static void pick_fonts(ui_next_class_t cls)
{
    const ui_next_fonts_t *f = ui_next_fonts_for(cls);
    P.f_txt = f->txt;
    P.f_med = f->med;
    P.f_tiny = f->tiny;
    P.f_mono = f->mono;
    P.f_big = f->big;
    P.f_bpm = f->bpm;
    P.f_val = f->val;
    P.f_micro = f->micro;
}

static void ext_tag_cb(lv_event_t *e)
{
    lv_area_t a;
    lv_obj_get_coords(lv_event_get_target_obj(e), &a);
    ui_next_dashed_round(lv_event_get_layer(e), &a, 4, P.pal->accent_ink, 3, 2);
}

static void build_top_bar(void)
{
    const ui_next_rect_t r = P.lay.top_bar;
    lv_obj_t *bar = ui_next_box(P.root, r.x, r.y, r.w, r.h, P.pal->panel);
    ui_next_box(bar, 0, r.h - 1, r.w, 1, P.pal->border);
    P.title = ui_next_label(bar, "PERFORM", P.f_med, P.pal->text);
    lv_obj_align(P.title, LV_ALIGN_LEFT_MID, P.lay.tok.pad_px, 0);

    lv_obj_t *right = lv_obj_create(bar);
    lv_obj_remove_style_all(right);
    lv_obj_remove_flag(right, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(right, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(right, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(right, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(right, P.lay.tok.gap_px, 0);
    lv_obj_align(right, LV_ALIGN_RIGHT_MID, -P.lay.tok.pad_px, 0);
    P.chip_layers = ui_next_chrome_chip(right, "LAYERS 1-4");
    if (P.lay.landscape && P.lay.cls != UI_NEXT_CLASS_S) {
        /* EXT chip: the word, then a small dashed tag (ON on M and L, LOW on S). */
        lv_obj_t *ext = ui_next_chrome_chip(right, "EXT");
        lv_obj_set_style_pad_column(ext, 6, 0);
        lv_obj_t *xt = lv_obj_create(ext);
        lv_obj_remove_style_all(xt);
        lv_obj_remove_flag(xt, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_size(xt, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
        lv_obj_set_style_pad_hor(xt, 4, 0);
        lv_obj_set_style_pad_ver(xt, 1, 0);
        ui_next_label(xt, "ON", P.f_micro, P.pal->accent_ink);
        lv_obj_add_event_cb(xt, ext_tag_cb, LV_EVENT_DRAW_POST, NULL);
    }
}

static lv_obj_t *wf_obj(int x, int y, int w, int h, int d)
{
    lv_obj_t *o = ui_next_box(P.root, x, y, w, h, P.pal->well);
    lv_obj_set_style_radius(o, 4, 0);
    lv_obj_set_style_clip_corner(o, true, 0);
    lv_obj_add_event_cb(o, wf_draw_cb, LV_EVENT_DRAW_MAIN, (void *)(intptr_t)d);
    char t[24];
    snprintf(t, sizeof t, d ? "B  119.0 BPM  9A" : "A  124.0 BPM  8A");
    lv_obj_t *tag = ui_next_box(o, 6, 4, LV_SIZE_CONTENT, LV_SIZE_CONTENT, P.pal->frame);
    lv_obj_set_style_bg_opa(tag, 166, 0);
    lv_obj_set_style_radius(tag, 3, 0);
    lv_obj_set_style_pad_hor(tag, 5, 0);
    lv_obj_set_style_pad_ver(tag, 1, 0);
    ui_next_label(tag, t, P.f_tiny, P.pal->text);
    if (!P.landscape && d == 1) lv_obj_align(tag, LV_ALIGN_TOP_RIGHT, -6, 4);
    else if (P.landscape && d == 1) lv_obj_align(tag, LV_ALIGN_BOTTOM_LEFT, 6, -4);
    return o;
}

static void build_waveforms(void)
{
    const ui_next_rect_t z = P.lay.waveforms;
    const int pad = P.lay.tok.pad_px, gap = P.lay.tok.gap_px;
    static const int LANE[] = {44, 56, 72}, VUW[] = {16, 20, 24};
    const int lane = LANE[P.lay.cls], vuw = VUW[P.lay.cls];
    const int x0 = z.x + pad, y0 = z.y + pad, w = z.w - 2 * pad, h = z.h - 2 * pad;

    if (P.landscape) {
        const int dh = (h - lane - 2 * gap) / 2;
        for (int d = 0; d < 2; d++) {
            const int y = y0 + d * (dh + lane + 2 * gap);
            P.vu[d] = ui_next_box(P.root, x0, y, vuw, dh, P.pal->frame);
            lv_obj_add_event_cb(P.vu[d], vu_draw_cb, LV_EVENT_DRAW_MAIN, (void *)(intptr_t)d);
            P.wf[d] = wf_obj(x0 + vuw + 3, y, w - vuw - 3, dh, d);
        }
        P.lane = ui_next_box(P.root, x0 + vuw + 3, y0 + dh + gap, w - vuw - 3, lane, P.pal->nav);
    } else {
        const int dw = (w - lane - 2 * gap) / 2;
        for (int d = 0; d < 2; d++) {
            const int x = x0 + d * (dw + lane + 2 * gap);
            const int vx = d == 0 ? x : x + dw - vuw;
            const int wx = d == 0 ? x + vuw + 3 : x;
            P.vu[d] = ui_next_box(P.root, vx, y0, vuw, h, P.pal->frame);
            lv_obj_add_event_cb(P.vu[d], vu_draw_cb, LV_EVENT_DRAW_MAIN, (void *)(intptr_t)d);
            P.wf[d] = wf_obj(wx, y0, dw - vuw - 3, h, d);
        }
        P.lane = ui_next_box(P.root, x0 + dw + gap, y0, lane, h, P.pal->nav);
    }
    lv_obj_set_style_radius(P.lane, 3, 0);
    lv_obj_set_style_clip_corner(P.lane, true, 0);
    lv_obj_add_event_cb(P.lane, lane_draw_cb, LV_EVENT_DRAW_MAIN, NULL);
    if (P.landscape && P.lay.cls != UI_NEXT_CLASS_S) {
        /* Phrase offset chip at the left of the lane (PB-17), from the M class up. */
        P.lane_chip = ui_next_box(P.lane, 8, 0, 10, 10, P.pal->well);
        lv_obj_set_width(P.lane_chip, LV_SIZE_CONTENT);
        lv_obj_set_height(P.lane_chip, LV_SIZE_CONTENT);
        lv_obj_set_style_radius(P.lane_chip, 3, 0);
        lv_obj_set_style_border_width(P.lane_chip, 1, 0);
        lv_obj_set_style_border_color(P.lane_chip, ui_next_col(P.pal->accent_ink), 0);
        lv_obj_set_style_pad_hor(P.lane_chip, 6, 0);
        lv_obj_set_style_pad_ver(P.lane_chip, 1, 0);
        const lv_font_t *bf = P.lay.cls == UI_NEXT_CLASS_L ? &ui_next_cousine_bold_14 : &ui_next_cousine_bold_12;
        ui_next_label(P.lane_chip, "", bf, P.pal->text);
        lv_obj_align(P.lane_chip, LV_ALIGN_LEFT_MID, 8, 0);
    }
}

static void build_link_bar(void)
{
    const ui_next_rect_t r = P.lay.link_bar;
    const int pad = P.lay.tok.pad_px;
    lv_obj_t *bar = ui_next_box(P.root, r.x + pad, r.y, r.w - 2 * pad, r.h - 2, 0x0E141B);
    if (P.day) lv_obj_set_style_bg_color(bar, ui_next_col(P.pal->nav), 0);
    lv_obj_set_style_border_width(bar, 1, 0);
    lv_obj_set_style_border_color(bar, ui_next_col(P.pal->border), 0);
    lv_obj_set_style_radius(bar, 6, 0);

    const bool one_row = P.lay.cls == UI_NEXT_CLASS_L;
    const int inner_pad = pad / 2, g = P.lay.tok.gap_px * 6 / 10;
    const int iw = r.w - 2 * pad - 2 * inner_pad - 2, ih = r.h - 2 - 2 * inner_pad - 2;
    const int cols = one_row ? 14 : 12, rows = one_row ? 1 : 2;
    const int cell_h = (ih - (rows - 1) * g) / rows;
    int col_i = 0, row_i = 0;
    for (unsigned i = 0; i < LINK_CELLS; i++) {
        const int span = one_row ? 2 : (i < 4 ? 3 : 4);
        if (col_i + span > cols) {
            col_i = 0;
            row_i++;
        }
        const int cw = (iw - (cols - 1) * g) / cols * span + (span - 1) * g;
        const int cx = inner_pad + col_i * ((iw - (cols - 1) * g) / cols + g);
        const int cy = inner_pad + row_i * (cell_h + g);
        link_cell_t *c = &P.cells[i];
        c->solid = i == 4 || i == 6;
        c->span = (uint8_t)span;
        c->cell = ui_next_box(bar, cx, cy, cw, cell_h, c->solid ? P.pal->ok : P.pal->nav);
        lv_obj_set_style_bg_opa(c->cell, c->solid ? LV_OPA_COVER : LV_OPA_TRANSP, 0);
        lv_obj_set_style_radius(c->cell, 4, 0);
        c->label = ui_next_label(c->cell, "", P.f_tiny, c->solid ? 0x0B0F14 : P.pal->muted);
        c->value = ui_next_label(c->cell, "", P.f_val, c->solid ? 0x0B0F14 : P.pal->text);
        if (c->solid) {
            lv_obj_align(c->label, LV_ALIGN_TOP_MID, 0, -1);
            lv_obj_align(c->value, LV_ALIGN_BOTTOM_MID, 0, 2);
        } else {
            lv_obj_align(c->label, LV_ALIGN_TOP_LEFT, 2, 0);
            lv_obj_align(c->value, LV_ALIGN_TOP_RIGHT, -2, 0);
            const int th = P.lay.tok.text_px / 2;
            c->track = ui_next_box(c->cell, 2, cell_h - th - 1, cw - 4, th, P.day ? 0xD5DCE5 : 0x222C38);
            lv_obj_set_style_radius(c->track, 3, 0);
            c->fill = ui_next_box(c->track, 0, 0, 0, th, P.pal->warn);
            c->zero = ui_next_box(c->track, (cw - 4) / 2 - 1, -1, 2, th + 2, P.pal->text);
            lv_obj_set_style_bg_opa(c->zero, 200, 0);
        }
        col_i += span;
    }
}

lv_obj_t *ui_next_perform_create(lv_obj_t *parent, uint16_t w, uint16_t h, bool day)
{
    memset(&P, 0, sizeof P);
    if (!ui_next_layout_compute(w, h, &P.lay)) return NULL;
    P.day = day;
    P.pal = ui_next_palette(day);
    P.landscape = P.lay.landscape;
    P.page = UI_NEXT_PAGE_PERFORM;
    ui_next_chrome_begin(&P.lay, day, UI_NEXT_PAGE_PERFORM);
    static const uint16_t WIN[] = {144, 180, 216};
    P.mcols = WIN[P.lay.cls];
    pick_fonts(P.lay.cls);

    P.root = ui_next_box(parent, 0, 0, w, h, P.pal->frame);
    build_waveforms();
    build_link_bar();
    ui_next_chrome_dock(P.root, 0, true);
    ui_next_chrome_dock(P.root, 1, true);
    build_top_bar();
    ui_next_chrome_nav(P.root);
    return P.root;
}

/* ---- update ---- */

static void set_meter(link_cell_t *c, const char *name, const char *val, float n, bool signed_, bool neg,
                      uint32_t fixed_col)
{
    lv_label_set_text(c->label, name);
    lv_label_set_text(c->value, val);
    if (!c->fill) return;
    const int tw = lv_obj_get_width(c->track), th = lv_obj_get_height(c->track);
    if (n > 1.0f) n = 1.0f;
    const uint32_t colr = fixed_col ? fixed_col : sev_color(ui_next_severity(n));
    int x = 0, w = (int)(n * (float)tw);
    if (signed_) {
        w = (int)(n * (float)tw / 2.0f);
        x = neg ? tw / 2 - w : tw / 2;
    }
    lv_obj_set_pos(c->fill, x, 0);
    lv_obj_set_size(c->fill, w, th);
    lv_obj_set_style_bg_color(c->fill, ui_next_col(colr), 0);
}

static void set_solid(link_cell_t *c, const char *name, const char *val, uint32_t colr)
{
    lv_label_set_text(c->label, name);
    lv_label_set_text(c->value, val);
    lv_obj_set_style_bg_color(c->cell, ui_next_col(colr), 0);
    lv_obj_align(c->label, LV_ALIGN_TOP_MID, 0, -1);
    lv_obj_align(c->value, LV_ALIGN_BOTTOM_MID, 0, 2);
}

void ui_next_perform_update(const ui_next_perform_model_t *m)
{
    if (!P.root) return;
    P.model = *m;
    const ui_next_link_t *lk = &m->link;
    char s[24];

    if (P.lane_chip) {
        char b[16];
        snprintf(b, sizeof b, "+%u BEATS", (unsigned)lk->phrase_off);
        lv_label_set_text(lv_obj_get_child(P.lane_chip, 0), b);
    }
    lv_obj_t *lbl = lv_obj_get_child(P.chip_layers, 0);
    lv_label_set_text(lbl, m->layers);

    for (int d = 0; d < 2; d++) {
        ui_next_chrome_dock_set(d, &m->deck[d]);
        lv_obj_invalidate(P.wf[d]);
        lv_obj_invalidate(P.vu[d]);
    }
    lv_obj_invalidate(P.lane);

    const float ms = lk->beat_ms, ph = lk->phase_beat;
    snprintf(s, sizeof s, "%.1f", m->tempo_pct);
    set_meter(&P.cells[0], "TEMPO", s, .63f, true, true, 0);
    snprintf(s, sizeof s, "%c%d ms", ms >= 0 ? '+' : '-', (int)fabsf(ms));
    set_meter(&P.cells[1], "BEAT", s, fabsf(ms) / 20.0f, true, ms < 0, 0);
    snprintf(s, sizeof s, "%c%.2f", ph >= 0 ? '+' : '-', fabsf(ph));
    set_meter(&P.cells[2], "PHASE", s, fabsf(ph) / .3f, true, ph < 0, 0);
    snprintf(s, sizeof s, "+%u b", (unsigned)lk->phrase_off);
    set_meter(&P.cells[3], "PHRASE", s, (float)lk->phrase_off / 4.0f, true, false, 0);

    snprintf(s, sizeof s, "%u %%", (unsigned)lk->bass_pct);
    set_solid(&P.cells[4], "BASS CLASH", s,
              lk->bass_pct < 25 ? P.pal->ok : lk->bass_pct < 50 ? P.pal->warn : P.pal->bad);
    snprintf(s, sizeof s, "%u %%", (unsigned)lk->overlap_pct);
    set_meter(&P.cells[5], "OVERLAP", s, (float)lk->overlap_pct / 100.0f, false, false, P.pal->warn);
    set_solid(&P.cells[6], "KEY FIT", m->key_change, lk->key_fit ? P.pal->ok : P.pal->bad);
}
