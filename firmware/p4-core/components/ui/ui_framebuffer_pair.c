#include "ui_framebuffer_pair.h"
#include <string.h>

bool ui_framebuffer_pair_init(ui_framebuffer_pair_t *p, unsigned w, unsigned h)
{
    if (!p || !w || w > UINT16_MAX || !h || h > UI_FRAMEBUFFER_MAX_ROWS) return false;
    memset(p, 0, sizeof(*p));
    p->width = w; p->height = h; p->draw = 1;
    for (unsigned y = 0; y < h; ++y) p->left[y] = w;
    return true;
}

int ui_framebuffer_pair_writable(const ui_framebuffer_pair_t *p)
{
    return p && !p->pending ? (int)p->draw : -1;
}

bool ui_framebuffer_pair_dirty(ui_framebuffer_pair_t *p, int x, int y, int w, int h)
{
    if (!p || p->pending || w <= 0 || h <= 0 || x < 0 || y < 0 ||
        x >= p->width || y >= p->height || w > p->width - x || h > p->height - y) return false;
    for (int row = y; row < y + h; ++row) {
        if (p->left[row] > x) p->left[row] = x;
        if (p->right[row] < x + w) p->right[row] = x + w;
    }
    p->dirty = true;
    return true;
}

bool ui_framebuffer_pair_submit(ui_framebuffer_pair_t *p, uint32_t refresh)
{
    if (!p || !p->dirty || p->pending) return false;
    p->submitted_refresh = refresh;
    p->pending = true;
    return true;
}

bool ui_framebuffer_pair_acquire(ui_framebuffer_pair_t *p, uint32_t refresh,
    void *buffers[2], unsigned bpp, unsigned *first_row, unsigned *end_row)
{
    if (!p || !p->pending || (uint32_t)(refresh - p->submitted_refresh) == 0 ||
        (uint32_t)(refresh - p->submitted_refresh) > UINT32_MAX / 2u || !buffers ||
        !buffers[0] || !buffers[1] || buffers[0] == buffers[1] ||
        (bpp != 2 && bpp != 3) || !first_row || !end_row) return false;
    p->front = p->draw;
    p->draw ^= 1u;
    *first_row = p->height; *end_row = 0;
    for (unsigned y = 0; y < p->height; ++y) {
        unsigned left = p->left[y], right = p->right[y];
        if (left < right) {
            size_t offset = ((size_t)y * p->width + left) * bpp;
            memcpy((uint8_t *)buffers[p->draw] + offset,
                   (const uint8_t *)buffers[p->front] + offset, (right - left) * bpp);
            if (*first_row > y) *first_row = y;
            *end_row = y + 1;
        }
        p->left[y] = p->width; p->right[y] = 0;
    }
    p->pending = false; p->dirty = false;
    return true;
}
