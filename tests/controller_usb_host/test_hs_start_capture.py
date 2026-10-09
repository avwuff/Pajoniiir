"""Execute the production HAL activation wrapper against mocked registers."""
import pathlib
import subprocess
import tempfile
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[2]


class StartCapture(unittest.TestCase):
    def test_actual_wrapper(self):
        source = (ROOT / 'firmware/p4-core/components/usb_storage/usb_dwc_hal_compat.c').read_text()
        start = source.index('void __real_usb_dwc_hal_chan_activate')
        end = source.index('/* Capture only port transitions', start)
        wrapper = source[start:end]
        preamble = r'''
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#define CONFIG_CONTROLLER_USB_HS_AUDIO_DIAGNOSTIC 1
#define CONFIG_USB_HOST_DIAGNOSTIC_BUS_TRACE 1
#define USB_BUS_TRACE_ISOC_LATE_START 3
typedef struct {uint32_t val;} reg_t;
typedef struct {reg_t hcchar_reg, hctsiz_reg;} regs_t;
typedef struct {regs_t *regs; struct {unsigned chan_idx;} flags;} usb_dwc_hal_chan_t;
typedef struct {regs_t host_chans[8]; reg_t hfnum_reg, hprt_reg;} usb_dwc_dev_t;
typedef struct {uint32_t time_ms,kind,intr,hprt,hcchar,hctsiz,event;} usb_bus_trace_event_t;
static usb_dwc_dev_t hardware;
#define USB_DWC_LL_GET_HW(root) (&hardware)
static unsigned activated, recorded;
static usb_bus_trace_event_t captured;
static uint64_t esp_timer_get_time(void) {return 123000;}
static void usb_bus_trace_record(unsigned root,const usb_bus_trace_event_t *e) {
 assert(root==0 && activated>recorded); recorded++; captured=*e;
}
'''
        harness = r'''
void __real_usb_dwc_hal_chan_activate(usb_dwc_hal_chan_t *c,void *l,int n,int s) {
 (void)c;(void)l;(void)n;(void)s; activated++;
 hardware.hfnum_reg.val++; /* Model the boundary during HAL register writes. */
}
int main(void) {
 usb_dwc_hal_chan_t c={.regs=&hardware.host_chans[1],.flags={.chan_idx=1}};
 c.regs->hcchar_reg.val=1u<<18;
 hardware.hfnum_reg.val=0x12000005;
 __wrap_usb_dwc_hal_chan_activate(&c,0,64,7); assert(recorded==0);
 hardware.hfnum_reg.val=0x12000005;
 __wrap_usb_dwc_hal_chan_activate(&c,0,64,5);
 assert(recorded==1 && captured.intr==0x12000006 && captured.event==5 && captured.kind==3);
 hardware.hfnum_reg.val=0x12000005;
 __wrap_usb_dwc_hal_chan_activate(&c,0,64,4); assert(recorded==2);
 c.regs->hcchar_reg.val|=1u<<15;
 __wrap_usb_dwc_hal_chan_activate(&c,0,64,4); assert(recorded==2);
 c.regs->hcchar_reg.val=1u<<18;
 __wrap_usb_dwc_hal_chan_activate(&c,0,2,4); assert(recorded==2);
 regs_t other={.hcchar_reg={.val=1u<<18}};c.regs=&other;
 __wrap_usb_dwc_hal_chan_activate(&c,0,64,4);assert(recorded==2 && activated==6);
 c.regs=&hardware.host_chans[1];c.regs->hcchar_reg.val=1u<<18;
 hardware.hfnum_reg.val=0x12000005;
 __wrap_usb_dwc_hal_chan_activate(&c,0,64,6);
 assert(recorded==3 && captured.intr==0x12000006 && captured.event==6);
 return 0;
}
'''
        with tempfile.TemporaryDirectory(prefix='p4-hs-start-') as directory:
            path = pathlib.Path(directory)
            (path / 'capture.c').write_text(preamble + wrapper + harness)
            exe = path / 'capture.exe'
            subprocess.run(['gcc', '-std=c11', '-O2', '-Wall', '-Wextra', '-Werror',
                            str(path / 'capture.c'), '-o', str(exe)], check=True)
            subprocess.run([str(exe)], check=True)


if __name__ == '__main__':
    unittest.main()
