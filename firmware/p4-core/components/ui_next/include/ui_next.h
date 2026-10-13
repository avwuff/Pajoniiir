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

/* Browse page (screen 02). CORE-GAP: LB-1 to LB-9, LB-16 to LB-22. */
#define UI_NEXT_BROWSE_TAGS 5
#define UI_NEXT_BROWSE_TRACKS 15
#define UI_NEXT_BROWSE_CATS 9

typedef enum { UI_NEXT_KEY_NONE, UI_NEXT_KEY_FIT, UI_NEXT_KEY_NEAR, UI_NEXT_KEY_FAR } ui_next_key_class_t;

typedef struct {
    char title[24], artist[24], genre[16], bpm[8], key[5], time[8];
    ui_next_key_class_t key_class; /* fit to the loaded deck, LB-18 */
    uint8_t tags[3];               /* indices into the tag list */
    uint8_t n_tags;
    uint32_t art0, art1;           /* placeholder cover gradient */
    uint8_t env;                   /* index into preview_env (LB-6) */
} ui_next_track_t;

typedef struct {
    char name[10];
    uint32_t color;
    bool on;                       /* in the filter */
} ui_next_tag_t;

typedef struct {
    char crumb[32];                /* "Playlists / Warm-up" */
    char count[12];                /* "38 OF 96" */
    char match[24];                /* "MATCH TO DECK B 9A" */
    uint8_t category;              /* active entry of the category rail */
    uint8_t tag_badge;             /* tags in the filter, 0 = none */
    ui_next_tag_t tags[UI_NEXT_BROWSE_TAGS];
    ui_next_track_t track[UI_NEXT_BROWSE_TRACKS];
    int8_t open_row;               /* row showing its swipe actions, -1 = none (LB-8) */
    const ui_next_env_t *preview_env[4];
    ui_next_deck_t deck[2];        /* dock: title, time, overview */
} ui_next_browse_model_t;

lv_obj_t *ui_next_browse_create(lv_obj_t *parent, uint16_t w, uint16_t h, bool day);
void ui_next_browse_update(const ui_next_browse_model_t *m);
void ui_next_demo_browse(ui_next_browse_model_t *m, const ui_next_env_t *env4);

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
