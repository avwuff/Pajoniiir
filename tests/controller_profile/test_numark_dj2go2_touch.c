#include "controller_profile.h"
#include "controller_profile_manager.h"
#include "control_link.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static unsigned replayed;

static bool replay_cb(uint8_t type, uint8_t id, int16_t value, void *ctx)
{
    (void)ctx;
    assert(type == CTRL_TYPE_PITCH && value >= 0 && value <= 16383);
    assert(id != CTRL_ID_DECK1_TEMPO && id != CTRL_ID_DECK2_TEMPO);
    ++replayed;
    return true;
}

/* A 7-bit CC must be monotonic, hit 0 / 8192 / 16383 and map to one event id. */
static void sweep_cc7(const cp_profile_t *p, cp_runtime_t *rt, uint8_t status,
                      uint8_t cc, uint8_t id)
{
    int16_t previous = -1;
    for (unsigned raw = 0; raw < 128; ++raw) {
        cp_event_t e;
        assert(cp_runtime_process(p, rt, status, cc, (uint8_t)raw, &e));
        assert(e.type == CTRL_TYPE_PITCH && e.id == id);
        assert(e.value >= previous && e.value >= 0 && e.value <= 16383);
        if (raw == 0) assert(e.value == 0);
        if (raw == 64) assert(e.value == 8192);
        if (raw == 127) assert(e.value == 16383);
        previous = e.value;
    }
}

/* Press is Note On with velocity > 0. Release can be Note Off or Note On 0. */
static void check_button(const cp_profile_t *p, cp_runtime_t *rt, uint8_t on_status,
                         uint8_t note, uint8_t id)
{
    cp_event_t e;
    assert(cp_runtime_process(p, rt, on_status, note, 127, &e));
    assert(e.type == CTRL_TYPE_BUTTON && e.id == id && e.value == 1);
    assert(cp_runtime_process(p, rt, (uint8_t)(on_status - 0x10), note, 0, &e));
    assert(e.type == CTRL_TYPE_BUTTON && e.id == id && e.value == 0);
    assert(cp_runtime_process(p, rt, on_status, note, 0, &e));
    assert(e.type == CTRL_TYPE_BUTTON && e.id == id && e.value == 0);
}

static void check_pad(const cp_profile_t *p, cp_runtime_t *rt, uint8_t on_status,
                      uint8_t note, uint8_t id, int mode, unsigned pad, bool shifted)
{
    cp_event_t e;
    assert(cp_runtime_process(p, rt, on_status, note, 100, &e));
    assert(e.type == CTRL_TYPE_BUTTON && e.id == id);
    assert(e.value == CTRL_PAD_ACTION_VALUE(mode, pad, shifted, true));
    assert(cp_runtime_process(p, rt, (uint8_t)(on_status - 0x10), note, 0, &e));
    assert(e.value == CTRL_PAD_ACTION_VALUE(mode, pad, shifted, false));
}

static void check_led(const cp_profile_t *p, uint8_t led, uint8_t deck, uint8_t status,
                      uint8_t note)
{
    uint8_t out[3];
    assert(cp_profile_map_led(p, led, deck, 1, out));
    assert(out[0] == status && out[1] == note && out[2] == 0x7F);
    assert(cp_profile_map_led(p, led, deck, 0, out));
    assert(out[0] == status && out[1] == note && out[2] == 0x00);
}

