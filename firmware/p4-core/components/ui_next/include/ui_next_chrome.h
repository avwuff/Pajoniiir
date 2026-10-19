#pragma once

/* Parts every page shares: navigation, deck dock, chips (FW-3, FW-4).
 * Internal to the component. Call ui_next_chrome_begin() once per page build. */

#include "ui_next.h"
#include "ui_next_util.h"

#ifdef __cplusplus
extern "C" {
#endif

void ui_next_chrome_begin(const ui_next_layout_t *lay, bool day, ui_next_page_t active);

/* Left rail in landscape, bar at the bottom in portrait. */
lv_obj_t *ui_next_chrome_nav(lv_obj_t *root);

/* Deck card `d` of the dock. `full` adds BPM, key chip and state badge (Perform);
 * the other pages show letter, title and time. */
void ui_next_chrome_dock(lv_obj_t *root, int d, bool full);
void ui_next_chrome_dock_set(int d, const ui_next_deck_t *k);

/* Pill with mono text, as on the top bar. Returns the pill. */
lv_obj_t *ui_next_chrome_chip(lv_obj_t *parent, const char *s);

typedef enum { UI_NEXT_ICON_CHEVRON, UI_NEXT_ICON_SEARCH, UI_NEXT_ICON_ARROW } ui_next_icon_t;

/* Line icon in a square of `size`, drawn in the text colour. */
lv_obj_t *ui_next_chrome_icon(lv_obj_t *parent, int size, ui_next_icon_t kind);

/* Top bar button: optional icon and text; `on` is the amber state. Height and
 * padding follow the size class (FW-5: 44 px or more). */
lv_obj_t *ui_next_chrome_button(lv_obj_t *parent, const char *text, int icon, bool on);

/* Top bar panel with its bottom border. Returns a flex row that fills it. */
lv_obj_t *ui_next_chrome_topbar(lv_obj_t *root);

/* Three-band overview of `env` into `a`, scaled down. Cue marks and the played
 * shade are drawn only when pos01 >= 0 (PB-3, LB-6). */
void ui_next_chrome_draw_overview(lv_layer_t *layer, const lv_area_t *a, const ui_next_env_t *env,
                                  float pos01, const ui_next_palette_t *pal);

/* Same overview with the options the pages differ on: cue marks, played shade
 * and the playhead colour (the dock uses amber, Track info a teal line). */
void ui_next_chrome_draw_overview_ex(lv_layer_t *layer, const lv_area_t *a, const ui_next_env_t *env,
                                     float pos01, bool cues, bool shade, uint32_t head_rgb,
                                     const ui_next_palette_t *pal);

/* Three-band profile of half heights (0..100 = 0..50 view units), `cols` columns
 * stretched over `a`. Used by the Record waveform and the colour preview. */
void ui_next_chrome_draw_profile(lv_layer_t *layer, const lv_area_t *a, const uint8_t *const prof[3], int cols,
                                 const ui_next_palette_t *pal);

/* Action button of a page body: amber when `on`, else a chip. Returns the box. */
lv_obj_t *ui_next_chrome_action(lv_obj_t *parent, int x, int y, int w, int h, const char *text, bool on);

#ifdef __cplusplus
}
#endif
