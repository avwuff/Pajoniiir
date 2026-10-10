#pragma once
#include <stdint.h>

#define P4_RESTART_CAPTURE_MAGIC UINT32_C(0x50345244)
#define P4_RESTART_CAPTURE_VERSION 1u
#define P4_RESTART_CAPTURE_CORES 2u
#define P4_RESTART_CAPTURE_VALID 1u

enum {
    P4_RESTART_PHASE_NONE = 0,
    P4_RESTART_PHASE_REQUESTED = 1,
    P4_RESTART_PHASE_NOOS = 2,
};

/* Fixed little-endian RV32 ABI, copied by IDF's user DRAM coredump region.
 * VALID is published last. It records one exception per core per boot.
 * Queue entries remain raw, including validity bits; sampling is sequential. */
typedef struct {
    uint32_t magic, version, bytes, flags;
    uint32_t restart_phase, restart_core, restart_cycle, noos_cycle;
    uint32_t frame_mepc, frame_ra, frame_sp, frame_mcause, frame_mtval, frame_mhartid;
    uint32_t csr_mepc, csr_mcause, csr_mtval, csr_mhartid, csr_mexstatus, csr_mhint;
    uint32_t stpc[3], sttval[3];
    uint32_t capture_cycle, reserved[5];
} p4_restart_capture_t;

extern volatile p4_restart_capture_t p4_restart_capture[P4_RESTART_CAPTURE_CORES];
