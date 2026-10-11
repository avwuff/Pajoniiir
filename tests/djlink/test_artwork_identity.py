"""Run production cache ownership/lookup with host allocation stubs, no LVGL/RTOS."""
from pathlib import Path
import os
import shutil
import subprocess

root = Path(__file__).resolve().parents[2]
component = root / "firmware/p4-core/components"
source = (component / "ui/ui_artwork.c").read_text(encoding="utf-8")
out = root / ".cache/artwork-identity"
out.mkdir(parents=True, exist_ok=True)

def block(marker):
    start = source.index(marker)
    opening = source.index("{", start)
    depth, end = 1, opening + 1
    while depth:
        depth += (source[end] == "{") - (source[end] == "}")
        end += 1
    return source[start:end]

slot_start = source.index("typedef struct {\n    uint32_t key, gen, stamp;")
slot_end = source.index("} art_slot_t;", slot_start) + len("} art_slot_t;")
code = r'''
#include "ui_artwork.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdatomic.h>
typedef enum { SLOT_EMPTY, SLOT_PENDING, SLOT_READY, SLOT_NONE } slot_state_t;
typedef enum { RESULT_SKIPPED, RESULT_READY, RESULT_NONE } result_t;
typedef struct { uint32_t key, gen, epoch; } art_req_t;
''' + source[slot_start:slot_end] + r'''
static art_slot_t s_slot[UI_ARTWORK_SLOTS];
static ui_artwork_thumb_t storage[UI_ARTWORK_SLOTS], *s_thumbs=storage;
static uint32_t s_clock,s_gen=1;
static uint32_t s_content_revision;
static bool s_init_done=true,s_init_failed;
static bool art_init(void) {return !s_init_failed;}
static bool s_retry;
static ui_artwork_stats_t s_stats;
static ui_artwork_thumb_t staged, *s_stage=&staged;
static art_req_t s_stage_req;
static result_t s_stage_result;
static uint32_t s_stage_read_us,s_stage_decode_us;
static atomic_bool s_stage_ready;
static atomic_uint s_epoch;
static uint32_t live_generation=1;
static unsigned queue_spaces,stage_releases;
static void *s_queue,*s_stage_free;
static uint32_t library_generation(void) {return live_generation;}
static int64_t esp_timer_get_time(void) {return 0;}
static unsigned uxQueueSpacesAvailable(void *q) {(void)q;return queue_spaces;}
static void xSemaphoreGive(void *s) {(void)s;++stage_releases;}
'''
for marker in ("static void art_content_changed(", "uint32_t ui_artwork_content_revision(",
               "static const uint16_t *slot_pixels(", "static int slot_victim(",
               "void ui_artwork_forget(",
               "void ui_artwork_begin_page(", "bool ui_artwork_poll(",
               "const uint16_t *ui_artwork_get_identity(", "void ui_artwork_publish_identity("):
    code += block(marker) + "\n"
