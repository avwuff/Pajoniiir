#pragma once

/* New DJ UI (docs/reference/new-ui). Replaces dj_ui.
 * Screens read a plain model struct and never call the core. Whatever the core
 * does not provide yet is listed in docs/reference/new-ui/CORE_GAPS.md. */

#include <stdbool.h>
#include <stdint.h>

#include "lvgl.h"
#include "ui_next_layout.h"
#include "ui_next_wave.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    UI_NEXT_PAGE_PERFORM,
    UI_NEXT_PAGE_BROWSE,
    UI_NEXT_PAGE_FX,
    UI_NEXT_PAGE_RECORD,
    UI_NEXT_PAGE_SETTINGS,
    UI_NEXT_PAGE_COUNT,
} ui_next_page_t;

/* One deck as the Perform page shows it. Strings are copied. */
typedef struct {
    const ui_next_env_t *env;  /* stored 3-band envelope (PB-1) */
    uint32_t col;              /* envelope column under the playhead */
    char title[48];
    char bpm[12];
    char key[6];
    bool key_fit;              /* PB-10 */
    char badge[12];            /* "MASTER", "SYNC OFF" (PB-6) */
    bool badge_on;
    char time[10];
    float pos01;               /* playhead on the overview */
    float vu_l, vu_r;          /* 0..1 (PB-19) */
    float peak_l, peak_r;
} ui_next_deck_t;

typedef struct {
    ui_next_deck_t deck[2];
    ui_next_link_t link;       /* PB-17 */
    float tempo_pct;           /* deck B against deck A */
    char key_change[10];       /* "8A>9A" */
    const char *layers;        /* "LAYERS 1-4" */
} ui_next_perform_model_t;

/* Builds the Perform page in `parent` for the frame `w` x `h`. One instance. */
lv_obj_t *ui_next_perform_create(lv_obj_t *parent, uint16_t w, uint16_t h, bool day);
void ui_next_perform_update(const ui_next_perform_model_t *m);

/* Boot page (screen 14): ring mark, name, library load bar, licence footer. */
typedef struct {
    float progress01;        /* library load, SY-4 */
    char status[40];         /* "Reading library: 96 tracks" */
    char device_name[32];    /* SY-5, a placeholder until the maker picks one */
    char footer[64];         /* licence notice, SY-6 / ST-11 */
} ui_next_boot_model_t;

lv_obj_t *ui_next_boot_create(lv_obj_t *parent, uint16_t w, uint16_t h, bool day);
void ui_next_boot_update(const ui_next_boot_model_t *m);
/* CORE-GAP: SY-4 (progress), SY-5 (device name), SY-6 (notice source). */
void ui_next_demo_boot(ui_next_boot_model_t *m);

/* Demo model at tick `t` (60 ms per tick). CORE-GAP: all of PB-1 to PB-19. */
void ui_next_demo_perform(ui_next_perform_model_t *m, const ui_next_env_t *ea,
                          const ui_next_env_t *eb, uint32_t t, ui_next_class_t cls);

#ifdef __cplusplus
}
#endif
