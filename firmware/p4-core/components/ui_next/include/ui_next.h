#pragma once

/* New DJ UI (docs/reference/new-ui). Replaces dj_ui.
 * Screens read a plain model struct and never call the core. Whatever the core
 * does not provide yet is listed in docs/reference/new-ui/CORE_GAPS.md. */

#include <stdbool.h>
#include <stdint.h>

#include "lvgl.h"
#include "ui_next_layout.h"
#include "ui_next_key.h"
#include "ui_next_sync.h"
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
    uint32_t duration_ms, pos_ms;
    ui_next_grid_t grid;       /* beat grid of the track (PB-5) */
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
    uint32_t window_ms;        /* time shown by the zoomed waveforms */
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
    char match[40];                /* "MATCH TO DECK B 9A", or the edit instruction */
    uint8_t category;              /* active entry of the category rail */
    uint8_t tag_badge;             /* tags in the filter, 0 = none */
    ui_next_tag_t tags[UI_NEXT_BROWSE_TAGS];
    ui_next_track_t track[UI_NEXT_BROWSE_TRACKS];
    int8_t open_row;               /* row showing its swipe actions, -1 = none (LB-8) */
    int8_t lift_row;               /* edit mode: row lifted by the finger, -1 = none (LB-20) */
    int8_t ins_row;                /* edit mode: row with the amber landing line above it */
    const ui_next_env_t *preview_env[4];
    ui_next_deck_t deck[2];        /* dock: title, time, overview */
} ui_next_browse_model_t;

lv_obj_t *ui_next_browse_create(lv_obj_t *parent, uint16_t w, uint16_t h, bool day);
void ui_next_browse_update(const ui_next_browse_model_t *m);

/* Playlist edit (screen 05): same model, grips and numbers, a lifted row. The EDIT
 * button appears only in a playlist. CORE-GAP: LB-20. */
lv_obj_t *ui_next_browse_edit_create(lv_obj_t *parent, uint16_t w, uint16_t h, bool day);
void ui_next_demo_browse_edit(ui_next_browse_model_t *m, const ui_next_env_t *env4);
void ui_next_demo_browse(ui_next_browse_model_t *m, const ui_next_env_t *env4);

/* Search page (screen 06). CORE-GAP: LB-10 (live search), LB-19 (accent folding:
 * the core returns the matched span of each title, the UI only paints it). */
#define UI_NEXT_SEARCH_RESULTS 8

typedef struct {
    char pre[24], hit[16], post[24];  /* title split around the matched letters */
    char artist[24], bpm[6], key[5];
    uint32_t art0, art1;
} ui_next_search_result_t;

typedef struct {
    char query[24];
    char status[32];                  /* "ACCENTS IGNORED / 8 RESULTS" */
    ui_next_search_result_t result[UI_NEXT_SEARCH_RESULTS];
    uint8_t n;
    int8_t selected;
    ui_next_deck_t deck[2];
} ui_next_search_model_t;

lv_obj_t *ui_next_search_create(lv_obj_t *parent, uint16_t w, uint16_t h, bool day);
void ui_next_search_update(const ui_next_search_model_t *m);
void ui_next_demo_search(ui_next_search_model_t *m, const ui_next_env_t *env2);

/* Track info page (screen 03). CORE-GAP: LB-11 (phrase map, hot cues, file data),
 * LB-13 (key and phrase analysis), PB-8 (8 hot cues). */
#define UI_NEXT_TI_CUES 8
#define UI_NEXT_TI_PHRASES 6

typedef struct {
    char title[32], artist[24], album[24], genre[12], label[16];
    uint32_t art0, art1;
    char stat[6][10];                 /* BPM, KEY, TIME, BITRATE, PLAYS, GRID */
    struct { char time[8]; uint32_t color; } cue[UI_NEXT_TI_CUES];   /* letters A..H */
    struct { char name[8]; uint8_t weight; uint32_t color; } phrase[UI_NEXT_TI_PHRASES];
    char preview[28];                 /* "PREVIEW 02:44 OF 06:40" */
    float play01;
    const ui_next_env_t *env;
    ui_next_deck_t deck[2];
} ui_next_trackinfo_model_t;

lv_obj_t *ui_next_trackinfo_create(lv_obj_t *parent, uint16_t w, uint16_t h, bool day);
void ui_next_trackinfo_update(const ui_next_trackinfo_model_t *m);
void ui_next_demo_trackinfo(ui_next_trackinfo_model_t *m, const ui_next_env_t *env3);

