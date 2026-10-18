#pragma once
#include "audio_engine.h"

/* Observation-time diagnostics only. Call without the audio state mutex.
 * Only per-heap free/low-water counters are sampled. Largest blocks and walk
 * timings are unavailable (zero in C, null in HTTP), on every target: an
 * unbounded IRQ-masking TLSF walk must not disturb UAC silence/audio. */
void audio_engine_snapshot_memory(audio_engine_diagnostics_snapshot_t *snapshot);
