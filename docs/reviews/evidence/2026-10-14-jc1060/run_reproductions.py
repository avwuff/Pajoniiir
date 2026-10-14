import hashlib, importlib.util, json, os, pathlib, subprocess, sys
ROOT = next(p for p in pathlib.Path(__file__).resolve().parents if (p/'firmware/p4-core').is_dir())
OUT = ROOT / '.cache/jc1060-review-20261014'
SRC = OUT / 'contributor'
GCC = pathlib.Path('C:/msys64/ucrt64/bin/gcc.exe')
os.environ['PATH'] += os.pathsep + str(GCC.parent)
results = {}
if subprocess.run(['git','diff','--quiet','78dc3b7d07966579b189f73700f9bff89c888d6b','--','firmware','tests','tools/controller_profile/compile_profile.py'],cwd=ROOT).returncode:
    raise RuntimeError('Production/test/compiler baseline differs from the frozen review; use a matching checkout')

def run(name, args, cwd=ROOT):
    p = subprocess.run([str(a) for a in args], cwd=cwd, capture_output=True, text=True)
    (OUT / (name + '.log')).write_text(p.stdout + p.stderr, encoding='utf-8')
    results[name] = {'exit_code': p.returncode, 'command': [str(a) for a in args], 'log': name + '.log'}
    print(name, p.returncode, (p.stdout + p.stderr)[-2200:])
    if p.returncode: raise RuntimeError(name)

def function(text, name):
    start = text.index(name)
    start = text.rfind('\n', 0, start) + 1
    opening = text.index('{', start)
    depth = 1
    i = opening + 1
    while depth:
        depth += (text[i] == '{') - (text[i] == '}')
        i += 1
    return text[start:i]

# Execute the exact contributor route helper at the documented manager boundary.
route = function((SRC/'firmware/p4-core/components/usb_storage/usb_storage_shared.c').read_text(), 'shared_storage_device_route_allowed(')
c = '''#include <stdbool.h>
#include <stdint.h>
#include <assert.h>
#include <stdio.h>
typedef int esp_err_t;
#define ESP_OK 0
#define ESP_ERR_NOT_FOUND 1
typedef struct { bool storage_via_hub; uint8_t storage_root; } caps_t;
static caps_t caps = {true,1};
static caps_t *board_capabilities_get(void) { return &caps; }
static bool observed, root_matches, require_direct_seen;
static esp_err_t usb_host_manager_device_matches_root(uint8_t a, uint8_t r, bool direct, bool *matches) {
 (void)a; assert(r==1); require_direct_seen=direct; *matches=root_matches;
 return observed ? ESP_OK : ESP_ERR_NOT_FOUND;
}
#define SHARED_STORAGE_ROOT_INDEX (board_capabilities_get()->storage_root)
'''+route+'''
int main(void) {
 observed=true; root_matches=true; assert(shared_storage_device_route_allowed(4));
 root_matches=false; bool wrong=shared_storage_device_route_allowed(4);
 assert(wrong && !require_direct_seen);
 caps.storage_via_hub=false; assert(!shared_storage_device_route_allowed(4));
 caps.storage_via_hub=true; observed=false; assert(!shared_storage_device_route_allowed(4));
 puts("REPRODUCED: hub-enabled route accepts ESP_OK/matches=false; non-hub and unknown remain rejected");
}
'''
(OUT/'route_repro.c').write_text(c)
run('route-build', [GCC, '-std=c11', '-Wall', '-Wextra', '-Werror', OUT/'route_repro.c','-o',OUT/'route_repro.exe'])
run('route-run', [OUT/'route_repro.exe'])