code += r'''
static void stage(uint32_t key,uint32_t generation,result_t result,uint16_t pixel) {
    staged.deck[0]=pixel;
    s_stage_req=(art_req_t){key,generation,atomic_load(&s_epoch)};
    s_stage_result=result;
    atomic_store(&s_stage_ready,true);
}
static void test_local_delayed_publication(void) {
    memset(s_slot,0,sizeof(s_slot));memset(storage,0,sizeof(storage));
    s_init_failed=false;live_generation=s_gen=10;
    s_slot[0]=(art_slot_t){.key=7,.gen=10,.state=SLOT_PENDING};
    stage(7,10,RESULT_READY,111);
    assert(ui_artwork_poll() && s_slot[0].state==SLOT_READY);
    assert(storage[0].deck[0]==111);

    /* A media change can precede the UI's first request for the new catalog.
     * An old pending slot still exists, but must not receive decoded pixels. */
    s_slot[0]=(art_slot_t){.key=7,.gen=10,.state=SLOT_PENDING};
    live_generation=11;stage(7,10,RESULT_READY,211);
    uint32_t revision=ui_artwork_content_revision();
    assert(ui_artwork_poll() && s_slot[0].state==SLOT_EMPTY);
    assert(storage[0].deck[0]==111 && ui_artwork_content_revision()==revision);

    /* A previous medium's worker completion cannot overwrite a replacement
     * request with the same numeric track key in the new catalog. */
    s_slot[0]=(art_slot_t){.key=7,.gen=11,.state=SLOT_PENDING};
    live_generation=11;stage(7,10,RESULT_READY,222);
    revision=ui_artwork_content_revision();
    assert(!ui_artwork_poll());
    assert(s_slot[0].state==SLOT_PENDING && storage[0].deck[0]==111);
    assert(ui_artwork_content_revision()==revision);
    assert(!atomic_load(&s_stage_ready));
    stage(7,11,RESULT_READY,333);
    assert(ui_artwork_poll() && storage[0].deck[0]==333);

    /* Page changes retire even a fully decoded result from the current medium. */
    s_slot[0]=(art_slot_t){.key=7,.gen=11,.state=SLOT_PENDING};
    stage(7,11,RESULT_READY,444);ui_artwork_begin_page();
    revision=ui_artwork_content_revision();
    assert(ui_artwork_poll() && s_slot[0].state==SLOT_EMPTY);
    assert(storage[0].deck[0]==333 && ui_artwork_content_revision()==revision);

    /* Missing/corrupt/decode-failed artwork publishes NONE, not old pixels.
     * Unknown and already-evicted requests cannot populate another track. */
    s_slot[0]=(art_slot_t){.key=7,.gen=11,.state=SLOT_PENDING};
    stage(7,11,RESULT_NONE,555);
    assert(ui_artwork_poll() && s_slot[0].state==SLOT_NONE);
    s_slot[0]=(art_slot_t){.key=8,.gen=11,.state=SLOT_PENDING};
    stage(7,11,RESULT_READY,666);
    assert(!ui_artwork_poll() && s_slot[0].state==SLOT_PENDING);
    assert(storage[0].deck[0]==333);
    s_retry=true;queue_spaces=1;
    assert(ui_artwork_poll() && !s_retry);
    assert(stage_releases==7 && s_stats.skipped==3 && s_stats.none==1);
}
int main(void) {
    media_persistent_id_t a={.valid=true},b={.valid=true},absent={.valid=true};
    a.bytes[31]=1;b.bytes[31]=2;absent.bytes[31]=3;
    ui_artwork_thumb_t thumb={0};thumb.deck[0]=123;thumb.row[0]=124;
    ui_artwork_publish_identity(7,&a,&thumb);
    uint32_t first_revision=ui_artwork_content_revision();
    assert(first_revision);
    thumb.deck[0]=999; /* mutate/reuse the worker's buffer after publication */
    assert(ui_artwork_get_identity(7,&a,UI_ARTWORK_DECK)[0]==123);
    assert(!ui_artwork_get_identity(7,&b,UI_ARTWORK_DECK));
    ui_artwork_publish_identity(7,&b,&thumb); /* identical 32-bit lookup key */
    assert(ui_artwork_get_identity(7,&a,UI_ARTWORK_DECK)[0]==123);
    assert(ui_artwork_get_identity(7,&b,UI_ARTWORK_DECK)[0]==999);
    s_gen++; /* a local USB catalog refresh must not expire remote ownership */
    assert(ui_artwork_get_identity(7,&a,UI_ARTWORK_ROW)[0]==124);
    assert(!ui_artwork_get_identity(7,&absent,UI_ARTWORK_DECK));
    const uint16_t *old_pixels=ui_artwork_get_identity(7,&a,UI_ARTWORK_DECK);
    uint32_t previous_revision=ui_artwork_content_revision();
    assert(ui_artwork_content_revision()==previous_revision); /* reads do not change content */
    for(int i=1;i<UI_ARTWORK_SLOTS;++i) {
        s_slot[i].state=SLOT_PENDING;s_slot[i].gen=s_gen;
    }
    thumb.deck[0]=555;
    ui_artwork_publish_identity(7,&b,&thumb); /* force the same storage address to be reused */
    assert(ui_artwork_get_identity(7,&b,UI_ARTWORK_DECK)==old_pixels);
    assert(old_pixels[0]==555 && ui_artwork_content_revision()!=previous_revision);
    previous_revision=ui_artwork_content_revision();
    ui_artwork_forget(7);
    assert(!ui_artwork_get_identity(7,&b,UI_ARTWORK_DECK));
    assert(ui_artwork_content_revision()!=previous_revision);
    s_init_failed=true;assert(!ui_artwork_get_identity(7,&a,UI_ARTWORK_DECK));
    test_local_delayed_publication();
    puts("PASS production artwork deep-copy, full-ID isolation, epoch independence and same-address content revisions");
    puts("PASS production artwork poll: stale media/page, same-key replacement, fallback, evicted result and bounded retry");
}
'''
c_file = out / "test.c"
c_file.write_text(code, encoding="utf-8")
target = out / ("test.exe" if os.name == "nt" else "test")
gcc = shutil.which("gcc")
if not gcc:
    raise RuntimeError("gcc is required on PATH")
subprocess.run([gcc, "-std=c11", "-O2", "-Wall", "-Wextra", "-Werror",
                "-I" + str(component / "ui/include"),
                "-I" + str(component / "media_identity/include"), str(c_file),
                str(component / "media_identity/media_identity.c"),
                "-o", str(target)], check=True)
subprocess.run([str(target)], check=True, timeout=30)
