"""Execute production sync admission with a late acknowledgement from an older barrier."""
from pathlib import Path
import os
import shutil
import subprocess

ROOT = Path(__file__).resolve().parents[2]
source = (ROOT/'firmware/p4-core/components/service_log/service_log.c').read_text()

def function(signature):
    start = source.index(signature)
    end = source.index('{', start) + 1
    depth = 1
    while depth:
        depth += (source[end] == '{') - (source[end] == '}')
        end += 1
    return source[start:end]

code = r'''
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
typedef uint32_t TickType_t;
typedef int SemaphoreHandle_t;
typedef struct { int kind; int ack_sem; uint32_t barrier_id; } writer_item_t;
#define WRITER_ITEM_SYNC 1
#define SYNC_WAIT_MS 10000u
#define pdTRUE 1
#define pdMS_TO_TICKS(n) (n)
#define ESP_LOGW(...) ((void)0)
static int s_queue=1,s_writer=1,s_sync_ack=2,s_sync_lock=1;
static uint32_t s_sync_next_id,s_sync_completed_id;
static TickType_t now;
static unsigned sends,waits,signals;
static bool fail_enqueue,complete_current;
static writer_item_t prior,current;
static TickType_t xTaskGetTickCount(void) { return now; }
static int xQueueSend(int queue,const writer_item_t *item,TickType_t ticks) {
 assert(queue && ticks==SYNC_WAIT_MS);if(fail_enqueue)return 0;
 prior=current;current=*item;++sends;return pdTRUE;
}
static void xSemaphoreGive(int sem) {
 if(sem==s_sync_ack){assert(s_sync_completed_id==current.barrier_id);++signals;}
}
static int xSemaphoreTake(int sem,TickType_t ticks) {
 if(sem==s_sync_lock)return pdTRUE;
 assert(sem==s_sync_ack);if(!ticks)return 0;++waits;
 if(sends==1){now+=ticks;return 0;} /* first request times out */
 if(waits==2){ /* old request completes while the second waits */
   now+=3;s_sync_completed_id=prior.barrier_id;return pdTRUE;
 }
 if(complete_current){now+=2;s_sync_completed_id=current.barrier_id;return pdTRUE;}
 now+=ticks;return 0;
}
'''
code += function('void service_log_sync(void)')
if 'static void acknowledge_barrier(' in source:
    code += function('static void acknowledge_barrier(')
code += r'''
int main(void) {
 service_log_sync();assert(sends==1 && waits==1);
 complete_current=true;service_log_sync();
 assert(sends==2 && waits==3); /* a stale ack must not finish request two */
 assert(s_sync_completed_id==current.barrier_id);
 fail_enqueue=true;service_log_sync();assert(sends==2 && waits==3);
 fail_enqueue=false;complete_current=false;now=UINT32_MAX-5;
 service_log_sync();assert(sends==3 && waits==4);
'''
if 'static void acknowledge_barrier(' in source:
    code += ' acknowledge_barrier(&current);assert(signals==1);\n'
code += '}\n'
out = ROOT/'.cache/service_log_sync_review'
out.mkdir(parents=True, exist_ok=True)
cfile = out/'test.c'
cfile.write_text(code)
exe = out/('test.exe' if os.name == 'nt' else 'test')
subprocess.run([shutil.which('gcc'), '-std=c11', '-Wall', '-Wextra', '-Werror',
                '-Wno-unused-function', '-Wno-unused-variable', str(cfile), '-o', str(exe)], check=True)
subprocess.run([str(exe)], check=True)
print('Production service-log late acknowledgement and bounded timeout PASS')
