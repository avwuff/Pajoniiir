#pragma once

/* Three-band envelope from the PWV4 colour preview of a Rekordbox track (PB-1, LB-6).
 * PWV4 has six bytes per column: byte 0 is the height, bytes 3, 4 and 5 are the bass,
 * mid and treble intensities (see anlz_color_preview_column in the core). The core's
 * own comment calls them a "normalized bass/mid/treble mix"; that reading is not
 * verified on a real export. No core call: the caller passes the bytes. */

#include <stdbool.h>
#include <stdint.h>

#include "ui_next_wave.h"

#ifdef __cplusplus
extern "C" {
#endif

#define UI_NEXT_PWV4_ENTRY 6u

/* Resamples the preview to UI_NEXT_ENV_COLUMNS columns (the maximum over a span when the
 * preview is longer, the nearest column when shorter). Height is normalised to the loudest
 * column; the strongest band of a column takes that height and the others follow in ratio.
 * Returns false for an empty preview. */
bool ui_next_env_from_pwv4(const uint8_t *pwv4, uint32_t len, ui_next_env_t *out);

#ifdef __cplusplus
}
#endif
