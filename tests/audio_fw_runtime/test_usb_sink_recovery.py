"""Execute production USB MAIN pause, explicit PLAY and loop range boundaries.

Hardware/decoder I/O is stubbed. This is not USB reconnect timing acceptance.
"""
from pathlib import Path
import os
import shutil
import subprocess

root = Path(__file__).resolve().parents[2]
component = root / 'firmware/p4-core/components/audio_engine'
source = (component / 'audio_engine.c').read_text(encoding='utf-8')
build = Path(__file__).parent / 'build/usb-sink'
build.mkdir(parents=True, exist_ok=True)

def block(marker):
    start = source.index(marker)
    opening = source.index('{', start)
    depth, end = 1, opening + 1
    while depth:
        depth += (source[end] == '{') - (source[end] == '}')
        end += 1
    return source[start:end]

code = r'''
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "audio_eof_policy.h"
#include "audio_start_gate.h"
#include "audio_loop_range.h"
#include "audio_loop_resize.h"
#define AE_FW 1
#define AUDIO_ENGINE_DECK_COUNT 2
#define AE_START_PREBUFFER_FRAMES 512u
#define AE_SEEK_REASON_USER 1
#define ESP_OK 0
#define ESP_ERR_INVALID_ARG 1
#define ESP_ERR_INVALID_STATE 2
#define ESP_ERR_NOT_SUPPORTED 3
#define ESP_ERR_TIMEOUT 4
#define ESP_FAIL 5
#define ESP_LOGI(...) ((void)0)
#define ESP_LOGE(...) ((void)0)
typedef int esp_err_t;
static unsigned lock_depth;
#define AE_LOCK() (++lock_depth)
#define AE_UNLOCK() (--lock_depth)
typedef struct {
 bool loaded,playing,paused,eof,playback_finished,timeline_preroll_pending;
 bool usb_resume_pending,loop_active;
 uint32_t usb_resume_position_ms,duration_ms,loop_start_ms,loop_end_ms;
 int last_error; char last_error_text[64];
} audio_engine_state_t;
static audio_engine_state_t s_engines[2];
static bool s_usb_main_sink=true, ready, s_output_run=true;
static bool s_start_seek_pending[2],s_start_waiting[2];
static uint32_t s_start_prebuffer_frames[2],s_start_wait_count[2],s_playback_session_epoch;
static uint32_t s_output_sink_faults,s_headphone_sink_errors;
static void *s_codec;
static int s_resamplers[2];
static struct {bool initialized;} s_keylocks[2];
static uint32_t position[2], seeks[2],last_seek[2],loop_changes;
static int seek_error;static bool disappear_in_seek;
static bool atomic_load_bool(const bool *p) {return *p;}
static void atomic_store_bool(bool *p,bool v) {*p=v;}
static uint32_t atomic_load_u32(const uint32_t *p) {return *p;}
static void atomic_store_u32(uint32_t *p,uint32_t v) {*p=v;}
static bool controller_usb_audio_stream_available(void) {return ready;}
static uint32_t audio_engine_deck_position_ms(uint8_t d) {assert(!lock_depth);return position[d];}
static uint32_t audio_engine_position_ms_for_deck(uint8_t d) {return position[d];}
static uint32_t deck_pcm_used(uint8_t d) {(void)d;return 1024;}
static void audio_resampler_reset(int *p) {(void)p;}
static bool deck_is_valid(uint8_t d) {return d<2;}
static bool deck_transport_supported(uint8_t d) {return d<2;}
static void ae_note_loop_change(audio_engine_state_t *e,bool resize) {(void)e;(void)resize;++loop_changes;}
static int audio_engine_request_user_seek(uint8_t d,uint32_t p) {position[d]=p;return ESP_OK;}
static int audio_engine_seek_for_deck_reason(uint8_t d,uint32_t p,int reason) {
 assert(!lock_depth && reason==AE_SEEK_REASON_USER); ++seeks[d];last_seek[d]=p;
 if (seek_error) return seek_error;
 position[d]=p;
 if(disappear_in_seek) ready=false;
 return ESP_OK;
}
'''
for marker in ['static void audio_output_pause_usb_main(',
               'static void audio_output_mark_sink_fault(',
               'static esp_err_t audio_engine_play_for_deck(uint8_t deck)\n{',
               'esp_err_t audio_engine_deck_set_loop(']:
    code += block(marker) + '\n'
