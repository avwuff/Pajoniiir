#include "restart_diagnostics.h"
#include "esp_attr.h"
#include "esp_private/panic_internal.h"
#include "riscv/csr.h"
#include "riscv/rvruntime-frames.h"
#include "soc/soc_caps.h"

#if !SOC_ASYNCHRONOUS_BUS_ERROR_MODE
#error "P4 diagnostic CSR contract requires asynchronous bus-error support"
#endif

_Static_assert(sizeof(p4_restart_capture_t) == 128, "restart capture ABI changed");

#define RECORD_INIT { .magic = P4_RESTART_CAPTURE_MAGIC, \
    .version = P4_RESTART_CAPTURE_VERSION, .bytes = sizeof(p4_restart_capture_t) }
COREDUMP_DRAM_ATTR volatile p4_restart_capture_t
    p4_restart_capture[P4_RESTART_CAPTURE_CORES] = { RECORD_INIT, RECORD_INIT };

void __real_panic_arch_fill_info(void *frame, panic_info_t *info);
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

static inline __attribute__((always_inline)) void mark_restart(uint32_t phase)
{
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
    mark_restart(P4_RESTART_PHASE_NOOS);
    __real_esp_restart_noos();
}

void IRAM_ATTR __wrap_panic_arch_fill_info(void *frame, panic_info_t *info)
{
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
