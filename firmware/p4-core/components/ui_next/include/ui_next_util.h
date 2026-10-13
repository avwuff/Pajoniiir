#pragma once

/* Drawing and widget helpers shared by the ui_next screens. Internal to the
 * component: screens include this, the rest of the firmware does not. */

#include "lvgl.h"
#include "ui_next_layout.h"
#include "ui_next_theme.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Fonts by size class (FW-1). The demo boards use weight 500 and 600 as "med". */
typedef struct {
    const lv_font_t *txt;   /* body text */
    const lv_font_t *med;   /* titles, row names */
    const lv_font_t *tiny;  /* captions, bar numbers */
    const lv_font_t *mono;  /* numbers, chips */
    const lv_font_t *big;   /* deck letter */
    const lv_font_t *bpm;
    const lv_font_t *val;   /* link bar values */
    const lv_font_t *title; /* boot name */
    const lv_font_t *row;   /* list row title, buttons (weight 500/600) */
    const lv_font_t *small; /* artist, genre, tag names */
    const lv_font_t *rmono; /* list numbers */
    const lv_font_t *tagf;  /* tag pills */
} ui_next_fonts_t;

const ui_next_fonts_t *ui_next_fonts_for(ui_next_class_t cls);

static inline lv_color_t ui_next_col(uint32_t rgb) { return lv_color_hex(rgb); }

/* Plain filled rectangle child: no scroll, no click, no theme style. */
lv_obj_t *ui_next_box(lv_obj_t *parent, int x, int y, int w, int h, uint32_t rgb);
lv_obj_t *ui_next_label(lv_obj_t *parent, const char *s, const lv_font_t *f, uint32_t rgb);

/* Draw-callback primitives, in absolute screen coordinates. */
void ui_next_fill_rect(lv_layer_t *layer, int x1, int y1, int x2, int y2, uint32_t rgb, lv_opa_t opa);
void ui_next_seg(lv_layer_t *layer, float x1, float y1, float x2, float y2, int w, uint32_t rgb, lv_opa_t opa);
void ui_next_text_at(lv_layer_t *layer, const char *s, int x, int y, const lv_font_t *f, uint32_t rgb);

#ifdef __cplusplus
}
#endif
