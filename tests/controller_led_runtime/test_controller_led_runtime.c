#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "controller_led_runtime.h"
#include "controller_usb_host.h"
#include "controller_profile.h"
#include "controller_profile_runtime.h"
#include "control_link.h"
#include "flx4_led_snapshot.h"

static unsigned s_checks;
static esp_err_t s_send_result = ESP_OK;
static uint8_t s_last_sent[4];
static uint32_t s_send_calls;
static bool s_identity_available;
static uint32_t s_output_generation = 1u;
static uint32_t s_accepted_sends;
static bool s_reconnect_during_identity;
static uint32_t s_output_slots = UINT32_MAX;

#define CHECK(expr) do { \
    s_checks++; \
    if (!(expr)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr); \
        exit(1); \
    } \
} while (0)

uint32_t controller_usb_host_output_generation(void)
{
    return s_output_generation;
}

esp_err_t controller_usb_host_send_packet_for_generation(
    const uint8_t packet[4], uint32_t generation)
{
    s_send_calls++;
    if (generation != s_output_generation) return ESP_ERR_INVALID_STATE;
    if (s_output_slots == 0u) return ESP_ERR_TIMEOUT;
    memcpy(s_last_sent, packet, sizeof(s_last_sent));
    if (s_send_result == ESP_OK) {
        s_accepted_sends++;
        if (s_output_slots != UINT32_MAX) s_output_slots--;
    }
    return s_send_result;
}

esp_err_t controller_usb_host_send_packet(const uint8_t packet[4])
{
    return controller_usb_host_send_packet_for_generation(
        packet, controller_usb_host_output_generation());
}

bool controller_usb_host_get_identity(controller_usb_identity_t *identity)
{
    if (!identity || !s_identity_available) return false;
    if (s_reconnect_during_identity) {
        s_reconnect_during_identity = false;
        s_output_generation++;
    }
    memset(identity, 0, sizeof(*identity));
    identity->vid = 0x2B73u;
    identity->pid = 0x0045u;
    return true;
}

static void wr_u16(uint8_t *p, uint16_t value)
{
    p[0] = (uint8_t)(value & 0xFFu);
    p[1] = (uint8_t)(value >> 8u);
}

static void wr_u32(uint8_t *p, uint32_t value)
{
    p[0] = (uint8_t)(value & 0xFFu);
    p[1] = (uint8_t)((value >> 8u) & 0xFFu);
    p[2] = (uint8_t)((value >> 16u) & 0xFFu);
    p[3] = (uint8_t)(value >> 24u);
}

static void write_output(uint8_t *entry, uint8_t led, uint8_t status,
                         uint8_t data1, uint8_t off_value,
                         uint8_t on_value, uint8_t blink_value)
{
    entry[0] = led;
    entry[1] = CP_DECK_ANY;
    entry[2] = CP_OUT_NOTE_ONOFF;
    entry[3] = status;
    entry[4] = data1;
    entry[5] = off_value;
    entry[6] = on_value;
    entry[7] = blink_value;
}

static size_t build_profile(uint8_t blob[CP_HEADER_SIZE +
                                         2u * CP_OUTPUT_ENTRY_SIZE])
{
    const size_t total = CP_HEADER_SIZE + 2u * CP_OUTPUT_ENTRY_SIZE;
    memset(blob, 0, total);
    memcpy(blob, CP_MAGIC, 4u);
    wr_u16(blob + 4, CP_VERSION);
    wr_u16(blob + 6, CP_HEADER_SIZE);
    wr_u32(blob + 8, (uint32_t)total);
    wr_u16(blob + 16, 0x1234u);
    wr_u16(blob + 18, 0x5678u);
    wr_u32(blob + 20, CP_PF_LED_FEEDBACK);
    wr_u16(blob + 24, 0u);
    wr_u16(blob + 26, 2u);
    blob[28] = 0u;
    blob[29] = 2u;

    write_output(blob + CP_HEADER_SIZE,
                 LED_CUE, 0x92u, 0x33u, 0x11u, 0x66u, 0x44u);
    write_output(blob + CP_HEADER_SIZE + CP_OUTPUT_ENTRY_SIZE,
                 LED_TRACK_LOAD_DECK1, 0x92u, 0x55u,
                 0x01u, 0x02u, 0x03u);
    wr_u32(blob + 12, cp_crc32(blob + 16, total - 16u));
    return total;
}

static esp_err_t snapshot_send(led_id_t led, uint8_t state,
                               uint8_t deck, void *ctx)
{
    (void)ctx;
    return controller_led_runtime_send((uint8_t)led, state, deck);
}

