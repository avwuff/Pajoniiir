#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "ui_next_view.h"

#define E_ACUTE "\xC3\xA9"

static ui_next_entry_t lib[6];
static uint16_t view[6], scratch[6];

static void set(int i, const char *title, const char *artist, const char *genre, uint16_t bpm, const char *key,
                uint32_t dur, uint16_t tags)
{
    ui_next_entry_t *e = &lib[i];
    memset(e, 0, sizeof *e);
    e->id = (uint16_t)(100 + i);
    snprintf(e->title, sizeof e->title, "%s", title);
    snprintf(e->artist, sizeof e->artist, "%s", artist);
    snprintf(e->genre, sizeof e->genre, "%s", genre);
    e->bpm_x100 = bpm;
    snprintf(e->key, sizeof e->key, "%s", key);
    e->duration_s = dur;
    e->tags = tags;
}

static void fill(void)
{
    set(0, "Zebra", "Nova", "House", 12400, "8A", 300, 0x1);
    set(1, E_ACUTE "t" E_ACUTE, "Nova", "House", 12400, "9A", 240, 0x2);
    set(2, "Eclair", "Kilo", "Techno", 13200, "", 360, 0x3);
    set(3, "Alpha", "Kilo", "techno", 12600, "8B", 200, 0x0);
    set(4, "Beta", "Nova", "House", 0, "3A", 180, 0x1);
    set(5, "Gamma", "Echo", "Deep", 12000, "12B", 420, 0x4);
}

static const ui_next_filter_t NONE = {0};

static void test_sort_title_ignores_accents(void)
{
    ui_next_sort_t s = {UI_NEXT_COL_TITLE, false};
    assert(ui_next_view_build(lib, 6, &NONE, s, true, view, scratch, 6) == 6);
    const uint16_t want[] = {3, 4, 2, 1, 5, 0}; /* Alpha Beta Eclair Ete Gamma Zebra */
    assert(!memcmp(view, want, sizeof want));
}

static void test_sort_is_stable_and_reverses(void)
{
    ui_next_sort_t s = {UI_NEXT_COL_BPM, false};
    ui_next_view_build(lib, 6, &NONE, s, true, view, scratch, 6);
    const uint16_t up[] = {4, 5, 0, 1, 3, 2}; /* 0, 120, 124 (0 then 1 by index), 126, 132 */
    assert(!memcmp(view, up, sizeof up));
    ui_next_sort_tap(&s, UI_NEXT_COL_BPM);
    assert(s.desc);
    ui_next_view_build(lib, 6, &NONE, s, true, view, scratch, 6);
    const uint16_t down[] = {2, 3, 0, 1, 5, 4}; /* the 124 pair still in index order */
    assert(!memcmp(view, down, sizeof down));
    ui_next_sort_tap(&s, UI_NEXT_COL_TITLE);
    assert(s.col == UI_NEXT_COL_TITLE && !s.desc);
}

static void test_key_sort_puts_unknown_last(void)
{
    ui_next_sort_t s = {UI_NEXT_COL_KEY, false};
    ui_next_view_build(lib, 6, &NONE, s, true, view, scratch, 6);
    assert(view[0] == 4 && view[5] == 2); /* 3A first, no key last */
    s.desc = true;
    ui_next_view_build(lib, 6, &NONE, s, true, view, scratch, 6);
    assert(view[0] == 5 && view[5] == 2); /* 12B first, still no key last */
}

