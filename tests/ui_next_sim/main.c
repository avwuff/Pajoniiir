/* Renders the ui_next pages at the six board sizes, night and day, into PPM
 * files. No window, no hardware.
 * usage: ui_next_sim <out dir> [tick] [perform|boot] (default: all pages) */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lvgl.h"
#include "ui_next.h"

static uint32_t *s_fb;
static uint32_t s_w, s_h;

static void flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px)
{
    (void)area;
    memcpy(s_fb, px, (size_t)s_w * s_h * 4u);
    lv_display_flush_ready(disp);
}

static int write_ppm(const char *path)
{
    FILE *f = fopen(path, "wb");
    if (!f) return -1;
    fprintf(f, "P6\n%u %u\n255\n", s_w, s_h);
    for (uint32_t i = 0; i < s_w * s_h; i++) {
        uint8_t rgb[3] = {(uint8_t)(s_fb[i] >> 16), (uint8_t)(s_fb[i] >> 8), (uint8_t)s_fb[i]};
        fwrite(rgb, 1, 3, f);
    }
    return fclose(f);
}

typedef enum { SCREEN_PERFORM, SCREEN_BOOT, SCREEN_BROWSE, SCREEN_COUNT } screen_t;
static const char *const SCREEN_NAME[] = {"perform", "boot", "browse"};

static int render(const char *dir, screen_t screen, uint16_t w, uint16_t h, bool day, uint32_t tick,
                  const ui_next_env_t *ea, const ui_next_env_t *eb)
{
    s_w = w;
    s_h = h;
    s_fb = calloc((size_t)w * h, 4);
    uint32_t *buf = calloc((size_t)w * h, 4);
    lv_display_t *disp = lv_display_create(w, h);
    lv_display_set_color_format(disp, LV_COLOR_FORMAT_XRGB8888);
    lv_display_set_buffers(disp, buf, NULL, (uint32_t)w * h * 4u, LV_DISPLAY_RENDER_MODE_FULL);
    lv_display_set_flush_cb(disp, flush_cb);

    lv_obj_t *scr = lv_display_get_screen_active(disp);
    lv_obj_t *root = screen == SCREEN_BOOT     ? ui_next_boot_create(scr, w, h, day)
                     : screen == SCREEN_BROWSE ? ui_next_browse_create(scr, w, h, day)
                                               : ui_next_perform_create(scr, w, h, day);
    int rc = 0;
    if (!root) {
        fprintf(stderr, "layout rejected %ux%u\n", w, h);
        rc = 1;
    } else {
        if (screen == SCREEN_BOOT) {
            ui_next_boot_model_t bm;
            memset(&bm, 0, sizeof bm);
            ui_next_demo_boot(&bm);
            ui_next_boot_update(&bm);
        } else if (screen == SCREEN_BROWSE) {
            static ui_next_browse_model_t bm;
            static ui_next_env_t env4[4];
            for (uint32_t i = 0; i < 4; i++) ui_next_env_demo(&env4[i], 11u + 12u * i);
            memset(&bm, 0, sizeof bm);
            ui_next_demo_browse(&bm, env4);
            ui_next_browse_update(&bm);
        } else {
            ui_next_perform_model_t m;
            memset(&m, 0, sizeof m);
            ui_next_demo_perform(&m, ea, eb, tick, ui_next_class_for(w, h));
            ui_next_perform_update(&m);
        }
        lv_tick_inc(50);
        lv_timer_handler();
        lv_refr_now(disp);
        char path[512];
        snprintf(path, sizeof path, "%s/%s_%ux%u_%s.ppm", dir, SCREEN_NAME[screen], w, h, day ? "day" : "night");
        rc = write_ppm(path);
        if (!rc) printf("%s\n", path);
    }
    lv_display_delete(disp);
    free(buf);
    free(s_fb);
    return rc;
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        fprintf(stderr, "usage: %s <out dir> [tick]\n", argv[0]);
        return 2;
    }
    const uint32_t tick = argc > 2 ? (uint32_t)strtoul(argv[2], NULL, 10) : 100u;
    static ui_next_env_t ea, eb;
    ui_next_env_demo(&ea, 11);
    ui_next_env_demo(&eb, 23);
    lv_init();
    static const uint16_t SZ[][2] = {{800, 480}, {1024, 600}, {1280, 800}, {480, 800}, {600, 1024}, {800, 1280}};
    int rc = 0;
    for (int sc = 0; sc < SCREEN_COUNT; sc++) {
        if (argc > 3 && strcmp(argv[3], SCREEN_NAME[sc]) != 0) continue;
        for (unsigned i = 0; i < sizeof SZ / sizeof SZ[0]; i++) {
            rc |= render(argv[1], (screen_t)sc, SZ[i][0], SZ[i][1], false, tick, &ea, &eb);
            if (i == 0 || i == 3) rc |= render(argv[1], (screen_t)sc, SZ[i][0], SZ[i][1], true, tick, &ea, &eb);
        }
    }
    return rc ? 1 : 0;
}
