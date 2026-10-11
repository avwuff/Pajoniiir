/* SPDX-License-Identifier: Apache-2.0 */
#pragma once
#include "flx4_uac_descriptors.h"

/* Bounded direct-clock UAC2 HS candidate: 4ch PCM, asynchronous OUT,
 * explicit 16.16 feedback, single transaction/microframe at interval 1.
 * Rejects clock selectors, implicit feedback and unsupported subslots. */
bool controller_uac2_parse(const uint8_t *data, size_t length,
                           flx4_uac_playback_format_t *out);
bool controller_uac2_range_supports(const uint8_t *data, size_t length, uint32_t rate);
bool controller_uac2_feedback(const uint8_t *data, size_t length, uint32_t rate,
                              uint32_t *frames_q16);
bool controller_uac2_pack_pcm(uint8_t *dst, size_t bytes, const int16_t *src,
                              size_t frames, uint8_t subslot, uint8_t bits);
