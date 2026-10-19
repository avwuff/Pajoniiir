#pragma once

/* Musical key helpers for the UI (PB-10, LB-18). The library gives a key as text:
 * Camelot ("8A", "12B") or a classical name ("Am", "F#m", "Bb"). No core call. */

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint8_t number; /* 1..12 */
    char letter;    /* 'A' minor, 'B' major */
} ui_next_camelot_t;

/* Parses "8A", "08a", "12B", or a classical name. Returns false for an empty or
 * unknown text. */
bool ui_next_key_parse(const char *text, ui_next_camelot_t *out);

/* How a key sits against a reference key, as the Browse key chip shows it. */
typedef enum {
    UI_NEXT_REL_NONE,  /* no colour */
    UI_NEXT_REL_FIT,   /* same key, a step either way, or the relative major or minor */
    UI_NEXT_REL_PLUS2, /* energy lift of two steps on the same letter */
    UI_NEXT_REL_PLUS7, /* energy lift of seven steps on the same letter */
} ui_next_key_rel_t;

ui_next_key_rel_t ui_next_key_relation(ui_next_camelot_t ref, ui_next_camelot_t key);

/* Text in, text out. An unknown key on either side gives NONE. */
ui_next_key_rel_t ui_next_key_relation_text(const char *ref, const char *key);

/* True when the two keys fit: relation FIT. Same rule as PB-10. */
bool ui_next_key_text_fit(const char *a, const char *b);

#ifdef __cplusplus
}
#endif
