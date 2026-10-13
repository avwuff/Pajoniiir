/* Renders the ui_next pages at the six board sizes, night and day, into PPM
 * files. No window, no hardware.
 * usage: ui_next_sim <out dir> [tick] [perform|boot|browse|search|alerts|trackinfo|source|browseedit|fx|record|settings-audio|settings-display|settings-system] (default: all pages) */
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

typedef enum { SCREEN_PERFORM, SCREEN_BOOT, SCREEN_BROWSE, SCREEN_SEARCH, SCREEN_ALERTS, SCREEN_TRACKINFO, SCREEN_SOURCE, SCREEN_BROWSEEDIT, SCREEN_FX, SCREEN_RECORD, SCREEN_SET_AUDIO, SCREEN_SET_DISPLAY, SCREEN_SET_SYSTEM, SCREEN_COUNT } screen_t;
static const char *const SCREEN_NAME[] = {"perform", "boot", "browse", "search", "alerts", "trackinfo", "source", "browseedit", "fx", "record", "settings-audio", "settings-display", "settings-system"};

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
    static ui_next_settings_model_t setm;
    const bool is_settings = screen >= SCREEN_SET_AUDIO && screen <= SCREEN_SET_SYSTEM;
    static ui_next_env_t envset[2];
    if (is_settings) {
        ui_next_env_demo(&envset[0], 11);
        ui_next_env_demo(&envset[1], 23);
        memset(&setm, 0, sizeof setm);
        ui_next_demo_settings(&setm, (ui_next_settings_tab_t)(screen - SCREEN_SET_AUDIO), envset);
    }
    lv_obj_t *root = is_settings ? ui_next_settings_create(scr, w, h, day, &setm)
                     : screen == SCREEN_BOOT     ? ui_next_boot_create(scr, w, h, day)
                     : screen == SCREEN_BROWSE || screen == SCREEN_ALERTS ? ui_next_browse_create(scr, w, h, day)
                     : screen == SCREEN_SEARCH ? ui_next_search_create(scr, w, h, day)
                     : screen == SCREEN_TRACKINFO ? ui_next_trackinfo_create(scr, w, h, day)
                     : screen == SCREEN_SOURCE ? ui_next_source_create(scr, w, h, day)
                     : screen == SCREEN_BROWSEEDIT ? ui_next_browse_edit_create(scr, w, h, day)
                     : screen == SCREEN_FX ? ui_next_fx_create(scr, w, h, day)
                     : screen == SCREEN_RECORD ? ui_next_record_create(scr, w, h, day)
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
        } else if (screen == SCREEN_BROWSE || screen == SCREEN_ALERTS) {
            static ui_next_browse_model_t bm;
            static ui_next_env_t env4[4];
            for (uint32_t i = 0; i < 4; i++) ui_next_env_demo(&env4[i], 11u + 12u * i);
            memset(&bm, 0, sizeof bm);
            ui_next_demo_browse(&bm, env4);
            if (screen == SCREEN_ALERTS) bm.open_row = -1; /* the page behind the dim layer is at rest */
            ui_next_browse_update(&bm);
            if (screen == SCREEN_ALERTS) {
                static ui_next_alerts_model_t am;
                memset(&am, 0, sizeof am);
                ui_next_alerts_create(scr, w, h, day);
                ui_next_demo_alerts(&am);
                ui_next_alerts_update(&am);
            }
        } else if (is_settings) {
            ui_next_settings_update(&setm);
        } else if (screen == SCREEN_RECORD) {
            static ui_next_record_model_t rm;
            static ui_next_env_t envr[2];
            ui_next_env_demo(&envr[0], 11);
            ui_next_env_demo(&envr[1], 23);
            memset(&rm, 0, sizeof rm);
            ui_next_demo_record(&rm, envr);
            ui_next_record_update(&rm);
        } else if (screen == SCREEN_FX) {
            static ui_next_fx_model_t fm;
            static ui_next_env_t envfx[2];
            ui_next_env_demo(&envfx[0], 11);
            ui_next_env_demo(&envfx[1], 23);
            memset(&fm, 0, sizeof fm);
            ui_next_demo_fx(&fm, envfx);
            ui_next_fx_update(&fm);
        } else if (screen == SCREEN_BROWSEEDIT) {
            static ui_next_browse_model_t em;
            static ui_next_env_t env4e[4];
            for (uint32_t i = 0; i < 4; i++) ui_next_env_demo(&env4e[i], 11u + 12u * i);
            memset(&em, 0, sizeof em);
            ui_next_demo_browse_edit(&em, env4e);
            ui_next_browse_update(&em);
        } else if (screen == SCREEN_SOURCE) {
            static ui_next_source_model_t sm2;
            static ui_next_env_t env2b[2];
            ui_next_env_demo(&env2b[0], 11);
            ui_next_env_demo(&env2b[1], 23);
            memset(&sm2, 0, sizeof sm2);
            ui_next_demo_source(&sm2, env2b);
            ui_next_source_update(&sm2);
        } else if (screen == SCREEN_TRACKINFO) {
            static ui_next_trackinfo_model_t tm;
            static ui_next_env_t env3[3];
            for (uint32_t i = 0; i < 3; i++) ui_next_env_demo(&env3[i], 11u + 12u * i);
            memset(&tm, 0, sizeof tm);
            ui_next_demo_trackinfo(&tm, env3);
            ui_next_trackinfo_update(&tm);
        } else if (screen == SCREEN_SEARCH) {
            static ui_next_search_model_t sm;
            static ui_next_env_t env2[2];
            ui_next_env_demo(&env2[0], 11);
            ui_next_env_demo(&env2[1], 23);
            memset(&sm, 0, sizeof sm);
            ui_next_demo_search(&sm, env2);
            ui_next_search_update(&sm);
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
