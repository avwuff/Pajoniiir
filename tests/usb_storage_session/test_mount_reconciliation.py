"""Execute production mount publication after a collapsed unplug/replug edge."""
from pathlib import Path
import os
import shutil
import subprocess

root=Path(__file__).resolve().parents[2]
source=(root/'firmware/p4-core/components/usb_storage/usb_storage.c').read_text(encoding='utf-8')
def function(signature):
    start=source.index(signature);end=source.index('{',start)+1;depth=1
    while depth:
        depth+=(source[end]=='{')-(source[end]=='}');end+=1
    return source[start:end]
code=r'''
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdatomic.h>
#include "usb_storage_session.h"
typedef int esp_err_t;
#define ESP_OK 0
#define ESP_ERR_INVALID_STATE -1
#define ESP_ERR_NOT_SUPPORTED -2
#define ESP_LOGW(...) ((void)0)
#define ESP_LOGI(...) ((void)0)
#define USB_STORAGE_DEVICE_ROUTE_ALLOWED(addr) ((addr)>0)
#define USB_STORAGE_MOUNT_POINT "/usb"
#define portENTER_CRITICAL(mux) ((void)0)
#define portEXIT_CRITICAL(mux) ((void)0)
typedef struct { bool format_if_mount_failed;int max_files, allocation_unit_size; } esp_vfs_fat_mount_config_t;
static usb_storage_session_t s_session;
static bool s_announced_mounted, available;
static void *s_mount,*s_msc_dev;
static atomic_uint s_mount_attempts,s_mount_successes;
static atomic_int s_last_mount_result;
static int events[8],event_count,releases,mount_rc;
static void callback(bool mounted) { events[event_count++]=mounted;assert(available==mounted); }
static void (*s_cb)(bool)=callback;
static void media_io_gate_set_available(bool value) { available=value; }
static bool desired_matches(uint32_t epoch,uint8_t addr) { return usb_storage_session_matches(&s_session,epoch,addr); }
static void release_device(void) { assert(!s_announced_mounted && !available);releases++;s_mount=NULL;s_msc_dev=NULL; }
static int msc_host_install_device(uint8_t addr,void **out) { (void)addr;*out=(void*)2;return 0; }
static int usb_media_mount(void *dev,const char *path,const esp_vfs_fat_mount_config_t *cfg,void **out)
{ (void)dev;(void)path;(void)cfg;if (!mount_rc) *out=(void*)3;return mount_rc; }
static void log_device_info(const char *s) { (void)s; }
static void log_mount_layout(void) {}
'''
for signature in ['static void publish_unmounted(', 'static bool commit_mounted(', 'static esp_err_t mount_desired_device(']:
    code+=function(signature)
code+=r'''
static void rapid_reconnect(void) {
    usb_storage_session_on_connect(&s_session,1);
    assert(usb_storage_session_bind_handle(&s_session,s_session.epoch,1,1));
    assert(usb_storage_session_commit_mounted(&s_session,s_session.epoch,1));
    s_announced_mounted=available=true;s_mount=s_msc_dev=(void*)1;
    callback(true);
    assert(usb_storage_session_on_disconnect(&s_session,1)==USB_STORAGE_DISCONNECT_ACCEPTED);
    usb_storage_session_on_connect(&s_session,2);
}
int main(void) {
    rapid_reconnect();
    assert(mount_desired_device(s_session.epoch,2)==ESP_OK);
    assert(event_count==3 && events[0]==1 && events[1]==0 && events[2]==1 && releases==1);
    assert(s_session.mounted && available && s_announced_mounted);
    event_count=releases=0;usb_storage_session_reset(&s_session);
    rapid_reconnect();mount_rc=-3;
    assert(mount_desired_device(s_session.epoch,2)==-3);
    assert(event_count==2 && events[0]==1 && events[1]==0 && releases==2);
    assert(!s_session.mounted && !available && !s_announced_mounted);
    mount_rc=0;
    assert(mount_desired_device(s_session.epoch,2)==ESP_OK);
    assert(event_count==3 && events[2]==1);
}
'''
out=root/'.cache/storage_mount_review';out.mkdir(parents=True,exist_ok=True)
cfile=out/'test.c';cfile.write_text(code,encoding='utf-8')
exe=out/('test.exe' if os.name=='nt' else 'test')
component=root/'firmware/p4-core/components/usb_storage'
subprocess.run([shutil.which('gcc'),'-std=c11','-Wall','-Wextra','-Werror',
               '-I'+str(component/'include'),str(cfile),str(component/'usb_storage_session.c'),'-o',str(exe)],check=True)
subprocess.run([str(exe)],check=True)
print('PASS production storage mount: fast reconnect publishes false before release/remount; failed mount/retry ordering')
