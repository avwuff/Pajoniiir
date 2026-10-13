#include <stdio.h>
#include <string.h>

#include "ui_next.h"
#include "ui_next_chrome.h"
#include "ui_next_util.h"

/* Track info page (screen 03): header with cover and tags, full waveform with
 * the phrase map, hot cue chips, six file stats and four actions. Landscape puts
 * the waveform column (1.5) beside the stats column (1); portrait stacks them. */

static const char *const STAT_LABEL[6] = {"BPM", "KEY", "TIME", "BITRATE", "PLAYS", "GRID"};
static const char *const ACTION[4] = {"LOAD A", "LOAD B", "QUEUE", "EDIT GRID"};

static struct {
    ui_next_layout_t lay;
    const ui_next_palette_t *pal;
    const ui_next_fonts_t *f;
    bool day, landscape, wide;
    int row_h, pad, gap, fs;
    lv_obj_t *root, *title, *artist, *album, *art, *tag_genre, *tag_label, *tag_label_txt, *tag_genre_txt;
    lv_obj_t *wave, *phrase_box, *preview, *phrase_lbl[UI_NEXT_TI_PHRASES], *phrase_seg[UI_NEXT_TI_PHRASES];
    lv_obj_t *cue_chip[UI_NEXT_TI_CUES], *cue_badge[UI_NEXT_TI_CUES], *cue_time[UI_NEXT_TI_CUES];
    lv_obj_t *stat_val[6];
    ui_next_trackinfo_model_t m;
    lv_grad_dsc_t cover;
} T;

static void wave_cb(lv_event_t *e)
{
    lv_area_t a;
    lv_obj_get_coords(lv_event_get_target_obj(e), &a);
    ui_next_chrome_draw_overview_ex(lv_event_get_layer(e), &a, T.m.env, T.m.play01, true, false, 0x3FD6C0, T.pal);
}

