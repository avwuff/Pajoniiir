#pragma once
#include <stdint.h>
typedef struct {
    uint32_t mepc, ra, sp, mcause, mtval, mhartid;
} RvExcFrame;