static void test_filters(void)
{
    ui_next_sort_t s = {UI_NEXT_COL_TITLE, false};
    ui_next_filter_t f = {0};
    f.bpm_on = true;
    f.bpm_ref_x100 = 12400;
    f.bpm_pct = 4; /* 119.04 .. 128.96 */
    assert(ui_next_view_build(lib, 6, &f, s, true, view, scratch, 6) == 4); /* not 0 BPM, not 132 */
    f.key_on = true;
    strcpy(f.key_ref, "8A"); /* fits 8A, 9A, 7A, 8B */
    assert(ui_next_view_build(lib, 6, &f, s, true, view, scratch, 6) == 3);
    memset(&f, 0, sizeof f);
    strcpy(f.genre, "TECHNO"); /* case does not matter */
    assert(ui_next_view_build(lib, 6, &f, s, true, view, scratch, 6) == 2);
    memset(&f, 0, sizeof f);
    f.tag_mask = 0x3;
    assert(ui_next_view_build(lib, 6, &f, s, true, view, scratch, 6) == 4); /* ANY */
    f.tag_all = true;
    assert(ui_next_view_build(lib, 6, &f, s, true, view, scratch, 6) == 1); /* ALL: only Eclair */
    assert(view[0] == 2);
}

static void test_count_beyond_cap(void)
{
    ui_next_sort_t s = {UI_NEXT_COL_TITLE, false};
    assert(ui_next_view_build(lib, 6, &NONE, s, true, view, scratch, 3) == 6);
    assert(view[0] == 2 && view[1] == 1 && view[2] == 0); /* first 3 that pass, sorted */
}

static void test_groups(void)
{
    uint16_t first[6];
    ui_next_sort_t s = {UI_NEXT_COL_ARTIST, false};
    ui_next_view_build(lib, 6, &NONE, s, true, view, scratch, 6);
    assert(ui_next_view_groups(lib, view, 6, UI_NEXT_COL_ARTIST, true, first, 6) == 3); /* Echo Kilo Nova */
    assert(first[0] == 0 && first[1] == 1 && first[2] == 3);
    s.col = UI_NEXT_COL_GENRE;
    ui_next_view_build(lib, 6, &NONE, s, true, view, scratch, 6);
    assert(ui_next_view_groups(lib, view, 6, UI_NEXT_COL_GENRE, true, first, 6) == 3); /* techno twice, one group */
}

static void test_queue(void)
{
    ui_next_queue_t q = {0};
    uint16_t id;
    assert(ui_next_queue_add(&q, 7) && ui_next_queue_add(&q, 8) && ui_next_queue_add(&q, 9));
    assert(!ui_next_queue_add(&q, 8)); /* once only */
    assert(ui_next_queue_move(&q, 0, 2) && q.id[0] == 8 && q.id[2] == 7);
    assert(ui_next_queue_move(&q, 2, 0) && q.id[0] == 7 && q.id[1] == 8 && q.id[2] == 9);
    assert(ui_next_queue_remove(&q, 1) && q.n == 2 && q.id[1] == 9);
    assert(!ui_next_queue_remove(&q, 5));
    assert(ui_next_queue_pop(&q, &id) && id == 7 && q.n == 1);
    assert(ui_next_queue_pop(&q, &id) && id == 9 && !ui_next_queue_pop(&q, &id));
    for (int i = 0; i < UI_NEXT_QUEUE_MAX; i++) assert(ui_next_queue_add(&q, (uint16_t)i));
    assert(!ui_next_queue_add(&q, 99));
}

static void test_search(void)
{
    ui_next_hit_t h[6];
    assert(ui_next_search(lib, 6, "ETE", true, h, 6) == 1); /* only the accented title */
    assert(h[0].index == 1 && h[0].start == 0 && h[0].end == 5); /* two accented letters, 5 bytes */
    assert(ui_next_search(lib, 6, "ete", false, h, 6) == 0); /* accents kept: no match */
    assert(ui_next_search(lib, 6, "", true, h, 6) == 0);
    assert(ui_next_search(lib, 6, "a", true, h, 2) == 5 && h[0].index == 3); /* starts first, count past cap */
}

int main(void)
{
    fill();
    test_sort_title_ignores_accents();
    test_sort_is_stable_and_reverses();
    test_key_sort_puts_unknown_last();
    test_filters();
    test_count_beyond_cap();
    test_groups();
    test_queue();
    test_search();
    puts("ui_next_view: ok");
    return 0;
}
