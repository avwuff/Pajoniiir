#include "ui_next_layout.h"

#include <string.h>

/* Tokens and zone heights follow the CSS of the demo boards (--tb, --navs,
 * --dock, link bar height), which match the proposal's class table. */
static const ui_next_tokens_t TOKENS[] = {
    [UI_NEXT_CLASS_S] = {14, 44, 64, 52, 10, 8, 44, 76, 112, 64},
    [UI_NEXT_CLASS_M] = {15, 48, 72, 56, 12, 10, 48, 92, 128, 72},
    [UI_NEXT_CLASS_L] = {17, 56, 84, 64, 16, 12, 56, 108, 150, 66},
};

ui_next_class_t ui_next_class_for(uint16_t w, uint16_t h)
{
    uint16_t longest = w > h ? w : h;
    if (longest <= 800u) return UI_NEXT_CLASS_S;
    if (longest <= 1024u) return UI_NEXT_CLASS_M;
    return UI_NEXT_CLASS_L;
}

const ui_next_tokens_t *ui_next_tokens(ui_next_class_t cls)
{
    return &TOKENS[cls];
}

static ui_next_rect_t rect(int16_t x, int16_t y, int16_t w, int16_t h)
{
    ui_next_rect_t r = {x, y, w, h};
    return r;
}

bool ui_next_layout_compute(uint16_t w, uint16_t h, ui_next_layout_t *out)
{
    if (!out || w < 480u || h < 480u) return false;

    memset(out, 0, sizeof(*out));
    out->cls = ui_next_class_for(w, h);
    out->landscape = w > h;
    out->tok = *ui_next_tokens(out->cls);
    const ui_next_tokens_t *t = &out->tok;

    int16_t dock = out->landscape ? t->dock_land_px : t->dock_port_px;
    /* Landscape: the rail takes a column left of every zone but the top bar.
     * Portrait: the bar sits under everything. */
    int16_t x0 = out->landscape ? t->nav_px : 0;
    int16_t zw = (int16_t)(w - x0);
    int16_t zh = (int16_t)(h - (out->landscape ? 0 : t->nav_px));

    out->top_bar = rect(0, 0, (int16_t)w, t->top_bar_px);
    out->nav = out->landscape ? rect(0, t->top_bar_px, t->nav_px, (int16_t)(h - t->top_bar_px))
                              : rect(0, (int16_t)(h - t->nav_px), (int16_t)w, t->nav_px);

    /* Bottom-up: dock, link bar; waveforms take the rest. */
    int16_t y = (int16_t)(zh - dock);
    out->dock = rect(x0, y, zw, dock);
    y = (int16_t)(y - t->link_bar_px);
    out->link_bar = rect(x0, y, zw, t->link_bar_px);
    out->waveforms = rect(x0, t->top_bar_px, zw, (int16_t)(y - t->top_bar_px));

    if (out->landscape) {
        int16_t half = (int16_t)(zw / 2);
        out->deck_card[0] = rect(x0, y + t->link_bar_px, half, dock);
        out->deck_card[1] = rect((int16_t)(x0 + half), y + t->link_bar_px, (int16_t)(zw - half), dock);
    } else {
        int16_t row = (int16_t)(dock / 2);
        out->deck_card[0] = rect(x0, y + t->link_bar_px, zw, row);
        out->deck_card[1] = rect(x0, (int16_t)(y + t->link_bar_px + row), zw, (int16_t)(dock - row));
    }
    return true;
}
