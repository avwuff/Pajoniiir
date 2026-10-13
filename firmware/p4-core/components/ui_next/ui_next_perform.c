#include <math.h>
#include <stdio.h>
#include <string.h>

#include "ui_next.h"
#include "ui_next_fonts.h"
#include "ui_next_theme.h"

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
    lv_obj_t *root, *title, *chip_layers;
    lv_obj_t *wf[2], *lane, *vu[2], *ov[2], *nav_icon[UI_NEXT_PAGE_COUNT];
    lv_obj_t *dk_title[2], *dk_bpm[2], *dk_key[2], *dk_badge[2], *dk_time[2];
    link_cell_t cells[LINK_CELLS];
    ui_next_page_t page;
    const lv_font_t *f_txt, *f_med, *f_tiny, *f_mono, *f_big, *f_bpm, *f_val;
} P;

static const char *NAV_NAME[UI_NEXT_PAGE_COUNT] = {"PERFORM", "BROWSE", "FX", "RECORD", "SETTINGS"};

/* ---- small helpers ---- */

static lv_color_t col(uint32_t rgb) { return lv_color_hex(rgb); }

static lv_obj_t *box(lv_obj_t *parent, int x, int y, int w, int h, uint32_t rgb)
{
    lv_obj_t *o = lv_obj_create(parent);
    lv_obj_remove_style_all(o);
    lv_obj_remove_flag(o, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_pos(o, x, y);
    lv_obj_set_size(o, w, h);
    lv_obj_set_style_bg_color(o, col(rgb), 0);
    lv_obj_set_style_bg_opa(o, LV_OPA_COVER, 0);
    return o;
}

static lv_obj_t *label(lv_obj_t *parent, const char *s, const lv_font_t *f, uint32_t rgb)
{
    lv_obj_t *l = lv_label_create(parent);
    lv_label_set_text(l, s);
    lv_obj_set_style_text_font(l, f, 0);
    lv_obj_set_style_text_color(l, col(rgb), 0);
    return l;
}

static void fill_rect(lv_layer_t *layer, int x1, int y1, int x2, int y2, uint32_t rgb, lv_opa_t opa)
{
    if (x2 < x1 || y2 < y1) return;
    lv_draw_rect_dsc_t d;
    lv_draw_rect_dsc_init(&d);
    d.bg_color = col(rgb);
    d.bg_opa = opa;
    lv_area_t a = {x1, y1, x2, y2};
    lv_draw_rect(layer, &d, &a);
}

static void seg(lv_layer_t *layer, float x1, float y1, float x2, float y2, int w, uint32_t rgb, lv_opa_t opa)
{
    lv_draw_line_dsc_t d;
    lv_draw_line_dsc_init(&d);
    d.p1.x = (lv_value_precise_t)x1;
    d.p1.y = (lv_value_precise_t)y1;
    d.p2.x = (lv_value_precise_t)x2;
    d.p2.y = (lv_value_precise_t)y2;
    d.width = w;
    d.color = col(rgb);
    d.opa = opa;
    d.round_start = d.round_end = 1;
    lv_draw_line(layer, &d);
}

static void text_at(lv_layer_t *layer, const char *s, int x, int y, const lv_font_t *f, uint32_t rgb)
{
    lv_draw_label_dsc_t d;
    lv_draw_label_dsc_init(&d);
    d.text = s;
    d.font = f;
    d.color = col(rgb);
    lv_area_t a = {x, y, x + 60, y + 20};
    lv_draw_label(layer, &d, &a);
}

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
    for (int o = 0; o < m; o++) {
        const uint32_t cl = start + (uint32_t)o;
        if (cl % UI_NEXT_BEAT_COLUMNS) continue;
        const int bar = cl % UI_NEXT_BAR_COLUMNS == 0u;
        const int u = ua + o * span / m;
        if (land) fill_rect(layer, u, a.y1, u + bar, a.y2, P.pal->text, bar ? 128 : 40);
        else fill_rect(layer, a.x1, u, a.x2, u + bar, P.pal->text, bar ? 128 : 40);
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
            if (land) fill_rect(layer, ua + p, c0 - half, ua + p, c0 + half, colr[b], LV_OPA_COVER);
            else fill_rect(layer, c0 - half, ua + p, c0 + half, ua + p, colr[b], LV_OPA_COVER);
        }
    }

    /* Played half dimmed, playhead on top. */
    if (land) {
        fill_rect(layer, a.x1, a.y1, a.x1 + span / 2 - 1, a.y2, P.pal->frame, 140);
        fill_rect(layer, a.x1 + span / 2, a.y1, a.x1 + span / 2 + 1, a.y2, P.pal->accent, LV_OPA_COVER);
    } else {
        fill_rect(layer, a.x1, a.y1, a.x2, a.y1 + span / 2 - 1, P.pal->frame, 140);
        fill_rect(layer, a.x1, a.y1 + span / 2, a.x2, a.y1 + span / 2 + 1, P.pal->accent, LV_OPA_COVER);
    }

    /* Bar numbers, after the bands above the shade. */
    for (int o = 0; o < m; o++) {
        const uint32_t cl = start + (uint32_t)o;
        if (cl % UI_NEXT_BAR_COLUMNS) continue;
        char n[16];
        snprintf(n, sizeof n, "%u", (unsigned)(cl / UI_NEXT_BAR_COLUMNS + 1u));
        const int u0 = (land ? a.x1 : a.y1) + o * span / m;
        if (land) text_at(layer, n, u0 + 3, d == 0 ? a.y1 + 1 : a.y2 - 14, P.f_tiny, P.pal->text);
        else text_at(layer, n, d == 0 ? a.x1 + 2 : a.x2 - 16, u0 + 2, P.f_tiny, P.pal->text);
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

        seg(layer, PX(o, 0), PY(o, 0), PX(o, 16), PY(o, 16), 1, P.pal->text, 140);
        seg(layer, PX(oB, 84), PY(oB, 84), PX(oB, 100), PY(oB, 100), 1, P.pal->text, 140);
        const int tgt = bar ? oBar : oB;
        if (bar) {
            seg(layer, PX(o, 16), PY(o, 16), PX(o, 84), PY(o, 84), 1, P.pal->text, 90);
            /* Cubic from (o,16) to (tgt,84): control points (o,55) and (tgt,45). */
            float px = PX(o, 16), py = PY(o, 16);
            for (int s = 1; s <= 12; s++) {
                float t = (float)s / 12.0f, it = 1.0f - t;
                float x0 = (float)o, x3 = (float)tgt;
                float uu = it * it * it * x0 + 3 * it * it * t * x0 + 3 * it * t * t * x3 + t * t * t * x3;
                float vv = it * it * it * 16 + 3 * it * it * t * 55 + 3 * it * t * t * 45 + t * t * t * 84;
                float nx = PX(uu, vv), ny = PY(uu, vv);
                seg(layer, px, py, nx, ny, lw, P.pal->accent, LV_OPA_COVER);
                px = nx;
                py = ny;
            }
        }
        if (clash) {
            const float cx = PX(((float)o + (float)tgt) / 2, 50), cy = PY(((float)o + (float)tgt) / 2, 50);
            seg(layer, cx, cy, cx, cy, lw + 3, P.pal->bad, LV_OPA_COVER);
        }
    }
