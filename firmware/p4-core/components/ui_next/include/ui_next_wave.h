#pragma once

/* Three-band envelope and the pure maths behind the Perform page (PB-1, PB-17).
 * One stored envelope has 3 values per column: low, mid, high, each 0..1.
 * No LVGL, no core dependency. */

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define UI_NEXT_ENV_COLUMNS 1200u   /* demo track length in columns */
#define UI_NEXT_BEAT_COLUMNS 9u     /* one beat = 9 columns, a bar = 36 */
#define UI_NEXT_BAR_COLUMNS (UI_NEXT_BEAT_COLUMNS * 4u)

typedef struct {
    float lo[UI_NEXT_ENV_COLUMNS];
    float mi[UI_NEXT_ENV_COLUMNS];
    float hi[UI_NEXT_ENV_COLUMNS];
} ui_next_env_t;

/* Demo track with kick, snare and hat shapes. CORE-GAP: PB-1 (real envelopes). */
void ui_next_env_demo(ui_next_env_t *out, uint32_t seed);

/* Link bar values for two decks (PB-17). Layers 1 to 4 and 7 of the board. */
typedef struct {
    float beat_ms;       /* signed, deck B late is positive */
    float phase_beat;    /* signed, in beats */
    uint8_t phrase_off;  /* beats */
    uint8_t bass_pct;    /* share of the window where both decks have low-band energy */
    uint8_t overlap_pct; /* same for the mid band */
    bool key_fit;        /* Camelot distance <= 1 */
} ui_next_link_t;

/* Window: `m` columns starting at column `a` of deck A and `b` of deck B. */
void ui_next_link_compute(const ui_next_env_t *ea, uint32_t a, const ui_next_env_t *eb,
                          uint32_t b, uint32_t m, ui_next_link_t *out);

/* Camelot fit: same number with the other letter, or the next number with the
 * same letter, or identical. `num` 1..12, `letter` 'A' or 'B' (PB-10, LB-18). */
bool ui_next_key_fit(uint8_t num_a, char letter_a, uint8_t num_b, char letter_b);

/* Mean of the neighbouring columns (the demo smooths by +-w). */
float ui_next_env_smooth(const float *arr, uint32_t len, int32_t i, int32_t w);

#ifdef __cplusplus
}
#endif