/* Source page (screen 04). CORE-GAP: LB-4 (source colours), LB-12 (storage and
 * analysis status, safe eject), LB-15 / LB-23 (DJ link panel, protocol not chosen),
 * ST-13 (Ethernet). Only the NETWORK detail panel is on a board. */
#define UI_NEXT_SOURCES 5

typedef enum { UI_NEXT_SRC_USB, UI_NEXT_SRC_SD, UI_NEXT_SRC_NET, UI_NEXT_SRC_PHONE } ui_next_src_icon_t;

typedef struct {
    char name[12], status[12];
    uint32_t color;                  /* shown again in Browse (LB-4) */
    ui_next_src_icon_t icon;
    bool ready;                      /* green dot, else grey */
    uint8_t used_pct;                /* storage bar */
} ui_next_source_t;

typedef struct {
    ui_next_source_t source[UI_NEXT_SOURCES];
    int8_t selected;
    char summary[20];                /* "2 DRIVES READY" */
    char panel_title[12], panel_state[14];
    char value[6][16];               /* link, interface, IP, device name, share, tempo master */
    uint8_t player;                  /* 1..4 */
    ui_next_deck_t deck[2];
} ui_next_source_model_t;

lv_obj_t *ui_next_source_create(lv_obj_t *parent, uint16_t w, uint16_t h, bool day);
void ui_next_source_update(const ui_next_source_model_t *m);
void ui_next_demo_source(ui_next_source_model_t *m, const ui_next_env_t *env2);

/* FX page (screen 07). CORE-GAP: FX-1 to FX-5 (slots, XY pad, echo settings, HOLD,
 * deck selector). No stem faders: they do not fit the P4. */
#define UI_NEXT_FX_SLOTS 3

typedef struct {
    char name[10], text[12];         /* "ECHO", "40 %" */
    bool on;
    uint8_t amount;                  /* bar, 0..100 */
} ui_next_fx_slot_t;

typedef struct {
    ui_next_fx_slot_t slot[UI_NEXT_FX_SLOTS];
    float xy_x, xy_y;                /* puck position, 0..1 from the top left */
    char label_x[12], label_y[12];   /* "ECHO WET", "FILTER" */
    uint8_t fx_deck;                 /* 0 = deck A, 1 = deck B */
    char panel_title[20];            /* "ECHO  /  DECK A" */
    uint8_t time_beats;              /* 0..3 = 1/4, 1/2, 1, 2 */
    uint8_t wet, feedback;           /* percent */
    bool hold;
    ui_next_deck_t deck[2];
} ui_next_fx_model_t;

lv_obj_t *ui_next_fx_create(lv_obj_t *parent, uint16_t w, uint16_t h, bool day);
void ui_next_fx_update(const ui_next_fx_model_t *m);
void ui_next_demo_fx(ui_next_fx_model_t *m, const ui_next_env_t *env2);

/* Record page (screen 09). CORE-GAP: RC-1 to RC-3 (WAV record, timer, running
 * waveform, meters, free space, recent files), RC-4 (markers). */
#define UI_NEXT_REC_COLS 240
#define UI_NEXT_REC_FILES 3

typedef struct {
    char timer[10];                  /* "00:12:34" */
    const uint8_t *wave[3];          /* low, mid, high half heights, 0..100 = 0..50 view units */
    float head01;                    /* write position in the window */
    float level[2], peak[2];         /* left, right meters */
    char db[2][8];                   /* "-8 dB" */
    char status[12], free_space[14]; /* "RECORDING", "FREE 38 GB" */
    bool recording;
    char value[4][12];               /* source, format, rate, size */
    struct { char name[16], size[8]; } file[UI_NEXT_REC_FILES];
    ui_next_deck_t deck[2];
} ui_next_record_model_t;

lv_obj_t *ui_next_record_create(lv_obj_t *parent, uint16_t w, uint16_t h, bool day);
void ui_next_record_update(const ui_next_record_model_t *m);
void ui_next_demo_record(ui_next_record_model_t *m, const ui_next_env_t *env2);

/* Settings pages (screens 10, 11, 12): tabs AUDIO, DISPLAY, SYSTEM over a list of
 * rows. The model describes the rows, so the core can drive any of them.
 * CORE-GAP: ST-1 to ST-13, FW-6, FW-9, SY-8. */
#define UI_NEXT_SET_MAX 12

typedef enum {
    UI_NEXT_SET_SEG,     /* segmented choice */
    UI_NEXT_SET_SLIDER,  /* level with a value text */
    UI_NEXT_SET_TOGGLE,
    UI_NEXT_SET_STATUS,  /* dot and text */
    UI_NEXT_SET_BUTTON,
} ui_next_setting_kind_t;