int main(void)
{
    uint8_t blob[16384];
    FILE *f = fopen("../../controllers/numark_dj2go2_touch/profile.s3bin", "rb");
    assert(f);
    size_t len = fread(blob, 1, sizeof(blob), f);
    assert(!ferror(f) && feof(f));
    fclose(f);

    static cp_profile_t p;
    static cp_runtime_t rt;
    cp_event_t e;
    controller_profile_meta_t meta = {0};
    assert(cp_profile_parse(blob, len, &p) == CP_OK);
    assert(controller_profile_meta_parse(blob, len, &meta) == ESP_OK && meta.valid);
    assert(blob[4] == 4); /* S3CP v4: initial SysEx. */
    assert(p.vid == 0x15e4 && p.pid == 0x003c);
    assert(p.input_count == 100 && p.output_count == 30 && p.decks == 2);
    assert(p.flags & CP_PF_LED_FEEDBACK);
    assert(p.flags & CP_PF_USB_AUDIO);
    assert(p.flags & CP_PF_JOG_TOUCH);
    assert(p.flags & CP_PF_PITCH_14BIT);
    assert(!(p.flags & CP_PF_NATIVE_HEADPHONES)); /* Levels are software controls. */
    assert(!(p.flags & CP_PF_FILTER_ALWAYS));

    /* Initial SysEx is the Mixxx controller-status request. */
    static const uint8_t sysex[] = {0xF0, 0x00, 0x01, 0x3F, 0x3C, 0x48, 0xF7};
    assert(blob[30] == sizeof(sysex) && blob[31] == 0);
    assert(len >= sizeof(sysex) && memcmp(blob + len - sizeof(sysex), sysex, sizeof(sysex)) == 0);

    cp_runtime_init(&rt);
    for (unsigned d = 0; d < 2; ++d) {
        const uint8_t ns = d ? CTRL_NS_DECK2 : CTRL_NS_DECK1;
        const uint8_t on = (uint8_t)(0x90 + d);
        const uint8_t pad = (uint8_t)(0x94 + d);
        const uint8_t cc = (uint8_t)(0xB0 + d);

        check_button(&p, &rt, on, 0x00, (uint8_t)(ns + CTRL_DECK_CTL_PLAY));
        check_button(&p, &rt, on, 0x01, (uint8_t)(ns + CTRL_DECK_CTL_CUE));
        check_button(&p, &rt, on, 0x02, (uint8_t)(ns + CTRL_DECK_CTL_SYNC));
        check_button(&p, &rt, on, 0x1B, (uint8_t)(CTRL_ID_DECK1_PFL_SET + d));
        check_button(&p, &rt, on, 0x06, (uint8_t)(ns + CTRL_DECK_CTL_JOG_TOUCH));

        /* One CC drives jog_scratch. deck_core turns it into bend when untouched. */
        assert(cp_runtime_process(&p, &rt, cc, 0x06, 1, &e));
        assert(e.type == CTRL_TYPE_ENCODER && e.id == ns + CTRL_DECK_CTL_JOG_SCRATCH);
        assert(e.value == 1);
        assert(cp_runtime_process(&p, &rt, cc, 0x06, 127, &e) && e.value == -1);
        assert(cp_runtime_process(&p, &rt, cc, 0x06, 63, &e) && e.value == 63);
        assert(cp_runtime_process(&p, &rt, cc, 0x06, 65, &e) && e.value == -63);
        assert(!cp_runtime_process(&p, &rt, cc, 0x06, 0x00, &e));
        assert(!cp_runtime_process(&p, &rt, cc, 0x06, 0x40, &e));

        sweep_cc7(&p, &rt, cc, 0x09, (uint8_t)(ns + CTRL_DECK_CTL_TEMPO));
        sweep_cc7(&p, &rt, cc, 0x16, d ? CTRL_ID_CH2_VOLUME : CTRL_ID_CH1_VOLUME);

        for (unsigned i = 0; i < 4; ++i) {
            const uint8_t id = (uint8_t)(ns + CTRL_DECK_CTL_PAD_ACTION);
            check_pad(&p, &rt, pad, (uint8_t)(0x01 + i), id, CTRL_PAD_MODE_HOT_CUE, i, false);
            check_pad(&p, &rt, pad, (uint8_t)(0x09 + i), id, CTRL_PAD_MODE_HOT_CUE, i, true);
            check_pad(&p, &rt, pad, (uint8_t)(0x11 + i), id, CTRL_PAD_MODE_BEAT_LOOP, i, false);
            /* Sampler pads have no P4 action and stay unmatched. */
            assert(!cp_runtime_process(&p, &rt, pad, (uint8_t)(0x31 + i), 100, &e));
            assert(!cp_runtime_process(&p, &rt, (uint8_t)(pad - 0x10), (uint8_t)(0x31 + i), 0, &e));
        }
        check_button(&p, &rt, pad, 0x21, (uint8_t)(ns + CTRL_DECK_CTL_LOOP_IN));
        check_button(&p, &rt, pad, 0x22, (uint8_t)(ns + CTRL_DECK_CTL_LOOP_OUT));
        check_button(&p, &rt, pad, 0x23, (uint8_t)(ns + CTRL_DECK_CTL_RELOOP_EXIT));
        assert(cp_runtime_process(&p, &rt, pad, 0x24, 127, &e));
        assert(e.id == ns + CTRL_DECK_CTL_EXT_ACTION);
        assert(e.value == CTRL_DECK_EXT_VALUE(CTRL_DECK_EXT_ACTION_RELOOP_STOP, true));
        assert(cp_runtime_process(&p, &rt, (uint8_t)(pad - 0x10), 0x24, 0, &e));
        assert(e.value == CTRL_DECK_EXT_VALUE(CTRL_DECK_EXT_ACTION_RELOOP_STOP, false));

        /* LOAD is on channel 16: note 2 for deck 1, note 3 for deck 2. */
        check_button(&p, &rt, 0x9F, (uint8_t)(0x02 + d), (uint8_t)(CTRL_ID_LOAD_DECK1 + d));

        check_led(&p, LED_PLAY, (uint8_t)d, on, 0x00);
        check_led(&p, LED_CUE, (uint8_t)d, on, 0x01);
        check_led(&p, LED_SYNC, (uint8_t)d, on, 0x02);
        check_led(&p, LED_PFL, (uint8_t)d, on, 0x1B);
        check_led(&p, LED_LOOP_IN, (uint8_t)d, pad, 0x21);
        check_led(&p, LED_LOOP_OUT, (uint8_t)d, pad, 0x22);
        for (unsigned i = 0; i < 4; ++i) {
            check_led(&p, (uint8_t)(LED_HOT_CUE_PAD_1 + i), (uint8_t)d, pad, (uint8_t)(0x01 + i));
            check_led(&p, (uint8_t)(LED_BEAT_LOOP_PAD_1 + i), (uint8_t)d, pad, (uint8_t)(0x11 + i));
        }
        /* The Touch has four pads per bank. P4 pads 5 to 8 have no LED. */
        uint8_t out[3];
        assert(!cp_profile_map_led(&p, (uint8_t)(LED_HOT_CUE_PAD_1 + 4), (uint8_t)d, 1, out));
        assert(!cp_profile_map_led(&p, (uint8_t)(LED_BEAT_LOOP_PAD_1 + 4), (uint8_t)d, 1, out));
        /* Track-load LEDs are global ("any deck"). */
        check_led(&p, LED_TRACK_LOAD_DECK1, (uint8_t)d, 0x9F, 0x02);
        check_led(&p, LED_TRACK_LOAD_DECK2, (uint8_t)d, 0x9F, 0x03);
    }

    /* Browse encoder: +1 / -1 on channel 16, push is a button. */
    assert(cp_runtime_process(&p, &rt, 0xBF, 0x00, 1, &e));
    assert(e.type == CTRL_TYPE_ENCODER && e.id == CTRL_ID_BROWSE_DELTA && e.value == 1);
    assert(cp_runtime_process(&p, &rt, 0xBF, 0x00, 127, &e) && e.value == -1);
    check_button(&p, &rt, 0x9F, 0x06, CTRL_ID_BROWSE_PRESS);

    sweep_cc7(&p, &rt, 0xBF, 0x08, CTRL_ID_CROSSFADER);
    sweep_cc7(&p, &rt, 0xBF, 0x0A, CTRL_ID_MASTER_VOLUME);
    sweep_cc7(&p, &rt, 0xBF, 0x0C, CTRL_ID_HEADPHONE_LEVEL);

    /* Replay covers 2 channel levels, crossfader, master and headphone level. */
    assert(cp_runtime_emit_snapshot(&p, &rt, replay_cb, NULL) == 5 && replayed == 5);

    /* Unmapped addresses and a wrong status do not match. */
    assert(!cp_runtime_process(&p, &rt, 0x90, 0x7F, 127, &e));
    assert(!cp_runtime_process(&p, &rt, 0xB2, 0x09, 64, &e));

    /* A payload byte changed without a new CRC is rejected by the parser and by
     * the upload validator. */
    uint8_t bad[sizeof(blob)];
    memcpy(bad, blob, len);
    bad[32 + 3] ^= 0x01;
    assert(cp_profile_parse(bad, len, &p) == CP_ERR_CRC);
    assert(controller_profile_meta_parse(bad, len, &meta) != ESP_OK);

    puts("PASS Numark DJ2GO2 Touch compiled profile: buttons, Note On/Off, jog, 7-to-14-bit, pads, LEDs, replay, validators");
    return 0;
}
