"""Run the pinned URB allocator with the production CMake transformation."""
import pathlib
import re
import subprocess
import tempfile
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[2]


class PayloadAllocation(unittest.TestCase):
    def test_owned_payload(self):
        cmake = (ROOT / 'firmware/p4-core/cmake/apply_espressif_usb_isoc_payload_patch.cmake').read_text()
        old = re.search(r'set\(_payload_old \[=\[\n(.*?)\]=\]\)', cmake, re.S)[1]
        new = re.search(r'set\(_payload_new \[=\[\n(.*?)\]=\]\)', cmake, re.S)[1]
        source = (ROOT / 'tests/controller_usb_host/fixtures/usb_private_pinned.c').read_text()
        self.assertIn(old, source)
        source = source.replace(old, new).replace('esp_cache_get_alignment(DATA_BUFFER_CAPS,', 'esp_cache_get_alignment(payload_caps,').replace('heap_caps_malloc(data_buffer_size, DATA_BUFFER_CAPS)', 'heap_caps_malloc(data_buffer_size, payload_caps)')
        source = re.sub(r'^#include .*$', '', source, flags=re.M)
        harness = r'''
#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#define CONFIG_IDF_TARGET_LINUX 0
#define MALLOC_CAP_DEFAULT 0
#define MALLOC_CAP_DMA 1
#define MALLOC_CAP_CACHE_ALIGNED 2
#define MALLOC_CAP_INTERNAL 4
#define MALLOC_CAP_SPIRAM 8
typedef struct { int ignored; } usb_isoc_packet_desc_t;
typedef struct { void *data_buffer; size_t data_buffer_size; int num_isoc_packets; } usb_transfer_dummy_t;
typedef struct { usb_transfer_dummy_t transfer; } urb_t;
static unsigned align_caps, alloc_caps, live, fail_meta, fail_payload;
static void *heap_caps_calloc(size_t n,size_t sz,unsigned caps) {
 assert(caps==0);if(fail_meta)return NULL;void *p=calloc(n,sz);if(p)live++;return p;
}
static int esp_cache_get_alignment(unsigned caps,size_t *a) {
 align_caps=caps;*a=(caps & MALLOC_CAP_INTERNAL)?64:128;return 0;
}
static void *heap_caps_malloc(size_t sz,unsigned caps) {
 alloc_caps=caps;assert(caps==align_caps);assert(sz%((caps&4)?64:128)==0);
 if(fail_payload)return NULL;void *p=malloc(sz);if(p)live++;return p;
}
static void heap_caps_free(void *p) { if(p){assert(live);live--;free(p);} }
''' + source + r'''
int main(void) {
 for(int count=0;count<=56;count+=28) {
  urb_t *u=urb_alloc(5377,count);assert(u && u->transfer.num_isoc_packets==count);
#if CONFIG_PAJONIIIR_BOARD_M3 && CONFIG_CONTROLLER_USB_HS_AUDIO_DIAGNOSTIC && CONFIG_USB_HOST_DWC_DMA_CAP_MEMORY_IN_PSRAM
  assert(alloc_caps==(count?7:11));
#else
  assert(alloc_caps==DATA_BUFFER_CAPS);
#endif
  urb_free(u);assert(live==0);
 }
 fail_meta=1;assert(!urb_alloc(64,56));assert(live==0);fail_meta=0;
 fail_payload=1;assert(!urb_alloc(64,56));assert(live==0);
 urb_free(NULL);return 0;
}
'''
        with tempfile.TemporaryDirectory() as d:
            src = pathlib.Path(d) / 'payload.c'
            src.write_text(harness)
            for m3, hs, psram in ((1, 1, 1), (0, 1, 1), (1, 0, 1), (1, 1, 0)):
                exe = pathlib.Path(d) / f'payload{m3}{hs}{psram}.exe'
                subprocess.run(['gcc', '-std=c11', '-Wall', '-Wextra', '-Werror', '-Wno-misleading-indentation', f'-DCONFIG_PAJONIIIR_BOARD_M3={m3}', f'-DCONFIG_CONTROLLER_USB_HS_AUDIO_DIAGNOSTIC={hs}'] + (['-DCONFIG_USB_HOST_DWC_DMA_CAP_MEMORY_IN_PSRAM=1'] if psram else []) + [str(src), '-o', str(exe)], check=True)
                subprocess.run([str(exe)], check=True)


if __name__ == '__main__':
    unittest.main()
