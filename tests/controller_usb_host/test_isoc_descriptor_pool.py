"""Execute generated HCD allocation/free with bounded internal ISO lists."""
import pathlib
import re
import subprocess
import tempfile
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[2]


class DescriptorPool(unittest.TestCase):
    def test_patched_hcd(self):
        cmake = (ROOT / 'firmware/p4-core/cmake/apply_espressif_usb_isoc_descriptor_patch.cmake').read_text()
        helpers = re.search(r'set\(_isoc_pool_helpers \[=\[\n(.*?)\]=\]\)', cmake, re.S)[1]
        helpers = helpers[:helpers.index('static dma_buffer_block_t *buffer_block_alloc(')]
        harness = r'''
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#define USB_TRANSFER_TYPE_ISOCHRONOUS 1
#define MALLOC_CAP_DMA 1
#define MALLOC_CAP_CACHE_ALIGNED 2
#define MALLOC_CAP_INTERNAL 4
#define XFER_DESC_LIST_CAPS 11
#define USB_DWC_QTD_LIST_MEM_ALIGN 512
#define XFER_LIST_LEN_ISOC 64
typedef int usb_transfer_type_t;
typedef struct {uint32_t words[2];} usb_dwc_ll_dma_qtd_t;
static unsigned heap_allocs,heap_frees;
static void *heap_caps_aligned_calloc(size_t a,size_t n,size_t count,uint32_t caps) {
 assert(a==512 && count==1 && caps==11);heap_allocs++;return calloc(1,n);
}
static void heap_caps_free(void *p) {heap_frees++;free(p);}
''' + helpers + r'''
int main(void) {
#if CONFIG_PAJONIIIR_BOARD_M3 && CONFIG_CONTROLLER_USB_HS_AUDIO_DIAGNOSTIC && CONFIG_USB_HOST_DWC_DMA_CAP_MEMORY_IN_PSRAM
 void *lists[4];
 for(int cycle=0;cycle<1000;cycle++) {
  for(unsigned i=0;i<4;i++) {
   lists[i]=pajoniiir_descriptor_alloc(1,512);assert(lists[i] && ((uintptr_t)lists[i]%512)==0);
   for(unsigned j=0;j<i;j++)assert(lists[i]!=lists[j]);
   for(unsigned j=0;j<512;j++)assert(((unsigned char*)lists[i])[j]==0);
   memset(lists[i],0xa5,512);
  }
  assert(!pajoniiir_descriptor_alloc(1,512)); /* Exhaustion cannot steal live ownership. */
  assert(!pajoniiir_descriptor_alloc(1,256));
  pajoniiir_descriptor_free(lists[2]);
  void *replacement=pajoniiir_descriptor_alloc(1,512);assert(replacement==lists[2]);
  for(unsigned i=0;i<4;i++)pajoniiir_descriptor_free(lists[i]);
  assert(s_internal_isoc_claimed==0);
 }
 assert(heap_allocs==0 && heap_frees==0);
 assert(pajoniiir_descriptor_caps(1)==7);
#else
 void *iso=pajoniiir_descriptor_alloc(1,512);assert(iso);pajoniiir_descriptor_free(iso);
 assert(heap_allocs==1 && heap_frees==1 && pajoniiir_descriptor_caps(1)==11);
#endif
 unsigned before=heap_allocs;
 void *bulk=pajoniiir_descriptor_alloc(2,16);assert(bulk && heap_allocs==before+1);
 pajoniiir_descriptor_free(bulk);assert(pajoniiir_descriptor_caps(2)==11);
 return 0;
}
'''
        with tempfile.TemporaryDirectory(prefix='p4-isoc-pool-') as d:
            src = pathlib.Path(d) / 'pool.c'
            src.write_text(harness)
            for m3, hs, psram in ((1, 1, 1), (0, 1, 1), (1, 0, 1), (1, 1, 0)):
                exe = pathlib.Path(d) / f'pool{m3}{hs}{psram}.exe'
                subprocess.run(['gcc', '-std=c11', '-Wall', '-Wextra', '-Werror',
                                f'-DCONFIG_PAJONIIIR_BOARD_M3={m3}',
                                f'-DCONFIG_CONTROLLER_USB_HS_AUDIO_DIAGNOSTIC={hs}',
                                f'-DCONFIG_USB_HOST_DWC_DMA_CAP_MEMORY_IN_PSRAM={psram}',
                                str(src), '-o', str(exe)], check=True)
                subprocess.run([str(exe)], check=True)


if __name__ == '__main__':
    unittest.main()
