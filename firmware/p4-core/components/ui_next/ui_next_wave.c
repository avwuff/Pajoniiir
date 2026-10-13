#include "ui_next_wave.h"

#include <math.h>

/* Small deterministic generator, so captures stay identical between runs. */
static uint32_t rng_next(uint32_t *s)
{
    *s += 0x6D2B79F5u;
    uint32_t t = *s;
    t = (t ^ (t >> 15)) * (t | 1u);
    t ^= t + (t ^ (t >> 7)) * (t | 61u);
    return t ^ (t >> 14);
}

static float rng01(uint32_t *s)
{
    return (float)rng_next(s) / 4294967296.0f;
}

static float minf(float a, float b) { return a < b ? a : b; }
static float maxf(float a, float b) { return a > b ? a : b; }

void ui_next_env_demo(ui_next_env_t *out, uint32_t seed)
{
    uint32_t s = seed;
    const uint32_t n = UI_NEXT_ENV_COLUMNS;
    for (uint32_t i = 0; i < n; i++) {
        float p = (float)i / (float)n;
        float e = p < .08f ? .22f
                : p < .2f  ? .22f + (p - .08f) / .12f * .55f
                : p < .26f ? .8f
                : p < .5f  ? 1.0f
                : p < .6f  ? .28f
                : p < .9f  ? 1.0f
                           : maxf(.12f, .5f * (1.0f - (p - .9f) / .1f));
        float ph = (float)(i % UI_NEXT_BEAT_COLUMNS) / (float)UI_NEXT_BEAT_COLUMNS;
        float k = expf(-ph * 4.5f);
        float ht = expf(-fmodf(ph + .5f, 1.0f) * 7.0f);
        float sn = expf(-fmodf(ph * 2.0f, 1.0f) * 5.0f);
        out->lo[i] = minf(1.0f, e * (.22f + .78f * k) * (.92f + .16f * rng01(&s)));
        float brk = (p > .5f && p < .6f) ? .32f * (.6f + .4f * rng01(&s)) : 0.0f;
        out->mi[i] = minf(1.0f, e * (.18f + .32f * sn) * (.75f + .5f * rng01(&s)) + brk);
        out->hi[i] = minf(1.0f, e * (.1f + .55f * ht) * (.65f + .7f * rng01(&s)));
    }
}

float ui_next_env_smooth(const float *arr, uint32_t len, int32_t i, int32_t w)
{
    float sum = 0.0f;
    int32_t cnt = 0;
    for (int32_t j = i - w; j <= i + w; j++) {
        if (j >= 0 && (uint32_t)j < len) {
            sum += arr[j];
            cnt++;
        }
    }
    return cnt ? sum / (float)cnt : 0.0f;
}

bool ui_next_key_fit(uint8_t na, char la, uint8_t nb, char lb)
{
    int d = na > nb ? na - nb : nb - na;
    int ring = d < 12 - d ? d : 12 - d;
    return ring + (la == lb ? 0 : 1) <= 1;
}

void ui_next_link_compute(const ui_next_env_t *ea, uint32_t a, const ui_next_env_t *eb,
                          uint32_t b, uint32_t m, ui_next_link_t *out)
{
    uint32_t clash = 0, mid = 0;
    for (uint32_t o = 0; o < m; o++) {
        float la = ui_next_env_smooth(ea->lo, UI_NEXT_ENV_COLUMNS, (int32_t)(a + o), 2);
        float lb = ui_next_env_smooth(eb->lo, UI_NEXT_ENV_COLUMNS, (int32_t)(b + o), 2);
        float ma = ui_next_env_smooth(ea->mi, UI_NEXT_ENV_COLUMNS, (int32_t)(a + o), 2);
        float mb = ui_next_env_smooth(eb->mi, UI_NEXT_ENV_COLUMNS, (int32_t)(b + o), 2);
        if (minf(la, lb) > .30f) clash++;
        if (minf(ma, mb) > .16f) mid++;
    }
    out->bass_pct = (uint8_t)maxf(4.0f, floorf(100.0f * (float)clash / (float)m + .5f));
    out->overlap_pct = (uint8_t)maxf(8.0f, floorf(100.0f * (float)mid / (float)m + .5f));

    /* Beat error: columns of deck B off the nearest beat of deck A's grid. */
    int d = (int)((b - a) % UI_NEXT_BEAT_COLUMNS);
    if (d > (int)UI_NEXT_BEAT_COLUMNS / 2) d -= (int)UI_NEXT_BEAT_COLUMNS;
    out->phase_beat = (float)d / (float)UI_NEXT_BEAT_COLUMNS;
    out->beat_ms = out->phase_beat * 483.9f; /* 124 BPM: one beat is 483.9 ms */
    out->phrase_off = (uint8_t)(((b - a) % UI_NEXT_BAR_COLUMNS) / UI_NEXT_BEAT_COLUMNS);
    out->key_fit = false; /* set by the caller from the loaded keys */
}
