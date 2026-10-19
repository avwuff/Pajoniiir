#pragma once

/* UTF-8 text helpers for search and sorting (LB-10, LB-19). Pure, no core call. */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Decodes one code point at `s`; writes its byte length to `*len` (1 on a bad byte). */
uint32_t ui_next_utf8_next(const char *s, int *len);

/* Folds `s` for comparison: lower case, and with `strip_accents` the Latin letters lose their
 * accents (e with acute, grave, circumflex and so on become e; the ligatures expand to two
 * letters, so sharp s becomes ss and oe stays oe). Other scripts are copied unchanged.
 *
 * `ostart[i]` and `oend[i]` (optional, each `cap` entries) give, for output byte i, the byte
 * range of the source character it came from, so a hit in the folded text can be shown on
 * the original text with its accents. Returns the folded length, without the NUL. The output
 * is cut at `cap - 1` bytes. */
size_t ui_next_fold(const char *s, bool strip_accents, char *out, size_t cap, uint16_t *ostart, uint16_t *oend);

/* Orders two strings after folding, so "Ete" sorts next to "Ete" and "Zebra" after "Eclair". */
int ui_next_text_cmp(const char *a, const char *b, bool strip_accents);

/* Removes the last character (not the last byte) from `s`. Returns false when it was empty. */
bool ui_next_utf8_backspace(char *s);

#ifdef __cplusplus
}
#endif
