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
    test_palette_and_severity();
    puts("ui_next_wave: ok");
    return 0;
}
