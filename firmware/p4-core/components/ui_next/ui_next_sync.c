#include "ui_next_sync.h"

#include <math.h>

int ui_next_grid_floor(const ui_next_grid_t *g, int64_t t)
{
    if (!g || !g->beat || g->count == 0 || t < (int64_t)g->beat[0].time_ms) return -1;
    int lo = 0, hi = (int)g->count - 1;
    while (lo < hi) {
        const int mid = (lo + hi + 1) / 2;
        if ((int64_t)g->beat[mid].time_ms <= t) lo = mid;
        else hi = mid - 1;
    }
    return lo;
}

int ui_next_grid_nearest(const ui_next_grid_t *g, int64_t t)
{
    if (!g || !g->beat || g->count == 0) return -1;
    const int f = ui_next_grid_floor(g, t);
    if (f < 0) return 0;
    if (f + 1 >= (int)g->count) return f;
    const int64_t d0 = t - (int64_t)g->beat[f].time_ms, d1 = (int64_t)g->beat[f + 1].time_ms - t;
    return d1 < d0 ? f + 1 : f;
}

/* Spacing of the beat that starts at index `i`, using the next beat or else the previous. */
static float interval_ms(const ui_next_grid_t *g, int i)
{
    if (i + 1 < (int)g->count) return (float)((int64_t)g->beat[i + 1].time_ms - (int64_t)g->beat[i].time_ms);
    if (i > 0) return (float)((int64_t)g->beat[i].time_ms - (int64_t)g->beat[i - 1].time_ms);
    return 0.0f;
}

static int beat_in_bar(const ui_next_grid_t *g, int i)
{
    const uint8_t p = g->beat[i].phase;
    return p >= 1 && p <= 4 ? p - 1 : i % 4;
}

bool ui_next_grid_bar_pos(const ui_next_grid_t *g, int64_t t, float *out)
{
    if (!g || !g->beat || g->count < 2 || !out) return false;
    int f = ui_next_grid_floor(g, t);
    float frac;
    if (f < 0) {
        /* Before the first beat: continue the first interval backwards. */
        const float iv = interval_ms(g, 0);
        if (iv <= 0.0f) return false;
        frac = (float)(t - (int64_t)g->beat[0].time_ms) / iv; /* negative */
        f = 0;
    } else {
        const float iv = interval_ms(g, f);
        if (iv <= 0.0f) return false;
        frac = (float)(t - (int64_t)g->beat[f].time_ms) / iv;
    }
    float pos = (float)beat_in_bar(g, f) + frac;
    pos = fmodf(pos, 4.0f);
    if (pos < 0.0f) pos += 4.0f;
    *out = pos;
    return true;
}

/* Signed distance to the nearest whole beat, -0.5 to 0.5. */
static float frac_beat(float bar_pos)
{
    return bar_pos - roundf(bar_pos);
}

void ui_next_sync_compute(const ui_next_grid_t *a, int64_t pa, uint16_t bpm_a, uint16_t sa,
                          const ui_next_grid_t *b, int64_t pb, uint16_t bpm_b, uint16_t sb, ui_next_sync_t *out)
{
    out->valid = false;
    out->beat_ms = out->phase_beats = out->tempo_pct = 0.0f;
    out->phrase_beats = 0;
    float ba, bb;
    if (!ui_next_grid_bar_pos(a, pa, &ba) || !ui_next_grid_bar_pos(b, pb, &bb)) return;
    const float spa = (float)(sa ? sa : 1000u) / 1000.0f, spb = (float)(sb ? sb : 1000u) / 1000.0f;

    /* B is late when A sits on a beat and the next beat of B has not arrived yet. */
    float late = frac_beat(ba) - frac_beat(bb);
    if (late > 0.5f) late -= 1.0f;
    if (late < -0.5f) late += 1.0f;
    const int ib = ui_next_grid_nearest(b, pb);
    const float iv_b = ib >= 0 ? interval_ms(b, ib) : 0.0f;
    out->phase_beats = late;
    out->beat_ms = iv_b > 0.0f ? late * iv_b / spb : 0.0f;
    out->phrase_beats = (uint8_t)(((int)lroundf(bb) - (int)lroundf(ba) + 8) % 4);

    const float eff_a = (float)bpm_a * spa, eff_b = (float)bpm_b * spb;
    out->tempo_pct = eff_a > 0.0f ? (eff_b / eff_a - 1.0f) * 100.0f : 0.0f;
    out->valid = true;
}

