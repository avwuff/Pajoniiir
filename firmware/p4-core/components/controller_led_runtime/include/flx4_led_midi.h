#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

bool flx4_led_midi_build_packet(uint8_t led,
                                uint8_t state,
                                uint8_t deck,
                                uint8_t packet[4]);
bool flx4_led_midi_build_shifted_mirror_packet(uint8_t led,
                                               uint8_t state,
                                               uint8_t deck,
                                               uint8_t packet[4]);
bool flx4_led_midi_builtin_authoritative(uint8_t led);

#ifdef __cplusplus
}
#endif
