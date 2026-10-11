"""Execute copied FatFs VFS fault paths without a physical filesystem."""
from pathlib import Path
import os
import shutil
import subprocess

ROOT = Path(__file__).resolve().parents[2]
BASE = ROOT/'firmware/p4-core/components/fatfs'

def function(source, signature):
    start = source.rindex(signature)
    end = source.index('{', start) + 1
    depth = 1
    while depth:
        depth += (source[end] == '{') - (source[end] == '}')
        end += 1
    return source[start:end]

source = (BASE/'vfs/vfs_fat.c').read_text()
code = r'''
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
typedef int esp_err_t;
typedef int FRESULT;
typedef struct { off_t pos; } FIL;
typedef struct { int dummy; } DIR;
typedef struct { int lock;FIL files[1]; } vfs_fat_ctx_t;
typedef struct { DIR ffdir;int filinfo;long offset; } vfs_fat_dir_t;
#define ESP_OK 0
#define ESP_ERR_INVALID_ARG 1
#define ESP_ERR_INVALID_STATE 2
#define FR_OK 0
#define FF_VOLUMES 2
#define FA_READ 1
#define FA_OPEN_EXISTING 0
#define CONFIG_FATFS_IMMEDIATE_FSYNC 0
#define ESP_LOGD(...) ((void)0)
static vfs_fat_ctx_t context,*s_fat_ctxs[]={&context};
static unsigned locks;
static int rewind_result,read_result,open_result;
static void _lock_acquire(int *lock) { (void)lock;assert(!locks);++locks; }
static void _lock_release(int *lock) { (void)lock;assert(locks==1);--locks; }
static off_t f_tell(FIL *file) { return file->pos; }
static FRESULT f_lseek(FIL *file,off_t offset) { file->pos=offset;return FR_OK; }
static FRESULT f_write(FIL *file,const void *src,size_t size,unsigned *written) { (void)file;(void)src;(void)size;*written=0;return FR_OK; }
static FRESULT f_rewinddir(DIR *dir) { (void)dir;return rewind_result; }
static FRESULT f_readdir(DIR *dir,int *info) { (void)dir;(void)info;return read_result; }
static int fresult_to_errno(FRESULT res) { return res?EIO:0; }
static size_t find_context_index_by_path(const char *path) { assert(path);return 0; }
static void prepend_drive_to_path(vfs_fat_ctx_t *ctx,const char **path,void *unused) { (void)ctx;(void)path;(void)unused; }
static void *ff_memalloc(size_t size) { return malloc(size); }
static FRESULT f_open(FIL *file,const char *path,unsigned flags) { (void)file;(void)path;assert(flags==(FA_READ|FA_OPEN_EXISTING));return open_result; }
static FRESULT f_close(FIL *file) { (void)file;return FR_OK; }
static FRESULT test_contiguous_file(FIL *file,int *contiguous) { (void)file;*contiguous=1;return FR_OK; }
'''
for signature in ('static ssize_t vfs_fat_pwrite(', 'static void vfs_fat_seekdir(',
                  'esp_err_t esp_vfs_fat_test_contiguous_file('):
    code += function(source, signature)
code += r'''
int main(void) {
 context.files[0].pos=17;
 assert(vfs_fat_pwrite(&context,0,"a",1,99)==-1 && errno==ENOSPC && !locks && context.files[0].pos==17);
 vfs_fat_dir_t dir={.offset=2};rewind_result=1;
 vfs_fat_seekdir(&context,(DIR*)&dir,0);assert(!locks && errno==EIO);
 rewind_result=0;read_result=1;dir.offset=0;
 vfs_fat_seekdir(&context,(DIR*)&dir,1);assert(!locks && errno==EIO);
 struct { unsigned char before;bool contiguous;unsigned char after[4]; } value={.before=0xa5,.after={1,2,3,4}};
 assert(esp_vfs_fat_test_contiguous_file("/sd","/sd/a",&value.contiguous)==0 && value.contiguous && !locks);
 assert(value.before==0xa5 && value.after[0]==1 && value.after[1]==2 && value.after[2]==3 && value.after[3]==4);
 open_result=1;value.contiguous=false;
 assert(esp_vfs_fat_test_contiguous_file("/sd","/sd/missing",&value.contiguous)==-1 && !value.contiguous && !locks);
}
'''
out = ROOT/'.cache/fatfs_vfs_review'
out.mkdir(parents=True, exist_ok=True)
cfile = out/'test.c'
cfile.write_text(code)
exe = out/('test.exe' if os.name == 'nt' else 'test')
subprocess.run([shutil.which('gcc'), '-std=c11', '-Wall', '-Wextra', '-Werror', str(cfile), '-o', str(exe)], check=True)
subprocess.run([str(exe)], check=True)
print('Production FatFs ENOSPC/seekdir lock release, position restoration and bool boundary PASS')
