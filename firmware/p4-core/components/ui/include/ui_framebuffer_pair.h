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
    uint16_t carry_left[UI_FRAMEBUFFER_MAX_ROWS], carry_right[UI_FRAMEBUFFER_MAX_ROWS];
    uint8_t *written;
    size_t written_bytes, written_stride, copied_bytes;
    unsigned front, draw;
    uint32_t submitted_refresh;
    bool dirty, pending;
} ui_framebuffer_pair_t;

size_t ui_framebuffer_pair_workspace_size(unsigned width, unsigned height);
bool ui_framebuffer_pair_init(ui_framebuffer_pair_t *pair, unsigned width, unsigned height,
    void *workspace, size_t workspace_bytes);
int ui_framebuffer_pair_writable(const ui_framebuffer_pair_t *pair);
bool ui_framebuffer_pair_dirty(ui_framebuffer_pair_t *pair, int x, int y, int w, int h);
bool ui_framebuffer_pair_submit(ui_framebuffer_pair_t *pair, uint32_t refresh_after_submit);
/* Retire without copying pixels that the next frame may overwrite. */
bool ui_framebuffer_pair_acquire(ui_framebuffer_pair_t *pair, uint32_t refresh);
/* After all new writes, preserve old damage that was NOT overwritten. Flush
 * copied CPU rows before submission. Exact coverage uses caller-owned memory. */
bool ui_framebuffer_pair_compose(ui_framebuffer_pair_t *pair,
    void *buffers[2], unsigned bytes_per_pixel, unsigned *first_row, unsigned *end_row);
