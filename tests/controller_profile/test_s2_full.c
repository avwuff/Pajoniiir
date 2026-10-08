#include "controller_profile.h"
#include "controller_profile_manager.h"
#include "control_link.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static unsigned replay_count;
static bool replay(uint8_t type, uint8_t id, int16_t value, void *ctx)
{
    (void)ctx;
    assert(type == CTRL_TYPE_PITCH && value >= 0 && value <= 16383);
    assert(id != CTRL_ID_DECK1_TEMPO && id != CTRL_ID_DECK2_TEMPO);
    ++replay_count;
    return true;
}

int main(void)
{
    uint8_t blob[16384];
    FILE *f = fopen("../../controllers/ni_traktor_s2_mk3_full/profile.s3bin", "rb");
    assert(f);
    size_t len = fread(blob, 1, sizeof(blob), f);
    assert(!ferror(f) && feof(f)); fclose(f);
    static cp_profile_t p;
    static cp_runtime_t rt;
    cp_event_t e;
    controller_profile_meta_t meta = {0};
    assert(cp_profile_parse(blob, len, &p) == CP_OK);
    assert(controller_profile_meta_parse(blob, len, &meta) == ESP_OK && meta.valid);
    assert(p.vid == 0x17cc && p.pid == 0x1710 && p.input_count == 80 && p.output_count == 26);
    assert(p.flags & CP_PF_FILTER_ALWAYS);
    cp_runtime_init(&rt);
    for (unsigned d = 0; d < 2; ++d) {
        uint8_t ns = d ? CTRL_NS_DECK2 : CTRL_NS_DECK1;
        uint8_t note = (uint8_t)(0x91 + 2 * d);
        uint8_t cc = (uint8_t)(0xb5 + d);
        const uint8_t ids[] = {d ? CTRL_ID_CH2_VOLUME : CTRL_ID_CH1_VOLUME,
            d ? CTRL_ID_CH2_FILTER : CTRL_ID_CH1_FILTER,
            d ? CTRL_ID_CH2_EQ_LOW : CTRL_ID_CH1_EQ_LOW,
            d ? CTRL_ID_CH2_EQ_MID : CTRL_ID_CH1_EQ_MID,
            d ? CTRL_ID_CH2_EQ_HIGH : CTRL_ID_CH1_EQ_HIGH,
            d ? CTRL_ID_CH2_TRIM : CTRL_ID_CH1_TRIM};
        for (uint8_t control = 1; control <= 6; ++control) {
            int16_t previous = -1;
            for (unsigned raw = 0; raw < 128; ++raw) {
                assert(cp_runtime_process(&p, &rt, cc, control, (uint8_t)raw, &e));
                assert(e.type == CTRL_TYPE_PITCH && e.id == ids[control - 1]);
                assert(e.value >= previous && e.value >= 0 && e.value <= 16383);
                if (raw == 0 || raw == 64 || raw == 127)
                    assert(e.value == (raw == 0 ? 0 : raw == 64 ? 8192 : 16383));
                previous = e.value;
            }
        }
        int16_t previous = 16384;
        for (unsigned raw = 0; raw < 128; ++raw) {
            assert(cp_runtime_process(&p, &rt, note + 0x20, 0x2a, (uint8_t)raw, &e));
            assert(e.type == CTRL_TYPE_PITCH && e.id == ns + CTRL_DECK_CTL_TEMPO);
            assert(e.value <= previous && e.value >= 0 && e.value <= 16383);
            if (raw == 0 || raw == 64 || raw == 127)
                assert(e.value == (raw == 0 ? 16383 : raw == 64 ? 8192 : 0));
            previous = e.value;
        }
        assert(cp_runtime_process(&p, &rt, note, 0x14, 127, &e));
        assert(e.id == ns + CTRL_DECK_CTL_JOG_TOUCH && e.value == 1);
        assert(cp_runtime_process(&p, &rt, note, 0x14, 0, &e) && e.value == 0);
        assert(cp_runtime_process(&p, &rt, note + 0x20, 0x1e, 65, &e));
        assert(e.id == ns + CTRL_DECK_CTL_JOG_SCRATCH && e.value == 1);
        assert(cp_runtime_process(&p, &rt, note + 0x20, 0x1e, 63, &e) && e.value == -1);
        assert(cp_runtime_process(&p, &rt, note + 0x21, 0x1e, 65, &e));
        assert(e.id == ns + CTRL_DECK_CTL_JOG_SEARCH && e.value == 1);
        assert(cp_runtime_process(&p, &rt, note + 1, 0x10, 127, &e));
        assert(e.id == ns + CTRL_DECK_CTL_EXT_ACTION &&
               e.value == CTRL_DECK_EXT_VALUE(CTRL_DECK_EXT_ACTION_SYNC_MASTER, true));
        assert(cp_runtime_process(&p, &rt, note + 1, 0x13, 127, &e));
        assert(e.id == (d ? CTRL_ID_SHIFT_LOAD_DECK2 : CTRL_ID_SHIFT_LOAD_DECK1));
        for (unsigned pad = 0; pad < 8; ++pad) {
            uint8_t out[3];
            assert(cp_profile_map_led(&p, LED_HOT_CUE_PAD_1 + pad, d, 1, out));
            assert(out[0] == note && out[1] == 3 + pad && out[2] == 127);
        }
        for (unsigned v = 0; v < 128; ++v) {
            uint8_t out[3];
            assert(cp_profile_map_led(&p, LED_VU_METER, d, v, out));
            assert(out[0] == 0x95 + d && out[1] == 7 && out[2] == v);
        }
    }
    assert(cp_runtime_process(&p, &rt, 0xb7, 1, 64, &e));
    assert(e.id == CTRL_ID_CROSSFADER && e.value == 8192);
    assert(cp_runtime_process(&p, &rt, 0xb7, 7, 64, &e));
    assert(e.id == CTRL_ID_HEADPHONE_MIX && e.value == 8192);
    assert(cp_runtime_emit_snapshot(&p, &rt, replay, NULL) == 14 && replay_count == 14);
    /* Corrupt ABI combinations must fail at BOTH parser and upload validator. */
    uint8_t bad[sizeof(blob)];
    memcpy(bad, blob, len); bad[4] = 3;
    uint32_t crc = cp_crc32(bad + 16, len - 16);
    for (unsigned b = 0; b < 4; ++b) bad[12+b] = (uint8_t)(crc >> (b*8));
    assert(cp_profile_parse(bad, len, &p) == CP_ERR_BOUNDS);
    assert(controller_profile_meta_parse(bad, len, &meta) != ESP_OK);
    size_t inverted = 0;
    for (unsigned i = 0; i < 80; ++i)
        if (blob[32 + 16*i + 2] == CP_IN_CC7_TO14_INVERTED) { inverted = 32 + 16*i; break; }
    assert(inverted);
    const unsigned offsets[] = {0, 3, 4, 6};
    const uint8_t values[] = {0x91, 0, CTRL_TYPE_BUTTON, 2};
    for (unsigned i = 0; i < 4; ++i) {
        memcpy(bad, blob, len); bad[inverted + offsets[i]] = values[i];
        crc = cp_crc32(bad + 16, len - 16);
        for (unsigned b = 0; b < 4; ++b) bad[12+b] = (uint8_t)(crc >> (b*8));
        assert(cp_profile_parse(bad, len, &p) == CP_ERR_BOUNDS);
        assert(controller_profile_meta_parse(bad, len, &meta) != ESP_OK);
    }
    puts("PASS S2 full compiled profile: mixer/pitch range and center, jog, Shift, LEDs, replay, validators");
    return 0;
}
