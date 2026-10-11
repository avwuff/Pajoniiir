"""Execute manager init/daemon with a completion arriving after the startup timeout."""
from pathlib import Path
import os
import shutil
import subprocess

ROOT = Path(__file__).resolve().parents[2]
source = (ROOT/'firmware/p4-core/components/usb_host_manager/usb_host_manager.c').read_text()
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
#include <string.h>
#include <setjmp.h>
typedef int esp_err_t;
typedef int BaseType_t;
typedef struct { unsigned count; } StaticSemaphore_t;
typedef StaticSemaphore_t *SemaphoreHandle_t;
typedef void *TaskHandle_t;
typedef int manager_state_t;
typedef struct { bool override_fs_phy_index,root_port_unpowered; unsigned fs_phy_index;
 unsigned peripheral_map,daemon_stack_size,daemon_priority; int daemon_core_id,intr_flags; } usb_host_manager_config_t;
typedef struct { bool skip_phy_setup,root_port_unpowered; unsigned peripheral_map;int intr_flags;void *enum_filter_cb; } usb_host_config_t;
#define ESP_OK 0
#define ESP_ERR_INVALID_ARG 1
#define ESP_ERR_TIMEOUT 2
#define ESP_ERR_INVALID_STATE 3
#define ESP_ERR_NO_MEM 4
#define ESP_ERR_NOT_SUPPORTED 5
#define MANAGER_STOPPED 0
#define MANAGER_STARTING 1
#define MANAGER_READY 2
#define MANAGER_FAILED 3
#define pdTRUE 1
#define pdPASS 1
#define pdMS_TO_TICKS(n) (n)
#define tskNO_AFFINITY -1
#define portMAX_DELAY UINT32_MAX
#define RECOVERY_RETRY_BASE_MS 100
#define RECOVERY_RETRY_MAX_MS 1000
#define RECOVERY_QUEUE_DEPTH 8
#define USB_HOST_LIB_EVENT_FLAGS_NO_CLIENTS 1
#define USB_HOST_LIB_EVENT_FLAGS_ALL_FREE 2
#define ESP_LOGI(...) ((void)0)
#define ESP_LOGE(...) ((void)0)
#define ESP_LOGW(...) ((void)0)
#define configASSERT assert
static manager_state_t s_state;
static usb_host_manager_config_t s_config;
static esp_err_t s_install_result;
static TaskHandle_t s_topology_task,s_daemon_task,s_recovery_task;
static void *s_recovery_queue;
static int s_topology,s_recovery_arbiter;
static uint32_t s_root_power_requested_mask,s_daemon_iterations,s_daemon_errors,s_no_clients_events,s_all_free_events;
static StaticSemaphore_t s_startup_done_storage,s_topology_done_storage;
static SemaphoreHandle_t s_startup_done,s_topology_done;
typedef struct { unsigned port,reason; } recovery_request_t;
static unsigned unrelated_notification=77,installs;
static jmp_buf done;
static void state_set(manager_state_t state) { s_state=state; }
static SemaphoreHandle_t xSemaphoreCreateBinaryStatic(StaticSemaphore_t *storage) { storage->count=0;return storage; }
static int xSemaphoreTake(SemaphoreHandle_t sem,unsigned ticks) { (void)ticks;if(!sem->count)return 0;--sem->count;return pdTRUE; }
static void xSemaphoreGive(SemaphoreHandle_t sem) { ++sem->count; }
static void usb_host_topology_init(int *t) { *t=0; }
static void usb_host_recovery_arbiter_init(int *a,unsigned b,unsigned c) { (void)b;(void)c;*a=0; }
static void *xQueueCreate(unsigned n,size_t size) { assert(n && size);return (void*)1; }
static void vQueueDelete(void *q) { assert(q); }
static void vTaskDelete(void *t) { (void)t; }
static void vTaskDelay(unsigned ticks) { (void)ticks; }
static void recovery_task(void *arg) { (void)arg; }
static void topology_client_task(void *arg) { assert(!arg); }
static int xTaskCreate(void (*fn)(void*),const char *name,unsigned stack,void *arg,unsigned prio,TaskHandle_t *handle) {
 (void)stack;(void)prio;assert(!arg);*handle=(void*)1;
 if (!strcmp(name,"usb_topology")) { fn(arg);s_install_result=ESP_OK;xSemaphoreGive(s_topology_done); }
 return pdPASS;
}
static int xTaskCreatePinnedToCore(void (*fn)(void*),const char *name,unsigned stack,void *arg,unsigned prio,TaskHandle_t *handle,int core)
 { (void)core;return xTaskCreate(fn,name,stack,arg,prio,handle); }
static esp_err_t usb_host_install(const usb_host_config_t *cfg) { assert(cfg->peripheral_map==3);++installs;return ESP_OK; }
static esp_err_t usb_host_lib_handle_events(unsigned timeout,uint32_t *flags) { (void)timeout;(void)flags;longjmp(done,1);return ESP_OK; }
'''
code += function('static void usb_host_daemon_task(void *arg)')
code += function('esp_err_t usb_host_manager_init(')
code += r'''
int main(void) {
 usb_host_manager_config_t config={.peripheral_map=3,.daemon_stack_size=4096,.daemon_priority=3,.daemon_core_id=-1};
 assert(usb_host_manager_init(&config)==ESP_ERR_TIMEOUT && s_state==MANAGER_STARTING);
 assert(unrelated_notification==77 && !installs && !s_startup_done->count);
 if (!setjmp(done)) usb_host_daemon_task(NULL);
 assert(installs==1 && s_state==MANAGER_READY && s_startup_done->count==1);
 assert(unrelated_notification==77);
 assert(usb_host_manager_init(&config)==ESP_OK && installs==1);
 assert(xSemaphoreTake(s_startup_done,0)==pdTRUE && unrelated_notification==77);
}
'''
out = ROOT/'.cache/usb_manager_startup_review'
out.mkdir(parents=True, exist_ok=True)
cfile = out/'test.c'
cfile.write_text(code)
exe = out/('test.exe' if os.name == 'nt' else 'test')
subprocess.run([shutil.which('gcc'), '-std=c11', '-Wall', '-Wextra', '-Werror', str(cfile), '-o', str(exe)], check=True)
subprocess.run([str(exe)], check=True)
print('PASS production USB startup timeout/late daemon completion without caller notifications')
