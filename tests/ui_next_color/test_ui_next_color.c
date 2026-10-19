#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "ui_next_color.h"

static ui_next_env_t E;

static int near(float a, float b) { return fabsf(a - b) < 0.002f; }

static void set(uint8_t *p, uint32_t i, uint8_t h, uint8_t b0, uint8_t b1, uint8_t b2)
{
    uint8_t *e = &p[i * UI_NEXT_PWV4_ENTRY];
    e[0] = h; e[1] = 0; e[2] = 0; e[3] = b0; e[4] = b1; e[5] = b2;
}

static void test_full_length_preview(void)
{
    static uint8_t pv[1200 * UI_NEXT_PWV4_ENTRY];
    memset(pv, 0, sizeof pv);
    set(pv, 0, 100, 255, 0, 0);     /* all bass */
    set(pv, 1, 50, 0, 200, 100);    /* mid and treble, half the height */
    set(pv, 2, 100, 255, 255, 255); /* every band at the top */
    set(pv, 3, 0, 255, 255, 255);   /* silence: no height wins over the bands */
    assert(ui_next_env_from_pwv4(pv, sizeof pv, &E));
    assert(near(E.lo[0], 1.0f) && near(E.mi[0], 0.0f) && near(E.hi[0], 0.0f));
    assert(near(E.lo[1], 0.0f) && near(E.mi[1], 0.5f) && near(E.hi[1], 0.25f));
    assert(near(E.lo[2], 1.0f) && near(E.mi[2], 1.0f) && near(E.hi[2], 1.0f));
    assert(near(E.lo[3], 0.0f) && near(E.mi[3], 0.0f) && near(E.hi[3], 0.0f));
    assert(near(E.lo[4], 0.0f)); /* an all zero column stays flat */
}

static void test_short_preview_stretches(void)
{
    uint8_t pv[600 * UI_NEXT_PWV4_ENTRY];
    memset(pv, 0, sizeof pv);
    for (uint32_t i = 0; i < 600; i++) set(pv, i, (uint8_t)(i % 100 + 1), 255, 10, 10);
    assert(ui_next_env_from_pwv4(pv, sizeof pv, &E));
    /* 600 entries over 1200 columns: each entry covers two columns. */
    assert(near(E.lo[0], E.lo[1]) && near(E.lo[2], E.lo[3]));
    for (uint32_t c = 0; c < UI_NEXT_ENV_COLUMNS; c++) assert(E.lo[c] >= 0.0f && E.lo[c] <= 1.0f);
}

static void test_long_preview_keeps_peaks(void)
{
    static uint8_t pv[3600 * UI_NEXT_PWV4_ENTRY];
    memset(pv, 0, sizeof pv);
    set(pv, 1, 90, 255, 0, 0); /* one loud entry inside a span of three */
    set(pv, 0, 10, 255, 0, 0);
    set(pv, 2, 10, 255, 0, 0);
    set(pv, 3000, 100, 0, 255, 0);
    assert(ui_next_env_from_pwv4(pv, sizeof pv, &E));
    assert(near(E.lo[0], 0.9f)); /* the maximum of the span, not an average */
    assert(near(E.mi[1000], 1.0f));
}

static void test_rejects_empty(void)
{
    uint8_t pv[6] = {0};
    assert(!ui_next_env_from_pwv4(NULL, 0, &E));
    assert(!ui_next_env_from_pwv4(pv, 5, &E));
    assert(!ui_next_env_from_pwv4(pv, 6, NULL));
    assert(ui_next_env_from_pwv4(pv, 6, &E)); /* one entry, all zero: valid and flat */
    assert(near(E.lo[0], 0.0f) && near(E.hi[1199], 0.0f));
}

int main(void)
{
    test_full_length_preview();
    test_short_preview_stretches();
    test_long_preview_keeps_peaks();
    test_rejects_empty();
    puts("ui_next_color: ok");
    return 0;
}