# Production stream selector, with captured DDJ-400 AS fields, plus interval mutation.
testdir = ROOT/'tests/controller_usb_audio'
component = ROOT/'firmware/p4-core/components/controller_usb_audio'
test = (testdir/'test_controller_usb_audio_stream.c').read_text()
extra = '''
static void review_hs_uac1_intervals(void) {
 uint8_t captured_as[] = {
  9,2,52,0,1,1,0,0x80,250,
  9,4,1,1,1,1,2,0,4,
  7,0x24,1,1,1,1,0,
  11,0x24,2,1,4,2,16,1,0x44,0xac,0,
  9,5,1,13,0x40,2,4,0,0,
  7,0x25,1,1,0,0,0
 };
 flx4_uac_playback_format_t out;
 s_qualified_flx4=false; s_high_speed=true;
 bool interval4=select_stream_format(captured_as,sizeof(captured_as),&out);
 captured_as[42]=1;
 bool interval1=select_stream_format(captured_as,sizeof(captured_as),&out);
#ifdef REVIEW_CONTRIBUTOR
 assert(interval4 && interval1 && !out.uac2 && out.interval==1);
 puts("REPRODUCED: HS UAC1 bInterval=4 AND native bInterval=1 accepted by the 1-ms path (native interval=1 is 125 us)");
#else
 assert(!interval4 && !interval1);
 puts("BASELINE: both HS UAC1 cases rejected");
#endif
 captured_as[42]=2; assert(!select_stream_format(captured_as,sizeof(captured_as),&out));
 s_qualified_flx4=true; captured_as[42]=4; assert(!select_stream_format(captured_as,sizeof(captured_as),&out));
 s_high_speed=false;
}
'''
for variant in ('baseline','contributor'):
    source = (SRC if variant=='contributor' else ROOT)/'firmware/p4-core/components/controller_usb_audio/controller_usb_audio_stream.c'
    modified = test.replace('#include "controller_usb_audio_stream.c"', '#include "'+source.as_posix()+'"')
    modified = modified.replace('int main(void)', extra+'\nint main(void)').replace('    uint8_t d[sizeof(descriptor)];','    review_hs_uac1_intervals();\n    uint8_t d[sizeof(descriptor)];')
    target = OUT/('stream_'+variant+'.exe')
    harness = OUT/('stream_'+variant+'.c'); harness.write_text(modified)
    args = [GCC,'-std=c11','-O2','-Wall','-Wextra','-Wpedantic','-Werror',
            '-DCONFIG_PAJONIIIR_BOARD_M3=1','-DCONFIG_BSP_M3_HS_CONTROLLER=1','-DCONFIG_CONTROLLER_USB_HS_AUDIO_DIAGNOSTIC=1']
    if variant=='contributor': args+=['-DREVIEW_CONTRIBUTOR=1']
    args += ['-I'+str(p) for p in [testdir/'stubs',testdir,component,component/'include',ROOT/'firmware/p4-core/components/service_log/include']]
    args += [harness]+[component/f for f in ['controller_audio_ring.c','controller_audio_resampler.c','flx4_uac_packetizer.c','flx4_uac_descriptors.c','controller_uac2.c']]+['-o',target]
    run('stream-'+variant+'-build',args); run('stream-'+variant+'-run',[target])

