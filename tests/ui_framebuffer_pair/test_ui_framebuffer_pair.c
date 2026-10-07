#include "ui_framebuffer_pair.h"
#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

#define W 37u
#define H 29u
static unsigned rng = 19088743u;
static unsigned random_u32(void) { rng = rng * 1664525u + 1013904223u; return rng; }

static void run_pixels(unsigned bpp)
{
    ui_framebuffer_pair_t p;
    unsigned char image[2][W * H * 3] = {{0}}, reference[W * H * 3] = {0};
    unsigned char front_before[W * H * 3];
    void *buffers[] = {image[0], image[1]};
    assert(ui_framebuffer_pair_init(&p, W, H));
    assert(!ui_framebuffer_pair_submit(&p, 0));
    unsigned first, end;
    uint32_t refresh = UINT32_MAX - 20u;
    for (unsigned frame = 0; frame < 1000; ++frame) {
        int draw = ui_framebuffer_pair_writable(&p);
        assert(draw >= 0 && (unsigned)draw != p.front);
        memcpy(front_before, image[p.front], W * H * bpp);
        for (unsigned rect = 0; rect < 1u + frame % 7u; ++rect) {
            unsigned x = random_u32() % W, y = random_u32() % H;
            unsigned w = 1u + random_u32() % (W - x), h = 1u + random_u32() % (H - y);
            unsigned char color = (unsigned char)(random_u32() >> 24);
            for (unsigned row = y; row < y + h; ++row) {
                memset(image[draw] + (row * W + x) * bpp, color, w * bpp);
                memset(reference + (row * W + x) * bpp, color, w * bpp);
            }
            assert(ui_framebuffer_pair_dirty(&p, x, y, w, h));
            assert(memcmp(front_before, image[p.front], W * H * bpp) == 0);
        }
        assert(memcmp(image[draw], reference, W * H * bpp) == 0);
        assert(ui_framebuffer_pair_submit(&p, refresh));
        assert(ui_framebuffer_pair_writable(&p) == -1);
        assert(!ui_framebuffer_pair_dirty(&p, 0, 0, 1, 1));
        assert(!ui_framebuffer_pair_submit(&p, refresh));
        assert(!ui_framebuffer_pair_acquire(&p, refresh, buffers, bpp, &first, &end));
        assert(!ui_framebuffer_pair_acquire(&p, refresh - 1u, buffers, bpp, &first, &end));
        void *aliased[] = {image[0], image[0]};
        assert(!ui_framebuffer_pair_acquire(&p, refresh + 1u, aliased, bpp, &first, &end));
        /* A delayed completion may skip several refreshes; it must never
         * expose a partially written picture or resurrect stale widgets. */
        refresh += 1u + frame % 4u;
        assert(ui_framebuffer_pair_acquire(&p, refresh, buffers, bpp, &first, &end));
        assert(first < end && end <= H);
        assert(memcmp(image[0], reference, W * H * bpp) == 0);
        assert(memcmp(image[1], reference, W * H * bpp) == 0);
        assert(!ui_framebuffer_pair_submit(&p, refresh)); /* paused/no damage */
    }
}

int main(void)
{
    ui_framebuffer_pair_t p;
    assert(!ui_framebuffer_pair_init(NULL, W, H));
    assert(!ui_framebuffer_pair_init(&p, 0, H));
    assert(!ui_framebuffer_pair_init(&p, W, UI_FRAMEBUFFER_MAX_ROWS + 1));
    assert(ui_framebuffer_pair_init(&p, W, H));
    assert(!ui_framebuffer_pair_dirty(&p, -1, 0, 1, 1));
    assert(!ui_framebuffer_pair_dirty(&p, 0, H - 1, 1, 2));
    assert(!ui_framebuffer_pair_dirty(&p, INT_MAX, 0, INT_MAX, 1));
    run_pixels(2); run_pixels(3);
    puts("PASS framebuffer pair: front ownership, sealed submission, late/wrapped retirement, 2000 pixel-reference frames");
    return 0;
}
