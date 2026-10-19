#include "ui_next_format.h"

#include <math.h>
#include <stdio.h>

void ui_next_fmt_clock(char *out, size_t cap, uint32_t ms)
{
    uint32_t s = ms / 1000u;
    uint32_t m = s / 60u;
    if (m > 99u) m = 99u;
    snprintf(out, cap, "%02u:%02u", (unsigned)m, (unsigned)(s % 60u));
}

void ui_next_fmt_remaining(char *out, size_t cap, uint32_t pos_ms, uint32_t duration_ms)
{
    const uint32_t left = duration_ms > pos_ms ? duration_ms - pos_ms : 0u;
    char t[12];
    ui_next_fmt_clock(t, sizeof t, left);
    snprintf(out, cap, "-%s", t);
}

void ui_next_fmt_hms(char *out, size_t cap, uint32_t s)
{
    snprintf(out, cap, "%02u:%02u:%02u", (unsigned)(s / 3600u), (unsigned)(s / 60u % 60u), (unsigned)(s % 60u));
}

void ui_next_fmt_bpm(char *out, size_t cap, uint16_t bpm_x100, uint16_t speed_permille)
{
    const uint32_t v = ((uint32_t)bpm_x100 * (uint32_t)(speed_permille ? speed_permille : 1000u) + 500u) / 1000u;
    snprintf(out, cap, "%u.%u", (unsigned)(v / 100u), (unsigned)((v % 100u + 5u) / 10u % 10u));
}

void ui_next_fmt_size(char *out, size_t cap, uint64_t bytes)
{
    if (bytes >= 1000000000ull) {
        const uint64_t tenth = (bytes + 50000000ull) / 100000000ull;
        snprintf(out, cap, "%u.%u GB", (unsigned)(tenth / 10u), (unsigned)(tenth % 10u));
    } else if (bytes >= 1000000ull) {
        snprintf(out, cap, "%u MB", (unsigned)((bytes + 500000ull) / 1000000ull));
    } else {
        snprintf(out, cap, "%u kB", (unsigned)((bytes + 500ull) / 1000ull));
    }
}

void ui_next_fmt_free(char *out, size_t cap, uint64_t bytes)
{
    snprintf(out, cap, "FREE %u GB", (unsigned)(bytes / 1000000000ull));
}

static float peak_db(uint16_t peak)
{
    return peak ? 20.0f * log10f((float)peak / 32768.0f) : -200.0f;
}

void ui_next_fmt_db(char *out, size_t cap, uint16_t peak)
{
    const float db = peak_db(peak);
    if (db < -90.0f) snprintf(out, cap, "-inf");
    else snprintf(out, cap, "%d dB", (int)lroundf(db));
}

float ui_next_level01(uint16_t peak, float floor_db)
{
    if (floor_db >= 0.0f) return 0.0f;
    float v = 1.0f + peak_db(peak) / -floor_db;
    return v < 0.0f ? 0.0f : v > 1.0f ? 1.0f : v;
}
