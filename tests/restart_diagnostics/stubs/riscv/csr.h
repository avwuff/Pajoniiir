#pragma once
#include <stdint.h>
enum { mhartid=0xF14, mepc=0x341, mcause=0x342, mtval=0x343, mcycle=0xB00 };
#define MEXSTATUS 0x7E1
#define MHINT 0x7C5
#define STPC0 0xBF0
#define STPC1 0xBF1
#define STPC2 0xBF2
uint32_t test_read_csr(unsigned csr);
#define RV_READ_CSR(csr) test_read_csr(csr)
