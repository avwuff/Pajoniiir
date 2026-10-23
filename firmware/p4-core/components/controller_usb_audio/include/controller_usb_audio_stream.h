#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "usb/usb_host.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint64_t submitted_blocks;
    uint64_t dropped_blocks;
    uint64_t submitted_frames;
    uint32_t ring_queued_frames;
    uint32_t ring_capacity_frames;
    uint32_t ring_high_water_frames;
    uint64_t overrun_frames;
    uint64_t underrun_frames;
    uint64_t clock_trimmed_frames;
    uint64_t clock_duplicated_frames;
    uint32_t config_failures;
    uint32_t transfer_failures;
    uint32_t packet_failures;
    uint64_t packet_lost_frames;
    uint32_t stream_epoch;
    bool claimed;
    bool configuring;
    bool streaming;
    bool faulted;
    bool configuration_fault;
    uint8_t fault_control_step;
    int32_t last_control_status;
    int32_t last_control_bytes;
    uint32_t last_control_word;
    uint32_t clock_readback_retries;
    uint32_t clock_reported_rate;
    /* Startup: 1 feedback alloc, 2 feedback submit, 3 ISO alloc, 4 ISO submit,
     * 5 interface claim (index is the interface number). */
    uint8_t prime_fault_stage;
    uint8_t prime_fault_index;
    int32_t prime_fault_code;
    uint32_t sample_rate;
    uint8_t bits_per_sample;
    bool consumer_paced;
    bool high_speed;
    uint8_t subslot_bytes;
    uint32_t feedback_updates;
    uint32_t feedback_invalid;
    uint8_t pack24_mode;
    uint8_t test_signal;
} controller_usb_audio_stream_stats_t;

/* Test signals replace engine audio inside the packetizer, at -12 dBFS.
 * TONE is 440 Hz on channels 1/3 and 660 Hz on 2/4 (MAIN L/R, CUE L/R).
 * SILENCE sends digital zero on every channel. CH1..CH4 carry 440 Hz on that
 * one wire channel and zero on the other three. */
typedef enum {
    CONTROLLER_UAC_TEST_OFF = 0,
    CONTROLLER_UAC_TEST_TONE = 1,
    CONTROLLER_UAC_TEST_SILENCE = 2,
    CONTROLLER_UAC_TEST_CH1 = 3,
    CONTROLLER_UAC_TEST_CH2 = 4,
    CONTROLLER_UAC_TEST_CH3 = 5,
    CONTROLLER_UAC_TEST_CH4 = 6,
} controller_uac_test_signal_t;

/* Diagnostics, safe while streaming; both reset to production at each start.
 * pack24_mode is a controller_uac_pack24_mode_t and applies only to packed
 * 24-bit UAC1 streams. signal is a controller_uac_test_signal_t. */
esp_err_t controller_usb_audio_stream_set_pack24_mode(uint8_t mode);
esp_err_t controller_usb_audio_stream_set_test_signal(uint8_t signal);

/* Owner task only; only while quiesced. FLX4 preserves its qualified format/gain. */
esp_err_t controller_usb_audio_stream_set_policy(bool qualified_flx4, bool consumer_paced,
                                                 bool high_speed);
esp_err_t controller_usb_audio_stream_set_pacing(bool consumer_paced);
bool controller_usb_audio_stream_consumer_paced(void);
bool controller_usb_audio_stream_available(void);
bool controller_usb_audio_stream_needs_recovery(void);
/* HS configuration rejection may stop audio while preserving MIDI ownership. */
bool controller_usb_audio_stream_fault_isolatable(void);
/* Producer waits for actual ring free space, bounded and cancellation-aware. */
esp_err_t controller_usb_audio_stream_wait_room(uint32_t source_frames,
                                               uint32_t source_rate, uint32_t timeout_ms);

esp_err_t controller_usb_audio_stream_start(
    usb_host_client_handle_t client,
    usb_device_handle_t device,
    const uint8_t *config_descriptor,
    size_t config_descriptor_length,
    TaskHandle_t owner_task,
    UBaseType_t active_priority,
    UBaseType_t transition_priority);
void controller_usb_audio_stream_request_stop(bool device_gone);
bool controller_usb_audio_stream_poll_cleanup(void);
bool controller_usb_audio_stream_is_quiesced(void);

esp_err_t controller_usb_audio_stream_write(const int16_t *master_samples,
                                            const int16_t *headphone_samples,
                                            size_t frame_count,
                                            uint32_t source_sample_rate);
void controller_usb_audio_stream_get_stats(
    controller_usb_audio_stream_stats_t *out_stats);

#ifdef __cplusplus
}
#endif

/* SPDX-License-Identifier: Apache-2.0 */
