#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

/* Pack four signed 16-bit engine channels to 16-bit or packed 24-bit PCM.
 * 24-bit output multiplies by 256 (full amplitude), without signed-shift UB. */

#ifdef __cplusplus
extern "C" {
#endif

bool controller_uac_pack_pcm(uint8_t *dst, size_t bytes, const int16_t *src,
                             size_t frames, uint8_t sample_bytes);

/* Diagnostic packed 24-bit layouts for one 16-bit sample s (bytes in wire order):
 * LE  00 lo hi  UAC1 Type I little-endian, left-justified (production layout)
 * BE  hi lo 00  big-endian, left-justified
 * LOW lo hi ss  little-endian, right-justified, sign-extended (-48 dB) */
typedef enum {
    CONTROLLER_UAC_PACK24_LE = 0,
    CONTROLLER_UAC_PACK24_BE = 1,
    CONTROLLER_UAC_PACK24_LOW = 2,
} controller_uac_pack24_mode_t;

bool controller_uac_pack_pcm24_mode(uint8_t *dst, size_t bytes, const int16_t *src,
                                    size_t frames, controller_uac_pack24_mode_t mode);
/* Four channels, signed 32-bit little-endian, 16-bit source left-justified. */
bool controller_uac_pack_pcm32(uint8_t *dst, size_t bytes, const int16_t *src, size_t frames);

typedef struct {
    uint32_t sample_rate;
    uint8_t channels;
    uint8_t bytes_per_sample;
    uint32_t frame_accum;
} flx4_uac_packetizer_t;

void flx4_uac_packetizer_init(flx4_uac_packetizer_t *p,
                              uint32_t sample_rate,
                              uint8_t channels,
                              uint8_t bytes_per_sample);

uint16_t flx4_uac_packetizer_next_frames(flx4_uac_packetizer_t *p);
size_t flx4_uac_packetizer_next_bytes(flx4_uac_packetizer_t *p);

#ifdef __cplusplus
}
#endif
