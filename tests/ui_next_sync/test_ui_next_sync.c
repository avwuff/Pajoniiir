#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "ui_next_sync.h"

#define N 200

/* Constant grid: beat k at first + k * interval, phase cycling 1..4. */
static ui_next_beat_t GA[N], GB[N];

static void fill(ui_next_beat_t *g, uint32_t first, float interval)
{
    for (int k = 0; k < N; k++) {
        g[k].time_ms = first + (uint32_t)lroundf((float)k * interval);
        g[k].phase = (uint8_t)(k % 4 + 1);
    }
}

static int near(float a, float b, float tol) { return fabsf(a - b) <= tol; }

static void test_grid_lookup(void)
{
    fill(GA, 1000, 500.0f);
    ui_next_grid_t g = {GA, N};
    assert(ui_next_grid_floor(&g, 999) == -1);
    assert(ui_next_grid_floor(&g, 1000) == 0);
    assert(ui_next_grid_floor(&g, 1499) == 0);
    assert(ui_next_grid_floor(&g, 1500) == 1);
    assert(ui_next_grid_nearest(&g, 1240) == 0);
    assert(ui_next_grid_nearest(&g, 1260) == 1);
    assert(ui_next_grid_nearest(&g, 0) == 0);
    assert(ui_next_grid_nearest(&g, 999999) == N - 1);
    ui_next_grid_t empty = {NULL, 0};
    assert(ui_next_grid_nearest(&empty, 5) == -1);
}

static void test_bar_position(void)
{
    fill(GA, 1000, 500.0f);
    ui_next_grid_t g = {GA, N};
    float p;
    assert(ui_next_grid_bar_pos(&g, 1000, &p) && near(p, 0.0f, 0.001f));
    assert(ui_next_grid_bar_pos(&g, 1250, &p) && near(p, 0.5f, 0.001f));
    assert(ui_next_grid_bar_pos(&g, 2750, &p) && near(p, 3.5f, 0.001f));
    assert(ui_next_grid_bar_pos(&g, 3000, &p) && near(p, 0.0f, 0.001f)); /* next bar */
    assert(ui_next_grid_bar_pos(&g, 750, &p) && near(p, 3.5f, 0.001f));  /* before the first beat */
    ui_next_grid_t one = {GA, 1};
    assert(!ui_next_grid_bar_pos(&one, 1000, &p));
}

static void test_in_phase_decks_read_zero(void)
{
    fill(GA, 1000, 500.0f);
    fill(GB, 1000, 500.0f);
    ui_next_grid_t a = {GA, N}, b = {GB, N};
    ui_next_sync_t s;
    ui_next_sync_compute(&a, 5000, 12000, 1000, &b, 5000, 12000, 1000, &s);
    assert(s.valid && near(s.phase_beats, 0.0f, 0.001f) && near(s.beat_ms, 0.0f, 0.5f));
    assert(s.phrase_beats == 0 && near(s.tempo_pct, 0.0f, 0.001f));
}

static void test_b_late_is_positive(void)
{
    fill(GA, 1000, 500.0f);
    fill(GB, 1000, 500.0f);
    ui_next_grid_t a = {GA, N}, b = {GB, N};
    ui_next_sync_t s;
    /* A on a beat; B 100 ms before its next beat: its beat arrives late. */
    ui_next_sync_compute(&a, 5000, 12000, 1000, &b, 4900, 12000, 1000, &s);
    assert(near(s.phase_beats, 0.2f, 0.005f) && near(s.beat_ms, 100.0f, 1.0f));
    /* B 100 ms past its beat: early. */
    ui_next_sync_compute(&a, 5000, 12000, 1000, &b, 5100, 12000, 1000, &s);
    assert(near(s.phase_beats, -0.2f, 0.005f) && near(s.beat_ms, -100.0f, 1.0f));
}

static void test_phrase_offset_and_wrap(void)
{
    fill(GA, 1000, 500.0f);
    fill(GB, 1000, 500.0f);
    ui_next_grid_t a = {GA, N}, b = {GB, N};
    ui_next_sync_t s;
    ui_next_sync_compute(&a, 5000, 12000, 1000, &b, 5000 + 500, 12000, 1000, &s);
    assert(s.phrase_beats == 1 && near(s.phase_beats, 0.0f, 0.001f));
    ui_next_sync_compute(&a, 5000 + 1500, 12000, 1000, &b, 5000, 12000, 1000, &s);
    assert(s.phrase_beats == 1); /* B is 3 beats behind, which is 1 ahead modulo the bar */
    ui_next_sync_compute(&a, 5000, 12000, 1000, &b, 5000 + 2000, 12000, 1000, &s);
    assert(s.phrase_beats == 0); /* a whole bar apart */
}

