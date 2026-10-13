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

/* Three-band overview of `env` into `a`, scaled down. Cue marks and the played
 * shade are drawn only when pos01 >= 0 (PB-3, LB-6). */
void ui_next_chrome_draw_overview(lv_layer_t *layer, const lv_area_t *a, const ui_next_env_t *env,
                                  float pos01, const ui_next_palette_t *pal);

#ifdef __cplusplus
}
#endif
