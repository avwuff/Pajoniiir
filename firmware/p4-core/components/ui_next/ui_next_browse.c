#include <stdio.h>
#include <string.h>

#include "ui_next.h"
#include "ui_next_chrome.h"
#include "ui_next_util.h"

/* Browse page (screen 02, 05, 06 later share its chrome). Geometry follows the
 * CSS of the board: every size comes from the text size `fs` and the row height.
 * Only the rows that fit are built (LB-1); the model holds more. */

#define MAX_ROWS 16
#define MAX_TAG_ITEMS (UI_NEXT_BROWSE_TAGS + 2)

typedef struct {
    lv_obj_t *rw, *rwi, *th, *art, *title, *artist, *dots[3], *names[3], *genre, *bpm, *key, *time;
    lv_obj_t *acts, *back;
} row_t;

static struct {
    ui_next_layout_t lay;
    const ui_next_palette_t *pal;
    const ui_next_fonts_t *f;
    bool day, landscape, wide; /* wide: x1 columns (preview, genre, time) */
    int fs, row_h, pad, gap;
    lv_obj_t *root, *crumb, *count, *match, *rail_item[UI_NEXT_BROWSE_CATS], *rail_bar[UI_NEXT_BROWSE_CATS], *rail_badge;
    lv_obj_t *tag_pill[UI_NEXT_BROWSE_TAGS], *tag_dot[UI_NEXT_BROWSE_TAGS], *tag_txt[UI_NEXT_BROWSE_TAGS];
    row_t rows[MAX_ROWS];
    int n_rows;
    ui_next_browse_model_t m;
} B;

static const char *const CATS[UI_NEXT_BROWSE_CATS] = {"ARTIST", "ALBUM", "GENRE", "TAG", "KEY",
                                                      "PLAYLIST", "HISTORY", "MATCH", "FOLDER"};

#define KEY_NEAR 0xE8E05A
#define KEY_FAR 0xFF9A3C
#define KEY_FIT 0x3FD6C0
#define DARK 0x05070A

static uint32_t key_color(ui_next_key_class_t c)
{
    return c == UI_NEXT_KEY_FIT ? KEY_FIT : c == UI_NEXT_KEY_NEAR ? KEY_NEAR : KEY_FAR;
}

