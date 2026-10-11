#include "restart_diagnostics.h"
#include "esp_private/panic_internal.h"
#include "riscv/rvruntime-frames.h"
#include "riscv/csr.h"
#include <assert.h>
#include <setjmp.h>
#include <stdio.h>
#include <string.h>

void __wrap_panic_arch_fill_info(void *, panic_info_t *);
void __wrap_panic_soc_fill_info(void *, panic_info_t *);
extern uint32_t p4_restart_panic_seen;
void __wrap_esp_restart(void);
void __wrap_esp_restart_noos(void);
static unsigned cpu, real_calls, reads;
static unsigned cache_joins;
static jmp_buf restarted;
static bool pseudo_panic_during_shutdown;
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
    if (pseudo_panic_during_shutdown) {
        panic_info_t info={0};
        __wrap_panic_soc_fill_info(NULL, &info);
        assert(info.called==1);
    }
    __wrap_esp_restart_noos();
    assert(0); for (;;) {}
}
void __attribute__((noreturn)) __real_esp_restart_noos(void)
{
    longjmp(restarted, 1);
}
void esp_cache_sync_ops_enter_critical_section(void)
{
    assert(cpu < 2);
    assert(p4_restart_capture[cpu].restart_phase == 2);
    assert(!p4_restart_capture[cpu].flags);
    cache_joins++;
}
void __real_panic_soc_fill_info(void *frame, panic_info_t *info)
{
    (void)frame;
    assert(p4_restart_panic_seen);
    info->called++;
}
static void reset(void)
{
    memset((void *)p4_restart_capture, 0, sizeof(p4_restart_capture));
    for (unsigned i=0; i<2; i++) {
        p4_restart_capture[i].magic=P4_RESTART_CAPTURE_MAGIC;
        p4_restart_capture[i].version=1;
        p4_restart_capture[i].bytes=128;
    }
    cpu=0; reads=0; real_calls=0; cache_joins=0;
    p4_restart_panic_seen=0; pseudo_panic_during_shutdown=false;
}
int main(void)
{
    assert(sizeof(p4_restart_capture)==256);
    /* Model stale noinit bytes at boot; a pre-app restart must initialize them. */
    memset((void *)p4_restart_capture, 0xa5, sizeof(p4_restart_capture));
    if (!setjmp(restarted)) __wrap_esp_restart();
    assert(p4_restart_capture[0].magic==P4_RESTART_CAPTURE_MAGIC);
    assert(p4_restart_capture[1].version==1 && p4_restart_capture[1].bytes==128);
    assert(p4_restart_capture[0].flags==0 && p4_restart_capture[1].flags==0);
    assert(p4_restart_capture[0].restart_phase==2);
    assert(p4_restart_capture[1].restart_phase==0);
    assert(cache_joins==1);
    p4_restart_capture_t initialized[2];
    memcpy(initialized,(const void *)p4_restart_capture,sizeof(initialized));
    p4_restart_capture_init();
    assert(memcmp(initialized,(const void *)p4_restart_capture,sizeof(initialized))==0);
    reset();
    if (!setjmp(restarted)) __wrap_esp_restart();
    assert(cache_joins==1);
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
    assert(cache_joins==1); /* Frozen panic restart must not wait for a peer. */
    frame.mepc=0xabcd; __wrap_panic_arch_fill_info(&frame,&info);
    p4_restart_capture_init(); /* Startup/restart calls cannot erase a capture. */
    assert(memcmp(&frozen,(const void *)r,sizeof(frozen))==0);
    assert(real_calls==2); /* Real IDF behavior continues even when frozen. */
    reset();
    if (!setjmp(restarted)) __wrap_esp_restart_noos();
    assert(cache_joins==0); /* Direct noos/abort path can have a stalled owner. */
    reset(); cpu=1;
    if (!setjmp(restarted)) __wrap_esp_restart();
    assert(cache_joins==1 && p4_restart_capture[1].restart_phase==2);
    reset(); cpu=2;
    if (!setjmp(restarted)) __wrap_esp_restart_noos();
    assert(cache_joins==0); /* Invalid core must never index or join the lock. */
    reset(); pseudo_panic_during_shutdown=true;
    if (!setjmp(restarted)) __wrap_esp_restart();
    assert(cache_joins==0 && p4_restart_panic_seen==1);
    reset(); frame=input;
    __wrap_panic_arch_fill_info(&frame,&info);
    cpu=1; frame=input; frame.mhartid=1;
    __wrap_panic_arch_fill_info(&frame,&info);
    assert(p4_restart_capture[1].flags==1 && p4_restart_capture[1].csr_mhartid==1);
    assert(p4_restart_capture[1].frame_mhartid==1 && r->csr_mhartid==0);
    reset(); frame=input; cpu=2;
    __wrap_panic_arch_fill_info(&frame,&info);
    assert(frame.mepc==0x9999 && !p4_restart_capture[0].flags && !p4_restart_capture[1].flags);
    cpu=0; __wrap_panic_arch_fill_info(NULL,&info);
    assert(!p4_restart_capture[0].flags);
    puts("restart diagnostics: capture, normal cache join, both-core delegation and architecture/pseudo-panic lock bypass PASS");
    return 0;
}
