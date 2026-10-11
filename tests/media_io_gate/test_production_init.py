"""Interleave two production initializers; inject allocation failure into both gates."""
from pathlib import Path
import os
import shutil
import subprocess

ROOT = Path(__file__).resolve().parents[2]

def function(source, signature):
    start = source.rindex(signature)
    end = source.index('{', start) + 1
    depth = 1
    while depth:
        depth += (source[end] == '{') - (source[end] == '}')
        end += 1
    return source[start:end]

for name in ('media_io_gate', 'sd_io_gate'):
    source = (ROOT/f'firmware/p4-core/components/{name}/{name}.c').read_text()
    code = r'''
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <setjmp.h>
typedef int esp_err_t;
typedef void *SemaphoreHandle_t;
#define ESP_OK 0
#define ESP_FAIL 1
#define ESP_ERR_INVALID_STATE 2
#define pdTRUE 1
#define pdMS_TO_TICKS(n) (n)
#define portMAX_DELAY UINT32_MAX
#define ESP_LOGE(...) ((void)0)
static SemaphoreHandle_t s_gate,taken;
static unsigned creates,deletes,takes;
static bool interleave,allocation_failure;
static int64_t s_acquired_us;
static struct { uint32_t waits,timeouts,max_wait_us,max_hold_us; } s_stats;
static jmp_buf failed;
#define ESP_ERROR_CHECK(rc) do { if ((rc)!=ESP_OK) longjmp(failed,1); } while(0)
static int64_t esp_timer_get_time(void) { return 1; }
static void note_inc(uint32_t *n) { ++*n; }
static void note_max(uint32_t *n,int64_t us) { (void)n;(void)us; }
static void vSemaphoreDelete(SemaphoreHandle_t h) { assert(h);++deletes; }
static int xSemaphoreTake(SemaphoreHandle_t h,uint32_t ticks) { assert(h);(void)ticks;taken=h;++takes;return pdTRUE; }
static void xSemaphoreGive(SemaphoreHandle_t h) { assert(h==taken); }
static SemaphoreHandle_t xSemaphoreCreateMutex(void);
'''
    for suffix in ('init(void)', 'begin(void)', 'try_begin(uint32_t timeout_ms)', 'end(void)'):
        signature = ('esp_err_t ' if suffix.startswith('init') else 'bool ' if suffix.startswith('try') else 'void ') + name + '_' + suffix
        code += function(source, signature)
    code += f'''
static SemaphoreHandle_t xSemaphoreCreateMutex(void) {{
 if(allocation_failure)return NULL;
 SemaphoreHandle_t candidate=(void*)(uintptr_t)(++creates);
 if(interleave){{interleave=false;assert({name}_init()==ESP_OK);}}
 return candidate;
}}
int main(void) {{
 interleave=true;assert({name}_init()==ESP_OK);
 assert(creates==2 && deletes==1 && s_gate==(void*)2);
 {name}_begin();assert(taken==s_gate && takes==1);{name}_end();
 assert({name}_try_begin(0) && taken==s_gate);{name}_end();
 s_gate=NULL;allocation_failure=true;
 assert(!{name}_try_begin(0));
 if(!setjmp(failed)){{{name}_begin();assert(0);}}
 assert(takes==2);
}}
'''
    out = ROOT/f'.cache/{name}_init_review'
    out.mkdir(parents=True, exist_ok=True)
    cfile = out/'test.c'
    cfile.write_text(code)
    exe = out/('test.exe' if os.name == 'nt' else 'test')
    subprocess.run([shutil.which('gcc'), '-std=c11', '-Wall', '-Wextra', '-Werror',
                    '-Wno-unused-function', '-Wno-unused-variable', str(cfile), '-o', str(exe)], check=True)
    subprocess.run([str(exe)], check=True)
    print(f'Production {name} single-mutex publication and allocation fail-closed PASS')
