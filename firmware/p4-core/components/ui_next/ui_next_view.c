#include <string.h>

#include "ui_next_key.h"
#include "ui_next_text.h"
#include "ui_next_view.h"

void ui_next_sort_tap(ui_next_sort_t *s, ui_next_col_id_t col)
{
    if (s->col == col) {
        s->desc = !s->desc;
    } else {
        s->col = col;
        s->desc = false;
    }
}

bool ui_next_filter_pass(const ui_next_filter_t *f, const ui_next_entry_t *e)
{
    if (f->bpm_on) {
        if (!e->bpm_x100 || !f->bpm_ref_x100) return false;
        const int32_t d = (int32_t)e->bpm_x100 - (int32_t)f->bpm_ref_x100;
        if ((d < 0 ? -d : d) * 100 > (int32_t)f->bpm_ref_x100 * f->bpm_pct) return false;
    }
    if (f->key_on && !ui_next_key_text_fit(f->key_ref, e->key)) return false;
    if (f->genre[0] && ui_next_text_cmp(f->genre, e->genre, true) != 0) return false;
    if (f->tag_mask) {
        const uint16_t hit = e->tags & f->tag_mask;
        if (f->tag_all ? hit != f->tag_mask : hit == 0) return false;
    }
    return true;
}

/* Order of two entries on a column, ascending. Ties go to the caller. */
static int cmp_col(const ui_next_entry_t *a, const ui_next_entry_t *b, ui_next_col_id_t col, bool strip)
{
    switch (col) {
    case UI_NEXT_COL_TITLE: return ui_next_text_cmp(a->title, b->title, strip);
    case UI_NEXT_COL_ARTIST: return ui_next_text_cmp(a->artist, b->artist, strip);
    case UI_NEXT_COL_GENRE: return ui_next_text_cmp(a->genre, b->genre, strip);
    case UI_NEXT_COL_BPM: return (a->bpm_x100 > b->bpm_x100) - (a->bpm_x100 < b->bpm_x100);
    case UI_NEXT_COL_TIME: return (a->duration_s > b->duration_s) - (a->duration_s < b->duration_s);
    case UI_NEXT_COL_KEY: {
        /* Camelot order; an unknown key goes last whatever the direction of the sort. */
        ui_next_camelot_t ka, kb;
        const bool pa = ui_next_key_parse(a->key, &ka), pb = ui_next_key_parse(b->key, &kb);
        if (pa != pb) return pa ? -1 : 1;
        if (!pa) return 0;
        const int va = ka.number * 2 + (ka.letter == 'B'), vb = kb.number * 2 + (kb.letter == 'B');
        return (va > vb) - (va < vb);
    }
    }
    return 0;
}

static bool key_unknown(const ui_next_entry_t *e)
{
    ui_next_camelot_t k;
    return !ui_next_key_parse(e->key, &k);
}

/* True when index `x` goes after index `y` in the view. */
static bool after(const ui_next_entry_t *e, uint16_t x, uint16_t y, ui_next_sort_t s, bool strip)
{
    int c = cmp_col(&e[x], &e[y], s.col, strip);
    if (c != 0) {
        const bool pinned = s.col == UI_NEXT_COL_KEY && (key_unknown(&e[x]) || key_unknown(&e[y]));
        if (s.desc && !pinned) c = -c;
        return c > 0;
    }
    return x > y;
}

size_t ui_next_view_build(const ui_next_entry_t *e, size_t n, const ui_next_filter_t *f, ui_next_sort_t sort,
                          bool strip_accents, uint16_t *out, uint16_t *scratch, size_t cap)
{
    size_t count = 0;
    for (size_t i = 0; i < n; i++) {
        if (!ui_next_filter_pass(f, &e[i])) continue;
        if (count < cap) out[count] = (uint16_t)i;
        count++;
    }
    const size_t m = count < cap ? count : cap;

    /* Bottom-up merge sort on the indices. */
    uint16_t *src = out, *dst = scratch;
    for (size_t w = 1; w < m; w *= 2) {
        for (size_t lo = 0; lo < m; lo += 2 * w) {
            const size_t mid = lo + w < m ? lo + w : m, hi = lo + 2 * w < m ? lo + 2 * w : m;
            size_t a = lo, b = mid, k = lo;
            while (a < mid && b < hi)
                dst[k++] = after(e, src[a], src[b], sort, strip_accents) ? src[b++] : src[a++];
            while (a < mid) dst[k++] = src[a++];
            while (b < hi) dst[k++] = src[b++];
        }
        uint16_t *t = src;
        src = dst;
        dst = t;
    }
    if (src != out && m) memcpy(out, src, m * sizeof *out);
    return count;
}

size_t ui_next_view_groups(const ui_next_entry_t *e, const uint16_t *view, size_t n, ui_next_col_id_t col,
                           bool strip_accents, uint16_t *first, size_t cap)
{
    size_t groups = 0;
    for (size_t i = 0; i < n; i++) {
        if (i && cmp_col(&e[view[i - 1]], &e[view[i]], col, strip_accents) == 0) continue;
        if (groups < cap) first[groups] = (uint16_t)i;
        groups++;
    }
    return groups;
}

bool ui_next_queue_add(ui_next_queue_t *q, uint16_t id)
{
    if (q->n >= UI_NEXT_QUEUE_MAX) return false;
    for (uint8_t i = 0; i < q->n; i++)
        if (q->id[i] == id) return false;
    q->id[q->n++] = id;
    return true;
}

bool ui_next_queue_remove(ui_next_queue_t *q, uint8_t pos)
{
    if (pos >= q->n) return false;
    memmove(&q->id[pos], &q->id[pos + 1], (size_t)(q->n - pos - 1) * sizeof q->id[0]);
    q->n--;
    return true;
}

bool ui_next_queue_pop(ui_next_queue_t *q, uint16_t *id)
{
    if (!q->n) return false;
    if (id) *id = q->id[0];
    return ui_next_queue_remove(q, 0);
}

bool ui_next_queue_move(ui_next_queue_t *q, uint8_t from, uint8_t to)
{
    if (from >= q->n || to >= q->n) return false;
    const uint16_t id = q->id[from];
    if (from < to) memmove(&q->id[from], &q->id[from + 1], (size_t)(to - from) * sizeof q->id[0]);
    else memmove(&q->id[to + 1], &q->id[to], (size_t)(from - to) * sizeof q->id[0]);
    q->id[to] = id;
    return true;
}

size_t ui_next_search(const ui_next_entry_t *e, size_t n, const char *query, bool strip_accents,
                      ui_next_hit_t *out, size_t cap)
{
    char q[48], t[128];
    uint16_t os[128], oe[128];
    const size_t ql = ui_next_fold(query, strip_accents, q, sizeof q, NULL, NULL);
    if (!ql) return 0;

    size_t found = 0, kept = 0;
    /* Two passes keep the order stable: title starts first, then matches inside a title. */
    for (int pass = 0; pass < 2; pass++) {
        for (size_t i = 0; i < n; i++) {
            const size_t tl = ui_next_fold(e[i].title, strip_accents, t, sizeof t, os, oe);
            const char *at = strstr(t, q);
            if (!at || (size_t)(at - t) + ql > tl) continue;
            const size_t p = (size_t)(at - t);
            if ((p == 0) != (pass == 0)) continue;
            found++;
            if (kept < cap) {
                out[kept].index = (uint16_t)i;
                out[kept].start = (uint8_t)os[p];
                out[kept].end = (uint8_t)oe[p + ql - 1];
                kept++;
            }
        }
    }
    return found;
}