static void test_tempo_and_speed(void)
{
    fill(GA, 1000, 500.0f);
    fill(GB, 1000, 500.0f);
    ui_next_grid_t a = {GA, N}, b = {GB, N};
    ui_next_sync_t s;
    ui_next_sync_compute(&a, 5000, 12400, 1000, &b, 5000, 11900, 1000, &s);
    assert(near(s.tempo_pct, -4.03f, 0.05f));
    /* Same track BPM, B played 4 % slower. */
    ui_next_sync_compute(&a, 5000, 12400, 1000, &b, 5000, 12400, 960, &s);
    assert(near(s.tempo_pct, -4.0f, 0.01f));
    /* Real time error scales with the speed: 100 ms of track time at 0.5x is 200 ms. */
    ui_next_sync_compute(&a, 5000, 12000, 1000, &b, 4900, 12000, 500, &s);
    assert(near(s.beat_ms, 200.0f, 1.0f));
    ui_next_grid_t none = {NULL, 0};
    ui_next_sync_compute(&none, 5000, 12000, 1000, &b, 5000, 12000, 1000, &s);
    assert(!s.valid);
}

static ui_next_env_t EA, EB;

static void test_overlap_follows_the_energy(void)
{
    ui_next_env_demo(&EA, 11);
    ui_next_env_demo(&EB, 23);
    uint8_t bass, over;
    /* Same place in both tracks, in the loud middle section. */
    ui_next_sync_overlap(&EA, 64000, 40000, &EB, 64000, 40000, 8000, &bass, &over);
    assert(bass >= 4 && bass <= 100 && over >= 8 && over <= 100);
    uint8_t bass2, over2;
    /* Break at 50 to 60 % of the demo: little low-band energy in both. */
    ui_next_sync_overlap(&EA, 64000, 35000, &EB, 64000, 35000, 3000, &bass2, &over2);
    assert(bass2 <= bass);
    /* Floors: never below 4 and 8 percent. */
    ui_next_sync_overlap(&EA, 64000, 100, &EB, 64000, 100, 200, &bass2, &over2);
    assert(bass2 >= 4 && over2 >= 8);
    /* Sampling clamps at both ends of the track. */
    assert(ui_next_env_sample(&EA, 0, 64000, -5000) == EA.lo[0]);
    assert(ui_next_env_sample(&EA, 0, 64000, 999999) == EA.lo[UI_NEXT_ENV_COLUMNS - 1]);
}

static void test_screen_beats_and_weave(void)
{
    fill(GA, 1000, 500.0f);
    fill(GB, 1000, 520.0f); /* B runs a little slower: its beats drift */
    ui_next_grid_t a = {GA, N}, b = {GB, N};
    ui_next_screen_beat_t sa[64], sb[64];
    const uint32_t win = 8000;
    int na = ui_next_sync_screen_beats(&a, 20000, win, sa, 64);
    int nb = ui_next_sync_screen_beats(&b, 20000, win, sb, 64);
    assert(na == 16 || na == 17);
    assert(nb >= 14 && nb <= 16);
    for (int i = 0; i < na; i++) assert(sa[i].x01 >= 0.0f && sa[i].x01 <= 1.0f);
    for (int i = 1; i < na; i++) assert(near(sa[i].x01 - sa[i - 1].x01, 500.0f / win, 0.002f));
    int bars = 0;
    for (int i = 0; i < na; i++) bars += sa[i].bar;
    assert(bars == 4);

    ui_next_weave_t w[64];
    int nw = ui_next_sync_weave(sa, na, sb, nb, 0.5f * 520.0f / win, w, 64);
    assert(nw > 10);
    for (int i = 0; i < nw; i++) {
        assert(fabsf(w[i].xa - w[i].xb) <= 0.5f * 520.0f / win + 0.001f);
        if (w[i].bar) assert(w[i].has_bar);
        else assert(!w[i].has_bar);
    }
    /* A distance limit drops pairs. */
    assert(ui_next_sync_weave(sa, na, sb, nb, 0.0f, w, 64) < nw);
    assert(ui_next_sync_screen_beats(&a, 20000, 0, sa, 64) == 0);
}

int main(void)
{
    test_grid_lookup();
    test_bar_position();
    test_in_phase_decks_read_zero();
    test_b_late_is_positive();
    test_phrase_offset_and_wrap();
    test_tempo_and_speed();
    test_overlap_follows_the_energy();
    test_screen_beats_and_weave();
    puts("ui_next_sync: ok");
    return 0;
}
