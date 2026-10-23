#include "flx4_uac_packetizer.h"

#include <string.h>

bool controller_uac_pack_pcm(uint8_t *dst, size_t bytes, const int16_t *src,
                             size_t frames, uint8_t sample_bytes)
{
    if (!dst || !src || (sample_bytes != 2u && sample_bytes != 3u) ||
        frames > SIZE_MAX / (4u * sample_bytes) || bytes < frames * 4u * sample_bytes)
        return false;
    for (size_t i = 0; i < frames * 4u; ++i) {
        uint32_t sample = (uint32_t)((int32_t)src[i] * (sample_bytes == 3u ? 256 : 1));
        for (uint8_t b = 0; b < sample_bytes; ++b) {
            *dst++ = (uint8_t)(sample >> (8u * b));
        }
    }
    return true;
}

size_t controller_uac_pack24_frame_bytes(controller_uac_pack24_mode_t mode)
{
    return mode == CONTROLLER_UAC_PACK24_S16SHORT ? 8u : 12u;
}

bool controller_uac_pack_pcm24_mode(uint8_t *dst, size_t bytes, const int16_t *src,
                                    size_t frames, controller_uac_pack24_mode_t mode)
{
    if (!dst || !src || mode > CONTROLLER_UAC_PACK24_S16SHORT || frames > SIZE_MAX / 12u ||
        bytes < frames * controller_uac_pack24_frame_bytes(mode))
        return false;
    if (mode >= CONTROLLER_UAC_PACK24_S16PAD) {
        const bool swap = mode == CONTROLLER_UAC_PACK24_S16PAD_BE;
        for (size_t i = 0; i < frames * 4u; ++i) {
            const uint16_t bits = (uint16_t)src[i];
            *dst++ = (uint8_t)(swap ? bits >> 8 : bits & 0xffu);
            *dst++ = (uint8_t)(swap ? bits & 0xffu : bits >> 8);
        }
        if (mode != CONTROLLER_UAC_PACK24_S16SHORT) memset(dst, 0, frames * 4u);
        return true;
    }
    for (size_t i = 0; i < frames * 4u; ++i) {
        const uint16_t bits = (uint16_t)src[i];
        const uint8_t lo = (uint8_t)(bits & 0xffu);
        const uint8_t hi = (uint8_t)(bits >> 8);
        if (mode == CONTROLLER_UAC_PACK24_BE) {
            *dst++ = hi; *dst++ = lo; *dst++ = 0u;
        } else if (mode == CONTROLLER_UAC_PACK24_LOW) {
            *dst++ = lo; *dst++ = hi; *dst++ = (hi & 0x80u) ? 0xffu : 0u;
        } else {
            *dst++ = 0u; *dst++ = lo; *dst++ = hi;
        }
    }
    return true;
}

void flx4_uac_packetizer_init(flx4_uac_packetizer_t *p,
                              uint32_t sample_rate,
                              uint8_t channels,
                              uint8_t bytes_per_sample)
{
    if (!p) {
        return;
    }

    p->sample_rate = sample_rate;
    p->channels = channels;
    p->bytes_per_sample = bytes_per_sample;
    p->frame_accum = 0u;
}

uint16_t flx4_uac_packetizer_next_frames(flx4_uac_packetizer_t *p)
{
    if (!p || p->sample_rate == 0u) {
        return 0u;
    }

    p->frame_accum += p->sample_rate;
    const uint16_t frames = (uint16_t)(p->frame_accum / 1000u);
    p->frame_accum -= (uint32_t)frames * 1000u;
    return frames;
}

size_t flx4_uac_packetizer_next_bytes(flx4_uac_packetizer_t *p)
{
    const uint16_t frames = flx4_uac_packetizer_next_frames(p);
    if (!p) {
        return 0u;
    }

    return (size_t)frames * (size_t)p->channels * (size_t)p->bytes_per_sample;
}
