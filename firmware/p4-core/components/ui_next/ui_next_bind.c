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

/* Copies `s` into `d` and cuts on a character boundary, so a long title never ends on half an accent. */
static void copy_utf8(char *d, size_t cap, const char *s)
{
    size_t n = strlen(s);
    if (n >= cap) {
        n = cap - 1;
        while (n && ((unsigned char)s[n] & 0xC0) == 0x80) n--;
    }
    memcpy(d, s, n);
    d[n] = 0;
}

size_t ui_next_bind_browse(const ui_next_browse_in_t *in, ui_next_browse_model_t *out)
{
    const size_t total = ui_next_view_build(in->entry, in->n, &in->filter, in->sort, in->strip_accents, in->view,
                                            in->scratch, in->cap);
    const size_t shown = total < in->cap ? total : in->cap;

    copy_utf8(out->crumb, sizeof out->crumb, in->crumb);
    snprintf(out->count, sizeof out->count, "%u OF %u", (unsigned)total, (unsigned)in->n);
    if (in->match_deck && in->match_key && in->match_key[0])
        snprintf(out->match, sizeof out->match, "MATCH TO DECK %c %s", in->match_deck, in->match_key);
    else
        out->match[0] = 0;
    out->category = in->category;

    out->tag_badge = 0;
    for (int i = 0; i < UI_NEXT_BROWSE_TAGS; i++) {
        out->tags[i] = in->tag[i];
        out->tags[i].on = (in->filter.tag_mask >> i) & 1u;
        out->tag_badge += out->tags[i].on;
    }

    for (int r = 0; r < UI_NEXT_BROWSE_TRACKS; r++) {
        ui_next_track_t *t = &out->track[r];
        memset(t, 0, sizeof *t);
        const size_t pos = in->first + (size_t)r;
        if (pos >= shown) continue;
        const ui_next_entry_t *e = &in->entry[in->view[pos]];
        copy_utf8(t->title, sizeof t->title, e->title);
        copy_utf8(t->artist, sizeof t->artist, e->artist);
        copy_utf8(t->genre, sizeof t->genre, e->genre);
        if (e->bpm_x100) snprintf(t->bpm, sizeof t->bpm, "%u", (unsigned)((e->bpm_x100 + 50u) / 100u));
        snprintf(t->key, sizeof t->key, "%s", e->key);
        ui_next_fmt_clock(t->time, sizeof t->time, e->duration_s * 1000u);
        t->key_class = in->match_key ? ui_next_bind_key_class(in->match_key, e->key) : UI_NEXT_KEY_NONE;
        for (int g = 0; g < UI_NEXT_BROWSE_TAGS && t->n_tags < 3; g++)
            if ((e->tags >> g) & 1u) t->tags[t->n_tags++] = (uint8_t)g;
        t->env = (uint8_t)(e->id % 4u);
    }
    out->open_row = -1;
    out->lift_row = -1;
    out->ins_row = -1;
    return total;
}

size_t ui_next_bind_search(const ui_next_entry_t *entry, size_t n, const char *query, bool strip_accents,
                           const char *ref_key, ui_next_search_model_t *out)
{
    ui_next_hit_t hit[UI_NEXT_SEARCH_RESULTS];
    const size_t found = ui_next_search(entry, n, query, strip_accents, hit, UI_NEXT_SEARCH_RESULTS);
    const size_t shown = found < UI_NEXT_SEARCH_RESULTS ? found : UI_NEXT_SEARCH_RESULTS;
    (void)ref_key;

    copy_utf8(out->query, sizeof out->query, query);
    if (found) snprintf(out->status, sizeof out->status, "%s%u RESULTS", strip_accents ? "ACCENTS IGNORED / " : "", (unsigned)found);
    else snprintf(out->status, sizeof out->status, "%s", query[0] ? "NO RESULT" : "");
    for (size_t i = 0; i < UI_NEXT_SEARCH_RESULTS; i++) {
        ui_next_search_result_t *r = &out->result[i];
        memset(r, 0, sizeof *r);
        if (i >= shown) continue;
        const ui_next_entry_t *e = &entry[hit[i].index];
        char tmp[48];
        copy_utf8(r->pre, sizeof r->pre, (snprintf(tmp, sizeof tmp, "%.*s", hit[i].start, e->title), tmp));
        copy_utf8(r->hit, sizeof r->hit, (snprintf(tmp, sizeof tmp, "%.*s", hit[i].end - hit[i].start, e->title + hit[i].start), tmp));
        copy_utf8(r->post, sizeof r->post, e->title + hit[i].end);
        copy_utf8(r->artist, sizeof r->artist, e->artist);
        if (e->bpm_x100) snprintf(r->bpm, sizeof r->bpm, "%u", (unsigned)((e->bpm_x100 + 50u) / 100u));
        snprintf(r->key, sizeof r->key, "%s", e->key);
    }
    out->n = (uint8_t)shown;
    out->selected = shown ? 0 : -1;
    return found;
}
