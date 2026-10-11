#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "ui_position_interpolator.h"

static void test_first_sample_returns_snapshot(void)
{
    ui_position_interpolator_t interp;
    ui_position_interpolator_init(&interp);

    uint32_t pos = ui_position_interpolator_update(&interp,
                                                   1000,
                                                   10000,
                                                   true,
                                                   1000,
                                                   1000000);

    assert(pos == 1000);
}

static void test_playing_position_advances_between_identical_snapshots(void)
{
    ui_position_interpolator_t interp;
    ui_position_interpolator_init(&interp);

    assert(ui_position_interpolator_update(&interp, 1000, 10000, true, 1000, 1000000) == 1000);
    assert(ui_position_interpolator_update(&interp, 1000, 10000, true, 1000, 1033000) == 1033);
    assert(ui_position_interpolator_update(&interp, 1000, 10000, true, 1000, 1066000) == 1066);
}

static void test_pitch_speed_changes_interpolated_position(void)
{
    ui_position_interpolator_t interp;
    ui_position_interpolator_init(&interp);

    assert(ui_position_interpolator_update(&interp, 2000, 10000, true, 1050, 1000000) == 2000);
    assert(ui_position_interpolator_update(&interp, 2000, 10000, true, 1050, 1100000) == 2105);
}

static void test_paused_position_tracks_snapshot_without_advancing(void)
{
    ui_position_interpolator_t interp;
    ui_position_interpolator_init(&interp);

    assert(ui_position_interpolator_update(&interp, 3000, 10000, false, 1000, 1000000) == 3000);
    assert(ui_position_interpolator_update(&interp, 3000, 10000, false, 1000, 1500000) == 3000);
}

static void test_playing_zero_speed_tracks_authoritative_snapshot(void)
{
    ui_position_interpolator_t interp;
    ui_position_interpolator_init(&interp);

    assert(ui_position_interpolator_update(&interp, 5000, 10000, true, 0, 1000000) == 5000);
    assert(ui_position_interpolator_update(&interp, 5000, 10000, true, 0, 1033000) == 5000);
    assert(ui_position_interpolator_update(&interp, 4920, 10000, true, 0, 1066000) == 4920);
    assert(ui_position_interpolator_update(&interp, 4920, 10000, true, 1000, 1099000) == 4953);
}

static void test_seek_or_cue_rebases_to_snapshot(void)
{
    ui_position_interpolator_t interp;
    ui_position_interpolator_init(&interp);

    assert(ui_position_interpolator_update(&interp, 5000, 10000, true, 1000, 1000000) == 5000);
    assert(ui_position_interpolator_update(&interp, 5000, 10000, true, 1000, 1100000) == 5100);
    assert(ui_position_interpolator_update(&interp, 0, 10000, true, 1000, 1110000) == 0);
    assert(ui_position_interpolator_update(&interp, 0, 10000, true, 1000, 1143000) == 33);
}

static void test_interpolated_position_clamps_to_duration(void)
{
    ui_position_interpolator_t interp;
    ui_position_interpolator_init(&interp);

    assert(ui_position_interpolator_update(&interp, 9900, 10000, true, 1000, 1000000) == 9900);
    assert(ui_position_interpolator_update(&interp, 9900, 10000, true, 1000, 1500000) == 10000);
}

int main(void)
{
    test_first_sample_returns_snapshot();
    test_playing_position_advances_between_identical_snapshots();
    test_pitch_speed_changes_interpolated_position();
    test_paused_position_tracks_snapshot_without_advancing();
    test_playing_zero_speed_tracks_authoritative_snapshot();
    test_seek_or_cue_rebases_to_snapshot();
    test_interpolated_position_clamps_to_duration();

    const uint32_t speeds[] = {950, 1050};
    for (unsigned i = 0; i < 2; ++i) {
        ui_position_interpolator_t interp;
        ui_position_interpolator_init(&interp);
        for (unsigned frame = 0; frame <= 25; ++frame) {
            uint32_t pos = ui_position_interpolator_update_observed(&interp,
                30000, 200000, true, speeds[i], 1000000u + frame * 20000u, frame == 0);
            assert(pos == 30000 + frame * 20u * speeds[i] / 1000u);
        }
        uint32_t held = interp.last_position_ms;
        assert(ui_position_interpolator_update_observed(&interp, 30000, 200000,
            false, speeds[i], 1520000, false) == held);
        assert(ui_position_interpolator_update_observed(&interp, 30000, 200000,
            false, speeds[i], 1600000, false) == held);
        assert(ui_position_interpolator_update_observed(&interp, 0, 200000,
            true, speeds[i], 1620000, true) == 0); /* fresh seek/play handoff */
        assert(ui_position_interpolator_update_observed(&interp, 0, 200000,
            true, speeds[i], 1640000, false) == 20u * speeds[i] / 1000u);
        assert(ui_position_interpolator_update_observed(&interp, 90000, 200000,
            true, speeds[i], 1660000, true) == 90000);
        assert(ui_position_interpolator_update_observed(&interp, 89920, 200000,
            true, 0, 1680000, true) == 89920); /* authoritative scratch */
        assert(ui_position_interpolator_update_observed(&interp, 90000, 200000,
            true, 0, 1700000, false) == 89920); /* retained sample cannot move it */
        assert(ui_position_interpolator_update_observed(&interp, 89920, 200000,
            true, speeds[i], 1720000, false) == 89920 + 20u * speeds[i] / 1000u);
        assert(ui_position_interpolator_update_observed(&interp, 30000, 200000,
            true, speeds[i], 1740000, true) == 30000); /* actual loop wrap rebases */
    }

    puts("ui_position_interpolator tests passed: retained samples do not rebase; pause, seek, scratch and +/-5% retained bursts");
    return 0;
}
