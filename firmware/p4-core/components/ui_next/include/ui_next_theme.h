#pragma once

/* Colour tokens of the new DJ UI (FW-6, SY-7). 0xRRGGBB, no LVGL dependency.
 * Night values come from the demo boards. Day values come from the Day board
 * (15-perform-day); the few the board does not show are marked "derived". */

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint32_t frame;     /* behind everything */
    uint32_t panel;     /* top bar, cards */
    uint32_t nav;       /* nav rail, dock */
    uint32_t sel;       /* active nav item */
    uint32_t well;      /* waveform and overview background */
    uint32_t border;
    uint32_t border_hi; /* buttons, chips outline */
    uint32_t chip;
    uint32_t text;
    uint32_t muted;
    uint32_t on_accent; /* text on amber */
    uint32_t accent;    /* amber: playhead, active, bar 1 */
    uint32_t accent_ink;/* amber as text or thin line on the panel */
    uint32_t ok, warn, bad;
    uint32_t wave_low, wave_mid, wave_high;
} ui_next_palette_t;

const ui_next_palette_t *ui_next_palette(bool day);

/* Severity of a link bar meter: error size 0..1 -> ok, warn or bad (PB-17). */
typedef enum { UI_NEXT_SEV_OK, UI_NEXT_SEV_WARN, UI_NEXT_SEV_BAD } ui_next_sev_t;
ui_next_sev_t ui_next_severity(float err01);

#ifdef __cplusplus
}
#endif
