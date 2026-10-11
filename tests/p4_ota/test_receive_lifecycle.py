"""Execute production OTA receive lifecycle with flash/PSA fault injection."""
from pathlib import Path
import os
import shutil
import subprocess

root = Path(__file__).resolve().parents[2]
source = (root/'firmware/p4-core/components/p4_ota/p4_ota.c').read_text(encoding='utf-8')
def function(signature):
    start = source.index(signature)
    end = source.index('{', start)+1
    depth = 1
    while depth:
        depth += (source[end]=='{') - (source[end]=='}')
        end += 1
    return source[start:end]

code = r'''
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
typedef int esp_err_t;
#define ESP_OK 0
#define ESP_FAIL -1
#define ESP_ERR_INVALID_STATE -2
#define ESP_ERR_INVALID_ARG -3
#define ESP_ERR_NOT_FOUND -4
#define ESP_ERR_INVALID_SIZE -5
#define ESP_ERR_INVALID_CRC -6
#define ESP_ERR_INVALID_RESPONSE -7
#define P4_OTA_IDLE 0
#define P4_OTA_RECEIVING 1
#define P4_OTA_READY_TO_REBOOT 2
#define P4_OTA_FAILED 3
#define DDJ_OTA_TARGET_P4 1
#define DDJ_OTA_SHA256_SIZE 32
#define DDJ_OTA_VERSION_SIZE 32
#define portMAX_DELAY 0
#define ESP_LOGW(...) ((void)0)
#define PSA_SUCCESS 0
#define PSA_ALG_SHA_256 0
typedef int psa_status_t;
typedef int psa_hash_operation_t;
typedef int esp_ota_handle_t;
typedef struct { size_t size; const char *label; } esp_partition_t;
typedef struct { int target; size_t image_size; uint8_t image_sha256[32]; char version[32]; } ddj_ota_manifest_t;
typedef struct { int state; size_t expected_size, received_size; char last_error[64], target_slot[17], target_version[33]; } p4_ota_status_t;
typedef struct { char project_name[32], version[32]; } esp_app_desc_t;
typedef int p4_ota_finish_policy_t;
#define P4_OTA_FINISH_INVALID_STATE 0
#define P4_OTA_FINISH_INCOMPLETE 1
#define P4_OTA_FINISH_VERIFY 2
#define P4_OTA_PROJECT_NAME "main-deck-m3"
static int s_lock=1, locks, begins, aborts, hash_aborts, boot_sets;
static p4_ota_status_t s_status;
static const esp_partition_t *s_target;
static esp_ota_handle_t s_handle;
static bool s_handle_open, s_image_sha_active;
static psa_hash_operation_t s_image_sha;
static uint8_t s_expected_sha256[32];
static char s_expected_version[32];
static int write_rc, hash_rc;
static esp_partition_t partition={4096,"ota_1"};
static void xSemaphoreTake(int lock,int ticks) { (void)lock;(void)ticks;assert(!locks);locks++; }
static void xSemaphoreGive(int lock) { (void)lock;assert(locks==1);locks--; }
static const char *esp_err_to_name(int rc) { (void)rc;return "injected fault"; }
static const esp_partition_t *esp_ota_get_next_update_partition(void *p) { (void)p;return &partition; }
static bool p4_ota_policy_size_valid(size_t n,size_t cap) { return n>=24 && n<=cap; }
static int esp_ota_begin(const esp_partition_t *p,size_t n,int *h) { (void)p;(void)n;begins++;*h=begins;return 0; }
static int esp_ota_abort(int h) { assert(h>0);aborts++;return 0; }
static int esp_ota_write(int h,const void *p,size_t n) { assert(h>0 && p && n);return write_rc; }
static int esp_ota_end(int h) { assert(h>0);return 0; }
static int esp_ota_get_partition_description(const esp_partition_t *p,esp_app_desc_t *d)
{ (void)p;strcpy(d->project_name,"main-deck-m3");strcpy(d->version,"M3-test");return 0; }
static int esp_ota_set_boot_partition(const esp_partition_t *p) { assert(p==&partition);boot_sets++;return 0; }
static int psa_hash_operation_init(void) { return 0; }
static int psa_hash_setup(int *h,int alg) { (void)alg;*h=1;return 0; }
static int psa_hash_update(int *h,const void *p,size_t n) { assert(*h && p && n);return hash_rc; }
static int psa_hash_abort(int *h) { assert(*h);hash_aborts++;*h=0;return 0; }
static int psa_hash_finish(int *h,uint8_t *out,size_t cap,size_t *n)
{ assert(*h && cap==32);memset(out,0,32);*n=32;*h=0;return 0; }
static int p4_ota_policy_finish(bool receiving,bool open,size_t got,size_t wanted)
{ return !receiving ? P4_OTA_FINISH_INVALID_STATE : (!open || got!=wanted ? P4_OTA_FINISH_INCOMPLETE : P4_OTA_FINISH_VERIFY); }
static bool p4_ota_policy_text_matches(const char *s,size_t cap,const char *expected)
{ return memchr(s,0,cap) && !strcmp(s,expected); }
'''
for signature in ['static void copy_text(', 'static void close_receive_resources_locked(',
                  'esp_err_t p4_ota_begin(', 'esp_err_t p4_ota_write(',
                  'esp_err_t p4_ota_finish(', 'void p4_ota_abort(']:
    code += function(signature)