static void test_snapshot_headroom_and_full_queue_retry(void)
{
    controller_profile_runtime_clear();
    controller_led_runtime_set_builtin_flx4_enabled(true);
    flx4_led_publisher_t publisher;
    flx4_led_snapshot_input_t input = {0};
    flx4_led_publisher_init(&publisher);
    s_output_slots = 256u; /* Current product MIDI OUT queue depth. */
    const uint32_t before = s_accepted_sends;
    CHECK(flx4_led_publisher_publish(&publisher, &input, true,
                                    snapshot_send, NULL) == ESP_OK);
    CHECK(s_accepted_sends - before == 162u); /* Includes shifted pad banks. */
    CHECK(s_output_slots == 94u);
    CHECK(controller_led_runtime_send(LED_VU_METER, 64u, 0u) == ESP_OK);
    CHECK(controller_led_runtime_send(LED_VU_METER, 64u, 1u) == ESP_OK);
    CHECK(controller_led_runtime_send(LED_TRACK_LOAD_DECK1, 1u, 0u) == ESP_OK);
    CHECK(controller_led_runtime_send(LED_TRACK_LOAD_DECK2, 1u, 1u) == ESP_OK);
    CHECK(s_output_slots == 90u);
    CHECK(flx4_led_publisher_publish(&publisher, &input, false,
                                    snapshot_send, NULL) == ESP_OK);
    CHECK(s_output_slots == 90u); /* Unchanged values are suppressed. */

    s_output_slots = 0u;
    input.play[0] = 1u;
    CHECK(flx4_led_publisher_publish(&publisher, &input, false,
                                    snapshot_send, NULL) == ESP_ERR_TIMEOUT);
    s_output_slots = 1u;
    CHECK(flx4_led_publisher_publish(&publisher, &input, false,
                                    snapshot_send, NULL) == ESP_OK);
    CHECK(s_output_slots == 0u);
    CHECK(s_last_sent[1] == 0x90u && s_last_sent[2] == 0x0Bu &&
          s_last_sent[3] == 0x7Fu);
    s_output_slots = UINT32_MAX;
}

int main(void)
{
    controller_profile_runtime_init();

    uint8_t packet[4] = {0};
    CHECK(!controller_led_runtime_build_packet(
        LED_PLAY, 1u, CTRL_DECK_1, packet));
    controller_led_runtime_set_builtin_flx4_enabled(true);
    CHECK(controller_led_runtime_build_packet(
        LED_PLAY, 1u, CTRL_DECK_1, packet));
    CHECK(packet[0] == 0x09u);
    CHECK(packet[1] == 0x90u);
    CHECK(packet[2] == 0x0Bu);
    CHECK(packet[3] == 0x7Fu);

    uint8_t blob[CP_HEADER_SIZE + 2u * CP_OUTPUT_ENTRY_SIZE];
    const size_t len = build_profile(blob);
    CHECK(controller_profile_runtime_activate(blob, len,
                                              0x1234u, 0x5678u));
    controller_led_runtime_set_builtin_flx4_enabled(false);

    CHECK(controller_led_runtime_build_packet(
        LED_CUE, 1u, CTRL_DECK_1, packet));
    CHECK(packet[0] == 0x09u);
    CHECK(packet[1] == 0x92u);
    CHECK(packet[2] == 0x33u);
    CHECK(packet[3] == 0x66u);

    CHECK(!controller_led_runtime_build_packet(
        LED_PLAY, 1u, CTRL_DECK_1, packet));

    CHECK(controller_led_runtime_build_packet(
        LED_TRACK_LOAD_DECK1, 1u, CTRL_DECK_1, packet));
    CHECK(packet[1] == 0x92u);
    CHECK(packet[2] == 0x55u);
    CHECK(packet[3] == 0x02u);

    CHECK(!controller_led_runtime_build_packet(
        LED_VU_METER, 63u, CTRL_DECK_2, packet));

    CHECK(!controller_led_runtime_build_packet(
        0xFFu, 1u, CTRL_DECK_1, packet));
    CHECK(!controller_led_runtime_build_packet(
        LED_PLAY, 1u, 0xFFu, packet));

    s_send_result = ESP_OK;
    CHECK(controller_led_runtime_send(
        LED_CUE, 2u, CTRL_DECK_1) == ESP_OK);
    CHECK(s_send_calls == 1u);
    CHECK(s_last_sent[1] == 0x92u);
    CHECK(s_last_sent[3] == 0x44u);

    s_send_result = ESP_ERR_TIMEOUT;
    CHECK(controller_led_runtime_send(
        LED_CUE, 1u, CTRL_DECK_1) == ESP_ERR_TIMEOUT);
    CHECK(s_send_calls == 2u);

    s_identity_available = true;
    s_send_result = ESP_OK;
    controller_led_runtime_set_builtin_flx4_enabled(true);
    CHECK(controller_led_runtime_send(
        LED_HOT_CUE_PAD_1, 1u, CTRL_DECK_2) == ESP_OK);
    CHECK(s_send_calls == 4u);
    CHECK(s_last_sent[1] == 0x9Au);
    CHECK(s_last_sent[2] == 0x00u);
    CHECK(s_last_sent[3] == 0x7Fu);

    controller_led_runtime_diagnostics_t diagnostics;
    controller_led_runtime_get_diagnostics(&diagnostics);
    CHECK(diagnostics.dynamic_packets == 4u);
    CHECK(diagnostics.builtin_packets == 2u);
    CHECK(diagnostics.builtin_fallbacks == 1u);
    CHECK(diagnostics.unsupported == 5u);
    CHECK(diagnostics.send_failures == 1u);

    /* Reconnect between primary and shifted mirror. The old bank address
     * must not be sent through the replacement connection's output gate. */
    const uint32_t accepted_before = s_accepted_sends;
    s_reconnect_during_identity = true;
    CHECK(controller_led_runtime_send(
        LED_HOT_CUE_PAD_1, 1u, CTRL_DECK_2) == ESP_ERR_INVALID_STATE);
    CHECK(s_accepted_sends == accepted_before + 1u);
    CHECK(controller_led_runtime_send(
        LED_HOT_CUE_PAD_1, 1u, CTRL_DECK_2) == ESP_OK);
    CHECK(s_accepted_sends == accepted_before + 3u);
    controller_led_runtime_get_diagnostics(&diagnostics);
    CHECK(diagnostics.send_failures == 2u);

    test_snapshot_headroom_and_full_queue_retry();

    printf("PASS P4 local controller LED runtime\n");
    printf("CHECKS=%u SENDS=%u\n", s_checks, s_send_calls);
    return 0;
}
