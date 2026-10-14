#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint8_t interface_num;
    uint8_t alternate_setting;
    uint8_t endpoint_addr;
    uint16_t max_packet_size;
    uint8_t channels;
    uint8_t bits_per_sample;
    uint8_t bytes_per_sample;
    uint32_t sample_rates[8];
    uint8_t sample_rate_count;
    uint8_t interval;
    uint16_t service_interval_us; /* selected cadence; interval retains raw bInterval */
    uint8_t sync_type;
    bool requires_feedback;
    bool pcm;
    bool pcm_explicit;
    bool frequency_control;
    bool uac2;
    uint8_t control_interface;
    uint8_t clock_source;
    uint8_t feedback_endpoint;
    uint8_t feedback_interval;
    uint8_t sync_address;
    uint8_t feedback_refresh;
} flx4_uac_playback_format_t;

typedef struct {
    flx4_uac_playback_format_t formats[8];
    uint8_t format_count;
} flx4_uac_descriptor_result_t;

bool flx4_uac_parse_playback_formats(const uint8_t *config_desc,
                                     size_t config_len,
                                     flx4_uac_descriptor_result_t *out);

bool flx4_uac_select_preferred_format(const flx4_uac_descriptor_result_t *result,
                                      flx4_uac_playback_format_t *out);

/* Full-speed UAC1 PCM, one packet/ms; explicit 3-byte feedback is validated. */
bool flx4_uac_format_supports_rate(const flx4_uac_playback_format_t *format,
                                  uint32_t rate);
bool flx4_uac_format_supports_bus_rate(const flx4_uac_playback_format_t *format,
                                      uint32_t rate, bool high_speed);

bool flx4_uac_feedback_q16(const flx4_uac_playback_format_t *format,
                           const uint8_t *data, size_t length, uint32_t rate,
                           uint32_t *frames_q16);

#ifdef __cplusplus
}
#endif
