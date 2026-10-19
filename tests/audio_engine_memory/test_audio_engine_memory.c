#include "audio_engine_memory.h"
#include "esp_heap_caps.h"
#include <assert.h>
#include <stdio.h>

static unsigned free_reads, minimum_reads;
uint32_t esp_get_free_heap_size(void) { return 90000; }
size_t heap_caps_get_free_size(uint32_t caps) { ++free_reads; return 10000 * caps; }
size_t heap_caps_get_minimum_free_size(uint32_t caps) { ++minimum_reads; return 1000 * caps; }
/* Deliberately no largest-block, timer or service-log implementations: linking
 * fails if any observation-time snapshot attempts a TLSF walk or journal work. */
int main(void)
{
    audio_engine_diagnostics_snapshot_t d = {0};
    audio_engine_snapshot_memory(NULL);
    assert(!free_reads && !minimum_reads);
    for (unsigned i = 0; i < 20; ++i) {
        d.internal_largest_free = d.dma_largest_free = d.psram_largest_free = 123;
        d.heap_walk_max_us[0] = d.heap_walk_max_us[1] = d.heap_walk_max_us[2] = 999;
        audio_engine_snapshot_memory(&d);
        assert(d.heap_free == 90000 && d.internal_free == 10000);
        assert(d.internal_min_free == 1000 && d.dma_free == 30000);
        assert(d.dma_min_free == 3000 && d.psram_min_free == 4000);
        assert(d.psram_free == 40000);
        assert(!d.internal_largest_free && !d.dma_largest_free && !d.psram_largest_free);
        assert(!d.heap_walk_max_us[0] && !d.heap_walk_max_us[1] && !d.heap_walk_max_us[2]);
    }
    assert(free_reads == 60 && minimum_reads == 60);
    puts("PASS memory snapshots retain free/low-water values without TLSF walks, timer or journal work");
    return 0;
}
