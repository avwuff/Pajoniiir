#include "ui_next_text.h"

#include <string.h>

/* Generated table: folded form of U+00C0..U+00FF, then U+0100..U+017F (NULL = keep the character). */
static const char *const FOLD_LATIN1[64] = {
    "a", "a", "a", "a", "a", "a", "ae", "c",
    "e", "e", "e", "e", "i", "i", "i", "i",
    "d", "n", "o", "o", "o", "o", "o", NULL,
    "o", "u", "u", "u", "u", "y", "th", "ss",
    "a", "a", "a", "a", "a", "a", "ae", "c",
    "e", "e", "e", "e", "i", "i", "i", "i",
    "d", "n", "o", "o", "o", "o", "o", NULL,
    "o", "u", "u", "u", "u", "y", "th", "y",
};
static const char *const FOLD_EXT_A[128] = {
    "a", "a", "a", "a", "a", "a", "c", "c",
    "c", "c", "c", "c", "c", "c", "d", "d",
    "d", "d", "e", "e", "e", "e", "e", "e",
    "e", "e", "e", "e", "g", "g", "g", "g",
    "g", "g", "g", "g", "h", "h", "h", "h",
    "i", "i", "i", "i", "i", "i", "i", "i",
    "i", "i", "ij", "ij", "j", "j", "k", "k",
    "k", "l", "l", "l", "l", "l", "l", "l",
    "l", "l", "l", "n", "n", "n", "n", "n",
    "n", "n", "n", "n", "o", "o", "o", "o",
    "o", "o", "oe", "oe", "r", "r", "r", "r",
    "r", "r", "s", "s", "s", "s", "s", "s",
    "s", "s", "t", "t", "t", "t", "t", "t",
    "u", "u", "u", "u", "u", "u", "u", "u",
    "u", "u", "u", "u", "w", "w", "y", "y",
    "y", "z", "z", "z", "z", "z", "z", "s",
};

uint32_t ui_next_utf8_next(const char *s, int *len)
{
    const unsigned char c = (unsigned char)s[0];
    if (c < 0x80) {
        *len = 1;
        return c;
    }
    int n = c >= 0xF0 ? 4 : c >= 0xE0 ? 3 : c >= 0xC0 ? 2 : 1;
    uint32_t cp = n == 4 ? c & 0x07u : n == 3 ? c & 0x0Fu : n == 2 ? c & 0x1Fu : c;
    if (n == 1) {
        *len = 1; /* a stray continuation byte */
        return c;
    }
    for (int i = 1; i < n; i++) {
        const unsigned char cc = (unsigned char)s[i];
        if ((cc & 0xC0) != 0x80) {
            *len = 1;
            return c;
        }
        cp = (cp << 6) | (cc & 0x3Fu);
    }
    *len = n;
    return cp;
}

/* Folded form of one code point, or NULL to keep the source bytes. */
static const char *fold_cp(uint32_t cp, bool strip, char *buf)
{
    if (cp < 0x80) {
        buf[0] = (char)(cp >= 'A' && cp <= 'Z' ? cp + 32u : cp);
        buf[1] = '\0';
        return buf;
    }
    if (!strip) return NULL;
    if (cp >= 0xC0 && cp <= 0xFF) return FOLD_LATIN1[cp - 0xC0];
    if (cp >= 0x100 && cp <= 0x17F) return FOLD_EXT_A[cp - 0x100];
    return NULL;
}

size_t ui_next_fold(const char *s, bool strip, char *out, size_t cap, uint16_t *ostart, uint16_t *oend)
{
    if (!out || cap == 0) return 0;
    size_t n = 0;
    uint16_t pos = 0;
    while (s && s[pos]) {
        int len;
        const uint32_t cp = ui_next_utf8_next(&s[pos], &len);
        char tmp[2];
        const char *f = fold_cp(cp, strip, tmp);
        const char *src = f ? f : &s[pos];
        const size_t m = f ? strlen(f) : (size_t)len;
        for (size_t i = 0; i < m && n + 1 < cap; i++) {
            if (ostart) ostart[n] = pos;
            if (oend) oend[n] = (uint16_t)(pos + len);
            out[n++] = src[i];
        }
        pos = (uint16_t)(pos + len);
    }
    out[n] = '\0';
    return n;
}

int ui_next_text_cmp(const char *a, const char *b, bool strip)
{
    char fa[160], fb[160];
    ui_next_fold(a, strip, fa, sizeof fa, NULL, NULL);
    ui_next_fold(b, strip, fb, sizeof fb, NULL, NULL);
    return strcmp(fa, fb);
}

bool ui_next_utf8_backspace(char *s)
{
    size_t n = s ? strlen(s) : 0;
    if (n == 0) return false;
    size_t i = n - 1;
    while (i > 0 && ((unsigned char)s[i] & 0xC0) == 0x80) i--;
    s[i] = '\0';
    return true;
}