#undef PX
#undef PY

    if (land) {
        fill_rect(layer, a.x1, a.y1, a.x1 + span / 2 - 1, a.y2, P.pal->frame, 140);
        fill_rect(layer, a.x1 + span / 2, a.y1, a.x1 + span / 2 + 1, a.y2, P.pal->accent, LV_OPA_COVER);
    } else {
        fill_rect(layer, a.x1, a.y1, a.x2, a.y1 + span / 2 - 1, P.pal->frame, 140);
        fill_rect(layer, a.x1, a.y1 + span / 2, a.x2, a.y1 + span / 2 + 1, P.pal->accent, LV_OPA_COVER);
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
            fill_rect(layer, x, y2 - 3, x + bw - 1, y2, c, fr <= lev[s] ? LV_OPA_COVER : 45);
        }
        const int py = a.y2 - (int)(pk[s] * (float)h);
        fill_rect(layer, x, py - 1, x + bw - 1, py, P.pal->text, LV_OPA_COVER);
    }
}

/* ---- overview strip in the dock (PB-3) ---- */

static void ov_draw_cb(lv_event_t *e)
{
    const int d = (int)(intptr_t)lv_event_get_user_data(e);
    lv_layer_t *layer = lv_event_get_layer(e);
    lv_area_t a;
    lv_obj_get_coords(lv_event_get_target_obj(e), &a);
    const ui_next_env_t *env = P.model.deck[d].env;
    if (!env) return;
    const int w = lv_area_get_width(&a), h = lv_area_get_height(&a), c0 = a.y1 + h / 2;
    const float amp[3] = {.49f, .38f, .26f};
    const uint32_t colr[3] = {P.pal->wave_low, P.pal->wave_mid, P.pal->wave_high};
    for (int i = 0; i < w; i++) {
        const uint32_t c = (uint32_t)i * 400u / (uint32_t)w * 3u;
        float v[3] = {0, 0, 0};
        for (uint32_t j = 0; j < 3u && c + j < UI_NEXT_ENV_COLUMNS; j++) {
            if (env->lo[c + j] > v[0]) v[0] = env->lo[c + j];
            if (env->mi[c + j] > v[1]) v[1] = env->mi[c + j];
            if (env->hi[c + j] > v[2]) v[2] = env->hi[c + j];
        }
        for (int b = 0; b < 3; b++) {
            int half = (int)(v[b] * amp[b] * (float)h + .5f);
            if (half < 1) half = 1;
            fill_rect(layer, a.x1 + i, c0 - half, a.x1 + i, c0 + half, colr[b], LV_OPA_COVER);
        }
    }
    static const float CUES[] = {.04f, .13f, .26f, .5f, .62f, .9f};
    for (unsigned i = 0; i < sizeof CUES / sizeof CUES[0]; i++) {
        int x = a.x1 + (int)(CUES[i] * (float)w);
        fill_rect(layer, x, a.y1, x + 2, a.y2, P.pal->bad, LV_OPA_COVER);
    }
    const int px = a.x1 + (int)(P.model.deck[d].pos01 * (float)w);
    fill_rect(layer, a.x1, a.y1, px - 1, a.y2, P.pal->frame, 140);
    fill_rect(layer, px, a.y1, px + 1, a.y2, P.pal->accent, LV_OPA_COVER);
}