# Actual deck actor with existing test boundary stubs; exact contributor CUE branch.
deckdir = ROOT/'tests/deck_core_dual'
decktest = (deckdir/'test_deck_core_dual.c').read_text()
web = (SRC/'firmware/p4-core/components/web_server/web_server.c').read_text()
cue = web.split('} else if (strcmp(action, "cue") == 0) {',1)[1].split('} else if (strcmp(action, "pfl") == 0)',1)[0]
cue = cue.replace('deck_core_queue_remote_event','review_remote_enqueue')
extra = '''
static ctrl_event_t review_press;
static int review_queue_free;
static esp_err_t review_remote_enqueue(const ctrl_event_t *ev) {
 if (!review_queue_free) return ESP_FAIL;
 --review_queue_free; review_press=*ev; return ESP_OK;
}
static esp_err_t review_web_cue(uint8_t deck) {
 esp_err_t queue_rc=ESP_OK;
'''+cue+'''
 return queue_rc;
}
static void review_defects(void) {
 deck_core_test_reset(); reset_audio_engine_stub(); publish_loaded_bpm(CTRL_DECK_1,120);
 audio_engine_stub_loop_active[0]=true;
 audio_engine_stub_loop_start_ms[0]=100000;
 audio_engine_stub_loop_end_ms[0]=250000;
 ctrl_event_t resize=deck_button(CTRL_ID_DECK1_LOOP_DOUBLE);
 deck_core_test_apply_event(&resize);
 assert(audio_engine_stub_loop_end_ms[0]==400000); /* loaded length 300000 */
 puts("REPRODUCED: production LOOP_DOUBLE sends end=400000 ms for loaded 300000-ms track");
 deck_core_test_reset(); reset_audio_engine_stub(); publish_loaded_bpm(CTRL_DECK_1,120);
 review_queue_free=1;
 assert(review_web_cue(CTRL_DECK_1)==ESP_FAIL);
 assert(review_press.value==1);
 deck_core_test_apply_event(&review_press);
 deck_state_t state=deck_core_get_deck_state(CTRL_DECK_1);
 assert(state.cue_held && state.cue_preview && state.playing);
 puts("REPRODUCED: exact web CUE branch rejects release after accepted press; production actor remains cue_held/preview/playing");
 ctrl_event_t release=review_press; release.value=0; deck_core_test_apply_event(&release);
 state=deck_core_get_deck_state(CTRL_DECK_1);
 assert(!state.cue_held && !state.cue_preview && !state.playing);
}
'''
decktest = decktest.replace('int main(void)',extra+'\nint main(void)').replace('int main(void)\n{','int main(void)\n{\n    review_defects();')
harness = OUT/'deck_repro.c'; harness.write_text(decktest)
inc = [deckdir/'stubs',ROOT/'tests/support/stubs']+[ROOT/'firmware/p4-core/components'/d/'include' for d in ['audio_engine','beat_jump','deck_core','control_link','hot_cue_store','media_identity','library']]
args=[GCC,'-std=c99','-Wall','-Wextra','-Wpedantic','-Werror=implicit-function-declaration','-Wno-unused-variable','-Wno-unused-parameter','-DDECK_CORE_PC_TEST']+['-I'+str(p) for p in inc]
args += [harness,deckdir/'control_link_stub.c',deckdir/'hot_cue_store_stub.c',ROOT/'tests/deck_loaded_track_store/anlz_clone_stub.c']
args += [ROOT/'firmware/p4-core/components'/p for p in ['beat_jump/beat_jump.c','control_link/flx4_led_snapshot.c','deck_core/deck_loaded_track_store.c','media_identity/media_identity.c']]+[deckdir/'deck_core_test_snapshot_wrapper.c','-o',OUT/'deck_repro.exe']
args[args.index('-o'):args.index('-o')] = [deckdir/'control_link_sink_wrapper.c',ROOT/'firmware/p4-core/components/library/anlz_snapshot.c',ROOT/'firmware/p4-core/components/deck_core/deck_load_lock.c',ROOT/'firmware/p4-core/components/deck_core/deck_net_sync.c','-lm']
run('deck-build',args); run('deck-run',[OUT/'deck_repro.exe'])

# Execute the production terminal sink-fault transition, not the whole USB task.
sink = function((ROOT/'firmware/p4-core/components/audio_engine/audio_engine.c').read_text(), 'audio_output_mark_sink_fault(')
sinkc = '''#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
typedef int esp_err_t;
#define ESP_OK 0
#define ESP_ERR_TIMEOUT 1
#define ESP_FAIL 2
#define ESP_ERR_INVALID_STATE 3
#define AUDIO_ENGINE_DECK_COUNT 2
#define AE_LOCK() ((void)0)
#define AE_UNLOCK() ((void)0)
#define ESP_LOGE(...) ((void)0)
typedef struct {bool loaded,playing,paused,eof,playback_finished; int last_error; char last_error_text[32];} audio_engine_state_t;
static audio_engine_state_t s_engines[2];
static unsigned s_output_sink_faults, s_headphone_sink_errors;
static void *s_codec;
static bool s_output_run=true;
static void atomic_store_bool(bool *p,bool v) {*p=v;}
'''+sink+'''
int main(void) {
 for(unsigned d=0;d<2;++d) {s_engines[d].loaded=true;s_engines[d].playing=true;}
 audio_output_mark_sink_fault(ESP_ERR_INVALID_STATE,ESP_FAIL);
 assert(!s_output_run && s_output_sink_faults==1);
 for(unsigned d=0;d<2;++d) assert(s_engines[d].loaded && !s_engines[d].playing && s_engines[d].eof && !s_engines[d].paused && !s_engines[d].playback_finished);
 puts("REPRODUCED production sink-fault helper: both loaded decks marked EOF/error; output service stopped (restart path audited separately)");
}
'''
(OUT/'sink_repro.c').write_text(sinkc)
run('sink-build',[GCC,'-std=c11','-Wall','-Wextra','-Werror',OUT/'sink_repro.c','-o',OUT/'sink_repro.exe'])
run('sink-run',[OUT/'sink_repro.exe'])

