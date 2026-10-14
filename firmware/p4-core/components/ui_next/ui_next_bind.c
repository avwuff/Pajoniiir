#include "ui_next_bind.h"

#include <stdio.h>
#include <string.h>

#include "ui_next_format.h"
#include "ui_next_key.h"
#include "ui_next_sync.h"

ui_next_key_class_t ui_next_bind_key_class(const char *ref, const char *key)
{
    switch (ui_next_key_relation_text(ref, key)) {
    case UI_NEXT_REL_FIT: return UI_NEXT_KEY_FIT;
    case UI_NEXT_REL_PLUS2: return UI_NEXT_KEY_NEAR;
    case UI_NEXT_REL_PLUS7: return UI_NEXT_KEY_FAR;
    default: return UI_NEXT_KEY_NONE;
    }
}

static void fill_deck(ui_next_deck_t *o, const ui_next_deck_in_t *d, bool fit, bool time_remain)
{
    o->env = d->env;
    o->duration_ms = d->duration_ms;
    o->pos_ms = d->pos_ms;
    o->grid = d->grid;
    snprintf(o->title, sizeof o->title, "%s", d->title);
    ui_next_fmt_bpm(o->bpm, sizeof o->bpm, d->bpm_x100, d->speed_permille);
    snprintf(o->key, sizeof o->key, "%.5s", d->key);
    o->key_fit = fit;
    if (d->master) {
        snprintf(o->badge, sizeof o->badge, "MASTER");
        o->badge_on = true;
    } else if (d->sync) {
        snprintf(o->badge, sizeof o->badge, "SYNC");
        o->badge_on = true;
    } else {
        snprintf(o->badge, sizeof o->badge, "SYNC OFF");
        o->badge_on = false;
    }
    if (time_remain) ui_next_fmt_remaining(o->time, sizeof o->time, d->pos_ms, d->duration_ms);
    else ui_next_fmt_clock(o->time, sizeof o->time, d->pos_ms);
    o->pos01 = d->duration_ms ? (float)d->pos_ms / (float)d->duration_ms : 0.0f;
    o->vu_l = o->vu_r = ui_next_level01(d->peak, -36.0f);
    o->peak_l = o->peak_r = o->vu_l * 0.99f;
}

void ui_next_bind_perform(const ui_next_perform_in_t *in, ui_next_perform_model_t *m)
{
    const ui_next_deck_in_t *a = &in->deck[0], *b = &in->deck[1];
    const bool fit = ui_next_key_text_fit(a->key, b->key);
    fill_deck(&m->deck[0], a, fit, in->time_remain);
    fill_deck(&m->deck[1], b, fit, in->time_remain);
    m->window_ms = in->window_ms;

    ui_next_sync_t s;
    ui_next_sync_compute(&a->grid, (int64_t)a->pos_ms, a->bpm_x100, a->speed_permille, &b->grid, (int64_t)b->pos_ms,
                         b->bpm_x100, b->speed_permille, &s);
    m->link.beat_ms = s.beat_ms;
    m->link.phase_beat = s.phase_beats;
    m->link.phrase_off = s.phrase_beats;
    m->link.key_fit = fit;
    m->tempo_pct = s.tempo_pct;
    if (a->env && b->env && in->window_ms)
        ui_next_sync_overlap(a->env, a->duration_ms, (int64_t)a->pos_ms, b->env, b->duration_ms, (int64_t)b->pos_ms,
                             in->window_ms, &m->link.bass_pct, &m->link.overlap_pct);
    else
        m->link.bass_pct = m->link.overlap_pct = 0;
    snprintf(m->key_change, sizeof m->key_change, "%.4s>%.4s", a->key, b->key);
    m->layers = in->cls == UI_NEXT_CLASS_S ? "LAYERS 1-4" : in->cls == UI_NEXT_CLASS_M ? "LAYERS 1-6" : "LAYERS 1-7";
}
