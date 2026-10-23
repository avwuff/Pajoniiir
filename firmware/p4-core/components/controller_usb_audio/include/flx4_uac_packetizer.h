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
 * LOW lo hi ss  little-endian, right-justified, sign-extended (-48 dB)
 * S16PAD keeps the 12-byte-per-frame packet length but writes 16-bit LE
 * 4-channel frames (8 bytes each) from the start and zero-fills the tail.
 * S16PAD_BE is the same with each 16-bit sample byte-swapped. S16SHORT sends
 * only the 16-bit LE frames, so the packet is 8 bytes per frame. */
typedef enum {
    CONTROLLER_UAC_PACK24_LE = 0,
    CONTROLLER_UAC_PACK24_BE = 1,
    CONTROLLER_UAC_PACK24_LOW = 2,
    CONTROLLER_UAC_PACK24_S16PAD = 3,
    CONTROLLER_UAC_PACK24_S16PAD_BE = 4,
    CONTROLLER_UAC_PACK24_S16SHORT = 5,
} controller_uac_pack24_mode_t;

/* Wire bytes per 4-channel frame for a pack mode. */
size_t controller_uac_pack24_frame_bytes(controller_uac_pack24_mode_t mode);

bool controller_uac_pack_pcm24_mode(uint8_t *dst, size_t bytes, const int16_t *src,
                                    size_t frames, controller_uac_pack24_mode_t mode);
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
