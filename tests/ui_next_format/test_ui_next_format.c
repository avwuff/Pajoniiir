#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "ui_next_format.h"

static void test_clock(void)
{
    char s[16];
    ui_next_fmt_clock(s, sizeof s, 0);
    assert(!strcmp(s, "00:00"));
    ui_next_fmt_clock(s, sizeof s, 224000);
    assert(!strcmp(s, "03:44"));
    ui_next_fmt_clock(s, sizeof s, 3600000u * 3u);
    assert(!strcmp(s, "99:00")); /* clamps at 99 minutes */
    ui_next_fmt_remaining(s, sizeof s, 100000, 400000);
    assert(!strcmp(s, "-05:00"));
    ui_next_fmt_remaining(s, sizeof s, 500000, 400000); /* past the end: nothing left */
    assert(!strcmp(s, "-00:00"));
    ui_next_fmt_hms(s, sizeof s, 12 * 60 + 34);
    assert(!strcmp(s, "00:12:34"));
    ui_next_fmt_hms(s, sizeof s, 3 * 3600 + 5);
    assert(!strcmp(s, "03:00:05"));
}

static void test_bpm_follows_speed(void)
{
    char s[16];
    ui_next_fmt_bpm(s, sizeof s, 12400, 1000);
    assert(!strcmp(s, "124.0"));
    ui_next_fmt_bpm(s, sizeof s, 12400, 960); /* pitch -4 % */
    assert(!strcmp(s, "119.0"));
    ui_next_fmt_bpm(s, sizeof s, 12850, 0);   /* unknown speed counts as 1x */
    assert(!strcmp(s, "128.5"));
}

static void test_size_and_free(void)
{
    char s[16];
    ui_next_fmt_size(s, sizeof s, 842000000ull);
    assert(!strcmp(s, "842 MB"));
    ui_next_fmt_size(s, sizeof s, 1100000000ull);
    assert(!strcmp(s, "1.1 GB"));
    ui_next_fmt_size(s, sizeof s, 124000000ull);
    assert(!strcmp(s, "124 MB"));
    ui_next_fmt_size(s, sizeof s, 320000ull);
    assert(!strcmp(s, "320 kB"));
    ui_next_fmt_free(s, sizeof s, 38400000000ull);
    assert(!strcmp(s, "FREE 38 GB"));
}

static void test_levels(void)
{
    char s[16];
    ui_next_fmt_db(s, sizeof s, 32768);
    assert(!strcmp(s, "0 dB"));
    ui_next_fmt_db(s, sizeof s, 13029); /* about -8 dB */
    assert(!strcmp(s, "-8 dB"));
    ui_next_fmt_db(s, sizeof s, 0);
    assert(!strcmp(s, "-inf"));
    /* Record board: -8 dB reads 0.68 on a 25 dB scale, -10 dB about 0.6. */
    float v8 = ui_next_level01(13029, -25.0f);
    assert(v8 > 0.67f && v8 < 0.69f);
    assert(ui_next_level01(32768, -25.0f) == 1.0f);
    assert(ui_next_level01(0, -25.0f) == 0.0f);
    assert(ui_next_level01(5, -25.0f) == 0.0f);
    assert(ui_next_level01(1000, 0.0f) == 0.0f);
}

int main(void)
{
    test_clock();
    test_bpm_follows_speed();
    test_size_and_free();
    test_levels();
    puts("ui_next_format: ok");
    return 0;
}