# Canonical gate intentionally accepts network_required=false without HTTP;
# JC1060 binds that flag only to Wi-Fi and calls HTTP after mark_ready.
gate=ROOT/'firmware/common/firmware_health'
run('startup-gate-build',[GCC,'-std=c11','-Wall','-Wextra','-Werror','-I'+str(gate/'include'),ROOT/'tests/p4_startup_gate/test_p4_startup_gate.c',gate/'p4_startup_gate.c','-o',OUT/'startup_gate.exe'])
run('startup-gate-run',[OUT/'startup_gate.exe'])

profilec='''#include <assert.h>
#include <stdio.h>
#include "controller_profile.h"
#include "controller_profile_manager.h"
#include "control_link.h"
int main(int argc,char **argv) {
 assert(argc==2); unsigned char blob[16384]; FILE *f=fopen(argv[1],"rb");assert(f);
 size_t n=fread(blob,1,sizeof(blob),f);assert(feof(f) && !ferror(f));fclose(f);
 static cp_profile_t p;static cp_runtime_t rt;cp_event_t e;controller_profile_meta_t meta;
 assert(cp_profile_parse(blob,n,&p)==CP_OK);
 assert(controller_profile_meta_parse(blob,n,&meta)==ESP_OK && meta.valid);
 assert(p.vid==0x2b73 && p.pid==0x0026 && p.input_count==284 && p.output_count==127);
 cp_runtime_init(&rt);
 for(unsigned d=0;d<2;++d) {
  const unsigned ids[]={d?CTRL_ID_DECK2_LOOP_HALVE:CTRL_ID_DECK1_LOOP_HALVE,d?CTRL_ID_DECK2_LOOP_DOUBLE:CTRL_ID_DECK1_LOOP_DOUBLE};
  for(unsigned i=0;i<2;++i) {
   assert(cp_runtime_process(&p,&rt,0x90+d,0x51+2*i,127,&e) && e.id==ids[i] && e.value==1);
   assert(cp_runtime_process(&p,&rt,0x90+d,0x51+2*i,0,&e) && e.id==ids[i] && e.value==0);
  }
 }
 blob[n-1]^=1; assert(cp_profile_parse(blob,n,&p)==CP_ERR_CRC);
 assert(controller_profile_meta_parse(blob,n,&meta)!=ESP_OK);
 puts("PASS DDJ-400 contributor v4: production parser/storage validator, four loop bindings press/release, wrong CRC rejected by both");
}
'''
(OUT/'ddj400_profile.c').write_text(profilec)
components=ROOT/'firmware/p4-core/components'
incs=[ROOT/'tests/controller_runtime/stubs']+[components/p/'include' for p in ['controller_profile','controller_profile_manager','control_link']]
run('ddj400-profile-build',[GCC,'-std=c11','-Wall','-Wextra','-Werror','-DCONTROLLER_PROFILE_MANAGER_PC_TEST']+['-I'+str(p) for p in incs]+[OUT/'ddj400_profile.c',components/'controller_profile/controller_profile.c',components/'controller_profile_manager/controller_profile_manager.c','-o',OUT/'ddj400_profile.exe'])
run('ddj400-profile-run',[OUT/'ddj400_profile.exe',SRC/'controllers/pioneer_ddj_400/profile.s3bin'])

spec=importlib.util.spec_from_file_location('compiler',ROOT/'tools/controller_profile/compile_profile.py')
compiler=importlib.util.module_from_spec(spec); sys.modules['compiler']=compiler;spec.loader.exec_module(compiler)
profile=json.loads((SRC/'controllers/pioneer_ddj_400/profile.json').read_text())
blob=compiler.compile_profile(profile)
assert blob==(SRC/'controllers/pioneer_ddj_400/profile.s3bin').read_bytes()
results['profile']={'byte_identical':True,'size':len(blob),'sha256':hashlib.sha256(blob).hexdigest(),'source_input_nodes':len(profile['inputs']),'source_output_nodes':len(profile['outputs']),'compiled_inputs':284,'compiled_outputs':127}
(OUT/'reproductions.json').write_text(json.dumps(results,indent=2))
print(json.dumps(results['profile']))
