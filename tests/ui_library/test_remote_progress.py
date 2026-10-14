"""Execute production cancellation retirement and network progress rendering."""
from pathlib import Path
import shutil
import subprocess
import tempfile

root = Path(__file__).resolve().parents[2]
source = (root / 'firmware/p4-core/components/ui/ui_library.c').read_text(encoding='utf-8')

def function(name):
    start = source.index('static void ' + name + '(')
    opening = source.index('{', start)
    depth, end = 1, opening + 1
    while depth:
        depth += (source[end] == '{') - (source[end] == '}')
        end += 1
    return source[start:end]

prelude = r'''
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "ui_load_gate.h"
#define CONFIG_PAJONIIIR_DJ_LINK_SERVICE 1
#define CONFIG_PAJONIIIR_DJ_OVERVIEW 1
static ui_load_gate_t gate;
static bool s_remote_load_active, disabled, s_dj_ready;
static unsigned s_download_total, s_download_done, s_active_tab, s_library_load_request_deck;
static int label;
static int *s_label_indicator_status = &label;
static char ordinary[40], alternate[40];
static void ui_library_finish_track_load_id(uint32_t id) {
    if (ui_load_gate_finish(&gate,id)) s_remote_load_active=false;
}
static bool ui_library_track_load_busy(void) {return gate.busy;}
static void ui_library_set_load_busy(bool busy,const char *hint) {(void)hint;disabled=busy;}
static void lv_label_set_text(int *l,const char *text) {assert(l==&label);snprintf(ordinary,sizeof ordinary,"%s",text);}
static void dj_ui_library_set_deck_status(unsigned deck,const char *text) {
    assert(deck==s_library_load_request_deck);snprintf(alternate,sizeof alternate,"%s",text);
}
'''
cases = r'''
int main(void) {
    uint32_t old,current;
    assert(ui_load_gate_try_begin(&gate,&old));disabled=true;s_remote_load_active=true;
    ui_load_gate_invalidate(&gate);
    assert(gate.busy);assert(!ui_load_gate_try_begin(&gate,&current));
    ui_library_retire_stale_load(old);
    assert(!disabled && !gate.busy && !s_remote_load_active);
    assert(ui_load_gate_try_begin(&gate,&current));disabled=true;s_remote_load_active=true;
    ui_library_retire_stale_load(old);
    assert(disabled && gate.busy && s_remote_load_active);
    s_dj_ready=true;s_active_tab=1;s_library_load_request_deck=1;
    ui_library_update_remote_progress();
    assert(!strcmp(ordinary,"RESOLVING SOURCE"));assert(!strcmp(alternate,ordinary));
    s_download_total=1024;s_download_done=512;
    ui_library_update_remote_progress();assert(!strcmp(ordinary,"DOWNLOAD 50%"));
    s_download_done=2048;
    ui_library_update_remote_progress();assert(!strcmp(ordinary,"DOWNLOAD 100%"));
    ui_library_retire_stale_load(current);
    strcpy(ordinary,"READY");strcpy(alternate,"READY");
    ui_library_update_remote_progress();assert(!strcmp(ordinary,"READY"));assert(!strcmp(alternate,"READY"));
    puts("production remote progress/cancellation PASS");return 0;
}
'''
# Ensure the tested retirement is used in the discarded-result branch and
# progress is rendered after ordinary/alternative status updates.
poll = source[source.index('static void ui_poll_track_load_result(void)'):source.index('static void ui_poll_track_load_result(void)') + 1000]
assert 'ui_library_retire_stale_load(result.load_id)' in poll
update = source[source.index('void ui_library_update('):source.index('bool ui_library_deck_observation_matches(')]
assert update.rindex('ui_library_update_remote_progress();') > update.rindex('dj_ui_library_set_progress(')
with tempfile.TemporaryDirectory(prefix='p4-ui-remote-') as tmp:
    path=Path(tmp); c=path/'test.c'; exe=path/'test.exe'
    c.write_text(prelude + function('ui_library_retire_stale_load') + function('ui_library_update_remote_progress') + cases)
    subprocess.run([shutil.which('gcc') or 'gcc','-std=c11','-Wall','-Wextra','-Werror',
        '-I'+str(root/'firmware/p4-core/components/ui/include'),str(c),
        str(root/'firmware/p4-core/components/ui/ui_load_gate.c'),'-o',str(exe)],check=True)
    subprocess.run([str(exe)],check=True)