code += r'''
int main(void) {
 for(unsigned d=0;d<2;++d) {s_engines[d].loaded=true;s_engines[d].playing=true;position[d]=12345+1000*d;}
 ready=false; audio_output_pause_usb_main();
 for(unsigned d=0;d<2;++d) {
  assert(s_engines[d].loaded && !s_engines[d].playing && s_engines[d].paused);
  assert(!s_engines[d].eof && s_engines[d].usb_resume_pending && !s_engines[d].last_error);
  assert(s_engines[d].usb_resume_position_ms==position[d]);
  assert(audio_engine_play_for_deck(d)==ESP_ERR_INVALID_STATE && !seeks[d]);
 }
 assert(s_output_run && !s_output_sink_faults);
 ready=true; /* Connecting a healthy sink does not resume either deck. */
 assert(!s_engines[0].playing && !s_engines[1].playing);
 seek_error=ESP_FAIL;
 assert(audio_engine_play_for_deck(0)==ESP_FAIL && s_engines[0].usb_resume_pending && !s_engines[0].playing);
 seek_error=0;disappear_in_seek=true;
 assert(audio_engine_play_for_deck(0)==ESP_ERR_INVALID_STATE && s_engines[0].usb_resume_pending);
 disappear_in_seek=false;ready=true;
 for(unsigned d=0;d<2;++d) {
  uint32_t saved=s_engines[d].usb_resume_position_ms;
  assert(audio_engine_play_for_deck(d)==ESP_OK && s_engines[d].playing && !s_engines[d].paused);
  assert(!s_engines[d].usb_resume_pending && last_seek[d]==saved);
  unsigned calls=seeks[d]; assert(audio_engine_play_for_deck(d)==ESP_OK && seeks[d]==calls);
 }
 /* Repeated loss preserves a parked EOF/drain flag rather than inventing EOF. */
 s_engines[1].eof=true;ready=false;audio_output_pause_usb_main();
 assert(s_engines[1].eof && s_engines[1].usb_resume_pending);
 s_usb_main_sink=false; s_engines[0].playing=true; s_engines[0].paused=false;
 audio_output_pause_usb_main(); assert(s_engines[0].playing);
 assert(audio_engine_play_for_deck(0)==ESP_OK); /* independent PCM MAIN */
 audio_output_mark_sink_fault(ESP_ERR_TIMEOUT,ESP_FAIL);
 assert(!s_output_run && s_output_sink_faults==1 && s_engines[0].eof && !s_engines[0].playing);
 /* Execute the real engine boundary, independent of actor metadata length. */
 for(unsigned d=0;d<2;++d) {
  s_engines[d].duration_ms=100;
  assert(audio_engine_deck_set_loop(d,50,100)==ESP_OK);
  unsigned notes=loop_changes;
  assert(audio_engine_deck_set_loop(d,50,101)==ESP_ERR_INVALID_ARG);
  assert(audio_engine_deck_set_loop(d,100,100)==ESP_ERR_INVALID_ARG);
  assert(audio_engine_deck_set_loop(d,80,50)==ESP_ERR_INVALID_ARG);
  assert(s_engines[d].loop_start_ms==50 && s_engines[d].loop_end_ms==100 && loop_changes==notes);
  s_engines[d].duration_ms=0;
  assert(audio_engine_deck_set_loop(d,50,101)==ESP_OK);
 }
 assert(!lock_depth);
 puts("PASS production USB MAIN loss/reconnect/explicit PLAY, repeated loss, seek failure, PCM isolation, fatal faults and engine loop EOF");
}
'''
c = build / 'recovery.c'; c.write_text(code)
exe = build / ('recovery.exe' if os.name == 'nt' else 'recovery')
gcc = shutil.which('gcc')
if not gcc: raise RuntimeError('gcc required')
subprocess.run([gcc, '-std=c11', '-Wall', '-Wextra', '-Werror', '-I'+str(component/'include'),
               str(c), str(component/'audio_eof_policy.c'),str(component/'audio_start_gate.c'),
               str(component/'audio_loop_resize.c'), '-o',str(exe)],check=True)
subprocess.run([str(exe)],check=True)
