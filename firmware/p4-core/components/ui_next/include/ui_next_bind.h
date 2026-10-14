#pragma once

/* From what the core publishes to what the Perform page shows. The caller fills plain
 * inputs (copied from `audio_engine`, `deck_core`, the library and the ANLZ snapshot);
 * this module formats the text, decides the chips and computes the numbers between the
 * decks. No core call, so it runs on the host. */

#include <stdbool.h>
#include <stdint.h>

#include "ui_next.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    char title[48];
    char key[16];                 /* library_track_t.key: Camelot or a classical name */
    uint16_t bpm_x100;            /* track BPM */
    uint16_t speed_permille;      /* audio_engine effective speed, 1000 = 1x, 0 = unknown */
    uint32_t pos_ms, duration_ms;
    bool master, sync;            /* tempo master, following the master */
    ui_next_grid_t grid;          /* from anlz_beat_t */
    const ui_next_env_t *env;     /* from ui_next_env_from_pwv4 */
    uint16_t peak;                /* audio_engine deck_peak_display, 32768 = full scale */
} ui_next_deck_in_t;

typedef struct {
    ui_next_deck_in_t deck[2];
    uint32_t window_ms;           /* time shown by the zoomed waveform */
    bool time_remain;             /* app_settings time_remain */
    ui_next_class_t cls;
} ui_next_perform_in_t;

void ui_next_bind_perform(const ui_next_perform_in_t *in, ui_next_perform_model_t *out);

/* The Browse key chip colour of a row against the key loaded on the other deck. */
ui_next_key_class_t ui_next_bind_key_class(const char *reference, const char *key);

#ifdef __cplusplus
}
#endif
