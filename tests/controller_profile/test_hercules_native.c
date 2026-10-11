#include "controller_profile.h"
#include "controller_profile_manager.h"
#include "controller_profile_runtime.h"
#include "control_link.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static void refresh_crc(uint8_t *blob, size_t len)
{
    uint32_t crc = cp_crc32(blob + 16, len - 16);
    for (unsigned i = 0; i < 4; ++i) blob[12+i] = (uint8_t)(crc >> (8*i));
}

int main(void)
{
    uint8_t blob[16384], bad[16384], midi[3];
    FILE *file = fopen("../../controllers/hercules_djcontrol_inpulse_500/profile.s3bin", "rb");
    assert(file);
    size_t len = fread(blob, 1, sizeof(blob), file);
    assert(feof(file) && !ferror(file)); fclose(file);
    static cp_profile_t profile;
    static cp_runtime_t runtime;
    controller_profile_meta_t meta;
    cp_event_t event;
    assert(cp_profile_parse(blob, len, &profile) == CP_OK);
    assert(controller_profile_meta_parse(blob, len, &meta) == ESP_OK);
    assert(profile.vid == 0x06f8 && profile.pid == 0xb12b);
    assert(profile.flags & CP_PF_NATIVE_HEADPHONES);
    assert(profile.flags & CP_PF_FILTER_ALWAYS);
    cp_runtime_init(&runtime);
    /* Hardware knobs/buttons must not change the software monitor or its LED. */
    const uint8_t native_cc[] = {4, 5, 0x24, 0x25};
    for (unsigned i = 0; i < sizeof(native_cc); ++i)
        for (unsigned value = 0; value < 128; ++value)
            assert(!cp_runtime_process(&profile, &runtime, 0xb0, native_cc[i], value, &event));
    assert(!cp_runtime_process(&profile, &runtime, 0x90, 2, 127, &event));
    assert(!cp_runtime_process(&profile, &runtime, 0x93, 2, 127, &event));
    assert(!cp_profile_map_led(&profile, LED_MASTER_CUE, 0, 1, midi));
    /* MASTER remains an intentional PCM5102A software-volume adaptation. */
    (void)cp_runtime_process(&profile, &runtime, 0xb0, 3, 64, &event);
    assert(cp_runtime_process(&profile, &runtime, 0xb0, 0x23, 0, &event));
    assert(event.id == CTRL_ID_MASTER_VOLUME && event.value == 8192);
    for (unsigned d = 0; d < 2; ++d) {
        uint8_t status = (uint8_t)(0x91+d);
        assert(cp_runtime_process(&profile, &runtime, status, 8, 127, &event));
        assert(event.id == (d ? CTRL_ID_DECK2_JOG_TOUCH : CTRL_ID_DECK1_JOG_TOUCH));
        assert(cp_runtime_process(&profile, &runtime, status, 8, 0, &event));
        assert(event.value == 0);
    }
    /* Both parser and storage admission reject native capability on v2/v3. */
    for (unsigned version = 2; version <= 3; ++version) {
        memcpy(bad, blob, len); bad[4] = version;
        assert(cp_profile_parse(bad, len, &profile) == CP_ERR_BOUNDS);
        assert(controller_profile_meta_parse(bad, len, &meta) != ESP_OK);
    }
    memcpy(bad, blob, len); bad[20] &= ~(1u << 1); refresh_crc(bad, len);
    assert(cp_profile_parse(bad, len, &profile) == CP_ERR_BOUNDS);
    assert(controller_profile_meta_parse(bad, len, &meta) != ESP_OK);
    /* Live profile ownership is generic and disappears after clear. */
    controller_profile_runtime_init();
    assert(!controller_profile_runtime_native_headphones());
    assert(controller_profile_runtime_activate(blob, len, 0x06f8, 0xb12b));
    assert(controller_profile_runtime_native_headphones());
    assert(!controller_profile_runtime_filter_requires_smart_cfx());
    memcpy(bad, blob, len); bad[20] &= ~CP_PF_NATIVE_HEADPHONES; refresh_crc(bad, len);
    assert(controller_profile_runtime_activate(bad, len, 0x06f8, 0xb12b));
    assert(!controller_profile_runtime_native_headphones());
    assert(controller_profile_runtime_activate(blob, len, 0x06f8, 0xb12b));
    assert(controller_profile_runtime_native_headphones());
    controller_profile_runtime_clear();
    assert(!controller_profile_runtime_native_headphones());
    assert(controller_profile_runtime_filter_requires_smart_cfx());
    puts("PASS Hercules native profile: parser/storage, native-control isolation, PCM master, jog, activation/clear");
    return 0;
}
