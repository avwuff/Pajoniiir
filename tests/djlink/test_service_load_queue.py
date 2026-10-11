"""Exercise production dequeue with a contended snapshot mutex."""
from pathlib import Path
import os
import shutil
import subprocess

root = Path(__file__).resolve().parents[2]
source = (root / 'firmware/p4-core/components/dj_link_service/dj_link_service.c').read_text()
start = source.index('bool dj_link_service_take_load(')
end = source.index('\nstatic portMUX_TYPE s_summary_lock', start)
code = r'''
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
typedef struct { unsigned id; } dj_link_incoming_load_t;
#define pdTRUE 1
static int s_loads=1,s_snapshot_lock=1,s_snapshot=1;
static bool locked,queued=true,current=true;
static unsigned receives,gives;
static int xSemaphoreTake(int lock,unsigned timeout) {
 assert(lock && !timeout);if(locked)return 0;locked=true;return 1;
}
static void xSemaphoreGive(int lock) { assert(lock && locked);locked=false;++gives; }
static int xQueueReceive(int queue,dj_link_incoming_load_t *out,unsigned timeout) {
 assert(queue && !timeout);++receives;if(!queued)return 0;queued=false;out->id=42;return 1;
}
static uint64_t esp_timer_get_time(void) {return 1000;}
static bool dj_link_browse_load_current(int snapshot,const dj_link_incoming_load_t *out,uint32_t now) {
 assert(snapshot && out->id==42 && now==1);return current;
}
''' + source[start:end] + r'''
int main(void) {
 dj_link_incoming_load_t out={0};
 locked=true;assert(!dj_link_service_take_load(&out));assert(queued && !receives);
 locked=false;assert(dj_link_service_take_load(&out));assert(!queued && gives==1);
 assert(!dj_link_service_take_load(&out));assert(!locked && gives==2);
 queued=true;current=false;assert(!dj_link_service_take_load(&out));assert(!queued && !locked);
 s_snapshot_lock=0;queued=true;assert(!dj_link_service_take_load(&out));assert(queued);
 puts("PASS production DJ Link load retention during snapshot contention and stale load rejection");
}
'''
directory = root / '.cache/djlink-load-queue'
directory.mkdir(parents=True, exist_ok=True)
c_file = directory / 'test.c'
c_file.write_text(code)
target = directory / ('test.exe' if os.name == 'nt' else 'test')
compiler = shutil.which('gcc')
if not compiler:
    raise RuntimeError('gcc is required')
subprocess.run([compiler, '-std=c99', '-Wall', '-Wextra', '-Werror', str(c_file), '-o', str(target)], check=True)
subprocess.run([str(target)], check=True)
