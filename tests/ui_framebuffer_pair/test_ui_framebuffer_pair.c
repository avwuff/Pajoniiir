#include "ui_framebuffer_pair.h"
#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

#define W 37u
#define H 29u
static unsigned rng = 19088743u;
static unsigned random_u32(void) { rng = rng * 1664525u + 1013904223u; return rng; }
#define WORKSPACE_BYTES (((W + 7u) / 8u) * H)

static void run_pixels(unsigned bpp)
{
    ui_framebuffer_pair_t p;
    unsigned char image[2][W * H * 3] = {{0}}, reference[W * H * 3] = {0};
    unsigned char front_before[W * H * 3];
    unsigned char workspace[WORKSPACE_BYTES];
    void *buffers[] = {image[0], image[1]};
    assert(ui_framebuffer_pair_init(&p, W, H, workspace, sizeof(workspace)));
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
        void *aliased[] = {image[0], image[0]};
        assert(!ui_framebuffer_pair_compose(&p, aliased, bpp, &first, &end));
        assert(ui_framebuffer_pair_compose(&p, buffers, bpp, &first, &end));
        assert(memcmp(front_before, image[p.front], W * H * bpp) == 0);
        assert(memcmp(image[draw], reference, W * H * bpp) == 0);
        assert(ui_framebuffer_pair_submit(&p, refresh));
        assert(ui_framebuffer_pair_writable(&p) == -1);
        assert(!ui_framebuffer_pair_dirty(&p, 0, 0, 1, 1));
        assert(!ui_framebuffer_pair_submit(&p, refresh));
        assert(!ui_framebuffer_pair_compose(&p, buffers, bpp, &first, &end));
        assert(!ui_framebuffer_pair_acquire(&p, refresh));
        assert(!ui_framebuffer_pair_acquire(&p, refresh - 1u));
        /* A delayed completion may skip several refreshes; it must never
         * expose a partially written picture or resurrect stale widgets. */
        refresh += 1u + frame % 4u;
        assert(ui_framebuffer_pair_acquire(&p, refresh));
        assert(memcmp(image[p.front], reference, W * H * bpp) == 0);
        if (frame % 11u == 0u) {
            /* A paused frame still needs the last moving frame's old damage;
             * it must not be lost or cause an unnecessary presentation. */
            assert(ui_framebuffer_pair_compose(&p, buffers, bpp, &first, &end));
            assert(memcmp(image[p.draw], reference, W * H * bpp) == 0);
        }
        assert(!ui_framebuffer_pair_submit(&p, refresh)); /* paused/no damage */
    }
}

static void run_full_overwrite(unsigned bpp)
{
    ui_framebuffer_pair_t p;
    unsigned char image[2][W * H * 3] = {{0}}, workspace[WORKSPACE_BYTES];
    void *buffers[] = {image[0], image[1]};
    assert(ui_framebuffer_pair_init(&p, W, H, workspace, sizeof(workspace)));
    unsigned first, end;
    for (unsigned frame = 0; frame < 100; ++frame) {
        memset(image[p.draw], frame + 1u, W * H * bpp);
        assert(ui_framebuffer_pair_dirty(&p, 0, 0, W, H));
        if (frame) assert(!ui_framebuffer_pair_submit(&p, frame));
        assert(ui_framebuffer_pair_compose(&p, buffers, bpp, &first, &end));
        /* Steady full redraw: zero old pixels copied, including partial-byte
         * right edges. There is no conservative bounding-span over-copy. */
        assert(p.copied_bytes == 0 && first == H && end == 0);
        for (unsigned i = 0; i < W * H * bpp; ++i) assert(image[p.draw][i] == frame + 1u);
        assert(ui_framebuffer_pair_submit(&p, frame));
        assert(ui_framebuffer_pair_acquire(&p, frame + 1u));
    }
}

static void run_disjoint_holes(void)
{
    ui_framebuffer_pair_t p;
    unsigned char image[2][W * H * 3] = {{0}}, workspace[WORKSPACE_BYTES];
    void *buffers[] = {image[0], image[1]};
    unsigned first, end;
    assert(ui_framebuffer_pair_init(&p, W, H, workspace, sizeof(workspace)));
    memset(image[p.draw], 42, W * H * 3);
    assert(ui_framebuffer_pair_dirty(&p, 0, 0, W, H));
    assert(ui_framebuffer_pair_compose(&p, buffers, 3, &first, &end));
    assert(ui_framebuffer_pair_submit(&p, 0));
    assert(ui_framebuffer_pair_acquire(&p, 1));
    for (unsigned y = 0; y < H; ++y) {
        memset(image[p.draw] + y * W * 3, 10, 3 * 3);
        memset(image[p.draw] + (y * W + W - 3) * 3, 20, 3 * 3);
    }
    assert(ui_framebuffer_pair_dirty(&p, 0, 0, 3, H));
    assert(ui_framebuffer_pair_dirty(&p, W - 3, 0, 3, H));
    assert(ui_framebuffer_pair_compose(&p, buffers, 3, &first, &end));
    assert(p.copied_bytes == (W - 6) * H * 3);
    for (unsigned y = 0; y < H; ++y)
        for (unsigned x = 0; x < W; ++x)
            for (unsigned c = 0; c < 3; ++c)
                assert(image[p.draw][(y * W + x) * 3 + c] == (x < 3 ? 10 : x >= W - 3 ? 20 : 42));
}

int main(void)
{
    ui_framebuffer_pair_t p;
    unsigned char workspace[WORKSPACE_BYTES];
    assert(!ui_framebuffer_pair_init(NULL, W, H, workspace, sizeof(workspace)));
    assert(!ui_framebuffer_pair_init(&p, 0, H, workspace, sizeof(workspace)));
    assert(!ui_framebuffer_pair_init(&p, W, UI_FRAMEBUFFER_MAX_ROWS + 1, workspace, sizeof(workspace)));
    assert(!ui_framebuffer_pair_init(&p, W, H, NULL, sizeof(workspace)));
    assert(!ui_framebuffer_pair_init(&p, W, H, workspace, sizeof(workspace) - 1));
    assert(ui_framebuffer_pair_workspace_size(800, 480) == 48000);
    assert(ui_framebuffer_pair_init(&p, W, H, workspace, sizeof(workspace)));
    assert(!ui_framebuffer_pair_dirty(&p, -1, 0, 1, 1));
    assert(!ui_framebuffer_pair_dirty(&p, 0, H - 1, 1, 2));
    assert(!ui_framebuffer_pair_dirty(&p, INT_MAX, 0, INT_MAX, 1));
    run_pixels(2); run_pixels(3);
    run_full_overwrite(2); run_full_overwrite(3); run_disjoint_holes();
    puts("PASS framebuffer pair: sealed ownership, 2000 pixel-reference frames, exact disjoint coverage, zero-copy full redraw");
    return 0;
}
