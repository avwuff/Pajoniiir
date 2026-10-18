#include "audio_engine_memory.h"
#include "esp_heap_caps.h"
#include "esp_system.h"

void audio_engine_snapshot_memory(audio_engine_diagnostics_snapshot_t *out)
{
    if (!out) return;
    /* Free/low-water queries read counters per registered heap. Largest-block
     * queries instead walk TLSF blocks under allocator locks that mask IRQs.
     * This observation path runs during playback and UAC idle silence on FS
     * as well as HS targets: never walk any pool here. */
    out->heap_free = esp_get_free_heap_size();
    out->internal_free = heap_caps_get_free_size(MALLOC_CAP_INTERNAL);
    out->internal_min_free = heap_caps_get_minimum_free_size(MALLOC_CAP_INTERNAL);
    const uint32_t dma_caps = MALLOC_CAP_INTERNAL | MALLOC_CAP_DMA;
    out->dma_free = heap_caps_get_free_size(dma_caps);
    out->dma_min_free = heap_caps_get_minimum_free_size(dma_caps);
    out->psram_free = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
    out->psram_min_free = heap_caps_get_minimum_free_size(MALLOC_CAP_SPIRAM);
    /* Clear reused snapshots. These zeros are unavailable C values, never a
     * measured block size or measured zero latency. HTTP exposes them as null. */
    out->internal_largest_free = 0;
    out->dma_largest_free = 0;
    out->psram_largest_free = 0;
    out->heap_walk_max_us[0] = 0;
    out->heap_walk_max_us[1] = 0;
    out->heap_walk_max_us[2] = 0;
}
