#include <string.h>

#include "ui_next_chrome.h"

static struct {
    ui_next_layout_t lay;
    const ui_next_palette_t *pal;
    const ui_next_fonts_t *f;
    bool day;
    ui_next_page_t page;
    lv_obj_t *title[2], *bpm[2], *key[2], *badge[2], *time[2], *ov[2];
    const ui_next_env_t *env[2];
    float pos01[2];
} C;

static const char *const NAV_NAME[UI_NEXT_PAGE_COUNT] = {"PERFORM", "BROWSE", "FX", "RECORD", "SETTINGS"};

void ui_next_chrome_begin(const ui_next_layout_t *lay, bool day, ui_next_page_t active)
{
    memset(&C, 0, sizeof C);
    C.lay = *lay;
    C.day = day;
    C.pal = ui_next_palette(day);
    C.f = ui_next_fonts_for(lay->cls);
    C.page = active;
}

/* ---- overview ---- */

void ui_next_chrome_draw_overview(lv_layer_t *layer, const lv_area_t *a, const ui_next_env_t *env,
                                  float pos01, const ui_next_palette_t *pal)
{
    if (!env) return;
    const int w = lv_area_get_width(a), h = lv_area_get_height(a), c0 = a->y1 + h / 2;
    const float amp[3] = {.49f, .38f, .26f};
    const uint32_t colr[3] = {pal->wave_low, pal->wave_mid, pal->wave_high};
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
            ui_next_fill_rect(layer, a->x1 + i, c0 - half, a->x1 + i, c0 + half, colr[b], LV_OPA_COVER);
        }
    }
    if (pos01 < 0.0f) return;
    static const float CUES[] = {.04f, .13f, .26f, .5f, .62f, .9f};
    for (unsigned i = 0; i < sizeof CUES / sizeof CUES[0]; i++) {
        int x = a->x1 + (int)(CUES[i] * (float)w);
        ui_next_fill_rect(layer, x, a->y1, x + 2, a->y2, pal->bad, LV_OPA_COVER);
    }
    const int px = a->x1 + (int)(pos01 * (float)w);
    ui_next_fill_rect(layer, a->x1, a->y1, px - 1, a->y2, pal->frame, 140);
    ui_next_fill_rect(layer, px, a->y1, px + 1, a->y2, pal->accent, LV_OPA_COVER);
}

static void ov_draw_cb(lv_event_t *e)
{
    const int d = (int)(intptr_t)lv_event_get_user_data(e);
    lv_area_t a;
    lv_obj_get_coords(lv_event_get_target_obj(e), &a);
    ui_next_chrome_draw_overview(lv_event_get_layer(e), &a, C.env[d], C.pos01[d], C.pal);
}

/* ---- nav icons, drawn from segments in a 24 unit box ---- */

static void icon_draw_cb(lv_event_t *e)
{
    const int page = (int)(intptr_t)lv_event_get_user_data(e);
    lv_layer_t *layer = lv_event_get_layer(e);
    lv_area_t a;
    lv_obj_get_coords(lv_event_get_target_obj(e), &a);
    const float k = (float)lv_area_get_width(&a) / 24.0f;
    const uint32_t c = (int)C.page == page ? C.pal->accent_ink : C.pal->muted;
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
        d.border_color = ui_next_col(c);
        lv_area_t r = {a.x1 + (int)(6 * k), a.y1 + (int)(6 * k), a.x1 + (int)(18 * k), a.y1 + (int)(18 * k)};
        lv_draw_rect(layer, &d, &r);
        return;
    }
    for (int i = 0; i < COUNT[page]; i++) {
        const int8_t *s = S[FIRST[page] + i];
        ui_next_seg(layer, (float)a.x1 + (float)s[0] * k, (float)a.y1 + (float)s[1] * k,
                    (float)a.x1 + (float)s[2] * k, (float)a.y1 + (float)s[3] * k, 2, c, LV_OPA_COVER);
    }
}