typedef enum { UI_NEXT_TAB_AUDIO, UI_NEXT_TAB_DISPLAY, UI_NEXT_TAB_SYSTEM } ui_next_settings_tab_t;

typedef struct {
    ui_next_setting_kind_t kind;
    char label[20];
    char opt[4][8];                  /* SEG options */
    uint8_t n_opt, sel;
    uint8_t pct;                     /* SLIDER */
    bool on;                         /* TOGGLE, or the dot of a STATUS */
    char value[16];                  /* SLIDER value, STATUS text, BUTTON caption */
    bool danger;                     /* BUTTON in red */
} ui_next_setting_t;

typedef struct {
    ui_next_settings_tab_t tab;
    ui_next_setting_t row[UI_NEXT_SET_MAX];
    uint8_t n;
    char saved[10];                  /* "SAVED" */
    /* DISPLAY side panel (ST-7). */
    struct { char name[12], hex[8]; uint32_t color; } wave_color[3];
    uint8_t color_sel;
    char preset[4][8];
    uint8_t preset_sel;
    const uint8_t *preview[3];       /* low, mid, high profiles of the preview */
    ui_next_deck_t deck[2];
} ui_next_settings_model_t;

/* The rows are built from the model at creation; update only moves the values. */
lv_obj_t *ui_next_settings_create(lv_obj_t *parent, uint16_t w, uint16_t h, bool day,
                                  const ui_next_settings_model_t *m);
void ui_next_settings_update(const ui_next_settings_model_t *m);
void ui_next_demo_settings(ui_next_settings_model_t *m, ui_next_settings_tab_t tab, const ui_next_env_t *env2);

/* Grid edit page (screen 08). CORE-GAP: LB-14 (shift, tempo, tap, set beat 1, lock,
 * reset, save), LB-21 (spacing in 1 ms steps, drift chip). */
#define UI_NEXT_GRID_COLS 144

typedef struct {
    char tempo[28];                  /* "124.00 BPM / 483.9 MS" */
    char drift[16];                  /* "DRIFT +6 MS" */
    char hint[56];                   /* "BEAT 1 FITS. BEAT 33 IS 6 MS LATE. USE SPACING." */
    const uint8_t *wave[3];          /* zoomed window, UI_NEXT_GRID_COLS columns */
    uint8_t first_beat;              /* column of beat 1 in the window */
    float window_x01, window_w01;    /* the zoomed part inside the overview */
    int8_t pressed_row, pressed_col; /* amber button, -1 = none */
    const ui_next_env_t *env;        /* whole-track overview */
    ui_next_deck_t deck[2];
} ui_next_gridedit_model_t;

lv_obj_t *ui_next_gridedit_create(lv_obj_t *parent, uint16_t w, uint16_t h, bool day);
void ui_next_gridedit_update(const ui_next_gridedit_model_t *m);
void ui_next_demo_gridedit(ui_next_gridedit_model_t *m, const ui_next_env_t *env3);

/* Alerts overlay (screen 13): dimmed page, toasts at the top right, one modal
 * dialog. Audio never stops because of an alert (SY-1). CORE-GAP: SY-1 to SY-3. */
#define UI_NEXT_TOASTS 3

typedef enum { UI_NEXT_TOAST_DONE, UI_NEXT_TOAST_CARE, UI_NEXT_TOAST_FAULT } ui_next_toast_kind_t;

typedef struct {
    ui_next_toast_kind_t kind;   /* teal done, amber care, red fault */
    char title[32], text[40];
} ui_next_toast_t;

typedef struct {
    ui_next_toast_t toast[UI_NEXT_TOASTS];
    uint8_t n_toasts;
    bool modal;
    char title[32], body[160], primary[16], secondary[16];
} ui_next_alerts_model_t;

/* Adds the overlay on top of the current page of `parent`. One instance. */
lv_obj_t *ui_next_alerts_create(lv_obj_t *parent, uint16_t w, uint16_t h, bool day);
void ui_next_alerts_update(const ui_next_alerts_model_t *m);
void ui_next_demo_alerts(ui_next_alerts_model_t *m);

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

/* Demo model at tick `t` (60 ms per tick). It feeds ui_next_bind_perform with synthetic
 * grids and envelopes, so the numbers are computed as they will be on the device. */
void ui_next_demo_perform(ui_next_perform_model_t *m, const ui_next_env_t *ea,
                          const ui_next_env_t *eb, uint32_t t, ui_next_class_t cls);

#ifdef __cplusplus
}
#endif
