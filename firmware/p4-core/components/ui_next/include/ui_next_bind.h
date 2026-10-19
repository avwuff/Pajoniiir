#pragma once

/* From what the core publishes to what the Perform page shows. The caller fills plain
 * inputs (copied from `audio_engine`, `deck_core`, the library and the ANLZ snapshot);
 * this module formats the text, decides the chips and computes the numbers between the
 * decks. No core call, so it runs on the host. */

#include <stdbool.h>
#include <stdint.h>

#include "ui_next.h"
#include "ui_next_view.h"

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

/* Browse: the visible window of the library view (LB-1, LB-7, LB-16, LB-22). The caller owns
 * the index and the two work arrays (`view` and `scratch`, `cap` entries each, cap >= n). */
typedef struct {
    const ui_next_entry_t *entry;
    size_t n;
    ui_next_filter_t filter;
    ui_next_sort_t sort;
    bool strip_accents;           /* Display setting (LB-19) */
    size_t first;                 /* position in the view of the first row shown */
    char crumb[32];
    uint8_t category;
    ui_next_tag_t tag[UI_NEXT_BROWSE_TAGS]; /* `on` is taken from filter.tag_mask */
    char match_deck;              /* 'A' or 'B': the deck the match strip follows, 0 = none */
    const char *match_key;        /* key loaded on that deck */
    uint16_t *view, *scratch;
    size_t cap;
} ui_next_browse_in_t;

/* Fills the rows, the count ("38 OF 96"), the match text and the tag badge. The deck docks and
 * the preview envelopes stay as the caller set them. Returns the number of tracks in the view. */
size_t ui_next_bind_browse(const ui_next_browse_in_t *in, ui_next_browse_model_t *out);

/* Search: runs the query on the index and fills the result rows with the title split around
 * the match (LB-10, LB-19). Deck docks stay as the caller set them. Returns the hit count. */
size_t ui_next_bind_search(const ui_next_entry_t *entry, size_t n, const char *query, bool strip_accents,
                           const char *ref_key, ui_next_search_model_t *out);

#ifdef __cplusplus
}
#endif
