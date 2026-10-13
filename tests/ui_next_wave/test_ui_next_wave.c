#include <assert.h>
#include <stdio.h>

#include "ui_next_theme.h"
#include "ui_next_wave.h"

static ui_next_env_t A, B;

static void test_envelope_is_deterministic_and_bounded(void)
{
    ui_next_env_demo(&A, 11);
    ui_next_env_demo(&B, 11);
    for (uint32_t i = 0; i < UI_NEXT_ENV_COLUMNS; i++) {
        assert(A.lo[i] == B.lo[i] && A.mi[i] == B.mi[i] && A.hi[i] == B.hi[i]);
        assert(A.lo[i] >= 0.0f && A.lo[i] <= 1.0f);
        assert(A.mi[i] >= 0.0f && A.mi[i] <= 1.0f);
        assert(A.hi[i] >= 0.0f && A.hi[i] <= 1.0f);
    }
    ui_next_env_demo(&B, 23);
    int differs = 0;
    for (uint32_t i = 0; i < UI_NEXT_ENV_COLUMNS; i++) differs |= A.lo[i] != B.lo[i];
    assert(differs);
}

static void test_camelot_fit(void)
{
    assert(ui_next_key_fit(8, 'A', 8, 'A'));
    assert(ui_next_key_fit(8, 'A', 9, 'A'));
    assert(ui_next_key_fit(8, 'A', 8, 'B'));
    assert(ui_next_key_fit(12, 'A', 1, 'A')); /* the wheel wraps */
    assert(!ui_next_key_fit(8, 'A', 9, 'B'));
    assert(!ui_next_key_fit(8, 'A', 10, 'A'));
}

static void test_link_values(void)
{
    ui_next_env_demo(&A, 11);
    ui_next_env_demo(&B, 23);
    ui_next_link_t l;
    ui_next_link_compute(&A, 900, &B, 900, 180, &l);
    assert(l.phase_beat == 0.0f && l.beat_ms == 0.0f && l.phrase_off == 0);
    ui_next_link_compute(&A, 900, &B, 900 + 9 + 2, 180, &l);
    assert(l.phrase_off == 1);
    assert(l.phase_beat > 0.2f && l.phase_beat < 0.25f);
    assert(l.bass_pct >= 4 && l.bass_pct <= 100 && l.overlap_pct >= 8 && l.overlap_pct <= 100);
}

static void test_palette_and_severity(void)
{
    assert(ui_next_palette(false)->accent == 0xFFC15A);
    assert(ui_next_palette(true)->wave_low == 0x7B3FE4);
    assert(ui_next_severity(0.0f) == UI_NEXT_SEV_OK);
    assert(ui_next_severity(0.3f) == UI_NEXT_SEV_WARN);
    assert(ui_next_severity(0.9f) == UI_NEXT_SEV_BAD);
}

int main(void)
{
    test_envelope_is_deterministic_and_bounded();
    test_camelot_fit();
    test_link_values();
    test_palette_and_severity();
    puts("ui_next_wave: ok");
    return 0;
}
