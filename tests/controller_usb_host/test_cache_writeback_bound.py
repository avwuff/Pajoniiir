"""Execute the actual cache wrapper, including error and context boundaries."""
import pathlib
import subprocess
import tempfile
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[2]


class WritebackBound(unittest.TestCase):
    def test_production_wrapper(self):
        source = (ROOT / 'firmware/p4-core/components/usb_storage/usb_dwc_hal_compat.c').read_text()
        wrapper = source[source.index('esp_err_t __real_esp_cache_msync'):source.index('void __real_usb_dwc_hal_chan_activate')]
        preamble = r'''
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <assert.h>
#define CONFIG_CONTROLLER_USB_HS_AUDIO_DIAGNOSTIC 1
#define CONFIG_ESP_MM_CACHE_MSYNC_C2M_CHUNKED_OPS 0
#define ESP_CACHE_MSYNC_FLAG_DIR_M2C 1
#define ESP_CACHE_MSYNC_FLAG_TYPE_INST 4
#define ESP_CACHE_MSYNC_FLAG_DIR_C2M 8
#define ESP_CACHE_MSYNC_FLAG_UNALIGNED 2
#define CACHE_TYPE_DATA 0
#define ESP_OK 0
typedef int esp_err_t;
static bool yielding=true,valid_range=true;
static unsigned calls,fail_at;
static uintptr_t expected;
static size_t copied,maximum;
static bool __attribute__((unused)) xPortCanYield(void) {return yielding;}
static bool __attribute__((unused)) cache_hal_vaddr_to_cache_level_id(uint32_t p,size_t n,uint32_t *l,uint32_t *id) {
 (void)p;(void)n;*l=1;*id=0;return valid_range;
}
static size_t __attribute__((unused)) cache_hal_get_cache_line_size(uint32_t l,int type) {(void)l;(void)type;return 64;}
static void reset(uintptr_t p) {calls=0;copied=0;maximum=0;expected=p;fail_at=0;}
'''
        harness = r'''
esp_err_t __real_esp_cache_msync(void *p,size_t n,int flags) {
 calls++;assert((uintptr_t)p==expected);
 if (fail_at==calls)return 7;
 if ((flags&ESP_CACHE_MSYNC_FLAG_DIR_M2C) && (flags&(ESP_CACHE_MSYNC_FLAG_UNALIGNED|ESP_CACHE_MSYNC_FLAG_DIR_C2M)))return 7;
 if(!(flags & ESP_CACHE_MSYNC_FLAG_UNALIGNED) && (((uintptr_t)p%64) || (n%64))) return 7;
 if(n>maximum)maximum=n;
 copied+=n;expected+=n;return ESP_OK;
}
int main(void) {
#if !CONFIG_CONTROLLER_USB_HS_AUDIO_DIAGNOSTIC
 reset(0x1000);assert(__wrap_esp_cache_msync((void *)0x1000,16384,0)==0);
 assert(calls==1 && maximum==16384 && copied==16384);
#else
 reset(0x1000);assert(__wrap_esp_cache_msync((void *)0x1000,9000,2)==ESP_OK);
 assert(calls==9 && maximum==1024 && copied==9000 && expected==0x1000+9000);
 reset(0x1000);fail_at=2;assert(__wrap_esp_cache_msync((void *)0x1000,16384,0)==7);
 assert(calls==2 && copied==1024);
 reset(0x1000);assert(__wrap_esp_cache_msync((void *)0x1000,9000,0)==7);assert(calls==1 && copied==0);
 reset(0x1000);assert(__wrap_esp_cache_msync((void *)0x1000,8192,1)==0);assert(calls==8 && maximum==1024);
 /* Previously delegated 2-KiB operations must also release the lock between chunks. */
 reset(0x1000);assert(__wrap_esp_cache_msync((void *)0x1000,2048,0)==0);assert(calls==2 && maximum==1024);
 reset(0x1000);assert(__wrap_esp_cache_msync((void *)0x1000,1024,1)==0);assert(calls==1 && copied==1024);
 reset(0x1000);assert(__wrap_esp_cache_msync((void *)0x1000,8192,3)==7);assert(calls==1 && copied==0);
 reset(0x1000);assert(__wrap_esp_cache_msync((void *)0x1000,8192,9)==7);assert(calls==1 && copied==0);
 reset(0x1000);yielding=false;assert(__wrap_esp_cache_msync((void *)0x1000,16384,0)==0);assert(calls==1);
 reset(0x1000);yielding=true;valid_range=false;
 assert(__wrap_esp_cache_msync((void *)0x1000,16384,0)==0);assert(calls==1);
#endif
 return 0;
}
'''
        with tempfile.TemporaryDirectory(prefix='p4-cache-bound-') as directory:
            path = pathlib.Path(directory)
            for enabled in (1, 0):
                text = (preamble + wrapper + harness).replace(
                    '#define CONFIG_CONTROLLER_USB_HS_AUDIO_DIAGNOSTIC 1',
                    '#define CONFIG_CONTROLLER_USB_HS_AUDIO_DIAGNOSTIC ' + str(enabled))
                (path / 'bound.c').write_text(text)
                exe = path / 'bound.exe'
                subprocess.run(['gcc', '-std=c11', '-O2', '-Wall', '-Wextra', '-Werror',
                                str(path / 'bound.c'), '-o', str(exe)], check=True)
                subprocess.run([str(exe)], check=True)


if __name__ == '__main__':
    unittest.main()
