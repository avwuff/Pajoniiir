"""Execute the production rescan admission/publication/callback boundary."""
from pathlib import Path
import os
import shutil
import subprocess

root = Path(__file__).resolve().parents[2]
source = (root/'firmware/p4-core/components/controller_profile_manager/controller_profile_manager.c').read_text(encoding='utf-8')
start = source.index('esp_err_t controller_profile_manager_scan_storage(void)')
opening = source.index('{', start)
depth, end = 1, opening+1
while depth:
    depth += (source[end]=='{') - (source[end]=='}')
    end += 1
code = r'''
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#define ESP_OK 0
#define ESP_FAIL 1
#define ESP_ERR_NO_MEM 2
#define ESP_ERR_INVALID_STATE 3
#define ESP_ERR_NOT_FOUND 4
#define CPM_TRANSFER_TRANSFERRING 2
#define CONFIG_CONTROLLER_PROFILE_SD_PATH "/sd/controllers"
#define ESP_LOGI(...) ((void)0)
#define ESP_LOGW(...) ((void)0)
typedef int esp_err_t;
typedef struct { bool controller_present; unsigned transfer_state, count; } controller_profile_registry_t;
static controller_profile_registry_t s_registry;
static bool s_storage_busy;
static unsigned locks, sd_locks, scans, callbacks;
static esp_err_t scan_result;
static bool cpm_lock(void) { assert(!locks); locks++; return true; }
static void cpm_unlock(void) { assert(locks==1); locks--; }
static void sd_io_gate_begin(void) { assert(!locks); sd_locks++; }
static void sd_io_gate_end(void) { assert(sd_locks==1); sd_locks--; }
static esp_err_t controller_profile_scan_dir(const char *path, controller_profile_registry_t *out)
{ (void)path;assert(sd_locks==1 && !locks && s_storage_busy);out->count=3;scans++;return scan_result; }
static void controller_profile_registry_apply_rescan(controller_profile_registry_t *live, const controller_profile_registry_t *scan)
{ assert(locks==1 && s_storage_busy);live->count=scan->count;live->transfer_state=1; }
static void callback(void)
{ assert(!locks && !sd_locks && !s_storage_busy && s_registry.controller_present && s_registry.count==3);callbacks++; }
static void (*s_reactivate_callback)(void)=callback;
'''
code += source[start:end] + r'''
int main(void) {
    s_registry.controller_present=true;
    assert(controller_profile_manager_scan_storage()==ESP_OK);
    assert(callbacks==1 && scans==1 && !s_storage_busy);
    s_registry.controller_present=false;
    assert(controller_profile_manager_scan_storage()==ESP_OK);
    assert(callbacks==1 && scans==2);
    s_registry.controller_present=true;scan_result=ESP_ERR_NOT_FOUND;
    assert(controller_profile_manager_scan_storage()==ESP_ERR_NOT_FOUND);
    assert(callbacks==1 && scans==3 && !s_storage_busy);
    scan_result=ESP_OK;s_registry.transfer_state=CPM_TRANSFER_TRANSFERRING;
    assert(controller_profile_manager_scan_storage()==ESP_ERR_INVALID_STATE);
    assert(callbacks==1 && scans==3 && !s_storage_busy && !locks);
    s_registry.transfer_state=0;s_reactivate_callback=NULL;
    assert(controller_profile_manager_scan_storage()==ESP_OK);
    assert(callbacks==1 && scans==4 && !s_storage_busy);
}
'''
out = root/'.cache/profile_rescan_review'
out.mkdir(parents=True, exist_ok=True)
cfile = out/'test.c'
cfile.write_text(code, encoding='utf-8')
exe = out/('test.exe' if os.name=='nt' else 'test')
subprocess.run([shutil.which('gcc'), '-std=c11', '-Wall', '-Wextra', '-Werror', str(cfile), '-o', str(exe)], check=True)
subprocess.run([str(exe)], check=True)
print('PASS production rescan: connected callback after locks/busy release; failure/disconnected/inflight/no-callback paths')
