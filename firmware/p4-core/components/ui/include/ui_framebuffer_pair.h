#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* LVGL-task owned. A submitted buffer stays sealed until a refresh observed
 * AFTER the driver's submission returns. The ISR never renders or copies. */
#define UI_FRAMEBUFFER_MAX_ROWS 1024u
typedef struct {
    uint16_t width, height;
    uint16_t left[UI_FRAMEBUFFER_MAX_ROWS], right[UI_FRAMEBUFFER_MAX_ROWS];
    unsigned front, draw;
    uint32_t submitted_refresh;
    bool dirty, pending;
} ui_framebuffer_pair_t;

bool ui_framebuffer_pair_init(ui_framebuffer_pair_t *pair, unsigned width, unsigned height);
int ui_framebuffer_pair_writable(const ui_framebuffer_pair_t *pair);
bool ui_framebuffer_pair_dirty(ui_framebuffer_pair_t *pair, int x, int y, int w, int h);
bool ui_framebuffer_pair_submit(ui_framebuffer_pair_t *pair, uint32_t refresh_after_submit);
/* After the driver selects the new front, carry its dirty pixels into the
 * retired buffer. Unchanged pixels already match, including paused waveforms.
 * The caller must write back copied CPU rows before further DMA writes. */
bool ui_framebuffer_pair_acquire(ui_framebuffer_pair_t *pair, uint32_t refresh,
    void *buffers[2], unsigned bytes_per_pixel, unsigned *first_row, unsigned *end_row);
