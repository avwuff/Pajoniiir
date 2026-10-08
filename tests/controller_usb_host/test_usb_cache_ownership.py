"""Compile pinned HCD functions with the actual CMake cache transformation."""
import pathlib
import re
import subprocess
import tempfile
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[2]


def function(source, signature):
    start = source.index(signature)
    begin = source.index('{', start)
    depth = 1
    end = begin + 1
    while depth:
        depth += (source[end] == '{') - (source[end] == '}')
        end += 1
    return source[start:end]


class CacheOwnership(unittest.TestCase):
    def test_pinned_functions(self):
        source = (ROOT / 'tests/controller_usb_host/fixtures/hcd_cache_functions.c').read_text()
        live = ROOT / 'firmware/main-deck-m3/managed_components/usb/src/hcd_dwc.c'
        if live.exists():
            dependency = live.read_text()
            for signature in ('static inline void cache_sync_data_buffer(', 'urb_t *hcd_urb_dequeue('):
                self.assertEqual(function(source, signature), function(dependency, signature),
                                 'Pinned fixture/dependency drift')
        cmake = (ROOT / 'firmware/p4-core/cmake/apply_espressif_usb_idle_recovery_patch.cmake').read_text()
        original = function(source, 'static inline void cache_sync_data_buffer(')
        original += '\n' + function(source, 'urb_t *hcd_urb_dequeue(')
        patched = original
        for block in ('sync', 'declarations', 'dequeue'):
            old = re.search(r'set\(_hs_' + block + r'_original \[=\[\n(.*?)\]=\]\)', cmake, re.S)[1]
            new = re.search(r'set\(_hs_' + block + r'_bounded \[=\[\n(.*?)\]=\]\)', cmake, re.S)[1]
            self.assertEqual(patched.count(old), 1, 'Pinned dependency drift')
            patched = patched.replace(old, new)
        preamble = r'''
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#define CONFIG_CONTROLLER_USB_HS_AUDIO_DIAGNOSTIC 1
#define USB_B_ENDPOINT_ADDRESS_EP_DIR_MASK 128
#define USB_DWC_XFER_TYPE_CTRL 0
#define ESP_CACHE_MSYNC_FLAG_DIR_M2C 1
#define ESP_CACHE_MSYNC_FLAG_UNALIGNED 2
#define ESP_OK 0
#define URB_HCD_STATE_IDLE 0
#define URB_HCD_STATE_DONE 3
typedef int esp_err_t;
typedef struct urb_s {struct {uint8_t *data_buffer;size_t data_buffer_size;} transfer;void *hcd_ptr;int hcd_var;} urb_t;
typedef struct {int pipes_active_tailq,pipes_idle_tailq,num_pipes_idle,num_pipes_queued;} port_t;
typedef struct {struct {unsigned bEndpointAddress,type;} ep_char; int num_urb_done,num_urb_pending,done_urb_tailq;
 struct {bool has_urb;} cs_flags; struct {unsigned buffer_num_to_exec,buffer_num_to_parse;} multi_buffer_control;port_t *port;} pipe_t;
typedef pipe_t *hcd_pipe_handle_t;
static urb_t owned;
static pipe_t *closing_pipe;
static bool locked;
static unsigned calls,unsafe_calls,max_bytes;
#define HCD_ENTER_CRITICAL() do {assert(!locked);locked=true;} while(0)
#define HCD_EXIT_CRITICAL() do {assert(locked);locked=false; if(closing_pipe) {closing_pipe->ep_char.bEndpointAddress=1;closing_pipe->ep_char.type=2;}} while(0)
#define TAILQ_FIRST(head) (&owned)
#define TAILQ_REMOVE(head,item,field) ((void)0)
#define TAILQ_INSERT_TAIL(head,item,field) ((void)0)
static int esp_cache_msync(void *address,size_t bytes,uint32_t flags) {
 assert(address>= (void *)owned.transfer.data_buffer);
 assert((uint8_t *)address+bytes<=owned.transfer.data_buffer+owned.transfer.data_buffer_size);
 assert(owned.hcd_var==URB_HCD_STATE_IDLE && owned.hcd_ptr==0);
 assert(flags==ESP_CACHE_MSYNC_FLAG_DIR_M2C || flags==ESP_CACHE_MSYNC_FLAG_UNALIGNED);
 calls++;unsafe_calls+=locked;if(bytes>max_bytes)max_bytes=(unsigned)bytes;return ESP_OK;
}
#define CACHE_SYNC_DATA_BUFFER_M2C(pipe,urb) cache_sync_data_buffer(pipe,urb,true)
'''
        harness = r'''
int main(void) {
 uint8_t memory[8192];port_t port={.num_pipes_queued=1};
 pipe_t pipe={.ep_char={.bEndpointAddress=128,.type=2},.num_urb_done=1,.cs_flags={.has_urb=true},.port=&port};
 closing_pipe=&pipe; /* Simulate endpoint retirement immediately after unlock. */
 owned=(urb_t){.transfer={.data_buffer=memory,.data_buffer_size=sizeof(memory)},.hcd_ptr=&pipe,.hcd_var=URB_HCD_STATE_DONE};
 assert(hcd_urb_dequeue(&pipe)==&owned);
 assert(!locked && pipe.num_urb_done==0 && !pipe.cs_flags.has_urb && port.num_pipes_idle==1 && port.num_pipes_queued==0);
#if PATCHED
 assert(calls==32 && unsafe_calls==0 && max_bytes==256);
#else
 assert(calls==1 && unsafe_calls==1 && max_bytes==8192);
#endif
 unsigned before=calls;assert(hcd_urb_dequeue(&pipe)==0 && calls==before);
 pipe.ep_char.bEndpointAddress=1;
 cache_sync_data_buffer(&pipe,&owned,true);assert(calls==before); /* OUT completion: no invalidate */
 owned.transfer.data_buffer_size=576;
 cache_sync_data_buffer(&pipe,&owned,false);
#if PATCHED
 assert(calls-before==3 && max_bytes==256 && unsafe_calls==0);
#else
 assert(calls-before==1);
#endif
 return 0;
}
'''
        with tempfile.TemporaryDirectory(prefix='p4-usb-cache-') as directory:
            path = pathlib.Path(directory)
            for name, body, enabled in [('upstream', original, 0), ('patched', patched, 1)]:
                c = path / (name + '.c')
                c.write_text(preamble + body + harness)
                exe = path / (name + '.exe')
                subprocess.run(['gcc', '-std=c11', '-O2', '-Wall', '-Wextra', '-Werror',
                                '-DPATCHED=' + str(enabled), str(c), '-o', str(exe)], check=True)
                subprocess.run([str(exe)], check=True)


if __name__ == '__main__':
    unittest.main()