/* ---- nav icons, drawn from segments in a 24 unit box ---- */

static void icon_draw_cb(lv_event_t *e)
{
    const int page = (int)(intptr_t)lv_event_get_user_data(e);
    lv_layer_t *layer = lv_event_get_layer(e);
    lv_area_t a;
    lv_obj_get_coords(lv_event_get_target_obj(e), &a);
    const float k = (float)lv_area_get_width(&a) / 24.0f;
    const uint32_t c = (int)P.page == page ? P.pal->accent_ink : P.pal->muted;
    static const int8_t S[][4] = {
        /* perform */ {3, 12, 7, 12}, {7, 12, 10, 4}, {10, 4, 14, 20}, {14, 20, 17, 12}, {17, 12, 21, 12},
        /* browse */ {4, 6, 20, 6}, {4, 12, 20, 12}, {4, 18, 14, 18},
        /* fx */ {12, 3, 12, 21}, {5, 8, 5, 16}, {19, 6, 19, 18}, {8, 11, 8, 13}, {15, 9, 15, 15},
        /* settings */ {4, 7, 13, 7}, {17, 7, 20, 7}, {4, 17, 7, 17}, {11, 17, 20, 17}, {15, 5, 15, 9}, {9, 15, 9, 19},
    };
    static const uint8_t FIRST[] = {0, 5, 8, 0, 13}, COUNT[] = {5, 3, 5, 0, 6};
    if (page == UI_NEXT_PAGE_RECORD) {
        lv_draw_rect_dsc_t d;
        lv_draw_rect_dsc_init(&d);
        d.bg_opa = LV_OPA_TRANSP;
        d.radius = LV_RADIUS_CIRCLE;
        d.border_width = 2;
        d.border_color = col(c);
        lv_area_t r = {a.x1 + (int)(6 * k), a.y1 + (int)(6 * k), a.x1 + (int)(18 * k), a.y1 + (int)(18 * k)};
        lv_draw_rect(layer, &d, &r);
        return;
    }
    for (int i = 0; i < COUNT[page]; i++) {
        const int8_t *s = S[FIRST[page] + i];
        seg(layer, (float)a.x1 + (float)s[0] * k, (float)a.y1 + (float)s[1] * k,
            (float)a.x1 + (float)s[2] * k, (float)a.y1 + (float)s[3] * k, 2, c, LV_OPA_COVER);
    }
}

