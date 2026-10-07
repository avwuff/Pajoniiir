#include "ui_framebuffer_pair.h"
#include <string.h>

size_t ui_framebuffer_pair_workspace_size(unsigned w, unsigned h)
{
    if (!w || w > UINT16_MAX || !h || h > UI_FRAMEBUFFER_MAX_ROWS) return 0;
    return ((size_t)w + 7u) / 8u * h;
}

bool ui_framebuffer_pair_init(ui_framebuffer_pair_t *p, unsigned w, unsigned h,
    void *workspace, size_t workspace_bytes)
{
    size_t required = ui_framebuffer_pair_workspace_size(w, h);
    if (!p || !required || !workspace || workspace_bytes < required) return false;
    memset(p, 0, sizeof(*p));
    p->width = w; p->height = h; p->draw = 1;
    p->written = workspace; p->written_bytes = required;
    p->written_stride = ((size_t)w + 7u) / 8u;
    memset(p->written, 0, required);
    for (unsigned y = 0; y < h; ++y) p->left[y] = p->carry_left[y] = w;
    return true;
}

int ui_framebuffer_pair_writable(const ui_framebuffer_pair_t *p)
{
    return p && p->written && !p->pending ? (int)p->draw : -1;
}

bool ui_framebuffer_pair_dirty(ui_framebuffer_pair_t *p, int x, int y, int w, int h)
{
    if (!p || !p->written || p->pending || w <= 0 || h <= 0 || x < 0 || y < 0 ||
        x >= p->width || y >= p->height || w > p->width - x || h > p->height - y) return false;
    unsigned end = (unsigned)(x + w), first = (unsigned)x / 8u, last = (end - 1u) / 8u;
    for (int row = y; row < y + h; ++row) {
        if (p->left[row] > x) p->left[row] = x;
        if (p->right[row] < end) p->right[row] = end;
        uint8_t *bits = p->written + (size_t)row * p->written_stride;
        unsigned start_mask = 0xffu << ((unsigned)x & 7u);
        unsigned end_mask = (1u << (((end - 1u) & 7u) + 1u)) - 1u;
        if (first == last) bits[first] |= start_mask & end_mask;
        else {
            bits[first] |= start_mask;
            if (last > first + 1u) memset(bits + first + 1u, 0xff, last - first - 1u);
            bits[last] |= end_mask;
        }
    }
    p->dirty = true;
    return true;
}

bool ui_framebuffer_pair_submit(ui_framebuffer_pair_t *p, uint32_t refresh)
{
    if (!p || !p->written || !p->dirty || p->pending) return false;
    /* All prior damage must be preserved before the frame is sealed. */
    for (unsigned y = 0; y < p->height; ++y)
        if (p->carry_left[y] < p->carry_right[y]) return false;
    p->submitted_refresh = refresh;
    p->pending = true;
    return true;
}

bool ui_framebuffer_pair_acquire(ui_framebuffer_pair_t *p, uint32_t refresh)
{
    if (!p || !p->written || !p->pending || (uint32_t)(refresh - p->submitted_refresh) == 0 ||
        (uint32_t)(refresh - p->submitted_refresh) > UINT32_MAX / 2u) return false;
    p->front = p->draw;
    p->draw ^= 1u;
    for (unsigned y = 0; y < p->height; ++y) {
        p->carry_left[y] = p->left[y]; p->carry_right[y] = p->right[y];
        p->left[y] = p->width; p->right[y] = 0;
    }
    memset(p->written, 0, p->written_bytes);
    p->pending = false; p->dirty = false;
    return true;
}

bool ui_framebuffer_pair_compose(ui_framebuffer_pair_t *p,
    void *buffers[2], unsigned bpp, unsigned *first_row, unsigned *end_row)
{
    if (!p || !p->written || p->pending || !buffers || !buffers[0] || !buffers[1] ||
        buffers[0] == buffers[1] || (bpp != 2 && bpp != 3) || !first_row || !end_row) return false;
    *first_row = p->height; *end_row = 0; p->copied_bytes = 0;
    for (unsigned y = 0; y < p->height; ++y) {
        unsigned x = p->carry_left[y], right = p->carry_right[y];
        const uint8_t *bits = p->written + (size_t)y * p->written_stride;
        while (x < right) {
            if (bits[x / 8u] == 0xffu) {
                unsigned next = (x / 8u + 1u) * 8u;
                x = next < right ? next : right;
                continue;
            }
            if (bits[x / 8u] & (1u << (x & 7u))) { ++x; continue; }
            unsigned start = x;
            while (x < right && !(bits[x / 8u] & (1u << (x & 7u)))) {
                if (!(x & 7u) && bits[x / 8u] == 0u && right - x >= 8u) x += 8u;
                else ++x;
            }
            size_t offset = ((size_t)y * p->width + start) * bpp;
            size_t bytes = (size_t)(x - start) * bpp;
            memcpy((uint8_t *)buffers[p->draw] + offset,
                   (const uint8_t *)buffers[p->front] + offset, bytes);
            p->copied_bytes += bytes;
            if (*first_row > y) *first_row = y;
            *end_row = y + 1u;
        }
        p->carry_left[y] = p->width; p->carry_right[y] = 0;
    }
    return true;
}
