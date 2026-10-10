#include "restart_diagnostics.h"
#include "esp_private/panic_internal.h"
#include "riscv/rvruntime-frames.h"
#include "riscv/csr.h"
#include <assert.h>
#include <setjmp.h>
#include <stdio.h>
#include <string.h>

void __wrap_panic_arch_fill_info(void *, panic_info_t *);
void __wrap_esp_restart(void);
void __wrap_esp_restart_noos(void);
static unsigned cpu, real_calls, reads;
static jmp_buf restarted;
static RvExcFrame input = { .mepc=0x1234, .ra=0x5678, .sp=0x4ff33c80,
    .mcause=7, .mtval=0x3ff1009c, .mhartid=0 };

uint32_t test_read_csr(unsigned csr)
{
    reads++;
    if (csr == mhartid) return cpu;
    return csr + 0x10000;
}
void __real_panic_arch_fill_info(void *frame, panic_info_t *info)
{
    real_calls++;
    info->called++;
    if (frame != NULL) {
        /* Model IDF normalizing the frame only after diagnostic publication. */
        if (cpu < 2) assert(p4_restart_capture[cpu].flags == 1);
        ((RvExcFrame *)frame)->mepc = 0x9999;
    }
}
void __attribute__((noreturn)) __real_esp_restart(void)
{
    assert(p4_restart_capture[cpu].restart_phase == 1 ||
           p4_restart_capture[cpu].flags == 1);
    __wrap_esp_restart_noos();
    assert(0); for (;;) {}
}
void __attribute__((noreturn)) __real_esp_restart_noos(void)
{
    longjmp(restarted, 1);
}
static void reset(void)
{
    memset((void *)p4_restart_capture, 0, sizeof(p4_restart_capture));
    for (unsigned i=0; i<2; i++) {
        p4_restart_capture[i].magic=P4_RESTART_CAPTURE_MAGIC;
        p4_restart_capture[i].version=1;
        p4_restart_capture[i].bytes=128;
    }
    cpu=0; reads=0; real_calls=0;
}
int main(void)
{
    assert(sizeof(p4_restart_capture)==256);
    assert(p4_restart_capture[0].magic==P4_RESTART_CAPTURE_MAGIC);
    assert(p4_restart_capture[1].version==1 && p4_restart_capture[1].bytes==128);
    reset();
    if (!setjmp(restarted)) __wrap_esp_restart();
    assert(p4_restart_capture[0].restart_phase==2);
    assert(p4_restart_capture[0].restart_cycle==mcycle+0x10000);
    assert(p4_restart_capture[0].noos_cycle==mcycle+0x10000);
    panic_info_t info={0}; RvExcFrame frame=input;
    __wrap_panic_arch_fill_info(&frame,&info);
    assert(frame.mepc==0x9999 && info.called==1 && real_calls==1);
    const volatile p4_restart_capture_t *r=&p4_restart_capture[0];
    assert(r->frame_mepc==0x1234 && r->frame_ra==0x5678);
    assert(r->frame_sp==input.sp && r->frame_mcause==7 && r->frame_mtval==input.mtval);
    assert(r->csr_mexstatus==MEXSTATUS+0x10000 && r->csr_mhint==MHINT+0x10000);
    assert(r->stpc[0]==STPC0+0x10000 && r->stpc[1]==STPC1+0x10000 && r->stpc[2]==STPC2+0x10000);
    assert(r->sttval[0]==0x10bf8 && r->sttval[1]==0x10bf9 && r->sttval[2]==0x10bfa);
    assert(r->csr_mepc==mepc+0x10000 && r->csr_mcause==mcause+0x10000 && r->csr_mtval==mtval+0x10000);
    p4_restart_capture_t frozen; memcpy(&frozen,(const void *)r,sizeof(frozen));
    if (!setjmp(restarted)) __wrap_esp_restart();
    frame.mepc=0xabcd; __wrap_panic_arch_fill_info(&frame,&info);
    assert(memcmp(&frozen,(const void *)r,sizeof(frozen))==0);
    assert(real_calls==2); /* Real IDF behavior continues even when frozen. */
    cpu=1; frame=input; frame.mhartid=1;
    __wrap_panic_arch_fill_info(&frame,&info);
    assert(p4_restart_capture[1].flags==1 && p4_restart_capture[1].csr_mhartid==1);
    assert(p4_restart_capture[1].frame_mhartid==1 && r->csr_mhartid==0);
    reset(); frame=input; cpu=2;
    __wrap_panic_arch_fill_info(&frame,&info);
    assert(frame.mepc==0x9999 && !p4_restart_capture[0].flags && !p4_restart_capture[1].flags);
    cpu=0; __wrap_panic_arch_fill_info(NULL,&info);
    assert(!p4_restart_capture[0].flags);
    puts("restart diagnostics: capture-before-normalization, core isolation, frozen evidence and restart delegation PASS");
    return 0;
}
