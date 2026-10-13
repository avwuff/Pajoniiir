#pragma once

/* New DJ UI layout engine (docs/reference/new-ui, FW-1 / FW-2).
 * Pure geometry: no LVGL, no core dependency. Screens place their widgets in
 * the zones computed here. */

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Size class comes from the long side: <= 800 S, <= 1024 M, above L. */
typedef enum { UI_NEXT_CLASS_S, UI_NEXT_CLASS_M, UI_NEXT_CLASS_L } ui_next_class_t;

typedef struct {
    int16_t x, y, w, h;
} ui_next_rect_t;

/* Per-class style tokens (text, spacing, row size, touch target). */
typedef struct {
    uint8_t text_px;
    uint8_t top_bar_px;
    uint8_t nav_px;      /* rail width in landscape, bar height in portrait */
    uint8_t row_px;
    uint8_t pad_px;
    uint8_t gap_px;
    uint8_t touch_px;    /* minimum touch target, always >= 44 */
    uint8_t dock_land_px;
    uint8_t dock_port_px;
    uint8_t link_bar_px; /* Perform link bar between the waveforms and the dock */
} ui_next_tokens_t;

/* Zones of the Perform page, top to bottom: top bar, waveforms (the rest),
 * link bar, deck dock. The overview strip lives inside each deck card. */
typedef struct {
    ui_next_class_t cls;
    bool landscape;
    ui_next_tokens_t tok;
    ui_next_rect_t top_bar;
    ui_next_rect_t nav;        /* rail on the left, bar at the bottom */
    ui_next_rect_t waveforms;
    ui_next_rect_t link_bar;
    ui_next_rect_t dock;
    ui_next_rect_t deck_card[2]; /* side by side in landscape, stacked in portrait */
} ui_next_layout_t;

ui_next_class_t ui_next_class_for(uint16_t w, uint16_t h);
const ui_next_tokens_t *ui_next_tokens(ui_next_class_t cls);

/* Returns false for a size smaller than the S class minimum (480 x 480 area). */
bool ui_next_layout_compute(uint16_t w, uint16_t h, ui_next_layout_t *out);

#ifdef __cplusplus
}
#endif
