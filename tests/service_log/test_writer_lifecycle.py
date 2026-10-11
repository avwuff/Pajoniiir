"""Run production writer/init/open paths through deterministic storage and task failures."""
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
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <setjmp.h>
#include <sys/stat.h>
#undef NULL
#define NULL 0
typedef int esp_err_t;
typedef int SemaphoreHandle_t;
typedef struct { unsigned value; } service_log_record_t;
typedef struct { int kind;service_log_record_t record;int ack_sem;uint32_t barrier_id; } writer_item_t;
#define ESP_OK 0
#define ESP_FAIL 1
#define ESP_ERR_NO_MEM 2
#define pdTRUE 1
#define pdPASS 1
#define pdMS_TO_TICKS(n) (n)
#define WRITER_ITEM_RECORD 0
#define WRITER_ITEM_SYNC 1
#define WRITER_WAIT_MS 2000u
#define WRITER_BATCH 32u
#define WRITER_STACK 4096
#define WRITER_PRIO 2
#define SERVICE_LOG_QUEUE_LEN 128
#define SERVICE_LOG_TEXT_MAX 48
#define SERVICE_LOG_LINE_MAX 200
#define SERVICE_LOG_BOOT 1
#define SERVICE_LOG_INFO 1
#define LOG_DIR "unused"
#define LOG_BASE "unused/system.log"
static int s_queue,s_sync_ack,s_sync_lock,s_writer;
static unsigned allocated,deleted,queue_deleted,written,received;
static uint32_t s_boot_id,s_sync_completed_id;
static uint64_t s_current_bytes;
static int64_t s_last_sync_us;
static char s_fw_version[48],s_partition[48],s_reset_reason[48];
static FILE *s_fp;
static bool s_available,s_header_written,s_dirty,storage_ready,task_failure,mkdir_failure;
static esp_err_t s_last_error;
static writer_item_t queue[4];
static unsigned count;
static jmp_buf done;
static void publish_status(void) {}
static void service_log_sanitize(char *out,size_t len,const char *text) { snprintf(out,len,"%s",text); }
static int xQueueCreate(unsigned len,size_t size) { assert(len && size);return 1; }
static void vQueueDelete(int queue_id) { assert(queue_id);++queue_deleted; }
static int xSemaphoreCreateBinary(void) { return ++allocated; }
static int xSemaphoreCreateMutex(void) { return ++allocated; }
static void vSemaphoreDelete(int sem) { assert(sem);++deleted; }
static void xSemaphoreGive(int sem) { assert(sem); }
static uint32_t load_boot_id(void) { return 7; }
static int64_t esp_timer_get_time(void) { return 17; }
static int xTaskCreate(void (*fn)(void*),const char *name,unsigned stack,void *arg,unsigned prio,int *task) {
 assert(fn && name && stack && !arg && prio);if(task_failure)return 0;*task=1;return pdPASS;
}
static void service_log_note(int event,int sev,const char *text) { assert(event && sev && text); }
static bool sd_ready(void) { return storage_ready; }
static void sd_io_gate_begin(void) {}
static void sd_io_gate_end(void) {}
static void flush_if_dirty(bool force) { (void)force; }
static int fake_mkdir(const char *path,int mode) { (void)path;(void)mode;return mkdir_failure?-1:0; }
static FILE *fake_fopen(const char *path,const char *mode) { (void)path;(void)mode;return tmpfile(); }
static int fake_stat(const char *path,struct stat *st) { (void)path;st->st_size=0;return 0; }
static int service_log_format_header(char *dst,size_t len,uint32_t boot,const char *fw,const char *part,const char *reason)
 { (void)dst;(void)len;(void)boot;(void)fw;(void)part;(void)reason;return 0; }
static int xQueuePeek(int q,writer_item_t *item,unsigned ticks) {
 assert(q);if(!count){if(!ticks)return 0;assert(ticks==WRITER_WAIT_MS);longjmp(done,2);}*item=queue[0];return pdTRUE;
}
static int xQueueReceive(int q,writer_item_t *item,unsigned ticks) {
 assert(q && !ticks && count && s_fp);++received;*item=queue[0];--count;
 memmove(queue,queue+1,count*sizeof(*queue));return pdTRUE;
}
static void vTaskDelay(unsigned ticks) { assert(ticks==WRITER_WAIT_MS);longjmp(done,1); }
static void write_record(const service_log_record_t *record) { assert(s_fp && record);++written; }
#define mkdir fake_mkdir
#define fopen fake_fopen
#define stat fake_stat
'''
# Keep the struct stat type unaffected by the function-like macro.
code = code.replace('#define stat fake_stat', '#define stat(path, st) fake_stat(path, st)')
for signature in ('static esp_err_t open_active(', 'static void ensure_open(',
                  'static void acknowledge_barrier(', 'static void writer_task(',
                  'esp_err_t service_log_init('):
    code += function(signature)
code += r'''
int main(void) {
 task_failure=true;assert(service_log_init("v","ota","reset")==ESP_ERR_NO_MEM);
 assert(allocated==2 && deleted==2 && queue_deleted==1 && !s_queue && !s_sync_ack && !s_sync_lock);
 task_failure=false;assert(service_log_init("v","ota","reset")==ESP_OK);
 assert(allocated==4 && s_sync_ack && s_sync_lock);
 queue[0].record.value=1;queue[1].record.value=2;count=2;
 if(!setjmp(done))writer_task(NULL);
 assert(count==2 && !received && !written && !s_fp);
 storage_ready=true;
 if(!setjmp(done))writer_task(NULL);
 assert(!count && received==2 && written==2 && s_available);
 fclose(s_fp);s_fp=NULL;mkdir_failure=true;
 assert(open_active()==ESP_FAIL && !s_available && !s_fp);
}
'''
out = ROOT/'.cache/service_log_writer_review'
out.mkdir(parents=True, exist_ok=True)
cfile = out/'test.c'
cfile.write_text(code)
exe = out/('test.exe' if os.name == 'nt' else 'test')
subprocess.run([shutil.which('gcc'), '-std=c11', '-Wall', '-Wextra', '-Werror', str(cfile), '-o', str(exe)], check=True)
subprocess.run([str(exe)], check=True)
print('Production service-log early records, failed reopen and init cleanup PASS')
