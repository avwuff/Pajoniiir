#pragma once

/* Text and scale helpers the screens share. Pure functions, no core call. */

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* "03:44", or "-03:44" for the time left. Hours roll into minutes up to 99:59. */
void ui_next_fmt_clock(char *out, size_t cap, uint32_t ms);
void ui_next_fmt_remaining(char *out, size_t cap, uint32_t pos_ms, uint32_t duration_ms);

/* "00:12:34" from a count of seconds (the Record timer). */
void ui_next_fmt_hms(char *out, size_t cap, uint32_t seconds);

/* BPM shown on a deck: the track BPM times the playback speed, "124.0". */
void ui_next_fmt_bpm(char *out, size_t cap, uint16_t bpm_x100, uint16_t speed_permille);

/* "842 MB", "1.1 GB", "320 kB": one decimal from 1 GB up. */
void ui_next_fmt_size(char *out, size_t cap, uint64_t bytes);

/* "FREE 38 GB". */
void ui_next_fmt_free(char *out, size_t cap, uint64_t bytes);

/* "-8 dB" from a peak where 32768 is full scale; "-inf" below -90 dB. */
void ui_next_fmt_db(char *out, size_t cap, uint16_t peak);

/* Meter position 0..1 for a peak: 1 at full scale, 0 at `floor_db` (a negative number).
 * The Record board uses -25 (-8 dB reads 0.68); Perform uses a wider range. */
float ui_next_level01(uint16_t peak, float floor_db);

#ifdef __cplusplus
}
#endif
