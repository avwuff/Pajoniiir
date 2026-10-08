"""Execute the actual configure-time SDMMC initialization replacement."""
import pathlib
import re
import subprocess
import tempfile
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[2]


class SharedHost(unittest.TestCase):
    def test_off_on_preserves_sd_owner(self):
        text = (ROOT / 'firmware/p4-core/cmake/apply_esp_hosted_shared_sdmmc_patch.cmake').read_text()
        old = re.search(r'set\(_p4_sdio_old \[=\[(.*?)\]=\]\)', text, re.S)[1]
        new = re.search(r'set\(_p4_sdio_new \[=\[(.*?)\]=\]\)', text, re.S)[1]
        upstream = ROOT / 'firmware/main-deck-m3/managed_components/espressif__esp_hosted/host/port/esp/freertos/src/port_esp_hosted_host_sdio.c'
        if upstream.exists():
            self.assertEqual(upstream.read_text().count(old), 1)
        preamble = r'''
#include <assert.h>
#include <stdbool.h>
#define ESP_OK 0
#define ESP_IDF_VERSION_VAL(a,b,c) ((a)*10000+(b)*100+(c))
typedef struct {bool host_initialized; int num_of_init_slots;} sdmmc_host_state_t;
static bool initialized;
static int init_calls, query_error, init_error;
static int sdmmc_host_init(void) {
    init_calls++;
    if (initialized) return 9; /* IDF 6 controller already claimed */
    if (init_error) return init_error;
    initialized=true; return ESP_OK;
}
static int __attribute__((unused)) sdmmc_host_get_state(sdmmc_host_state_t *s) {
    if(query_error)return query_error;
    s->host_initialized=initialized; return ESP_OK;
}
'''
        def function(name, block):
            return f'static int {name}(void) {{ int res;\n' + block + '\nreturn res; } return ESP_OK; }\n'
        harness = r'''
int main(void) {
    initialized=true; init_calls=0;
    assert(upstream()==9); /* reproduces OFF->ON with slot 0 still mounted */
    init_calls=0;
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(6,0,0)
    assert(patched()==ESP_OK && init_calls==0 && initialized);
    /* Repeated release/recreate of slot 1 must preserve the same controller. */
    for(int i=0;i<10;i++)assert(patched()==ESP_OK && init_calls==0);
    initialized=false; assert(patched()==ESP_OK && init_calls==1);
    query_error=5; assert(patched()==5 && init_calls==1); query_error=0;
    initialized=false; init_error=7; assert(patched()==7 && init_calls==2);
#else
    assert(patched()==9 && init_calls==1); /* other IDF behavior unchanged */
#endif
    return 0;
}
'''
        with tempfile.TemporaryDirectory() as d:
            src = pathlib.Path(d) / 'shared.c'
            src.write_text(preamble + function('upstream', old) + function('patched', new) + harness)
            for version in (60002, 50400):
                exe = pathlib.Path(d) / f'shared-{version}.exe'
                subprocess.run(['gcc', '-std=c11', '-Wall', '-Wextra', '-Werror', f'-DESP_IDF_VERSION={version}', str(src), '-o', str(exe)], check=True)
                subprocess.run([str(exe)], check=True)


if __name__ == '__main__':
    unittest.main()