float ui_next_env_sample(const ui_next_env_t *env, int band, uint32_t duration_ms, int64_t t)
{
    if (!env || duration_ms == 0) return 0.0f;
    const float *arr = band == 0 ? env->lo : band == 1 ? env->mi : env->hi;
    float pos = (float)t * (float)UI_NEXT_ENV_COLUMNS / (float)duration_ms;
    if (pos < 0.0f) pos = 0.0f;
    const float last = (float)(UI_NEXT_ENV_COLUMNS - 1u);
    if (pos > last) pos = last;
    const uint32_t i0 = (uint32_t)pos, i1 = i0 + 1u < UI_NEXT_ENV_COLUMNS ? i0 + 1u : i0;
    const float f = pos - (float)i0;
    return arr[i0] + (arr[i1] - arr[i0]) * f;
}

void ui_next_sync_overlap(const ui_next_env_t *ea, uint32_t da, int64_t pa, const ui_next_env_t *eb, uint32_t db,
                          int64_t pb, uint32_t window_ms, uint8_t *bass, uint8_t *overlap)
{
    enum { N = 180 };
    int clash = 0, mid = 0;
    for (int i = 0; i < N; i++) {
        const int64_t off = (int64_t)window_ms * i / N - (int64_t)window_ms / 2;
        const float la = ui_next_env_sample(ea, 0, da, pa + off), lb = ui_next_env_sample(eb, 0, db, pb + off);
        const float ma = ui_next_env_sample(ea, 1, da, pa + off), mb = ui_next_env_sample(eb, 1, db, pb + off);
        if (fminf(la, lb) > .30f) clash++;
        if (fminf(ma, mb) > .16f) mid++;
    }
    int bp = (int)lroundf(100.0f * (float)clash / N), op = (int)lroundf(100.0f * (float)mid / N);
    if (bp < 4) bp = 4;
    if (op < 8) op = 8;
    *bass = (uint8_t)bp;
    *overlap = (uint8_t)op;
}

int ui_next_sync_screen_beats(const ui_next_grid_t *g, int64_t pos, uint32_t window_ms, ui_next_screen_beat_t *out, int max)
{
    if (!g || !g->beat || g->count == 0 || window_ms == 0) return 0;
    const int64_t t0 = pos - (int64_t)window_ms / 2, t1 = pos + (int64_t)window_ms / 2;
    int i = ui_next_grid_floor(g, t0);
    if (i < 0) i = 0;
    int n = 0;
    for (; i < (int)g->count && n < max; i++) {
        const int64_t t = (int64_t)g->beat[i].time_ms;
        if (t < t0) continue;
        if (t > t1) break;
        out[n].x01 = (float)(t - t0) / (float)window_ms;
        out[n].bar = beat_in_bar(g, i) == 0;
        out[n].index = i;
        n++;
    }
    return n;
}

static int nearest_x(const ui_next_screen_beat_t *b, int nb, float x, bool bars_only)
{
    int best = -1;
    float bd = 1e9f;
    for (int i = 0; i < nb; i++) {
        if (bars_only && !b[i].bar) continue;
        const float d = fabsf(b[i].x01 - x);
        if (d < bd) {
            bd = d;
            best = i;
        }
    }
    return best;
}

int ui_next_sync_weave(const ui_next_screen_beat_t *a, int na, const ui_next_screen_beat_t *b, int nb, float max_dx,
                       ui_next_weave_t *out, int max)
{
    int n = 0;
    for (int i = 0; i < na && n < max; i++) {
        const int j = nearest_x(b, nb, a[i].x01, false);
        if (j < 0 || fabsf(b[j].x01 - a[i].x01) > max_dx) continue;
        out[n].xa = a[i].x01;
        out[n].xb = b[j].x01;
        out[n].bar = a[i].bar;
        out[n].has_bar = false;
        out[n].xbar = 0.0f;
        if (a[i].bar) {
            const int k = nearest_x(b, nb, a[i].x01, true);
            if (k >= 0) {
                out[n].has_bar = true;
                out[n].xbar = b[k].x01;
            }
        }
        n++;
    }
    return n;
}
