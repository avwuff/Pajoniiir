"""Execute production HCD first-fault hooks, including destructive-parse fencing."""
import pathlib
import re
import subprocess
import tempfile
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[2]


class Capture(unittest.TestCase):
    def test_production_hooks(self):
        source = (ROOT / 'firmware/p4-core/cmake/apply_espressif_usb_isoc_capture_patch.cmake').read_text()
        def block(name):
            return re.search(r'set\(_capture_' + name + r'_new \[=\[\n(.*?)\]=\]\)', source, re.S)[1]
        harness = r'''
#include <assert.h>
#include <string.h>
#include <stddef.h>
#include "usb_isoc_fault.h"
#define USB_DWC_XFER_TYPE_ISOCHRONOUS 1
#define USB_B_ENDPOINT_ADDRESS_EP_DIR_MASK 128
#define XFER_LIST_LEN_ISOC 64
typedef struct {uint32_t val;} reg_t;
typedef struct {reg_t hfnum_reg,hptxsts_reg;} dev_t;
typedef struct {reg_t hcchar_reg,hctsiz_reg,hcdma_reg,hcint_reg;} regs_t;
typedef struct {uint32_t buffer_status_val,buffer;} usb_dwc_ll_dma_qtd_t;
static dev_t device,other;
#define USB_DWC_LL_GET_HW(x) (&device)
typedef struct {struct {int num_isoc_packets;} transfer;} urb_t;
typedef struct {usb_isoc_activation_t capture; urb_t *urb; void *xfer_desc_list;
 struct {struct {unsigned interval,start_idx;} isoc;} flags;
 struct {unsigned executing;} status_flags;} dma_buffer_block_t;
typedef struct {dev_t *dev;} hal_t;
typedef struct {hal_t *hal; uint32_t capture_irq_cycle,capture_lock_cycle,capture_irq_hfnum;} port_t;
typedef struct {regs_t *regs;struct {unsigned chan_idx;} flags;} chan_t;
typedef struct {port_t *port; chan_t *chan_obj;
 struct {unsigned type,bEndpointAddress;} ep_char;
 struct {unsigned buffer_num_to_exec;} multi_buffer_control;unsigned num_urb_pending,capture_chan_intr;} pipe_t;
static uint32_t clock_value=0xfffffff0u;
static unsigned isr=1, enables;
static uint32_t esp_cpu_get_cycle_count(void){return clock_value+=3;}
static bool xPortInIsrContext(void){return isr;}
static void usb_dwc_hal_chan_activate(chan_t *c,void *p,int n,unsigned start){
 (void)c;(void)p;assert(n==64);(void)start;enables++;device.hfnum_reg.val++;
}
''' + block('state') + r'''
static void activate(pipe_t *pipe,dma_buffer_block_t *buffer_to_exec) {
 unsigned start_idx=buffer_to_exec->flags.isoc.start_idx;int desc_list_len=64;
''' + block('exec') + r'''
}
static void done(pipe_t *pipe,dma_buffer_block_t *buffer_done,int stop_idx) {
''' + block('done') + r'''
}
''' + block('parse') + r'''
 /* Simulate the original destructive parser after the production hook. */
 memset(buffer->xfer_desc_list,0,512);
}
int main(void) {
 usb_dwc_ll_dma_qtd_t raw[64]={0}; urb_t urb={{56}};
 hal_t hal={&device};port_t port={&hal,0xfffffff0u,0xfffffff4u,0x1234003du};
 regs_t regs={{17},{18},{19},{20}};chan_t chan={&regs,{3}};
 pipe_t pipe={.port=&port,.chan_obj=&chan,.ep_char={1,0},.multi_buffer_control={2},.num_urb_pending=1,.capture_chan_intr=123};
 dma_buffer_block_t buffer={.urb=&urb,.xfer_desc_list=raw,.flags={{1,63}}};
 usb_isoc_fault_t out;
 memset(&out,0xa5,sizeof(out));assert(!usb_isoc_fault_snapshot(&out));
 assert(out.qtd[0]==0xa5a5a5a5u);
 activate(&pipe,&buffer);done(&pipe,&buffer,55);
 _buffer_parse_isoc(&pipe,&buffer,false);assert(!usb_isoc_fault_snapshot(&out));
#if CONFIG_PAJONIIIR_BOARD_M3 && CONFIG_CONTROLLER_USB_HS_AUDIO_DIAGNOSTIC
 assert(usb_isoc_fault_enabled());
 /* Packet 1 wraps list slot63 -> slot0. Before-parse raw data must survive. */
 raw[0].buffer_status_val=0x80000050u;raw[0].buffer=0x12345678;
 raw[1].buffer_status_val=0x80000060u;
 activate(&pipe,&buffer);done(&pipe,&buffer,55);
 regs.hcchar_reg.val=99; /* Subsequent channel activation cannot change completion capture. */
 _buffer_parse_isoc(&pipe,&buffer,false);assert(usb_isoc_fault_snapshot(&out));
 assert(out.first_packet==1 && out.activation.start==63 && out.activation.sequence==2);
 assert(out.qtd[0]==0x80000050u && out.qtd[1]==0x12345678 && out.qtd[2]==0x80000060u);
 assert(raw[0].buffer_status_val==0 && out.hcchar==17 && out.activation.done_stop_idx==55);
 assert(out.hcint==123);
 assert(out.buffer_pending==2 && out.urb_pending==1 && out.activation.from_isr==1);
 assert((uint32_t)(out.activation.irq_locked_cycle-out.activation.irq_entry_cycle)==4);
 usb_isoc_fault_t frozen=out;
 raw[63].buffer_status_val=0x10000010u;
 activate(&pipe,&buffer);done(&pipe,&buffer,1);_buffer_parse_isoc(&pipe,&buffer,false);
 assert(usb_isoc_fault_snapshot(&out) && memcmp(&out,&frozen,sizeof(out))==0);
 /* Test independent boot state, root/input filtering and task activations. */
 s_capture_state=0;memset(&s_capture_fault,0,sizeof(s_capture_fault));
 hal.dev=&other;raw[63].buffer_status_val=0x80000010u;
 _buffer_parse_isoc(&pipe,&buffer,false);assert(!usb_isoc_fault_snapshot(&out));
 hal.dev=&device;raw[63].buffer_status_val=0x80000010u;
 _buffer_parse_isoc(&pipe,&buffer,true);assert(!usb_isoc_fault_snapshot(&out));
 isr=0;activate(&pipe,&buffer);done(&pipe,&buffer,2);
 raw[63].buffer_status_val=0x20000010u;_buffer_parse_isoc(&pipe,&buffer,false);
 assert(usb_isoc_fault_snapshot(&out) && out.first_packet==0);
 assert(!out.activation.from_isr && out.activation.irq_entry_cycle==0);
 s_capture_state=1;assert(!usb_isoc_fault_snapshot(&out));
 s_capture_state=0;urb.transfer.num_isoc_packets=28;
 buffer.flags.isoc.interval=2;buffer.flags.isoc.start_idx=62;
 activate(&pipe,&buffer);done(&pipe,&buffer,54);
 raw[0].buffer_status_val=0x80000020u;
 _buffer_parse_isoc(&pipe,&buffer,false);
 assert(usb_isoc_fault_snapshot(&out) && out.first_packet==1 && out.activation.interval==2);
#else
 assert(!usb_isoc_fault_enabled());
#endif
 assert(enables>=1);assert(!usb_isoc_fault_snapshot(NULL));
 return 0;
}
'''
        with tempfile.TemporaryDirectory(prefix='p4-isoc-capture-') as d:
            src = pathlib.Path(d) / 'capture.c'
            src.write_text(harness)
            for m3, hs in ((1, 1), (0, 1), (1, 0)):
                exe = pathlib.Path(d) / f'capture{m3}{hs}.exe'
                subprocess.run(['gcc', '-std=c11', '-Wall', '-Wextra', '-Werror',
                                '-Wno-unused-parameter', '-Wno-unused-function', '-Wno-unused-variable',
                                f'-DCONFIG_PAJONIIIR_BOARD_M3={m3}',
                                f'-DCONFIG_CONTROLLER_USB_HS_AUDIO_DIAGNOSTIC={hs}',
                                '-I', str(ROOT / 'firmware/p4-core/components/usb_storage/include'),
                                str(src), '-o', str(exe)], check=True)
                subprocess.run([str(exe)], check=True)


if __name__ == '__main__':
    unittest.main()
