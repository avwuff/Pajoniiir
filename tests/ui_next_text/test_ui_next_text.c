#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "ui_next_text.h"

#define E_ACUTE "\xC3\xA9"      /* e with acute */
#define E_ACUTE_UP "\xC3\x89"
#define E_CIRC "\xC3\xAA"
#define E_GRAVE "\xC3\xA8"

static char out[160];
static uint16_t os[160], oe[160];

static void test_ascii_lowercases(void)
{
    assert(ui_next_fold("Sample Track 01", true, out, sizeof out, NULL, NULL) == 15);
    assert(!strcmp(out, "sample track 01"));
}

static void test_accents_fold_like_the_board(void)
{
    ui_next_fold(E_ACUTE_UP "t" E_ACUTE " Indien", true, out, sizeof out, NULL, NULL);
    assert(!strcmp(out, "ete indien"));
    ui_next_fold("Nuit d'" E_ACUTE "t" E_ACUTE, true, out, sizeof out, NULL, NULL);
    assert(!strcmp(out, "nuit d'ete"));
    ui_next_fold("Po" E_GRAVE "te Urbain", true, out, sizeof out, NULL, NULL);
    assert(!strcmp(out, "poete urbain"));
    ui_next_fold("Temp" E_CIRC "te d'Hiver", true, out, sizeof out, NULL, NULL);
    assert(!strcmp(out, "tempete d'hiver"));
}

static void test_ligatures_and_extended_letters(void)
{
    ui_next_fold("Stra\xC3\x9F" "e", true, out, sizeof out, NULL, NULL); /* sharp s */
    assert(!strcmp(out, "strasse"));
    ui_next_fold("C\xC5\x93ur", true, out, sizeof out, NULL, NULL);       /* oe ligature */
    assert(!strcmp(out, "coeur"));
    ui_next_fold("\xC3\x86on \xC5\x81\xC3\xB3" "d\xC5\xBA", true, out, sizeof out, NULL, NULL);
    assert(!strcmp(out, "aeon lodz"));
    ui_next_fold("\xC5\xA0koda \xC4\x8C" "ech", true, out, sizeof out, NULL, NULL);
    assert(!strcmp(out, "skoda cech"));
    ui_next_fold("\xC3\x97 \xC3\xB7", true, out, sizeof out, NULL, NULL); /* x and division signs stay */
    assert(!strcmp(out, "\xC3\x97 \xC3\xB7"));
}

static void test_accents_kept_when_not_stripped(void)
{
    ui_next_fold(E_ACUTE_UP "T" E_ACUTE, false, out, sizeof out, NULL, NULL);
    assert(!strcmp(out, E_ACUTE_UP "t" E_ACUTE)); /* ASCII lowers, accents stay as they are */
    ui_next_fold("Caf" E_ACUTE, false, out, sizeof out, NULL, NULL);
    assert(strstr(out, "caf") == out && strstr(out, E_ACUTE));
}

static void test_offsets_map_back_to_the_source(void)
{
    const char *src = "Beaut" E_ACUTE " Secr" E_GRAVE "te"; /* B e a u t e-acute ' ' S e c r e-grave t e */
    size_t n = ui_next_fold(src, true, out, sizeof out, os, oe);
    assert(!strcmp(out, "beaute secrete") && n == 14);
    const char *hit = strstr(out, "ete");
    assert(hit);
    size_t i = (size_t)(hit - out);       /* first "ete" is in "Secrete" */
    assert(src[os[i]] == (char)0xC3);     /* starts on the accent grave letter's first byte */
    assert(oe[i + 2] == strlen(src));     /* ends at the end of the source */
    /* An expanding letter: both output bytes point at the same source character. */
    ui_next_fold("a\xC3\x9F" "b", true, out, sizeof out, os, oe);
    assert(!strcmp(out, "assb"));
    assert(os[1] == os[2] && oe[1] == oe[2] && oe[1] - os[1] == 2);
}

static void test_bounds_and_bad_bytes(void)
{
    char small[5];
    size_t n = ui_next_fold("abcdefgh", true, small, sizeof small, NULL, NULL);
    assert(n == 4 && !strcmp(small, "abcd"));
    assert(ui_next_fold("", true, out, sizeof out, NULL, NULL) == 0 && out[0] == '\0');
    assert(ui_next_fold(NULL, true, out, sizeof out, NULL, NULL) == 0);
    ui_next_fold("a\xFF" "b\xC3", true, out, sizeof out, NULL, NULL); /* stray and truncated bytes survive */
    assert(out[0] == 'a' && out[strlen(out) - 1] != '\0');
}

static void test_compare(void)
{
    assert(ui_next_text_cmp("Ete", E_ACUTE_UP "t" E_ACUTE, true) == 0);
    assert(ui_next_text_cmp("ete", E_ACUTE_UP "t" E_ACUTE, false) != 0);
    assert(ui_next_text_cmp("Eclair", "Zebra", true) < 0);
    assert(ui_next_text_cmp(E_ACUTE_UP "clair", "Zebra", true) < 0); /* accented E before Z */
    assert(ui_next_text_cmp("b", "A", true) > 0);
}

static void test_backspace_removes_a_character(void)
{
    char q[16];
    snprintf(q, sizeof q, "ab" E_ACUTE);
    assert(ui_next_utf8_backspace(q) && !strcmp(q, "ab"));
    assert(ui_next_utf8_backspace(q) && !strcmp(q, "a"));
    assert(ui_next_utf8_backspace(q) && q[0] == '\0');
    assert(!ui_next_utf8_backspace(q));
}

int main(void)
{
    test_ascii_lowercases();
    test_accents_fold_like_the_board();
    test_ligatures_and_extended_letters();
    test_accents_kept_when_not_stripped();
    test_offsets_map_back_to_the_source();
    test_bounds_and_bad_bytes();
    test_compare();
    test_backspace_removes_a_character();
    puts("ui_next_text: ok");
    return 0;
}
