#include "ui_position_interpolator.h"

static uint32_t clamp_to_duration(uint32_t position_ms, uint32_t duration_ms)
{
    if (duration_ms > 0 && position_ms > duration_ms) {
        return duration_ms;
    }
    return position_ms;
}

static uint32_t abs_delta_u32(uint32_t a, uint32_t b)
{
    return a > b ? a - b : b - a;
}

void ui_position_interpolator_init(ui_position_interpolator_t *interp)
{
    if (!interp) {
        return;
    }

    interp->initialized = false;
    interp->last_playing = false;
    interp->anchor_position_ms = 0;
    interp->anchor_time_us = 0;
    interp->last_position_ms = 0;
}

uint32_t ui_position_interpolator_update(ui_position_interpolator_t *interp,
                                         uint32_t snapshot_position_ms,
                                         uint32_t duration_ms,
                                         bool playing,
                                         uint32_t speed_permille,
                                         uint64_t now_us)
{
    return ui_position_interpolator_update_observed(interp, snapshot_position_ms,
        duration_ms, playing, speed_permille, now_us, true);
}

uint32_t ui_position_interpolator_update_observed(ui_position_interpolator_t *interp,
    uint32_t snapshot_position_ms, uint32_t duration_ms, bool playing,
    uint32_t speed_permille, uint64_t now_us, bool snapshot_fresh)
{
    if (!interp) {
        return clamp_to_duration(snapshot_position_ms, duration_ms);
    }

    snapshot_position_ms = clamp_to_duration(snapshot_position_ms, duration_ms);
    /* Zero is an explicit stationary/authoritative mode used while the audible
     * scratch head owns deck position. Return every snapshot exactly so a held
     * platter stays parked and jog motion is not filtered by the normal 120 ms
     * forward-prediction rebase threshold. Re-anchor here so normal playback
     * resumes from the last scratch position when the platter is released. */
    if (speed_permille == 0) {
        if (!snapshot_fresh && interp->initialized)
            snapshot_position_ms = clamp_to_duration(interp->last_position_ms, duration_ms);
        interp->initialized = true;
        interp->last_playing = playing;
        interp->anchor_position_ms = snapshot_position_ms;
        interp->anchor_time_us = now_us;
        interp->last_position_ms = snapshot_position_ms;
        return snapshot_position_ms;
    }

    if (!interp->initialized || !playing || !interp->last_playing) {
        if (!snapshot_fresh && interp->initialized)
            snapshot_position_ms = clamp_to_duration(interp->last_position_ms, duration_ms);
        interp->initialized = true;
        interp->last_playing = playing;
        interp->anchor_position_ms = snapshot_position_ms;
        interp->anchor_time_us = now_us;
        interp->last_position_ms = snapshot_position_ms;
        return snapshot_position_ms;
    }

    uint64_t elapsed_us = now_us >= interp->anchor_time_us
                        ? now_us - interp->anchor_time_us
                        : 0;
    uint64_t advance_ms = (elapsed_us * speed_permille) / 1000000u;
    uint64_t predicted = (uint64_t)interp->anchor_position_ms + advance_ms;
    uint32_t predicted_ms = predicted > UINT32_MAX ? UINT32_MAX : (uint32_t)predicted;
    predicted_ms = clamp_to_duration(predicted_ms, duration_ms);

    if (snapshot_fresh && abs_delta_u32(snapshot_position_ms, predicted_ms) >
        UI_POSITION_INTERPOLATOR_REBASE_THRESHOLD_MS) {
        interp->last_playing = playing;
        interp->anchor_position_ms = snapshot_position_ms;
        interp->anchor_time_us = now_us;
        interp->last_position_ms = snapshot_position_ms;
        return snapshot_position_ms;
    }

    interp->last_playing = playing;
    interp->last_position_ms = predicted_ms;
    return predicted_ms;
}
