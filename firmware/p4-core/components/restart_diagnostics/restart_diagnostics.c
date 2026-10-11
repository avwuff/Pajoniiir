#include "restart_diagnostics.h"
#include <stdbool.h>
#include "esp_attr.h"
#include "esp_private/panic_internal.h"
#include "riscv/csr.h"
#include "riscv/rvruntime-frames.h"
#include "soc/soc_caps.h"
#include "esp_private/esp_cache_private.h"

#if !SOC_ASYNCHRONOUS_BUS_ERROR_MODE
#error "P4 diagnostic CSR contract requires asynchronous bus-error support"
#endif

_Static_assert(sizeof(p4_restart_capture_t) == 128, "restart capture ABI changed");

COREDUMP_NOINIT_ATTR volatile p4_restart_capture_t
    p4_restart_capture[P4_RESTART_CAPTURE_CORES];
/* BSS resets each boot even though the dump region itself is noinit. */
static DRAM_ATTR volatile uint32_t s_capture_ready;
/* Separate BSS marker covers pseudo-exceptions without changing the dump ABI. */
DRAM_ATTR uint32_t p4_restart_panic_seen;

void __real_panic_arch_fill_info(void *frame, panic_info_t *info);
void __real_panic_soc_fill_info(void *frame, panic_info_t *info);
void __real_esp_restart(void) __attribute__((noreturn));
void __real_esp_restart_noos(void) __attribute__((noreturn));

static inline __attribute__((always_inline)) void publish_barrier(void)
{
#ifdef ESP_PLATFORM
    __asm__ volatile("fence rw,rw" ::: "memory");
#else
    __asm__ volatile("" ::: "memory");
#endif
}

void IRAM_ATTR p4_restart_capture_init(void)
{
    if (s_capture_ready) return;
    /* Volatile bounded stores prevent a compiler-generated flash memset call. */
    volatile unsigned char *bytes = (volatile unsigned char *)p4_restart_capture;
    for (unsigned i = 0; i < sizeof(p4_restart_capture); ++i) {
        bytes[i] = 0;
    }
    for (unsigned i = 0; i < P4_RESTART_CAPTURE_CORES; ++i) {
        p4_restart_capture[i].magic = P4_RESTART_CAPTURE_MAGIC;
        p4_restart_capture[i].version = P4_RESTART_CAPTURE_VERSION;
        p4_restart_capture[i].bytes = sizeof(p4_restart_capture_t);
    }
    publish_barrier();
    s_capture_ready = 1;
}

static inline __attribute__((always_inline)) void mark_restart(uint32_t phase)
{
    p4_restart_capture_init();
    uint32_t core = RV_READ_CSR(mhartid);
    if (core >= P4_RESTART_CAPTURE_CORES) return;
    volatile p4_restart_capture_t *record = &p4_restart_capture[core];
    /* A panic restart must not rewrite evidence already captured for its dump. */
    if (record->flags & P4_RESTART_CAPTURE_VALID) return;
    record->restart_core = core;
    if (phase == P4_RESTART_PHASE_REQUESTED) {
        record->restart_cycle = RV_READ_CSR(mcycle);
    } else {
        record->noos_cycle = RV_READ_CSR(mcycle);
    }
    publish_barrier();
    record->restart_phase = phase;
}

void IRAM_ATTR __attribute__((noreturn)) __wrap_esp_restart(void)
{
    mark_restart(P4_RESTART_PHASE_REQUESTED);
    __real_esp_restart();
}

void IRAM_ATTR __attribute__((noreturn)) __wrap_esp_restart_noos(void)
{
    /* Only the normal esp_restart path can safely wait for a live peer.
     * A panic/abort can have already stalled a peer which owns the cache lock.
     * IDF 6.0.2 resets that peer before Cache_WriteBack_All(), without joining
     * the lock used by msync/mmap. Hold the same lock across the no-return reset
     * so an in-flight sync finishes before the peer is reset, and no new sync
     * can overlap the final writeback. Do this after shutdown handlers. */
    p4_restart_capture_init();
    const uint32_t core = RV_READ_CSR(mhartid);
    const bool normal_restart = core < P4_RESTART_CAPTURE_CORES &&
        !__atomic_load_n(&p4_restart_panic_seen, __ATOMIC_ACQUIRE) &&
        p4_restart_capture[core].restart_phase == P4_RESTART_PHASE_REQUESTED &&
        !(p4_restart_capture[core].flags & P4_RESTART_CAPTURE_VALID);
    mark_restart(P4_RESTART_PHASE_NOOS);
    if (normal_restart) {
        esp_cache_sync_ops_enter_critical_section();
    }
    __real_esp_restart_noos();
}

void IRAM_ATTR __wrap_panic_arch_fill_info(void *frame, panic_info_t *info)
{
    __atomic_store_n(&p4_restart_panic_seen, 1u, __ATOMIC_RELEASE);
    /* Also handle an exception after BSS init but before common app startup. */
    p4_restart_capture_init();
    uint32_t core = RV_READ_CSR(mhartid);
    if (frame != 0 && core < P4_RESTART_CAPTURE_CORES) {
        volatile p4_restart_capture_t *record = &p4_restart_capture[core];
        if (!(record->flags & P4_RESTART_CAPTURE_VALID)) {
            const RvExcFrame *saved = frame;
            record->frame_mepc = saved->mepc;
            record->frame_ra = saved->ra;
            record->frame_sp = saved->sp;
            record->frame_mcause = saved->mcause;
            record->frame_mtval = saved->mtval;
            record->frame_mhartid = saved->mhartid;
            record->csr_mhartid = core;
            record->csr_mepc = RV_READ_CSR(mepc);
            record->csr_mcause = RV_READ_CSR(mcause);
            record->csr_mtval = RV_READ_CSR(mtval);
            record->csr_mexstatus = RV_READ_CSR(MEXSTATUS);
            record->csr_mhint = RV_READ_CSR(MHINT);
            record->stpc[0] = RV_READ_CSR(STPC0);
            record->stpc[1] = RV_READ_CSR(STPC1);
            record->stpc[2] = RV_READ_CSR(STPC2);
            /* P4 TRM CSR table: STTVAL0/1/2 = 0xBF8/0xBF9/0xBFA.
             * IDF 6.0.2 csr.h names STPC but omits these address CSRs. */
            record->sttval[0] = RV_READ_CSR(0xBF8);
            record->sttval[1] = RV_READ_CSR(0xBF9);
            record->sttval[2] = RV_READ_CSR(0xBFA);
            record->capture_cycle = RV_READ_CSR(mcycle);
            publish_barrier();
            record->flags = P4_RESTART_CAPTURE_VALID;
        }
    }
    /* Preserve IDF PC normalization, summary, dump and restart behavior. */
    __real_panic_arch_fill_info(frame, info);
}

void IRAM_ATTR __wrap_panic_soc_fill_info(void *frame, panic_info_t *info)
{
    /* Watchdog/abort/cache pseudo-exceptions skip the architecture hook.
     * Never wait for their potentially stalled cache-lock owner. */
    __atomic_store_n(&p4_restart_panic_seen, 1u, __ATOMIC_RELEASE);
    p4_restart_capture_init();
    __real_panic_soc_fill_info(frame, info);
}
