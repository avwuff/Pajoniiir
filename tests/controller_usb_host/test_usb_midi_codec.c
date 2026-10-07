#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "usb_midi_codec.h"

static unsigned s_tests_run;
#define CHECK(expr) do { \
    s_tests_run++; \
    if (!(expr)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr); \
        exit(1); \
    } \
} while (0)

static const uint8_t s_bulk[] = {
    9, 2, 41, 0, 2, 1, 0, 0x80, 50,
    9, 4, 0, 0, 0, 0xFF, 0, 0, 0,
    9, 4, 3, 1, 2, 0x01, 0x03, 0, 0,
    7, 5, 0x82, 0x02, 64, 0, 0,
    7, 5, 0x02, 0x02, 64, 0, 0,
};

static const uint8_t s_interrupt[] = {
    9, 2, 32, 0, 1, 1, 0, 0x80, 50,
    9, 4, 7, 0, 2, 0x01, 0x03, 0, 0,
    7, 5, 0x81, 0x03, 16, 0, 1,
    7, 5, 0x01, 0x03, 16, 0, 1,
};

int main(void)
{
    usb_midi_endpoints_t endpoints;
    CHECK(usb_midi_find_streaming_endpoints(s_bulk, sizeof(s_bulk),
                                            &endpoints));
    CHECK(endpoints.interface_num == 3u);
    CHECK(endpoints.alternate_setting == 1u);
    CHECK(endpoints.in_ep_addr == 0x82u);
    CHECK(endpoints.out_ep_addr == 0x02u);
    CHECK(endpoints.in_ep_mps == 64u);
    CHECK(endpoints.out_ep_mps == 64u);
    CHECK(usb_midi_config_has_interface_class(s_bulk, sizeof(s_bulk),
                                              0x01u, 0x03u));
    CHECK(!usb_midi_config_has_interface_class(s_bulk, sizeof(s_bulk),
                                               0x08u, 0xFFu));

    CHECK(usb_midi_find_streaming_endpoints(s_interrupt,
                                            sizeof(s_interrupt),
                                            &endpoints));
    CHECK(endpoints.interface_num == 7u);
    CHECK(endpoints.in_ep_addr == 0x81u);
    CHECK(endpoints.out_ep_addr == 0x01u);
    CHECK(endpoints.in_ep_mps == 16u);
    CHECK(endpoints.out_ep_mps == 16u);

    uint8_t malformed[sizeof(s_bulk)];
    memcpy(malformed, s_bulk, sizeof(malformed));
    malformed[2] = 42u;
    CHECK(!usb_midi_find_streaming_endpoints(malformed,
                                             sizeof(malformed),
                                             &endpoints));
    memcpy(malformed, s_bulk, sizeof(malformed));
    malformed[9] = 0u;
    CHECK(!usb_midi_find_streaming_endpoints(malformed,
                                             sizeof(malformed),
                                             &endpoints));
    memcpy(malformed, s_bulk, sizeof(malformed));
    malformed[34] = 0x83u;
    CHECK(!usb_midi_find_streaming_endpoints(malformed,
                                             sizeof(malformed),
                                             &endpoints));

    const uint8_t note_on[4] = {0x09, 0x90, 0x0B, 0x7F};
    usb_midi_message_t message;
    CHECK(usb_midi_parse_event_packet(note_on, &message));
    CHECK(message.cable == 0u);
    CHECK(message.cin == 9u);
    CHECK(message.len == 3u);
    CHECK(message.status == 0x90u);
    CHECK(message.data1 == 0x0Bu);
    CHECK(message.data2 == 0x7Fu);

    const uint8_t reserved[4] = {0, 0, 0, 0};
    CHECK(!usb_midi_parse_event_packet(reserved, &message));
    CHECK(!usb_midi_parse_event_packet(NULL, &message));
    CHECK(!usb_midi_parse_event_packet(note_on, NULL));

    usb_midi_probe_snapshot_t snapshot;
    CHECK(!usb_midi_probe_snapshot_get(NULL));
    CHECK(!usb_midi_probe_snapshot_get(&snapshot));
    uint8_t device[18] = {18, 1};
    usb_midi_probe_snapshot_capture(device, s_bulk, sizeof(s_bulk), 7, 1, 1, true);
    CHECK(usb_midi_probe_snapshot_get(&snapshot));
    CHECK(snapshot.address == 7 && snapshot.speed == 1 && snapshot.direct_root);
    CHECK(snapshot.total_length == sizeof(s_bulk) && snapshot.captured_length == sizeof(s_bulk));
    CHECK(memcmp(snapshot.config, s_bulk, sizeof(s_bulk)) == 0);
    uint8_t oversized[USB_MIDI_PROBE_CONFIG_MAX + 5u];
    memset(oversized, 0x55, sizeof(oversized));
    usb_midi_probe_snapshot_capture(device, oversized, sizeof(oversized), 8, 2, 0, false);
    CHECK(usb_midi_probe_snapshot_get(&snapshot));
    CHECK(snapshot.total_length == sizeof(oversized) && snapshot.captured_length == USB_MIDI_PROBE_CONFIG_MAX);
    CHECK(snapshot.config[USB_MIDI_PROBE_CONFIG_MAX - 1] == 0x55);
    usb_midi_probe_snapshot_capture(NULL, s_bulk, sizeof(s_bulk), 1, 0, 0, false);
    CHECK(usb_midi_probe_snapshot_get(&snapshot) && snapshot.address == 8);
    usb_midi_probe_snapshot_capture(device, s_bulk, sizeof(s_bulk), 9, 1, 1, true);
    CHECK(usb_midi_probe_snapshot_get(&snapshot));
    CHECK(snapshot.config[sizeof(s_bulk)] == 0 && snapshot.address == 9);
    printf("PASS reusable controller USB-MIDI codec and bounded probe snapshots\n");
    printf("TESTS_RUN=%u\n", s_tests_run);
    return 0;
}
