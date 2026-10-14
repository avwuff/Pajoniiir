/* SPDX-License-Identifier: Apache-2.0 */
#pragma once
#include <stdbool.h>
#include <stdint.h>

/* Exclusive end may equal EOF. Until a length is known, ordered uint32 ranges
 * retain their historical behavior; later callers use the decoded length. */
static inline bool audio_loop_range_valid(uint32_t start_ms, uint32_t end_ms,
                                          uint32_t duration_ms)
{
    return end_ms > start_ms && (!duration_ms || end_ms <= duration_ms);
}
