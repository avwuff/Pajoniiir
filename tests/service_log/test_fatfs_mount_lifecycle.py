"""Fault-inject production SD/SPI mount ownership and format/remount paths."""
from pathlib import Path
import os
import shutil
import subprocess

ROOT = Path(__file__).resolve().parents[2]
BASE = ROOT/'firmware/p4-core/components/fatfs/vfs'

def function(source, signature):
    start = source.rindex(signature)
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
#include <stdlib.h>
#include <string.h>
typedef int esp_err_t;
typedef int FRESULT;
typedef int wl_handle_t;
typedef int esp_partition_subtype_t;
typedef unsigned char BYTE;
typedef unsigned UINT;
typedef struct { int dummy; } FATFS;
typedef struct { bool readonly; } esp_partition_t;
typedef struct { struct { unsigned sector_size; } csd; } sdmmc_card_t;
typedef struct { unsigned max_files,allocation_unit_size;bool format_if_mount_failed,use_one_fat,disk_status_check_enable; } esp_vfs_fat_mount_config_t;
typedef struct { const char *base_path,*fat_drive;unsigned max_files; } esp_vfs_fat_conf_t;
typedef unsigned vfs_fat_x_ctx_flags_t;
typedef struct { BYTE format;unsigned fats,align,roots;size_t unit; } MKFS_PARM;
typedef struct { const esp_partition_t *partition;bool by_label;BYTE pdrv;FATFS *fs;wl_handle_t wlhandle;unsigned flags;
 esp_vfs_fat_mount_config_t mount_config; } vfs_fat_spiflash_ctx_t;
#define ESP_OK 0
#define ESP_ERR_INVALID_STATE 2
#define ESP_ERR_NO_MEM 3
#define ESP_ERR_NOT_FOUND 4
#define ESP_FAIL 5
#define ESP_ERR_INVALID_ARG 6
#define FR_OK 0
#define FR_INVALID_DRIVE 10
#define FF_VOLUMES 2
#define WL_INVALID_HANDLE -1
#define ESP_PARTITION_SUBTYPE_ANY 0
#define ESP_PARTITION_SUBTYPE_DATA_FAT 1
#define ESP_PARTITION_TYPE_DATA 1
#define FORMATTED_DURING_LAST_MOUNT 1
#define CONFIG_WL_SECTOR_SIZE 512
#define MIN_REQ_SEC 16
#define FM_ANY 1
#define FM_SFD 2
#define ESP_LOGD(...) ((void)0)
#define ESP_LOGI(...) ((void)0)
#define ESP_LOGE(...) ((void)0)
#define ESP_LOGW(...) ((void)0)
#define ESP_RETURN_ON_FALSE(cond,err,...) do { if(!(cond))return (err); } while(0)
#define ESP_RETURN_ON_ERROR(expr,...) do { int rc=(expr);if(rc)return rc; } while(0)
#define ESP_GOTO_ON_ERROR(expr,label,...) do { ret=(expr);if(ret)goto label; } while(0)
#define ESP_GOTO_ON_FALSE(cond,err,label,...) do { if(!(cond)){ret=(err);goto label;} } while(0)
static esp_partition_t partition;
static FATFS filesystem;
static vfs_fat_spiflash_ctx_t *spi_ctx[2],*sd_ctx[2];
static int register_result,mount_result;
static bool drive_failure,allocation_failure;
static unsigned mounts,vfs_unregistrations,disk_unregistrations,wl_unmounts,sd_unmounts,mkfs_calls;
static const esp_partition_t *esp_partition_find_first(int type,int subtype,const char *label)
 { (void)type;(void)subtype;(void)label;return &partition; }
static esp_err_t wl_mount(const esp_partition_t *p,int *handle) { assert(p);*handle=7;return ESP_OK; }
static esp_err_t wl_unmount(int handle) { assert(handle==7);++wl_unmounts;return ESP_OK; }
static size_t wl_size(int h) { assert(h==7);return 1024*1024; }
static size_t wl_sector_size(int h) { assert(h==7);return 512; }
static esp_err_t ff_diskio_get_drive(BYTE *drive) { if(drive_failure)return ESP_ERR_NO_MEM;*drive=0;return ESP_OK; }
static esp_err_t ff_diskio_register_wl_partition(BYTE drive,int handle) { assert(!drive && handle==7);return ESP_OK; }
static esp_err_t ff_diskio_register_raw_partition(BYTE drive,const esp_partition_t *p) { assert(!drive && p);return ESP_OK; }
static void ff_diskio_register_sdmmc(BYTE drive,sdmmc_card_t *card) { assert(!drive && card); }
static void ff_sdmmc_set_disk_status_check(BYTE drive,bool check) { (void)drive;(void)check; }
static void ff_diskio_unregister(BYTE drive) { assert(!drive);++disk_unregistrations; }
static void ff_diskio_clear_pdrv_wl(int handle) { assert(handle==7); }
static BYTE ff_diskio_get_pdrv_card(sdmmc_card_t *card) { assert(card);return 0; }
static esp_err_t esp_vfs_fat_register_cfg(const esp_vfs_fat_conf_t *config,FATFS **out)
 { assert(config);if(register_result)return register_result;*out=&filesystem;return ESP_OK; }