/* ---- build ---- */

static void pick_fonts(ui_next_class_t cls)
{
    static const lv_font_t *const TXT[] = {&ui_next_roboto_reg_14, &ui_next_roboto_reg_16, &ui_next_roboto_reg_18};
    static const lv_font_t *const MED[] = {&ui_next_roboto_med_16, &ui_next_roboto_med_18, &ui_next_roboto_med_18};
    static const lv_font_t *const TINY[] = {&ui_next_cousine_bold_10, &ui_next_cousine_bold_10, &ui_next_cousine_bold_12};
    static const lv_font_t *const MONO[] = {&ui_next_cousine_reg_12, &ui_next_cousine_reg_12, &ui_next_cousine_reg_14};
    static const lv_font_t *const BIG[] = {&ui_next_cousine_reg_20, &ui_next_cousine_reg_20, &ui_next_cousine_reg_20};
    static const lv_font_t *const BPM[] = {&ui_next_cousine_reg_16, &ui_next_cousine_reg_16, &ui_next_cousine_reg_20};
    static const lv_font_t *const VAL[] = {&ui_next_roboto_reg_12, &ui_next_roboto_med_14, &ui_next_roboto_med_16};
    P.f_txt = TXT[cls];
    P.f_med = MED[cls];
    P.f_tiny = TINY[cls];
    P.f_mono = MONO[cls];
    P.f_big = BIG[cls];
    P.f_bpm = BPM[cls];
    P.f_val = VAL[cls];
}

