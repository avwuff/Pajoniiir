#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "ui_next_key.h"

static void test_camelot_parse(void)
{
    ui_next_camelot_t k;
    assert(ui_next_key_parse("8A", &k) && k.number == 8 && k.letter == 'A');
    assert(ui_next_key_parse("08a", &k) && k.number == 8 && k.letter == 'A');
    assert(ui_next_key_parse("12B", &k) && k.number == 12 && k.letter == 'B');
    assert(!ui_next_key_parse("13A", &k));
    assert(!ui_next_key_parse("0A", &k));
    assert(!ui_next_key_parse("8C", &k));
    assert(!ui_next_key_parse("8AA", &k));
    assert(!ui_next_key_parse("", &k));
    assert(!ui_next_key_parse(NULL, &k));
}

static void test_classical_names_map_to_camelot(void)
{
    ui_next_camelot_t k;
    /* Well known pairs of the wheel. */
    assert(ui_next_key_parse("Am", &k) && k.number == 8 && k.letter == 'A');
    assert(ui_next_key_parse("C", &k) && k.number == 8 && k.letter == 'B');
    assert(ui_next_key_parse("Em", &k) && k.number == 9 && k.letter == 'A');
    assert(ui_next_key_parse("G", &k) && k.number == 9 && k.letter == 'B');
    assert(ui_next_key_parse("F#m", &k) && k.number == 11 && k.letter == 'A');
    assert(ui_next_key_parse("Bb", &k) && k.number == 6 && k.letter == 'B');
    assert(ui_next_key_parse("Dbm", &k) && k.number == 12 && k.letter == 'A');
    assert(ui_next_key_parse("Cmin", &k) && k.number == 5 && k.letter == 'A');
    assert(ui_next_key_parse("Fmaj", &k) && k.number == 7 && k.letter == 'B');
    assert(!ui_next_key_parse("H", &k));
}

/* The rows of the Browse board against deck B at 9A: colours must match the board. */
static void test_browse_board_colours(void)
{
    static const struct { const char *key; ui_next_key_rel_t rel; } ROW[] = {
        {"9A", UI_NEXT_REL_FIT},   {"8A", UI_NEXT_REL_FIT},   {"10A", UI_NEXT_REL_FIT},
        {"11A", UI_NEXT_REL_PLUS2}, {"4A", UI_NEXT_REL_PLUS7}, {"9B", UI_NEXT_REL_FIT},
        {"3B", UI_NEXT_REL_NONE},  {"5A", UI_NEXT_REL_NONE},  {"2B", UI_NEXT_REL_NONE},
        {"7A", UI_NEXT_REL_NONE},
    };
    for (unsigned i = 0; i < sizeof ROW / sizeof ROW[0]; i++)
        assert(ui_next_key_relation_text("9A", ROW[i].key) == ROW[i].rel);
}

static void test_wheel_wraps(void)
{
    assert(ui_next_key_text_fit("12A", "1A"));
    assert(ui_next_key_text_fit("1A", "12A"));
    assert(ui_next_key_relation_text("11A", "1A") == UI_NEXT_REL_PLUS2);
    assert(ui_next_key_relation_text("6A", "1A") == UI_NEXT_REL_PLUS7);
    assert(!ui_next_key_text_fit("8A", "9B"));
    assert(ui_next_key_relation_text("", "9A") == UI_NEXT_REL_NONE);
}

int main(void)
{
    test_camelot_parse();
    test_classical_names_map_to_camelot();
    test_browse_board_colours();
    test_wheel_wraps();
    puts("ui_next_key: ok");
    return 0;
}