source = (root/'firmware/p4-core/components/p4_ota_pull/p4_ota_pull.c').read_text(encoding='utf-8')
code += function('static esp_err_t http_status_error(')
code += r'''
int main(void) {
    assert(http_status_error(200)==ESP_OK);
    assert(http_status_error(404)==ESP_ERR_NOT_FOUND);
    int errors[]={0,301,401,403,429,500,503};
    for (size_t i=0;i<sizeof(errors)/sizeof(errors[0]);i++)
        assert(http_status_error(errors[i])==ESP_ERR_INVALID_RESPONSE);
    ddj_ota_manifest_t manifest={.target=DDJ_OTA_TARGET_P4,.image_size=24,.version="M3-test"};
    uint8_t image[24]={0};
    assert(p4_ota_begin(&manifest)==0 && begins==1);
    assert(p4_ota_begin(&manifest)==ESP_ERR_INVALID_STATE && begins==1);
    write_rc=-1;assert(p4_ota_write(image,24)==-1);
    assert(!s_handle_open && !s_image_sha_active && aborts==1 && hash_aborts==1);
    assert(p4_ota_begin(&manifest)==0 && begins==2);
    write_rc=0;hash_rc=-1;assert(p4_ota_write(image,24)==-1);
    assert(!s_handle_open && !s_image_sha_active && aborts==2 && hash_aborts==2);
    assert(p4_ota_begin(&manifest)==0 && begins==3);
    hash_rc=0;assert(p4_ota_write(image,24)==0);
    assert(p4_ota_finish()==0 && boot_sets==1 && s_status.state==P4_OTA_READY_TO_REBOOT);
    assert(p4_ota_begin(&manifest)==ESP_ERR_INVALID_STATE && begins==3);
    p4_ota_abort("late caller");assert(s_status.state==P4_OTA_READY_TO_REBOOT);
    assert(p4_ota_begin(&manifest)==ESP_ERR_INVALID_STATE && begins==3);
    assert(p4_ota_finish()==ESP_ERR_INVALID_STATE && boot_sets==1);
    assert(!locks && aborts==2 && hash_aborts==2);
}
'''
out=root/'.cache/ota_receive_review'
out.mkdir(parents=True,exist_ok=True)
cfile=out/'test.c'
cfile.write_text(code,encoding='utf-8')
exe=out/('test.exe' if os.name=='nt' else 'test')
subprocess.run([shutil.which('gcc'),'-std=c11','-Wall','-Wextra','-Werror',str(cfile),'-o',str(exe)],check=True)
subprocess.run([str(exe)],check=True)
print('PASS production OTA receive: retries release resources; activated slot rejects begin/late abort/duplicate finish')