static lv_obj_t *chip(lv_obj_t *parent, const char *s)
{
    lv_obj_t *c = lv_obj_create(parent);
    lv_obj_remove_style_all(c);
    lv_obj_remove_flag(c, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_size(c, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_color(c, col(P.pal->chip), 0);
    lv_obj_set_style_bg_opa(c, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(c, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_pad_hor(c, P.lay.tok.pad_px - 2, 0);
    lv_obj_set_style_pad_ver(c, 3, 0);
    lv_obj_set_flex_flow(c, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(c, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    label(c, s, P.f_mono, P.pal->text);
    return c;
}

static void build_top_bar(void)
{
    const ui_next_rect_t r = P.lay.top_bar;
    lv_obj_t *bar = box(P.root, r.x, r.y, r.w, r.h, P.pal->panel);
    box(bar, 0, r.h - 1, r.w, 1, P.pal->border);
    P.title = label(bar, "PERFORM", P.f_med, P.pal->text);
    lv_obj_align(P.title, LV_ALIGN_LEFT_MID, P.lay.tok.pad_px, 0);

    lv_obj_t *right = lv_obj_create(bar);
    lv_obj_remove_style_all(right);
    lv_obj_remove_flag(right, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(right, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(right, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(right, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(right, P.lay.tok.gap_px, 0);
    lv_obj_align(right, LV_ALIGN_RIGHT_MID, -P.lay.tok.pad_px, 0);
    P.chip_layers = chip(right, "LAYERS 1-4");
    if (P.lay.landscape && P.lay.cls != UI_NEXT_CLASS_S) {
        lv_obj_t *ext = chip(right, "EXT ON");
        (void)ext;
    }
}

static void build_nav(void)
{
    const ui_next_rect_t r = P.lay.nav;
    lv_obj_t *nav = box(P.root, r.x, r.y, r.w, r.h, P.pal->nav);
    if (P.lay.landscape) box(nav, r.w - 1, 0, 1, r.h, P.pal->border);
    else box(nav, 0, 0, r.w, 1, P.pal->border);
    const int isz = P.lay.cls == UI_NEXT_CLASS_S ? 24 : P.lay.cls == UI_NEXT_CLASS_M ? 26 : 29;
    for (int i = 0; i < UI_NEXT_PAGE_COUNT; i++) {
        const int iw = P.lay.landscape ? r.w : r.w / UI_NEXT_PAGE_COUNT;
        const int ih = P.lay.landscape ? r.h / UI_NEXT_PAGE_COUNT : r.h;
        lv_obj_t *item = box(nav, P.lay.landscape ? 0 : i * iw, P.lay.landscape ? i * ih : 0, iw, ih,
                             i == (int)P.page ? P.pal->sel : P.pal->nav);
        lv_obj_t *ic = lv_obj_create(item);
        lv_obj_remove_style_all(ic);
        lv_obj_remove_flag(ic, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_size(ic, isz, isz);
        lv_obj_align(ic, LV_ALIGN_CENTER, 0, -8);
        lv_obj_add_event_cb(ic, icon_draw_cb, LV_EVENT_DRAW_MAIN, (void *)(intptr_t)i);
        P.nav_icon[i] = ic;
        lv_obj_t *l = label(item, NAV_NAME[i], P.f_tiny, i == (int)P.page ? P.pal->accent_ink : P.pal->muted);
        lv_obj_align(l, LV_ALIGN_CENTER, 0, isz / 2 + 2);
    }
}

static lv_obj_t *wf_obj(int x, int y, int w, int h, int d)
{
    lv_obj_t *o = box(P.root, x, y, w, h, P.pal->well);
    lv_obj_set_style_radius(o, 4, 0);
    lv_obj_set_style_clip_corner(o, true, 0);
    lv_obj_add_event_cb(o, wf_draw_cb, LV_EVENT_DRAW_MAIN, (void *)(intptr_t)d);
    char t[24];
    snprintf(t, sizeof t, d ? "B  119.0 BPM  9A" : "A  124.0 BPM  8A");
    lv_obj_t *tag = box(o, 6, 4, LV_SIZE_CONTENT, LV_SIZE_CONTENT, P.pal->frame);
    lv_obj_set_style_bg_opa(tag, 166, 0);
    lv_obj_set_style_radius(tag, 3, 0);
    lv_obj_set_style_pad_hor(tag, 5, 0);
    lv_obj_set_style_pad_ver(tag, 1, 0);
    label(tag, t, P.f_tiny, P.pal->text);
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
            P.vu[d] = box(P.root, x0, y, vuw, dh, P.pal->frame);
            lv_obj_add_event_cb(P.vu[d], vu_draw_cb, LV_EVENT_DRAW_MAIN, (void *)(intptr_t)d);
            P.wf[d] = wf_obj(x0 + vuw + 3, y, w - vuw - 3, dh, d);
        }
        P.lane = box(P.root, x0 + vuw + 3, y0 + dh + gap, w - vuw - 3, lane, P.pal->nav);
    } else {
        const int dw = (w - lane - 2 * gap) / 2;
        for (int d = 0; d < 2; d++) {
            const int x = x0 + d * (dw + lane + 2 * gap);
            const int vx = d == 0 ? x : x + dw - vuw;
            const int wx = d == 0 ? x + vuw + 3 : x;
            P.vu[d] = box(P.root, vx, y0, vuw, h, P.pal->frame);
            lv_obj_add_event_cb(P.vu[d], vu_draw_cb, LV_EVENT_DRAW_MAIN, (void *)(intptr_t)d);
            P.wf[d] = wf_obj(wx, y0, dw - vuw - 3, h, d);
        }
        P.lane = box(P.root, x0 + dw + gap, y0, lane, h, P.pal->nav);
    }
    lv_obj_set_style_radius(P.lane, 3, 0);
    lv_obj_set_style_clip_corner(P.lane, true, 0);
    lv_obj_add_event_cb(P.lane, lane_draw_cb, LV_EVENT_DRAW_MAIN, NULL);
}

static void build_link_bar(void)
{
    const ui_next_rect_t r = P.lay.link_bar;
    const int pad = P.lay.tok.pad_px;
    lv_obj_t *bar = box(P.root, r.x + pad, r.y, r.w - 2 * pad, r.h - 2, 0x0E141B);
    if (P.day) lv_obj_set_style_bg_color(bar, col(P.pal->nav), 0);
    lv_obj_set_style_border_width(bar, 1, 0);
    lv_obj_set_style_border_color(bar, col(P.pal->border), 0);
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
        c->cell = box(bar, cx, cy, cw, cell_h, c->solid ? P.pal->ok : P.pal->nav);
        lv_obj_set_style_bg_opa(c->cell, c->solid ? LV_OPA_COVER : LV_OPA_TRANSP, 0);
        lv_obj_set_style_radius(c->cell, 4, 0);
        c->label = label(c->cell, "", P.f_tiny, c->solid ? 0x0B0F14 : P.pal->muted);
        c->value = label(c->cell, "", P.f_val, c->solid ? 0x0B0F14 : P.pal->text);
        if (c->solid) {
            lv_obj_align(c->label, LV_ALIGN_TOP_MID, 0, -1);
            lv_obj_align(c->value, LV_ALIGN_BOTTOM_MID, 0, 2);
        } else {
            lv_obj_align(c->label, LV_ALIGN_TOP_LEFT, 2, 0);
            lv_obj_align(c->value, LV_ALIGN_TOP_RIGHT, -2, 0);
            const int th = P.lay.tok.text_px / 2;
            c->track = box(c->cell, 2, cell_h - th - 1, cw - 4, th, P.day ? 0xD5DCE5 : 0x222C38);
            lv_obj_set_style_radius(c->track, 3, 0);
            c->fill = box(c->track, 0, 0, 0, th, P.pal->warn);
            c->zero = box(c->track, (cw - 4) / 2 - 1, -1, 2, th + 2, P.pal->text);
            lv_obj_set_style_bg_opa(c->zero, 200, 0);
        }
        col_i += span;
    }
}

static lv_obj_t *dock_card(int d)
{
    const ui_next_rect_t r = P.lay.deck_card[d];
    const int pad = P.lay.tok.pad_px, gap = P.lay.tok.gap_px;
    lv_obj_t *card = box(P.root, r.x, r.y, r.w, r.h, P.pal->nav);
    if (P.landscape && d == 1) box(card, 0, 0, 1, r.h, P.pal->border);
    if (!(P.landscape == false && d == 1)) box(card, 0, 0, r.w, 1, P.pal->border);

    lv_obj_t *head = lv_obj_create(card);
    lv_obj_remove_style_all(head);
    lv_obj_remove_flag(head, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_pos(head, pad, gap - 2);
    lv_obj_set_size(head, r.w - 2 * pad, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(head, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(head, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(head, gap, 0);

    label(head, d ? "B" : "A", P.f_big, P.pal->accent_ink);
    P.dk_title[d] = label(head, "", P.f_med, P.pal->text);
    lv_obj_set_flex_grow(P.dk_title[d], 1);
    lv_label_set_long_mode(P.dk_title[d], LV_LABEL_LONG_MODE_DOTS);
    P.dk_bpm[d] = label(head, "", P.f_bpm, P.pal->text);
    P.dk_key[d] = label(head, "", P.f_mono, P.pal->text);
    P.dk_badge[d] = label(head, "", P.f_tiny, P.pal->on_accent);
    P.dk_time[d] = label(head, "", P.f_mono, P.pal->text);
    lv_obj_t *chips[2] = {P.dk_key[d], P.dk_badge[d]};
    for (int i = 0; i < 2; i++) {
        lv_obj_set_style_bg_opa(chips[i], LV_OPA_COVER, 0);
        lv_obj_set_style_radius(chips[i], 5, 0);
        lv_obj_set_style_pad_hor(chips[i], 7, 0);
        lv_obj_set_style_pad_ver(chips[i], 1, 0);
    }

    lv_obj_update_layout(head);
    const int oy = gap - 2 + lv_obj_get_height(head) + 6;
    P.ov[d] = box(card, pad, oy, r.w - 2 * pad, r.h - oy - gap, P.pal->well);
    lv_obj_set_style_radius(P.ov[d], 4, 0);
    lv_obj_set_style_clip_corner(P.ov[d], true, 0);
    lv_obj_add_event_cb(P.ov[d], ov_draw_cb, LV_EVENT_DRAW_MAIN, (void *)(intptr_t)d);
    return card;
}

lv_obj_t *ui_next_perform_create(lv_obj_t *parent, uint16_t w, uint16_t h, bool day)
{
    memset(&P, 0, sizeof P);
    if (!ui_next_layout_compute(w, h, &P.lay)) return NULL;
    P.day = day;
    P.pal = ui_next_palette(day);
    P.landscape = P.lay.landscape;
    P.page = UI_NEXT_PAGE_PERFORM;
    static const uint16_t WIN[] = {144, 180, 216};
    P.mcols = WIN[P.lay.cls];
    pick_fonts(P.lay.cls);

    P.root = box(parent, 0, 0, w, h, P.pal->frame);
    build_waveforms();
    build_link_bar();
    dock_card(0);
    dock_card(1);
    build_top_bar();
    build_nav();
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
    lv_obj_set_style_bg_color(c->fill, col(colr), 0);
}

static void set_solid(link_cell_t *c, const char *name, const char *val, uint32_t colr)
{
    lv_label_set_text(c->label, name);
    lv_label_set_text(c->value, val);
    lv_obj_set_style_bg_color(c->cell, col(colr), 0);
    lv_obj_align(c->label, LV_ALIGN_TOP_MID, 0, -1);
    lv_obj_align(c->value, LV_ALIGN_BOTTOM_MID, 0, 2);
}

void ui_next_perform_update(const ui_next_perform_model_t *m)
{
    if (!P.root) return;
    P.model = *m;
    const ui_next_link_t *lk = &m->link;
    char s[24];

    lv_obj_t *lbl = lv_obj_get_child(P.chip_layers, 0);
    lv_label_set_text(lbl, m->layers);

    for (int d = 0; d < 2; d++) {
        const ui_next_deck_t *k = &m->deck[d];
        lv_label_set_text(P.dk_title[d], k->title);
        lv_label_set_text(P.dk_bpm[d], k->bpm);
        lv_label_set_text(P.dk_key[d], k->key);
        lv_label_set_text(P.dk_badge[d], k->badge);
        lv_label_set_text(P.dk_time[d], k->time);
        lv_obj_set_style_bg_color(P.dk_key[d], col(k->key_fit ? P.pal->ok : P.pal->chip), 0);
        lv_obj_set_style_text_color(P.dk_key[d], col(k->key_fit ? P.pal->on_accent : P.pal->text), 0);
        lv_obj_set_style_bg_color(P.dk_badge[d], col(k->badge_on ? P.pal->accent : (P.day ? 0xCBD3DD : 0x2A313C)), 0);
        lv_obj_set_style_text_color(P.dk_badge[d], col(k->badge_on ? P.pal->on_accent : P.pal->muted), 0);
        lv_obj_invalidate(P.wf[d]);
        lv_obj_invalidate(P.vu[d]);
        lv_obj_invalidate(P.ov[d]);
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