/* Transparent flex row child of fixed width; the label inside fills it. */
static lv_obj_t *cell_label(lv_obj_t *parent, int w, const lv_font_t *f, uint32_t rgb, lv_text_align_t al)
{
    lv_obj_t *c = lv_obj_create(parent);
    lv_obj_remove_style_all(c);
    lv_obj_remove_flag(c, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_size(c, w, LV_SIZE_CONTENT);
    lv_obj_t *l = ui_next_label(c, "", f, rgb);
    lv_obj_set_width(l, LV_PCT(100));
    lv_label_set_long_mode(l, LV_LABEL_LONG_MODE_DOTS);
    lv_obj_set_style_text_align(l, al, 0);
    return l;
}

static lv_obj_t *flex_row(lv_obj_t *parent, int w, int h, int pad_h, int gap)
{
    lv_obj_t *o = lv_obj_create(parent);
    lv_obj_remove_style_all(o);
    lv_obj_remove_flag(o, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_size(o, w, h);
    lv_obj_set_flex_flow(o, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(o, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_hor(o, pad_h, 0);
    lv_obj_set_style_pad_column(o, gap, 0);
    return o;
}

/* ---- top bar ---- */

static void build_top_bar(void)
{
    lv_obj_t *row = ui_next_chrome_topbar(B.root);
    ui_next_chrome_button(row, B.wide ? "BACK" : NULL, UI_NEXT_ICON_CHEVRON, false);
    B.crumb = ui_next_label(row, "", B.f->med, B.pal->text);
    if (B.wide) B.count = ui_next_label(row, "", B.f->mono, B.pal->muted);
    lv_obj_t *sp = lv_obj_create(row);
    lv_obj_remove_style_all(sp);
    lv_obj_set_size(sp, 1, 1);
    lv_obj_set_flex_grow(sp, 1);
    ui_next_chrome_button(row, B.wide ? "SEARCH" : NULL, UI_NEXT_ICON_SEARCH, false);
    if (B.wide) {
        ui_next_chrome_button(row, "EDIT", -1, false);
        ui_next_chrome_button(row, "PREVIEW ON", -1, true);
    }
}

/* ---- match strip, tags, rail ---- */

static lv_obj_t *pill(lv_obj_t *parent, const char *s, const lv_font_t *f, uint32_t bg, uint32_t fg)
{
    lv_obj_t *c = lv_obj_create(parent);
    lv_obj_remove_style_all(c);
    lv_obj_remove_flag(c, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_size(c, LV_SIZE_CONTENT, B.fs * 19 / 10);
    lv_obj_set_style_bg_color(c, ui_next_col(bg), 0);
    lv_obj_set_style_bg_opa(c, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(c, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_pad_hor(c, B.pad * 8 / 10, 0);
    lv_obj_set_flex_flow(c, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(c, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    ui_next_label(c, s, f, fg);
    return c;
}

static int build_match_strip(int x, int y, int w)
{
    const int h = B.fs * 24 / 10;
    lv_obj_t *bar = ui_next_box(B.root, x, y, w, h, B.pal->nav);
    ui_next_box(bar, 0, h - 1, w, 1, B.pal->border);
    lv_obj_t *row = flex_row(bar, w, h, B.pad, B.gap);
    B.match = ui_next_label(row, "", B.f->mono, B.pal->wave_mid);
    pill(row, "BPM +/- 4%", B.f->small, B.pal->chip, B.pal->text);
    pill(row, "KEY FITS", B.f->small, B.pal->chip, B.pal->text);
    if (B.wide) {
        static const struct { uint32_t c; const char *t; } LEG[] = {{KEY_FIT, "FITS"}, {KEY_NEAR, "+2"}, {KEY_FAR, "+7"}};
        for (int i = 0; i < 3; i++) {
            lv_obj_t *g = flex_row(row, LV_SIZE_CONTENT, LV_SIZE_CONTENT, 0, 4);
            const int sq = B.fs * 8 / 10;
            lv_obj_t *s = ui_next_box(g, 0, 0, sq, sq, LEG[i].c);
            lv_obj_set_style_radius(s, 3, 0);
            ui_next_label(g, LEG[i].t, B.f->small, B.pal->muted);
        }
    }
    return h;
}

static int build_tag_strip(int x, int y, int w)
{
    const int h = B.fs * 25 / 10;
    lv_obj_t *bar = ui_next_box(B.root, x, y, w, h, B.pal->well);
    ui_next_box(bar, 0, h - 1, w, 1, B.pal->border);
    lv_obj_t *row = flex_row(bar, w, h, B.pad, B.gap);

    lv_obj_t *any = pill(row, "ANY", B.f->mono, B.pal->chip, B.pal->accent_ink);
    lv_obj_set_style_border_width(any, 1, 0);
    lv_obj_set_style_border_color(any, ui_next_col(B.pal->accent_ink), 0);
    for (int i = 0; i < UI_NEXT_BROWSE_TAGS; i++) {
        lv_obj_t *p = pill(row, "", B.f->tagf, B.pal->panel, B.pal->text);
        lv_obj_set_height(p, B.fs * 18 / 10);
        lv_obj_set_style_border_width(p, 1, 0);
        lv_obj_set_style_pad_column(p, 6, 0);
        lv_obj_clean(p);
        B.tag_dot[i] = ui_next_box(p, 0, 0, 8, 8, DARK);
        lv_obj_set_style_radius(B.tag_dot[i], LV_RADIUS_CIRCLE, 0);
        B.tag_txt[i] = ui_next_label(p, "", B.f->tagf, B.pal->text);
        B.tag_pill[i] = p;
    }
    lv_obj_t *add = pill(row, "+ TAG", B.f->tagf, B.pal->well, B.pal->muted);
    lv_obj_set_style_border_width(add, 1, 0);
    lv_obj_set_style_border_color(add, ui_next_col(B.pal->border_hi), 0);
    return h;
}

static void build_rail(int x, int y, int w, int h)
{
    lv_obj_t *rail = ui_next_box(B.root, x, y, w, h, B.pal->nav);
    if (B.landscape) ui_next_box(rail, w - 1, 0, 1, h, B.pal->border);
    else ui_next_box(rail, 0, h - 1, w, 1, B.pal->border);
    lv_obj_t *flow = lv_obj_create(rail);
    lv_obj_remove_style_all(flow);
    lv_obj_remove_flag(flow, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_size(flow, w, h);
    lv_obj_set_flex_flow(flow, B.landscape ? LV_FLEX_FLOW_COLUMN : LV_FLEX_FLOW_ROW);
    for (int i = 0; i < UI_NEXT_BROWSE_CATS; i++) {
        const int ih = B.landscape ? B.row_h * 8 / 10 : h;
        lv_obj_t *it = flex_row(flow, B.landscape ? w : LV_SIZE_CONTENT, ih, B.pad, 0);
        lv_obj_set_style_border_side(it, LV_BORDER_SIDE_BOTTOM, 0);
        lv_obj_set_style_border_width(it, B.landscape ? 1 : 0, 0);
        lv_obj_set_style_border_color(it, ui_next_col(B.pal->panel), 0);
        lv_obj_set_style_bg_opa(it, LV_OPA_COVER, 0);
        lv_obj_t *l = ui_next_label(it, CATS[i], B.f->mono, B.pal->muted);
        lv_obj_set_style_text_letter_space(l, 1, 0);
        if (i == 3) {
            lv_obj_t *bd = ui_next_label(it, "", B.f->tiny, B.pal->on_accent);
            lv_obj_set_style_bg_color(bd, ui_next_col(B.pal->accent), 0);
            lv_obj_set_style_bg_opa(bd, LV_OPA_COVER, 0);
            lv_obj_set_style_radius(bd, LV_RADIUS_CIRCLE, 0);
            lv_obj_set_style_pad_hor(bd, 5, 0);
            lv_obj_set_style_margin_left(bd, 6, 0);
            B.rail_badge = bd;
        }
        B.rail_bar[i] = ui_next_box(it, 0, 0, 3, ih, B.pal->accent_ink);
        lv_obj_add_flag(B.rail_bar[i], LV_OBJ_FLAG_FLOATING);
        lv_obj_set_pos(B.rail_bar[i], -B.pad, 0);
        B.rail_item[i] = it;
    }
}

/* ---- list ---- */

static void sort_arrow_cb(lv_event_t *e)
{
    lv_layer_t *layer = lv_event_get_layer(e);
    lv_area_t a;
    lv_obj_get_coords(lv_event_get_target_obj(e), &a);
    lv_draw_triangle_dsc_t t;
    lv_draw_triangle_dsc_init(&t);
    t.color = ui_next_col(B.pal->accent_ink);
    t.opa = LV_OPA_COVER;
    t.p[0].x = a.x1 + 4;
    t.p[0].y = a.y1 + 1;
    t.p[1].x = a.x1 + 8;
    t.p[1].y = a.y1 + 8;
    t.p[2].x = a.x1;
    t.p[2].y = a.y1 + 8;
    lv_draw_triangle(layer, &t);
}

static void dash_cb(lv_event_t *e)
{
    lv_layer_t *layer = lv_event_get_layer(e);
    lv_area_t a;
    lv_obj_get_coords(lv_event_get_target_obj(e), &a);
    for (int x = a.x1; x < a.x2; x += 5) ui_next_fill_rect(layer, x, a.y2, x + 2, a.y2, B.pal->border_hi, LV_OPA_COVER);
}

static void preview_cb(lv_event_t *e)
{
    const int i = (int)(intptr_t)lv_event_get_user_data(e);
    lv_area_t a;
    lv_obj_get_coords(lv_event_get_target_obj(e), &a);
    ui_next_chrome_draw_overview(lv_event_get_layer(e), &a, B.m.preview_env[B.m.track[i].env], -1.0f, B.pal);
}

static void build_header(lv_obj_t *list, int w)
{
    const int h = B.fs * 22 / 10;
    lv_obj_t *lh = ui_next_box(list, 0, 0, w, h, B.pal->panel);
    ui_next_box(lh, 0, h - 1, w, 1, B.pal->border);
    lv_obj_t *row = flex_row(lh, w, h, B.pad, B.gap);
    const lv_font_t *f = B.f->mono;
    if (B.wide) cell_label(row, B.row_h * 2, f, B.pal->muted, LV_TEXT_ALIGN_LEFT);
    if (B.wide) lv_label_set_text(lv_obj_get_child(lv_obj_get_child(row, 0), 0), "PREVIEW");
    lv_obj_t *sp = lv_obj_create(row);
    lv_obj_remove_style_all(sp);
    lv_obj_set_size(sp, B.row_h * 78 / 100, 1);
    lv_obj_t *track = ui_next_label(row, "TRACK", f, B.pal->muted);
    lv_obj_set_flex_grow(track, 1);
    if (B.wide) {
        lv_obj_t *g = cell_label(row, B.fs * 65 / 10, f, B.pal->muted, LV_TEXT_ALIGN_LEFT);
        lv_label_set_text(g, "GENRE v");
        lv_obj_t *du = lv_obj_get_parent(g);
        lv_obj_add_event_cb(du, dash_cb, LV_EVENT_DRAW_POST, NULL);
    }
    lv_obj_t *bp = flex_row(row, B.fs * 46 / 10, LV_SIZE_CONTENT, 0, 4);
    lv_obj_set_flex_align(bp, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    ui_next_label(bp, "BPM", f, B.pal->accent_ink);
    lv_obj_t *ar = lv_obj_create(bp);
    lv_obj_remove_style_all(ar);
    lv_obj_set_size(ar, 9, 9);
    lv_obj_add_event_cb(ar, sort_arrow_cb, LV_EVENT_DRAW_MAIN, NULL);
    lv_obj_t *k = cell_label(row, B.fs * 42 / 10, f, B.pal->muted, LV_TEXT_ALIGN_CENTER);
    lv_label_set_text(k, "KEY");
    if (B.wide) {
        lv_obj_t *t = cell_label(row, B.fs * 34 / 10, f, B.pal->muted, LV_TEXT_ALIGN_RIGHT);
        lv_label_set_text(t, "TIME");
    }
}

static void build_row(lv_obj_t *list, int i, int y, int w)
{
    row_t *r = &B.rows[i];
    r->rw = ui_next_box(list, 0, y, w, B.row_h, B.pal->frame);
    ui_next_box(r->rw, 0, B.row_h - 1, w, 1, B.pal->panel);

    /* Swipe actions sit under the row and show when the row slides left. */
    const int aw = B.row_h * 46 / 10;
    r->acts = flex_row(r->rw, aw, B.row_h, 0, 2);
    lv_obj_set_pos(r->acts, w - aw, 0);
    static const char *const ACT[4] = {"LOAD A", "LOAD B", "QUEUE", "TAG"};
    for (int a = 0; a < 4; a++) {
        lv_obj_t *b = ui_next_box(r->acts, 0, 0, 0, B.row_h, a < 2 ? B.pal->accent : B.pal->border_hi);
        lv_obj_set_flex_grow(b, 1);
        lv_obj_t *l = ui_next_label(b, ACT[a], B.f->row, a < 2 ? B.pal->on_accent : (B.day ? 0xFFFFFF : B.pal->text));
        lv_obj_center(l);
    }

    r->rwi = flex_row(r->rw, w, B.row_h, B.pad, B.gap);
    lv_obj_set_style_bg_opa(r->rwi, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(r->rwi, ui_next_col(B.pal->frame), 0);

    if (B.wide) {
        r->th = ui_next_box(r->rwi, 0, 0, B.row_h * 2, B.row_h * 62 / 100, B.pal->well);
        lv_obj_set_style_radius(r->th, 4, 0);
        lv_obj_set_style_clip_corner(r->th, true, 0);
        lv_obj_add_event_cb(r->th, preview_cb, LV_EVENT_DRAW_MAIN, (void *)(intptr_t)i);
    }
    const int as = B.row_h * 78 / 100;
    r->art = ui_next_box(r->rwi, 0, 0, as, as, B.pal->panel);
    lv_obj_set_style_radius(r->art, 6, 0);
    lv_obj_set_style_bg_grad_dir(r->art, LV_GRAD_DIR_VER, 0);

    lv_obj_t *tx = lv_obj_create(r->rwi);
    lv_obj_remove_style_all(tx);
    lv_obj_remove_flag(tx, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_height(tx, LV_SIZE_CONTENT);
    lv_obj_set_flex_grow(tx, 1);
    lv_obj_set_flex_flow(tx, LV_FLEX_FLOW_COLUMN);
    r->title = ui_next_label(tx, "", B.f->row, B.pal->text);
    lv_obj_set_width(r->title, LV_PCT(100));
    lv_label_set_long_mode(r->title, LV_LABEL_LONG_MODE_DOTS);
    lv_obj_t *ar = flex_row(tx, LV_PCT(100), LV_SIZE_CONTENT, 0, B.gap);
    r->artist = ui_next_label(ar, "", B.f->small, B.pal->muted);
    for (int t = 0; t < 3; t++) {
        lv_obj_t *g = flex_row(ar, LV_SIZE_CONTENT, LV_SIZE_CONTENT, 0, 4);
        r->dots[t] = ui_next_box(g, 0, 0, 8, 8, DARK);
        lv_obj_set_style_radius(r->dots[t], LV_RADIUS_CIRCLE, 0);
        if (B.wide) r->names[t] = ui_next_label(g, "", B.f->small, B.day ? B.pal->muted : 0xC9D1DC);
        lv_obj_set_user_data(r->dots[t], g);
    }

    if (B.wide) r->genre = cell_label(r->rwi, B.fs * 65 / 10, B.f->small, B.pal->muted, LV_TEXT_ALIGN_LEFT);
    r->bpm = cell_label(r->rwi, B.fs * 46 / 10, B.f->rmono, B.pal->text, LV_TEXT_ALIGN_RIGHT);
    lv_obj_t *kc = lv_obj_create(r->rwi);
    lv_obj_remove_style_all(kc);
    lv_obj_remove_flag(kc, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_size(kc, B.fs * 42 / 10, LV_SIZE_CONTENT);
    r->key = ui_next_label(kc, "", B.f->small, DARK);
    lv_obj_set_width(r->key, B.fs * 36 / 10);
    lv_obj_align(r->key, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_align(r->key, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_pad_ver(r->key, 3, 0);
    lv_obj_set_style_radius(r->key, 6, 0);
    if (B.wide) r->time = cell_label(r->rwi, B.fs * 34 / 10, B.f->mono, B.pal->muted, LV_TEXT_ALIGN_RIGHT);

    /* Close arrow of an open row. */
    const int cs = B.row_h * 6 / 10;
    r->back = ui_next_box(r->rw, B.pad, (B.row_h - cs) / 2, cs, cs, B.day ? 0xDDE6F2 : 0x2A3444);
    lv_obj_set_style_radius(r->back, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_border_width(r->back, 2, 0);
    lv_obj_set_style_border_color(r->back, ui_next_col(B.pal->muted), 0);
    lv_obj_t *ic = ui_next_chrome_icon(r->back, cs * 6 / 10, UI_NEXT_ICON_ARROW);
    lv_obj_center(ic);
}

/* ---- build ---- */

lv_obj_t *ui_next_browse_create(lv_obj_t *parent, uint16_t w, uint16_t h, bool day)
{
    memset(&B, 0, sizeof B);
    if (!ui_next_layout_compute(w, h, &B.lay)) return NULL;
    B.day = day;
    B.pal = ui_next_palette(day);
    B.f = ui_next_fonts_for(B.lay.cls);
    B.landscape = B.lay.landscape;
    B.wide = B.landscape && w >= 1000u;
    B.fs = B.lay.tok.text_px;
    B.row_h = B.lay.tok.row_px;
    B.pad = B.lay.tok.pad_px;
    B.gap = B.lay.tok.gap_px;
    ui_next_chrome_begin(&B.lay, day, UI_NEXT_PAGE_BROWSE);

    B.root = ui_next_box(parent, 0, 0, w, h, B.pal->frame);
    const int x0 = B.lay.dock.x, zw = B.lay.dock.w;
    int y = B.lay.top_bar.h;
    y += build_match_strip(x0, y, zw);
    y += build_tag_strip(x0, y, zw);
    const int bh = B.lay.dock.y - y;

    static const int RAILW[] = {104, 112, 128}, RAILH[] = {42, 45, 51};
    int lx = x0, ly = y, lw = zw, lh = bh;
    if (B.landscape) {
        build_rail(x0, y, RAILW[B.lay.cls], bh);
        lx += RAILW[B.lay.cls];
        lw -= RAILW[B.lay.cls];
    } else {
        build_rail(x0, y, zw, RAILH[B.lay.cls]);
        ly += RAILH[B.lay.cls];
        lh -= RAILH[B.lay.cls];
    }
    lv_obj_t *list = ui_next_box(B.root, lx, ly, lw, lh, B.pal->frame);
    build_header(list, lw);
    const int hh = B.fs * 22 / 10;
    B.n_rows = (lh - hh + B.row_h - 1) / B.row_h;
    if (B.n_rows > MAX_ROWS) B.n_rows = MAX_ROWS;
    for (int i = 0; i < B.n_rows; i++) build_row(list, i, hh + i * B.row_h, lw);

    ui_next_chrome_dock(B.root, 0, false);
    ui_next_chrome_dock(B.root, 1, false);
    build_top_bar();
    ui_next_chrome_nav(B.root);
    return B.root;
}

/* ---- update ---- */

static void style_key(lv_obj_t *l, ui_next_key_class_t c)
{
    if (c == UI_NEXT_KEY_NONE) {
        lv_obj_set_style_bg_opa(l, LV_OPA_TRANSP, 0);
        lv_obj_set_style_text_color(l, ui_next_col(B.pal->text), 0);
    } else {
        lv_obj_set_style_bg_opa(l, LV_OPA_COVER, 0);
        lv_obj_set_style_bg_color(l, ui_next_col(key_color(c)), 0);
        lv_obj_set_style_text_color(l, ui_next_col(DARK), 0);
    }
}

void ui_next_browse_update(const ui_next_browse_model_t *m)
{
    if (!B.root) return;
    B.m = *m;
    lv_label_set_text(B.crumb, m->crumb);
    if (B.count) lv_label_set_text(B.count, m->count);
    lv_label_set_text(B.match, m->match);

    for (int i = 0; i < UI_NEXT_BROWSE_CATS; i++) {
        const bool on = i == m->category;
        lv_obj_t *it = B.rail_item[i];
        lv_obj_set_style_bg_color(it, ui_next_col(on ? B.pal->chip : B.pal->nav), 0);
        lv_obj_set_style_text_color(lv_obj_get_child(it, 0), ui_next_col(on ? B.pal->accent_ink : B.pal->muted), 0);
        if (on) lv_obj_remove_flag(B.rail_bar[i], LV_OBJ_FLAG_HIDDEN);
        else lv_obj_add_flag(B.rail_bar[i], LV_OBJ_FLAG_HIDDEN);
    }
    if (m->tag_badge) {
        char b[4];
        snprintf(b, sizeof b, "%u", (unsigned)m->tag_badge);
        lv_label_set_text(B.rail_badge, b);
        lv_obj_remove_flag(B.rail_badge, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(B.rail_badge, LV_OBJ_FLAG_HIDDEN);
    }

    for (int i = 0; i < UI_NEXT_BROWSE_TAGS; i++) {
        const ui_next_tag_t *t = &m->tags[i];
        lv_label_set_text(B.tag_txt[i], t->name);
        lv_obj_set_style_bg_color(B.tag_pill[i], ui_next_col(t->on ? t->color : B.pal->panel), 0);
        lv_obj_set_style_border_color(B.tag_pill[i], ui_next_col(t->color), 0);
        lv_obj_set_style_border_opa(B.tag_pill[i], t->on ? LV_OPA_COVER : 136, 0);
        lv_obj_set_style_bg_color(B.tag_dot[i], ui_next_col(t->on ? DARK : t->color), 0);
        lv_obj_set_style_text_color(B.tag_txt[i], ui_next_col(t->on ? DARK : B.pal->text), 0);
    }

    for (int i = 0; i < B.n_rows; i++) {
        row_t *r = &B.rows[i];
        const ui_next_track_t *t = &m->track[i];
        const bool open = i == m->open_row;
        lv_label_set_text(r->title, t->title);
        lv_label_set_text(r->artist, t->artist);
        for (int g = 0; g < 3; g++) {
            lv_obj_t *grp = lv_obj_get_user_data(r->dots[g]);
            if (g < t->n_tags) {
                lv_obj_remove_flag(grp, LV_OBJ_FLAG_HIDDEN);
                lv_obj_set_style_bg_color(r->dots[g], ui_next_col(m->tags[t->tags[g]].color), 0);
                if (r->names[g]) lv_label_set_text(r->names[g], m->tags[t->tags[g]].name);
            } else {
                lv_obj_add_flag(grp, LV_OBJ_FLAG_HIDDEN);
            }
        }
        lv_obj_set_style_bg_color(r->art, ui_next_col(t->art0), 0);
        lv_obj_set_style_bg_grad_color(r->art, ui_next_col(t->art1), 0);
        if (r->genre) lv_label_set_text(r->genre, t->genre);
        lv_label_set_text(r->bpm, t->bpm);
        lv_label_set_text(r->key, t->key);
        style_key(r->key, t->key_class);
        if (r->time) lv_label_set_text(r->time, t->time);
        if (r->th) lv_obj_invalidate(r->th);

        lv_obj_set_x(r->rwi, open ? -(B.row_h * 46 / 10) : 0);
        lv_obj_set_style_bg_color(r->rwi, ui_next_col(open ? B.pal->sel : B.pal->frame), 0);
        if (open) {
            lv_obj_remove_flag(r->acts, LV_OBJ_FLAG_HIDDEN);
            lv_obj_remove_flag(r->back, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_add_flag(r->acts, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(r->back, LV_OBJ_FLAG_HIDDEN);
        }
    }
    ui_next_chrome_dock_set(0, &m->deck[0]);
    ui_next_chrome_dock_set(1, &m->deck[1]);
}
