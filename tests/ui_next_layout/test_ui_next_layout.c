#include <assert.h>
#include <stdio.h>

#include "ui_next_layout.h"

static const struct { uint16_t w, h; ui_next_class_t cls; } SIZES[] = {
    {800, 480, UI_NEXT_CLASS_S},  {1024, 600, UI_NEXT_CLASS_M}, {1280, 800, UI_NEXT_CLASS_L},
    {480, 800, UI_NEXT_CLASS_S},  {600, 1024, UI_NEXT_CLASS_M}, {800, 1280, UI_NEXT_CLASS_L},
};

static int contains(ui_next_rect_t o, ui_next_rect_t i)
{
    return i.x >= o.x && i.y >= o.y && i.x + i.w <= o.x + o.w && i.y + i.h <= o.y + o.h;
}

static void test_class_from_long_side(void)
{
    for (unsigned i = 0; i < sizeof(SIZES) / sizeof(SIZES[0]); i++) {
        assert(ui_next_class_for(SIZES[i].w, SIZES[i].h) == SIZES[i].cls);
    }
    assert(ui_next_class_for(801, 480) == UI_NEXT_CLASS_M);
    assert(ui_next_class_for(1025, 600) == UI_NEXT_CLASS_L);
}

static void test_zones_tile_the_frame_without_overlap(void)
{
    for (unsigned i = 0; i < sizeof(SIZES) / sizeof(SIZES[0]); i++) {
        ui_next_layout_t l;
        assert(ui_next_layout_compute(SIZES[i].w, SIZES[i].h, &l));
        ui_next_rect_t frame = {0, 0, (int16_t)SIZES[i].w, (int16_t)SIZES[i].h};
        assert(contains(frame, l.top_bar) && contains(frame, l.nav));
        assert(contains(frame, l.waveforms) && contains(frame, l.link_bar) && contains(frame, l.dock));
        assert(l.waveforms.h > 0);

        /* Main column heights add up to the frame. */
        int total = l.top_bar.h + l.waveforms.h + l.link_bar.h + l.dock.h + (l.landscape ? 0 : l.nav.h);
        assert(total == SIZES[i].h);
        assert(l.waveforms.y == l.top_bar.h);
        assert(l.link_bar.y == l.waveforms.y + l.waveforms.h);
        assert(l.dock.y == l.link_bar.y + l.link_bar.h);
        assert(l.tok.touch_px >= 44);
    }
}

static void test_nav_moves_with_orientation(void)
{
    ui_next_layout_t l;
    assert(ui_next_layout_compute(800, 480, &l));
    assert(l.nav.x == 0 && l.nav.w == 64 && l.waveforms.x == 64);
    assert(ui_next_layout_compute(480, 800, &l));
    assert(l.nav.y == 800 - 64 && l.nav.w == 480 && l.waveforms.x == 0);
}

static void test_deck_cards_side_by_side_then_stacked(void)
{
    ui_next_layout_t l;
    assert(ui_next_layout_compute(1024, 600, &l));
    assert(l.deck_card[0].y == l.deck_card[1].y);
    assert(l.deck_card[0].w + l.deck_card[1].w == l.dock.w);
    assert(ui_next_layout_compute(600, 1024, &l));
    assert(l.deck_card[0].x == l.deck_card[1].x);
    assert(l.deck_card[0].h + l.deck_card[1].h == l.dock.h);
}

static void test_known_waveform_heights(void)
{
    ui_next_layout_t l;
    assert(ui_next_layout_compute(800, 480, &l) && l.waveforms.h == 480 - 44 - 64 - 76);
    assert(ui_next_layout_compute(1280, 800, &l) && l.waveforms.h == 800 - 56 - 66 - 108);
}

static void test_rejects_tiny_frames(void)
{
    ui_next_layout_t l;
    assert(!ui_next_layout_compute(320, 240, &l));
    assert(!ui_next_layout_compute(800, 480, 0));
}

int main(void)
{
    test_class_from_long_side();
    test_zones_tile_the_frame_without_overlap();
    test_nav_moves_with_orientation();
    test_deck_cards_side_by_side_then_stacked();
    test_known_waveform_heights();
    test_rejects_tiny_frames();
    puts("ui_next_layout: ok");
    return 0;
}
