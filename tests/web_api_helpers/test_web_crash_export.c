#include "web_crash_export.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

typedef struct { unsigned reads, sends; size_t total; bool busy, read_fail, send_fail, start_on_read; uint64_t clock, step; } fixture_t;
static bool idle(void *p) { return !((fixture_t *)p)->busy; }
static uint64_t now(void *p) { fixture_t *f=p; uint64_t n=f->clock; f->clock+=f->step; return n; }
static bool read_bytes(void *p,size_t offset,void *data,size_t n) {
    fixture_t *f=p; assert(offset==f->total); assert(n<=512); f->reads++;
    memset(data,0xa5,n); if(f->start_on_read) f->busy=true; return !f->read_fail;
}
static bool send_bytes(void *p,const void *data,size_t n) {
    fixture_t *f=p; assert(((const unsigned char *)data)[0]==0xa5);
    f->sends++; f->total+=n; return !f->send_fail;
}
static bool run(fixture_t *f,size_t size,size_t partition) {
    web_crash_export_ops_t ops={f,idle,now,read_bytes,send_bytes};
    return web_crash_export_copy(size,partition,&ops);
}
int main(void) {
    char out[65];
    assert(web_crash_sha_prefix((const uint8_t *)"ABCD01234",10,out)==9);
    assert(strcmp(out,"abcd01234")==0);
    assert(!web_crash_sha_prefix((const uint8_t *)"ABCD",4,out) && !out[0]);
    assert(!web_crash_sha_prefix((const uint8_t *)"abc\"d",6,out) && !out[0]);
    char full[65]; memset(full,'f',64);full[64]=0;
    assert(web_crash_sha_prefix((const uint8_t *)full,sizeof(full),out)==64);
    assert(!web_crash_sha_prefix(NULL,0,out));
    fixture_t f={0}; assert(run(&f,7456,65536)); assert(f.total==7456 && f.reads==15 && f.sends==15);
    f=(fixture_t){.busy=true}; assert(!run(&f,7456,65536) && f.reads==0);
    f=(fixture_t){.start_on_read=true}; assert(!run(&f,7456,65536) && f.reads==1 && f.sends==0);
    f=(fixture_t){.read_fail=true}; assert(!run(&f,7456,65536) && f.sends==0);
    f=(fixture_t){.send_fail=true}; assert(!run(&f,7456,65536) && f.reads==1);
    f=(fixture_t){.step=5000}; assert(!run(&f,7456,65536) && f.reads==0);
    f=(fixture_t){0}; assert(!run(&f,65537,65536) && f.reads==0);
    assert(!run(&f,WEB_CRASH_EXPORT_MAX_BYTES+1,WEB_CRASH_EXPORT_MAX_BYTES+1));
    assert(!run(&f,0,65536));
    puts("Crash export identity/bounds/abort tests passed"); return 0;
}
