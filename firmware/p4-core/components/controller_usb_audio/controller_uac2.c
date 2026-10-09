/* SPDX-License-Identifier: Apache-2.0 */
#include "controller_uac2.h"
#include <string.h>

static uint16_t r16(const uint8_t *p) { return p[0] | (uint16_t)p[1] << 8; }
static uint32_t r32(const uint8_t *p)
{ return r16(p) | (uint32_t)r16(p + 2) << 16; }

bool controller_uac2_parse(const uint8_t *data, size_t length,
                           flx4_uac_playback_format_t *out)
{
    if (!data || !out || length < 9 || data[0] != 9 || data[1] != 2) return false;
    memset(out, 0, sizeof(*out));
    size_t total = r16(data + 2);
    if (total < 9 || total > length) return false;
    struct { uint8_t id, interface; } clocks[8];
    struct { uint8_t id, clock, interface; } terminals[8];
    unsigned nc = 0, nt = 0;
    uint8_t interface = 0, subclass = 0, protocol = 0;
    /* Resolve AS terminal -> direct programmable clock in the same AC function. */
    for (size_t off = 9; off < total;) {
        uint8_t n = data[off];
        if (n == 0) {
            for (size_t i = off; i < total; ++i) if (data[i]) return false;
            break; /* Captured S2 has an all-zero tail. */
        }
        if (n < 2 || n > total - off) return false;
        const uint8_t *p = data + off;
        if (p[1] == 4) {
            if (n < 9) return false;
            interface = p[2]; subclass = p[5] == 1 ? p[6] : 0; protocol = p[7];
        } else if (p[1] == 0x24 && subclass == 1 && protocol == 0x20) {
            if (n < 3) return false;
            if (p[2] == 0x0a) {
                if (n < 8 || nc == 8 || (p[5] & 3) != 3 || ((p[5] >> 2) & 3) != 1) return false;
                for (unsigned i = 0; i < nc; ++i)
                    if (clocks[i].id == p[3] && clocks[i].interface == interface) return false;
                clocks[nc].id = p[3]; clocks[nc++].interface = interface;
            } else if (p[2] == 2) {
                if (n < 17 || nt == 8) return false;
                for (unsigned i = 0; i < nt; ++i)
                    if (terminals[i].id == p[3]) return false;
                terminals[nt].id = p[3]; terminals[nt].clock = p[7];
                terminals[nt++].interface = interface;
            }
        }
        off += n;
    }
    flx4_uac_playback_format_t current = {0}, selected = {0};
    uint8_t terminal = 0;
    bool as = false, found = false;
    for (size_t off = 9; off <= total;) {
        const uint8_t *p = data + off;
        bool boundary = off == total || (off < total && (p[0] == 0 || p[1] == 4));
        if (boundary && as && current.endpoint_addr) {
            for (unsigned t = 0; t < nt; ++t) if (terminals[t].id == terminal) {
                for (unsigned c = 0; c < nc; ++c)
                    if (clocks[c].id == terminals[t].clock &&
                        clocks[c].interface == terminals[t].interface) {
                        current.clock_source = clocks[c].id;
                        current.control_interface = clocks[c].interface;
                    }
            }
            if (!current.clock_source || !current.pcm_explicit || !current.pcm ||
                current.channels != 4 || current.interval != 1 ||
                current.sync_type != 1 || !current.feedback_endpoint ||
                current.feedback_interval < 1 || current.feedback_interval > 7 ||
                !((current.bytes_per_sample == 4 && current.bits_per_sample == 24) ||
                  (current.bytes_per_sample == 2 && current.bits_per_sample == 16)) ||
                current.max_packet_size < 7u * 4u * current.bytes_per_sample ||
                current.max_packet_size > 1024 || found) return false;
            current.uac2 = true; current.requires_feedback = true;
            current.frequency_control = true;
            selected = current; found = true;
        }
        if (off == total || p[0] == 0) break;
        uint8_t n = p[0]; /* Lengths were validated by the complete first pass. */
        if (p[1] == 4) {
            memset(&current, 0, sizeof(current)); terminal = 0;
            as = p[5] == 1 && p[6] == 2 && p[7] == 0x20 && p[3] != 0;
            current.interface_num = p[2]; current.alternate_setting = p[3];
        } else if (as && p[1] == 0x24) {
            if (n < 3) return false;
            if (p[2] == 1) {
                if (n < 16 || p[5] != 1) return false;
                terminal = p[3]; current.pcm_explicit = true;
                current.pcm = r32(p + 6) == 1; current.channels = p[10];
            } else if (p[2] == 2) {
                if (n < 6 || p[3] != 1) return false;
                current.bytes_per_sample = p[4]; current.bits_per_sample = p[5];
            }
        } else if (as && p[1] == 5) {
            if (n < 7 || (p[3] & 3) != 1 || (r16(p + 4) & 0x1800)) return false;
            if (!(p[2] & 0x80)) {
                if (current.endpoint_addr || ((p[3] >> 4) & 3)) return false;
                current.endpoint_addr = p[2]; current.max_packet_size = r16(p + 4);
                current.interval = p[6]; current.sync_type = (p[3] >> 2) & 3;
            } else if (((p[3] >> 4) & 3) == 1) {
                if (current.feedback_endpoint || r16(p + 4) != 4) return false;
                current.feedback_endpoint = p[2]; current.feedback_interval = p[6];
            }
        }
        off += n;
    }
    if (found) *out = selected;
    return found;
}

bool controller_uac2_range_supports(const uint8_t *data, size_t length, uint32_t rate)
{
    if (!data || length < 2 || !rate) return false;
    unsigned count = r16(data);
    if (!count || count > 8 || length != 2u + 12u * count) return false;
    bool supported = false;
    for (unsigned i = 0; i < count; ++i) {
        const uint8_t *p = data + 2 + 12 * i;
        uint32_t min = r32(p), max = r32(p + 4), step = r32(p + 8);
        if (!min || min > max) return false;
        if (rate >= min && rate <= max && (!step || (rate - min) % step == 0)) supported = true;
    }
    return supported;
}

bool controller_uac2_feedback(const uint8_t *data, size_t length, uint32_t rate,
                              uint32_t *frames_q16)
{
    if (!data || !frames_q16 || length != 4 || (rate != 48000 && rate != 44100)) return false;
    uint32_t v = r32(data), nominal = (uint32_t)(((uint64_t)rate << 16) / 8000u);
    if (v < nominal - nominal / 20 || v > nominal + nominal / 20) return false;
    *frames_q16 = v;
    return true;
}

bool controller_uac2_pack_pcm(uint8_t *dst, size_t bytes, const int16_t *src,
                              size_t frames, uint8_t subslot, uint8_t bits)
{
    if (!dst || !src || !((subslot == 4 && bits == 24) || (subslot == 2 && bits == 16)) ||
        frames > SIZE_MAX / (4u * subslot) || bytes < frames * 4u * subslot) return false;
    /* USB Audio Type I PCM is left-justified in the subslot. 24-in-32 must
     * preserve the same full-scale amplitude as packed 24-bit, not attenuate. */
    for (size_t i = 0; i < frames * 4u; ++i) {
        uint32_t sample = (uint32_t)((int32_t)src[i] * (subslot == 4 ? 65536 : 1));
        for (uint8_t b = 0; b < subslot; ++b) *dst++ = (uint8_t)(sample >> (8u * b));
    }
    return true;
}