static esp_err_t esp_vfs_fat_unregister_path(const char *path) { assert(path);++vfs_unregistrations;return ESP_OK; }
static void esp_vfs_set_readonly_flag(const char *path) { assert(path); }
static unsigned s_get_unused_context_id(void) { return 0; }
static bool s_get_context_id_by_label(const char *label,uint32_t *id) { (void)label;*id=0;return spi_ctx[0]!=NULL; }
static bool s_get_context_id_by_card(sdmmc_card_t *card,uint32_t *id) { assert(card);*id=0;return sd_ctx[0]!=NULL; }
static esp_err_t s_f_mount_rw(FATFS *fs,const char *drive,const esp_vfs_fat_mount_config_t *cfg,unsigned *flags,size_t sectors)
 { assert(fs && drive && cfg && sectors);if(flags)*flags=0;++mounts;return mount_result; }
static esp_err_t s_f_mount(sdmmc_card_t *card,FATFS *fs,const char *drive,BYTE pdrv,const esp_vfs_fat_mount_config_t *cfg,unsigned *flags)
 { assert(card && fs && drive && !pdrv && cfg);(void)flags;++mounts;return mount_result; }
static FRESULT f_mount(FATFS *fs,const char *drive,int immediate) { (void)fs;assert(drive);(void)immediate;++mounts;return FR_OK; }
static size_t esp_vfs_fat_get_allocation_unit_size(size_t sector,size_t desired) { return desired?desired:sector; }
static void *ff_memalloc(size_t size) { return allocation_failure?NULL:malloc(size); }
static FRESULT f_mkfs(const char *drive,const MKFS_PARM *opt,void *buffer,size_t size)
 { assert(drive && opt && buffer && size);++mkfs_calls;return FR_OK; }
static esp_err_t esp_vfs_fat_spiflash_unmount_rw_wl(const char *path,int handle) { assert(path && handle==7);return ESP_OK; }
static esp_err_t esp_vfs_fat_sdcard_unmount(const char *path,sdmmc_card_t *card)
 { assert(path && card);++sd_unmounts;free(sd_ctx[0]);sd_ctx[0]=NULL;return ESP_OK; }
'''
spi = (BASE/'vfs_fat_spiflash.c').read_text()
sd = (BASE/'vfs_fat_sdmmc.c').read_text()
code += '#define s_ctx spi_ctx\n'
for signature in ('esp_err_t esp_vfs_fat_spiflash_mount_rw_wl(',
                  'esp_err_t esp_vfs_fat_spiflash_mount_ro(',
                  'esp_err_t esp_vfs_fat_spiflash_format_cfg_rw_wl('):
    code += function(spi, signature)
code += '\n#undef s_ctx\n#define s_ctx sd_ctx\n'
for signature in ('static esp_err_t mount_to_vfs_fat(', 'esp_err_t esp_vfs_fat_sdcard_format_cfg('):
    code += function(sd, signature)
code += r'''
static vfs_fat_spiflash_ctx_t *seed(void) {
 vfs_fat_spiflash_ctx_t *ctx=calloc(1,sizeof(*ctx));assert(ctx);ctx->fs=&filesystem;ctx->wlhandle=7;return ctx;
}
int main(void) {
 esp_vfs_fat_mount_config_t config={.max_files=2};int handle=-1;
 sdmmc_card_t card={.csd={512}};FATFS *out=NULL;
 register_result=ESP_ERR_INVALID_STATE;
 assert(esp_vfs_fat_spiflash_mount_rw_wl("/old","data",&config,&handle)==ESP_ERR_INVALID_STATE);
 assert(!mounts && !vfs_unregistrations && disk_unregistrations==1 && wl_unmounts==1 && handle==-1);
 assert(esp_vfs_fat_spiflash_mount_ro("/old","data",&config)==ESP_ERR_INVALID_STATE);
 assert(!mounts && !vfs_unregistrations && disk_unregistrations==2);
 assert(mount_to_vfs_fat(&config,&card,0,"/old",&out,NULL)==ESP_ERR_INVALID_STATE);
 assert(!out && !mounts && !vfs_unregistrations && disk_unregistrations==3);
 drive_failure=true;
 assert(esp_vfs_fat_spiflash_mount_rw_wl("/new","data",&config,&handle)==ESP_ERR_NO_MEM && wl_unmounts==2 && handle==-1);
 drive_failure=false;register_result=ESP_OK;mount_result=ESP_FAIL;
 assert(esp_vfs_fat_spiflash_mount_rw_wl("/new","data",&config,&handle)==ESP_FAIL);
 assert(vfs_unregistrations==1 && wl_unmounts==3 && handle==-1);
 spi_ctx[0]=seed();
 assert(esp_vfs_fat_spiflash_format_cfg_rw_wl("/sd","data",NULL)==ESP_FAIL && !spi_ctx[0]);
 sd_ctx[0]=seed();
 assert(esp_vfs_fat_sdcard_format_cfg("/sd",&card,NULL)==ESP_FAIL && sd_unmounts==1 && !sd_ctx[0]);
 mount_result=ESP_OK;allocation_failure=true;sd_ctx[0]=seed();unsigned before=mkfs_calls;
 assert(esp_vfs_fat_sdcard_format_cfg("/sd",&card,NULL)==ESP_ERR_NO_MEM && sd_ctx[0] && mkfs_calls==before);
 free(sd_ctx[0]);
}
'''
out = ROOT/'.cache/fatfs_mount_review'
out.mkdir(parents=True, exist_ok=True)
cfile = out/'test.c'
cfile.write_text(code)
exe = out/('test.exe' if os.name == 'nt' else 'test')
subprocess.run([shutil.which('gcc'), '-std=c11', '-Wall', '-Wextra', '-Werror', str(cfile), '-o', str(exe)], check=True)
subprocess.run([str(exe)], check=True)
print('Production FatFs duplicate mount ownership, WL cleanup and remount error propagation PASS')
