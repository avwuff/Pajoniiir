#pragma once

/* Library view for Browse (LB-1, LB-7, LB-9, LB-16, LB-22): filter, stable sort, groups and
 * the queue, all in RAM on the index. Pure, no core call. */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint16_t id;
    char title[48], artist[32], genre[16];
    uint16_t bpm_x100;
    char key[5];
    uint32_t duration_s;
    uint16_t tags; /* one bit per tag (LB-22) */
} ui_next_entry_t;

typedef enum {
    UI_NEXT_COL_TITLE,
    UI_NEXT_COL_ARTIST,
    UI_NEXT_COL_BPM,
    UI_NEXT_COL_KEY,
    UI_NEXT_COL_TIME,
    UI_NEXT_COL_GENRE,
} ui_next_col_id_t;

typedef struct {
    ui_next_col_id_t col;
    bool desc;
} ui_next_sort_t;

/* Tap on a heading: another column sorts up, the same column reverses (LB-16). */
void ui_next_sort_tap(ui_next_sort_t *s, ui_next_col_id_t col);

/* The match strip and the tag chips. A part that is off lets every track through. */
typedef struct {
    bool bpm_on;
    uint16_t bpm_ref_x100;
    uint8_t bpm_pct;   /* kept when within this many percent of the reference */
    bool key_on;
    char key_ref[5];   /* kept when the key fits the reference (PB-10) */
    char genre[16];    /* empty = any; compared after folding */
    uint16_t tag_mask; /* tags in the filter, 0 = none */
    bool tag_all;      /* ALL: every tag of the mask; else ANY (LB-22) */
} ui_next_filter_t;

bool ui_next_filter_pass(const ui_next_filter_t *f, const ui_next_entry_t *e);

/* Fills `out` with the indices of the entries that pass the filter, sorted. Equal entries
 * keep their index order, so the list does not jump on a second tap. `scratch` holds `cap`
 * indices. Returns the count of entries that pass. Give `cap >= n` for a full view: when the
 * count is more than `cap`, only the first `cap` that pass are kept and sorted. */
size_t ui_next_view_build(const ui_next_entry_t *e, size_t n, const ui_next_filter_t *f, ui_next_sort_t sort,
                          bool strip_accents, uint16_t *out, uint16_t *scratch, size_t cap);

/* Groups of a view sorted on `col`: a new group starts where the field changes. Writes the
 * position in the view of each first row. Returns the group count. */
size_t ui_next_view_groups(const ui_next_entry_t *e, const uint16_t *view, size_t n, ui_next_col_id_t col,
                           bool strip_accents, uint16_t *first, size_t cap);

/* Live search on titles (LB-10, LB-19). The query and the titles are folded the same way, so
 * "ete" finds "Ete" with its accents. `hit` gives the matched byte range of the original title. */
typedef struct {
    uint16_t index;      /* into the entry array */
    uint8_t start, end;  /* byte range of the match in the title */
} ui_next_hit_t;

/* Fills `out` with up to `cap` hits, best first: a title that starts with the query, then the
 * others, each group in index order. Returns the number of hits found (can be more than `cap`).
 * An empty query finds nothing. */
size_t ui_next_search(const ui_next_entry_t *e, size_t n, const char *query, bool strip_accents,
                      ui_next_hit_t *out, size_t cap);

/* Queue of next tracks (LB-9): ids in RAM. A track is in the queue once. */
#define UI_NEXT_QUEUE_MAX 32

typedef struct {
    uint16_t id[UI_NEXT_QUEUE_MAX];
    uint8_t n;
} ui_next_queue_t;

bool ui_next_queue_add(ui_next_queue_t *q, uint16_t id);          /* false when full or present */
bool ui_next_queue_remove(ui_next_queue_t *q, uint8_t pos);
bool ui_next_queue_pop(ui_next_queue_t *q, uint16_t *id);         /* takes the first */
bool ui_next_queue_move(ui_next_queue_t *q, uint8_t from, uint8_t to);

#ifdef __cplusplus
}
#endif