lv_obj_t *ui_next_chrome_nav(lv_obj_t *root)
{
    const ui_next_rect_t r = C.lay.nav;
    lv_obj_t *nav = ui_next_box(root, r.x, r.y, r.w, r.h, C.pal->nav);
    if (C.lay.landscape) ui_next_box(nav, r.w - 1, 0, 1, r.h, C.pal->border);
    else ui_next_box(nav, 0, 0, r.w, 1, C.pal->border);
    const int isz = C.lay.cls == UI_NEXT_CLASS_S ? 24 : C.lay.cls == UI_NEXT_CLASS_M ? 26 : 29;
    for (int i = 0; i < UI_NEXT_PAGE_COUNT; i++) {
        const int iw = C.lay.landscape ? r.w : r.w / UI_NEXT_PAGE_COUNT;
        const int ih = C.lay.landscape ? r.h / UI_NEXT_PAGE_COUNT : r.h;
        lv_obj_t *item = ui_next_box(nav, C.lay.landscape ? 0 : i * iw, C.lay.landscape ? i * ih : 0, iw, ih,
                                     i == (int)C.page ? C.pal->sel : C.pal->nav);
        lv_obj_t *ic = lv_obj_create(item);
        lv_obj_remove_style_all(ic);
        lv_obj_remove_flag(ic, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_size(ic, isz, isz);
        lv_obj_align(ic, LV_ALIGN_CENTER, 0, -8);
        lv_obj_add_event_cb(ic, icon_draw_cb, LV_EVENT_DRAW_MAIN, (void *)(intptr_t)i);
        lv_obj_t *l = ui_next_label(item, NAV_NAME[i], C.f->tiny, i == (int)C.page ? C.pal->accent_ink : C.pal->muted);
        lv_obj_align(l, LV_ALIGN_CENTER, 0, isz / 2 + 2);
    }
    return nav;
}

lv_obj_t *ui_next_chrome_chip(lv_obj_t *parent, const char *s)
{
    lv_obj_t *c = lv_obj_create(parent);
    lv_obj_remove_style_all(c);
    lv_obj_remove_flag(c, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_size(c, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_color(c, ui_next_col(C.pal->chip), 0);
    lv_obj_set_style_bg_opa(c, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(c, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_pad_hor(c, C.lay.tok.pad_px - 2, 0);
    lv_obj_set_style_pad_ver(c, 3, 0);
    lv_obj_set_flex_flow(c, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(c, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    ui_next_label(c, s, C.f->mono, C.pal->text);
    return c;
}

/* ---- dock ---- */

void ui_next_chrome_dock(lv_obj_t *root, int d, bool full)
{
    const ui_next_rect_t r = C.lay.deck_card[d];
    const int pad = C.lay.tok.pad_px, gap = C.lay.tok.gap_px;
    lv_obj_t *card = ui_next_box(root, r.x, r.y, r.w, r.h, C.pal->nav);
    if (C.lay.landscape && d == 1) ui_next_box(card, 0, 0, 1, r.h, C.pal->border);
    if (!(C.lay.landscape == false && d == 1)) ui_next_box(card, 0, 0, r.w, 1, C.pal->border);

    lv_obj_t *head = lv_obj_create(card);
    lv_obj_remove_style_all(head);
    lv_obj_remove_flag(head, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_pos(head, pad, gap - 2);
    lv_obj_set_size(head, r.w - 2 * pad, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(head, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(head, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(head, gap, 0);

    ui_next_label(head, d ? "B" : "A", C.f->big, C.pal->accent_ink);
    C.title[d] = ui_next_label(head, "", C.f->med, C.pal->text);
    lv_obj_set_flex_grow(C.title[d], 1);
    lv_label_set_long_mode(C.title[d], LV_LABEL_LONG_MODE_DOTS);
    if (full) {
        C.bpm[d] = ui_next_label(head, "", C.f->bpm, C.pal->text);
        C.key[d] = ui_next_label(head, "", C.f->mono, C.pal->text);
        C.badge[d] = ui_next_label(head, "", C.f->tiny, C.pal->on_accent);
        lv_obj_t *chips[2] = {C.key[d], C.badge[d]};
        for (int i = 0; i < 2; i++) {
            lv_obj_set_style_bg_opa(chips[i], LV_OPA_COVER, 0);
            lv_obj_set_style_radius(chips[i], 5, 0);
            lv_obj_set_style_pad_hor(chips[i], 7, 0);
            lv_obj_set_style_pad_ver(chips[i], 1, 0);
        }
    }
    C.time[d] = ui_next_label(head, "", C.f->mono, C.pal->text);

    lv_obj_update_layout(head);
    const int oy = gap - 2 + lv_obj_get_height(head) + 6;
    C.ov[d] = ui_next_box(card, pad, oy, r.w - 2 * pad, r.h - oy - gap, C.pal->well);
    lv_obj_set_style_radius(C.ov[d], 4, 0);
    lv_obj_set_style_clip_corner(C.ov[d], true, 0);
    lv_obj_add_event_cb(C.ov[d], ov_draw_cb, LV_EVENT_DRAW_MAIN, (void *)(intptr_t)d);
}

void ui_next_chrome_dock_set(int d, const ui_next_deck_t *k)
{
    if (!C.title[d]) return;
    lv_label_set_text(C.title[d], k->title);
    lv_label_set_text(C.time[d], k->time);
    if (C.bpm[d]) {
        lv_label_set_text(C.bpm[d], k->bpm);
        lv_label_set_text(C.key[d], k->key);
        lv_label_set_text(C.badge[d], k->badge);
        lv_obj_set_style_bg_color(C.key[d], ui_next_col(k->key_fit ? C.pal->ok : C.pal->chip), 0);
        lv_obj_set_style_text_color(C.key[d], ui_next_col(k->key_fit ? C.pal->on_accent : C.pal->text), 0);
        lv_obj_set_style_bg_color(C.badge[d], ui_next_col(k->badge_on ? C.pal->accent : (C.day ? 0xCBD3DD : 0x2A313C)), 0);
        lv_obj_set_style_text_color(C.badge[d], ui_next_col(k->badge_on ? C.pal->on_accent : C.pal->muted), 0);
    }
    C.env[d] = k->env;
    C.pos01[d] = k->pos01;
    lv_obj_invalidate(C.ov[d]);
}
