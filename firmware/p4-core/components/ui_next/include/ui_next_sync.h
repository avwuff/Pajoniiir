#pragma once

/* Maths between two decks (PB-17, PB-18), from the beat grids and envelopes the core
 * already exposes. Pure functions over plain structs, no core call. A firmware glue
 * copies `anlz_beat_t` (time_ms, beat_phase) into `ui_next_beat_t`. */

#include <stdbool.h>
#include <stdint.h>

#include "ui_next_wave.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint32_t time_ms;
    uint8_t phase; /* beat in the bar, 1 to 4; 0 = unknown */
} ui_next_beat_t;

typedef struct {
    const ui_next_beat_t *beat;
    uint16_t count;
} ui_next_grid_t;

/* Index of the last beat at or before `t_ms`, or -1 when there is none. */
int ui_next_grid_floor(const ui_next_grid_t *g, int64_t t_ms);
/* Index of the beat closest to `t_ms`, or -1 for an empty grid. */
int ui_next_grid_nearest(const ui_next_grid_t *g, int64_t t_ms);
/* Position inside the bar at `t_ms`, 0.0 up to 4.0 (beat 1 starts at 0). False without a grid. */
bool ui_next_grid_bar_pos(const ui_next_grid_t *g, int64_t t_ms, float *out);

typedef struct {
    bool valid;
    float beat_ms;         /* signed, deck B late is positive, in real time */
    float phase_beats;     /* signed, -0.5 to 0.5 beats, B late is positive */
    uint8_t phrase_beats;  /* how many beats B is ahead of A in the bar, 0 to 3 */
    float tempo_pct;       /* tempo of B against A, in percent */
} ui_next_sync_t;

/* `bpm_x100` is the track BPM and `speed_permille` the playback speed (1000 = 1x). */
void ui_next_sync_compute(const ui_next_grid_t *a, int64_t pos_a_ms, uint16_t bpm_a_x100, uint16_t speed_a_permille,
                          const ui_next_grid_t *b, int64_t pos_b_ms, uint16_t bpm_b_x100, uint16_t speed_b_permille,
                          ui_next_sync_t *out);

/* Envelope value 0..1 of one band (0 low, 1 mid, 2 high) at a time of the track. The
 * envelope spans the whole track in UI_NEXT_ENV_COLUMNS columns. Linear between columns. */
float ui_next_env_sample(const ui_next_env_t *env, int band, uint32_t duration_ms, int64_t t_ms);

/* Share of the window (percent) where both decks have energy at the same screen place:
 * low band above .30 for the bass clash, mid band above .16 for the overlap. */
void ui_next_sync_overlap(const ui_next_env_t *ea, uint32_t dur_a_ms, int64_t pos_a_ms,
                          const ui_next_env_t *eb, uint32_t dur_b_ms, int64_t pos_b_ms,
                          uint32_t window_ms, uint8_t *bass_pct, uint8_t *overlap_pct);

typedef struct {
    float x01;  /* 0 to 1 across the window, the playhead at 0.5 */
    bool bar;   /* beat 1 of a bar */
    int index;  /* index in the grid */
} ui_next_screen_beat_t;

/* Beats of a deck that fall inside the window around `pos_ms`. Returns how many were written. */
int ui_next_sync_screen_beats(const ui_next_grid_t *g, int64_t pos_ms, uint32_t window_ms,
                              ui_next_screen_beat_t *out, int max);

typedef struct {
    float xa, xb;  /* the beat on deck A and the matching beat on deck B */
    bool bar;      /* the A beat opens a bar */
    bool has_bar;  /* `xbar` is set */
    float xbar;    /* where the nearest bar of B sits, for the curve of a bar */
} ui_next_weave_t;

/* Pairs each beat of A with the beat of B nearest on the screen, if closer than
 * `max_dx`. Bars of A are also paired with the nearest bar of B. */
int ui_next_sync_weave(const ui_next_screen_beat_t *a, int na, const ui_next_screen_beat_t *b, int nb,
                       float max_dx, ui_next_weave_t *out, int max);

#ifdef __cplusplus
}
#endif
