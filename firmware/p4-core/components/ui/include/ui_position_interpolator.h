#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define UI_POSITION_INTERPOLATOR_REBASE_THRESHOLD_MS 120u

typedef struct {
    bool initialized;
    bool last_playing;
    uint32_t anchor_position_ms;
    uint64_t anchor_time_us;
    uint32_t last_position_ms;
} ui_position_interpolator_t;

void ui_position_interpolator_init(ui_position_interpolator_t *interp);

uint32_t ui_position_interpolator_update(ui_position_interpolator_t *interp,
                                         uint32_t snapshot_position_ms,
                                         uint32_t duration_ms,
                                         bool playing,
                                         /* Zero disables prediction and makes
                                          * each snapshot authoritative. */
                                         uint32_t speed_permille,
                                         uint64_t now_us);

/* A retained snapshot is not a new transport observation and must never
 * trigger a seek/rebase. Playback/pause remains an authoritative live flag. */
uint32_t ui_position_interpolator_update_observed(ui_position_interpolator_t *interp,
    uint32_t snapshot_position_ms, uint32_t duration_ms, bool playing,
    uint32_t speed_permille, uint64_t now_us, bool snapshot_fresh);

#ifdef __cplusplus
}
#endif
