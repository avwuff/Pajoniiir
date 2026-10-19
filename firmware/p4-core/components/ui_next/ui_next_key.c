#include "ui_next_key.h"

#include <ctype.h>
#include <string.h>

/* Classical name to Camelot. Index 0..11 is the tonic C, C#, D, D#, E, F, F#, G, G#, A, A#, B.
 * Minor keys use the A ring, major keys the B ring. */
static const uint8_t MINOR_NUM[12] = {5, 12, 7, 2, 9, 4, 11, 6, 1, 8, 3, 10};
static const uint8_t MAJOR_NUM[12] = {8, 3, 10, 5, 12, 7, 2, 9, 4, 11, 6, 1};

static bool parse_classical(const char *t, ui_next_camelot_t *out)
{
    static const int8_t BASE[7] = {9, 11, 0, 2, 4, 5, 7}; /* A B C D E F G */
    if (t[0] < 'A' || t[0] > 'G') return false;
    int pc = BASE[t[0] - 'A'];
    int i = 1;
    if (t[i] == '#') {
        pc += 1;
        i++;
    } else if (t[i] == 'b') {
        pc -= 1;
        i++;
    }
    pc = (pc + 12) % 12;
    bool minor = false;
    if (t[i] == 'm' && (t[i + 1] == '\0' || strcmp(&t[i], "min") == 0)) {
        minor = true;
        i += strlen(&t[i]);
    } else if (strcmp(&t[i], "maj") == 0) {
        i += 3;
    }
    if (t[i] != '\0') return false;
    out->number = minor ? MINOR_NUM[pc] : MAJOR_NUM[pc];
    out->letter = minor ? 'A' : 'B';
    return true;
}

bool ui_next_key_parse(const char *text, ui_next_camelot_t *out)
{
    if (!text || !out || !text[0]) return false;
    if (isdigit((unsigned char)text[0])) {
        int n = 0, i = 0;
        while (isdigit((unsigned char)text[i]) && i < 2) n = n * 10 + (text[i++] - '0');
        if (n < 1 || n > 12) return false;
        const char l = (char)toupper((unsigned char)text[i]);
        if ((l != 'A' && l != 'B') || text[i + 1] != '\0') return false;
        out->number = (uint8_t)n;
        out->letter = l;
        return true;
    }
    return parse_classical(text, out);
}

/* Steps from `a` up to `b` on the wheel, 0..11. */
static int up(int a, int b) { return (b - a + 12) % 12; }

ui_next_key_rel_t ui_next_key_relation(ui_next_camelot_t ref, ui_next_camelot_t key)
{
    const int d = up(ref.number, key.number);
    if (key.letter != ref.letter) return d == 0 ? UI_NEXT_REL_FIT : UI_NEXT_REL_NONE;
    if (d == 0 || d == 1 || d == 11) return UI_NEXT_REL_FIT;
    if (d == 2) return UI_NEXT_REL_PLUS2;
    if (d == 7) return UI_NEXT_REL_PLUS7;
    return UI_NEXT_REL_NONE;
}

ui_next_key_rel_t ui_next_key_relation_text(const char *ref, const char *key)
{
    ui_next_camelot_t a, b;
    if (!ui_next_key_parse(ref, &a) || !ui_next_key_parse(key, &b)) return UI_NEXT_REL_NONE;
    return ui_next_key_relation(a, b);
}

bool ui_next_key_text_fit(const char *a, const char *b)
{
    return ui_next_key_relation_text(a, b) == UI_NEXT_REL_FIT;
}