static lv_obj_t *pill(lv_obj_t *parent, int x, int y, lv_obj_t **text_out)
{
    lv_obj_t *c = ui_next_box(parent, x, y, 0, T.fs * 19 / 10, T.pal->chip);
    lv_obj_set_width(c, LV_SIZE_CONTENT);
    lv_obj_set_style_radius(c, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_pad_hor(c, T.pad * 8 / 10, 0);
    lv_obj_set_flex_flow(c, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(c, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    *text_out = ui_next_label(c, "", T.f->small, T.pal->text);
    return c;
}

/* Header: cover 1.7 rows square, title, artist, album, tag pills. */
static int build_header(int x, int y)
{
    const int art = T.row_h * 17 / 10;
    T.art = ui_next_box(T.root, x, y, art, art, T.pal->chip);
    lv_obj_set_style_radius(T.art, 6, 0);
    const int tx = x + art + T.pad;
    T.title = ui_next_label(T.root, "", T.f->h2, T.pal->text);
    lv_obj_set_pos(T.title, tx, y);
    T.artist = ui_next_label(T.root, "", T.f->txt, T.pal->muted);
    T.album = ui_next_label(T.root, "", T.f->small, T.pal->muted);
    const int h2 = T.f->h2->line_height * 115 / 100;
    lv_obj_set_pos(T.artist, tx, y + h2);
    lv_obj_set_pos(T.album, tx, y + h2 + T.f->txt->line_height);
    const int ty = y + h2 + T.f->txt->line_height + T.f->small->line_height + 6;
    T.tag_genre = pill(T.root, tx, ty, &T.tag_genre_txt);
    if (T.wide) {
        T.tag_label = pill(T.root, tx + 0, ty, &T.tag_label_txt); /* moved after the genre pill on update */
    }
    return art;
}

static int phrase_bar(lv_obj_t *card, int x, int y, int w, int h)
{
    T.phrase_box = lv_obj_create(card);
    lv_obj_remove_style_all(T.phrase_box);
    lv_obj_remove_flag(T.phrase_box, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_pos(T.phrase_box, x, y);
    lv_obj_set_size(T.phrase_box, w, h);
    return h;
}

static void layout_phrases(void)
{
    const int w = lv_obj_get_width(T.phrase_box), h = lv_obj_get_height(T.phrase_box);
    int total = 0;
    for (int i = 0; i < UI_NEXT_TI_PHRASES; i++) total += T.m.phrase[i].weight;
    const int avail = w - (UI_NEXT_TI_PHRASES - 1) * 2;
    int before = 0;
    for (int i = 0; i < UI_NEXT_TI_PHRASES; i++) {
        const int x0 = avail * before / total + i * 2;
        before += T.m.phrase[i].weight;
        const int x1 = avail * before / total + i * 2;
        lv_obj_set_pos(T.phrase_seg[i], x0, 0);
        lv_obj_set_size(T.phrase_seg[i], x1 - x0, h);
    }
}

/* Hot cue chips: equal natural width, packed into lines, each line stretched. */
static int build_cues(int x, int y, int w)
{
    const int ch = T.row_h * 58 / 100, sq = T.fs * 15 / 10, nat = 16 + sq + 6 + T.f->mono->line_height * 25 / 10;
    int per = (w + 6) / (nat + 6);
    if (per < 1) per = 1;
    if (per > UI_NEXT_TI_CUES) per = UI_NEXT_TI_CUES;
    const int lines = (UI_NEXT_TI_CUES + per - 1) / per;
    for (int i = 0; i < UI_NEXT_TI_CUES; i++) {
        const int line = i / per, first = line * per;
        const int in_line = UI_NEXT_TI_CUES - first < per ? UI_NEXT_TI_CUES - first : per;
        const int cw = (w - (in_line - 1) * 6) / in_line;
        const int cx = x + (i - first) * (cw + 6);
        T.cue_chip[i] = ui_next_box(T.root, cx, y + line * (ch + 6), cw, ch, T.pal->panel);
        lv_obj_set_style_radius(T.cue_chip[i], 8, 0);
        T.cue_badge[i] = ui_next_box(T.cue_chip[i], 8, (ch - sq) / 2, sq, sq, T.pal->accent);
        lv_obj_set_style_radius(T.cue_badge[i], 4, 0);
        char l[2] = {(char)('A' + i), 0};
        lv_obj_t *lt = ui_next_label(T.cue_badge[i], l, T.f->mono, 0x05070A);
        lv_obj_center(lt);
        T.cue_time[i] = ui_next_label(T.cue_chip[i], "", T.f->mono, T.pal->text);
        lv_obj_set_pos(T.cue_time[i], 8 + sq + 6, (ch - T.f->mono->line_height) / 2);
    }
    return lines * ch + (lines - 1) * 6;
}

static int cues_height(int w)
{
    const int ch = T.row_h * 58 / 100, sq = T.fs * 15 / 10, nat = 16 + sq + 6 + T.f->mono->line_height * 25 / 10;
    int per = (w + 6) / (nat + 6);
    if (per < 1) per = 1;
    if (per > UI_NEXT_TI_CUES) per = UI_NEXT_TI_CUES;
    const int lines = (UI_NEXT_TI_CUES + per - 1) / per;
    return lines * ch + (lines - 1) * 6;
}

static int stat_h(void) { return 16 + T.f->tiny->line_height + 2 + T.f->bpm->line_height; }

static int build_stats(int x, int y, int w)
{
    const int cw = (w - 12) / 3, h = stat_h();
    for (int i = 0; i < 6; i++) {
        const int cx = x + (i % 3) * (cw + 6), cy = y + (i / 3) * (h + 6);
        const int cwi = i % 3 == 2 ? w - 2 * (cw + 6) : cw;
        lv_obj_t *s = ui_next_box(T.root, cx, cy, cwi, h, T.pal->panel);
        lv_obj_set_style_radius(s, 10, 0);
        lv_obj_t *l = ui_next_label(s, STAT_LABEL[i], T.f->tiny, T.pal->muted);
        lv_obj_set_pos(l, 10, 8);
        T.stat_val[i] = ui_next_label(s, "", T.f->bpm, T.pal->text);
        lv_obj_set_pos(T.stat_val[i], 10, 8 + T.f->tiny->line_height + 2);
    }
    return 2 * h + 6;
}

static int build_actions(int x, int y, int w)
{
    const int bh = T.row_h * 85 / 100, bw = (w - T.gap) / 2;
    for (int i = 0; i < 4; i++)
        ui_next_chrome_action(T.root, x + (i % 2) * (bw + T.gap), y + (i / 2) * (bh + T.gap),
                              i % 2 ? w - bw - T.gap : bw, bh, ACTION[i], i < 2);
    return 2 * bh + T.gap;
}

static void build_top_bar(void)
{
    lv_obj_t *row = ui_next_chrome_topbar(T.root);
    ui_next_chrome_button(row, T.wide ? "BACK" : NULL, UI_NEXT_ICON_CHEVRON, false);
    ui_next_label(row, "Track info", T.f->med, T.pal->text);
    lv_obj_t *sp = lv_obj_create(row);
    lv_obj_remove_style_all(sp);
    lv_obj_set_size(sp, 1, 1);
    lv_obj_set_flex_grow(sp, 1);
    if (T.wide) {
        ui_next_chrome_button(row, "PREVIOUS", -1, false);
        ui_next_chrome_button(row, "NEXT", -1, false);
    }
}

lv_obj_t *ui_next_trackinfo_create(lv_obj_t *parent, uint16_t w, uint16_t h, bool day)
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
    ui_next_chrome_begin(&T.lay, day, UI_NEXT_PAGE_BROWSE);

    T.root = ui_next_box(parent, 0, 0, w, h, T.pal->frame);
    const int x0 = T.lay.dock.x + T.pad, y0 = T.lay.top_bar.h + T.pad;
    const int iw = T.lay.dock.w - 2 * T.pad, ih = T.lay.dock.y - T.lay.top_bar.h - 2 * T.pad;
    const int stats_h = 2 * stat_h() + 6, acts_h = 2 * (T.row_h * 85 / 100) + T.gap;

    int c1w, c1h, c2x = 0, c2y = 0, c2w = 0;
    if (T.landscape) {
        c1w = (iw - T.pad) * 3 / 5;
        c1h = ih;
        c2x = x0 + c1w + T.pad;
        c2w = iw - c1w - T.pad;
        c2y = y0;
    } else {
        c1w = iw;
        c2w = iw;
        c1h = ih - stats_h - T.gap - acts_h - T.pad;
        c2x = x0;
        c2y = y0 + c1h + T.pad;
    }

    const int art = build_header(x0, y0);
    const int cues_h = cues_height(c1w);
    const int card_y = y0 + art + T.gap, card_h = c1h - art - T.gap - cues_h - T.gap;
    lv_obj_t *card = ui_next_box(T.root, x0, card_y, c1w, card_h, T.pal->panel);
    lv_obj_set_style_radius(card, 12, 0);
    lv_obj_set_style_border_width(card, 1, 0);
    lv_obj_set_style_border_color(card, ui_next_col(T.pal->border), 0);
    const int cp = T.gap, cw = c1w - 2 * cp - 2;
    const int pbh = T.fs * 17 / 10, pvh = T.f->mono->line_height;
    const int wave_h = card_h - 2 - 2 * cp - pbh - pvh - 12;
    T.wave = ui_next_box(card, cp - 1, cp - 1, cw, wave_h, T.pal->well);
    lv_obj_set_style_radius(T.wave, 4, 0);
    lv_obj_set_style_clip_corner(T.wave, true, 0);
    lv_obj_add_event_cb(T.wave, wave_cb, LV_EVENT_DRAW_MAIN, NULL);
    phrase_bar(card, cp - 1, cp - 1 + wave_h + 6, cw, pbh);
    for (int i = 0; i < UI_NEXT_TI_PHRASES; i++) {
        T.phrase_seg[i] = ui_next_box(T.phrase_box, 0, 0, 10, pbh, T.pal->chip);
        T.phrase_lbl[i] = ui_next_label(T.phrase_seg[i], "", T.f->tiny, 0xFFFFFF);
        lv_obj_center(T.phrase_lbl[i]);
    }
    T.preview = ui_next_label(card, "", T.f->mono, T.pal->muted);
    lv_obj_set_pos(T.preview, cp - 1, cp - 1 + wave_h + 6 + pbh + 6);
    build_cues(x0, y0 + c1h - cues_h, c1w);

    c2y += 0;
    const int sh = build_stats(c2x, c2y, c2w);
    build_actions(c2x, c2y + sh + T.gap, c2w);

    ui_next_chrome_dock(T.root, 0, false);
    ui_next_chrome_dock(T.root, 1, false);
    build_top_bar();
    ui_next_chrome_nav(T.root);
    return T.root;
}

void ui_next_trackinfo_update(const ui_next_trackinfo_model_t *m)
{
    if (!T.root) return;
    T.m = *m;
    lv_label_set_text(T.title, m->title);
    lv_label_set_text(T.artist, m->artist);
    lv_label_set_text(T.album, m->album);
    ui_next_cover(T.art, &T.cover, m->art0, m->art1);
    lv_label_set_text(T.tag_genre_txt, m->genre);
    if (T.tag_label) {
        lv_label_set_text(T.tag_label_txt, m->label);
        lv_obj_update_layout(T.tag_genre);
        lv_obj_set_x(T.tag_label, lv_obj_get_x(T.tag_genre) + lv_obj_get_width(T.tag_genre) + 6);
    }
    for (int i = 0; i < UI_NEXT_TI_PHRASES; i++) {
        lv_label_set_text(T.phrase_lbl[i], m->phrase[i].name);
        lv_obj_set_style_bg_color(T.phrase_seg[i], ui_next_col(m->phrase[i].color), 0);
    }
    layout_phrases();
    lv_label_set_text(T.preview, m->preview);
    for (int i = 0; i < UI_NEXT_TI_CUES; i++) {
        lv_label_set_text(T.cue_time[i], m->cue[i].time);
        lv_obj_set_style_bg_color(T.cue_badge[i], ui_next_col(m->cue[i].color), 0);
    }
    for (int i = 0; i < 6; i++) lv_label_set_text(T.stat_val[i], m->stat[i]);
    lv_obj_invalidate(T.wave);
    ui_next_chrome_dock_set(0, &m->deck[0]);
    ui_next_chrome_dock_set(1, &m->deck[1]);
}
