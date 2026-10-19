/* SPDX-License-Identifier: MIT
 * Execute the exact workaround fragments used by the private overlay. */
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define PPA_SRM_ROTATION_ANGLE_90 90
#define PPA_SRM_ROTATION_ANGLE_270 270
#define PPA_LL_SRM_SCALING_FRAG_MAX 16
#define PPA_SRM_COLOR_MODE_ARGB8888 32
#define PPA_SRM_COLOR_MODE_RGB888 24
#define PPA_LL_SRM_MB_SIZE_16_16 16
static int ppa_ll_srm_get_mb_size(int dev) { return dev; }
static uint32_t color_hal_pixel_format_fourcc_get_bit_depth(int mode) { return (uint32_t)mode; }
typedef int esp_color_fourcc_t;
typedef struct {
    struct { uint32_t block_w, block_h; } in;
    int rotation_angle;
    uint32_t scale_x_int, scale_x_frag, scale_y_int, scale_y_frag;
} descriptor_t;
typedef struct { struct { int dev; } hal; } platform_t;

static bool patched(descriptor_t *srm_trans_desc, int ppa_out_color_mode, int mb)
{
    platform_t p = {.hal = {.dev = mb}};
    platform_t *platform = &p;
#ifdef TEST_UNPATCHED
#include "../../firmware/boards/jc8012p4a1/ppa/workaround.before"
#else
#include "../../firmware/boards/jc8012p4a1/ppa/workaround.after"
#endif
    return bypass_mb_order;
}

/* Reference expressed in output axes, independently of the fragment variables. */
static bool reference(descriptor_t d, int bits, int mb)
{
    if (d.rotation_angle == 90 || d.rotation_angle == 270) {
        uint32_t n = d.in.block_w; d.in.block_w = d.in.block_h; d.in.block_h = n;
        n = d.scale_x_int; d.scale_x_int = d.scale_y_int; d.scale_y_int = n;
        n = d.scale_x_frag; d.scale_x_frag = d.scale_y_frag; d.scale_y_frag = n;
    }
    uint32_t width = d.in.block_w * (16*d.scale_x_int + d.scale_x_frag) / 16;
    uint32_t chunk = bits == 16 ? 64 : 32;
    uint32_t tail_w = (width-1) % chunk + 1;
    uint32_t tail_h = (d.in.block_h-1) % (uint32_t)mb + 1;
    tail_h = tail_h * (16*d.scale_y_int + d.scale_y_frag) / 16;
    return (width > chunk || d.in.block_h > (uint32_t)mb) && tail_w*tail_h*(uint32_t)bits < 1536;
}

int main(void)
{
    const uint32_t sizes[] = {15, 16, 17, 31, 32, 33, 63, 64, 65, 800, 1280};
    const int rotations[] = {0, 90, 180, 270};
    unsigned checked = 0;
    for (size_t w=0; w<11; ++w) for (size_t h=0; h<11; ++h)
    for (size_t r=0; r<4; ++r) for (int bits=16; bits<=32; bits+=8)
    for (int mb=16; mb<=32; mb+=16) for (uint32_t scale=0; scale<4; ++scale) {
        descriptor_t d = {.in={sizes[w], sizes[h]}, .rotation_angle=rotations[r],
            .scale_x_int=1, .scale_y_int=scale < 2 ? 1 : 2,
            .scale_x_frag=scale & 1 ? 8 : 0, .scale_y_frag=scale & 1 ? 0 : 8};
        if (patched(&d, bits, mb) != reference(d, bits, mb)) {
            fprintf(stderr, "FAIL PPA %u x %u rotation %d bits %d mb %d scale %u\n",
                    sizes[w], sizes[h], rotations[r], bits, mb, scale);
            return 1;
        }
        ++checked;
    }
    printf("PASS PPA rotated workaround: %u axis/scaling/format/chunk cases\n", checked);
    return 0;
}
