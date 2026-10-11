"""Execute production codec publication and fault-backoff boundaries with stubs."""
from pathlib import Path
import os
import re
import shutil
import subprocess

root = Path(__file__).resolve().parents[2]
source = (root / 'firmware/p4-core/components/audio_engine/audio_engine.c').read_text(encoding='utf-8')
def block(marker):
    start = source.index(marker)
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
#define AUDIO_ENGINE_DECK_COUNT 2
#define CONFIG_BSP_PCM5102A_MAIN_OUT 0
#define ESP_ERR_INVALID_STATE 1
#define ESP_ERR_INVALID_ARG 2
#define ESP_FAIL 3
#define ESP_OK 0
#define ESP_LOGE(...) ((void)0)
#define ESP_LOGI(...) ((void)0)
#define AE_LOCK() (++lock_depth)
#define AE_UNLOCK() (--lock_depth)
#define pdMS_TO_TICKS(ms) (ms)
typedef int esp_err_t;
typedef struct { uint32_t fixed_output_sample_rate; } board_t;
typedef struct { int bits_per_sample, channel; uint32_t sample_rate; } esp_codec_dev_sample_info_t;
typedef struct { int last_error; char last_error_text[32]; bool eof; } audio_engine_state_t;
static unsigned lock_depth, slept, configured;
static bool s_output_codec_open, s_usb_main_sink = true;
static void *s_codec;
static uint32_t s_output_sample_rate;
static board_t board;
static const board_t *board_capabilities_get(void) { return &board; }
static int esp_codec_dev_open(void *codec, const esp_codec_dev_sample_info_t *info)
{ (void)codec; (void)info; return 0; }
static bool atomic_load_bool(const bool *value) { return __atomic_load_n(value, __ATOMIC_ACQUIRE); }
static void atomic_store_bool(bool *value, bool state) { __atomic_store_n(value, state, __ATOMIC_RELEASE); }
static void audio_output_apply_fx_sample_rate(uint32_t rate) {
    /* Model output-task observation during retuning. */
    assert(!atomic_load_bool(&s_output_codec_open));
    assert(s_output_sample_rate == rate); configured++;
}
static void vTaskDelay(unsigned ticks) { assert(lock_depth == 0); slept += ticks; }
'''
code = prelude
for name in ('AE_READ_FAULT_RETRIES', 'AE_READ_FAULT_BACKOFF_MS'):
    code += re.search(r'^#define ' + name + r' .*$', source, re.M)[0] + '\n'
code += 'static uint32_t s_read_fault_streak[AUDIO_ENGINE_DECK_COUNT];\n'
code += block('static bool ae_note_read_fault(') + '\n'
code += block('static void ae_clear_read_faults(') + '\n'
code += block('static esp_err_t audio_output_service_open_codec(uint32_t sample_rate)\n{') + '\n'
decode = source[source.index('static void ae_decode_task(void *arg)'):]
assert decode.index('bool read_fault_retry = s_read_fault_streak[ctx->deck] > 0u;', decode.index('bool eof =')) < decode.index('AE_UNLOCK();', decode.index('bool eof ='))
steady = decode[decode.index('if (eof && samples <= 0)'):]
saved = source
source = steady
retry = block('if (samples <= 0) {')
source = saved
code += 'static void retry_turn(int samples, bool read_fault_retry) { for (int i=0; i<1; ++i) {\n' + retry + '\n} }\n'
code += r'''
int main(void) {
    assert(audio_output_service_open_codec(0) == ESP_ERR_INVALID_ARG);
    assert(!s_output_codec_open && !configured);
    assert(audio_output_service_open_codec(44100) == ESP_OK);
    assert(atomic_load_bool(&s_output_codec_open) && configured == 1 && !lock_depth);
    assert(audio_output_service_open_codec(48000) == ESP_OK && configured == 1);
    atomic_store_bool(&s_output_codec_open, false);
    board.fixed_output_sample_rate = 48000;
    assert(audio_output_service_open_codec(44100) == ESP_OK);
    assert(s_output_sample_rate == 48000 && configured == 2 && !lock_depth);
    audio_engine_state_t eng = {0};
    for (unsigned i=1; i<AE_READ_FAULT_RETRIES; ++i) {
        assert(!ae_note_read_fault(&eng, 0) && !eng.eof);
        retry_turn(0, s_read_fault_streak[0] > 0);
    }
    assert(slept == (AE_READ_FAULT_RETRIES-1)*AE_READ_FAULT_BACKOFF_MS);
    assert(ae_note_read_fault(&eng, 0) && eng.eof && eng.last_error == ESP_ERR_INVALID_STATE);
    assert(!strcmp(eng.last_error_text, "MEDIA READ ERR"));
    assert(s_read_fault_streak[1] == 0);
    ae_clear_read_faults(0); eng.eof = false;
    unsigned before = slept;
    retry_turn(0, false); retry_turn(1152, true);
    assert(slept == before);
    assert(!ae_note_read_fault(&eng, 0) && !eng.eof);
    puts("PASS production ready-publication, bounded backoff outside lock, reset and deck isolation");
}
'''
build = Path(__file__).resolve().parent / 'build/review-boundaries'
build.mkdir(parents=True, exist_ok=True)
cfile = build / 'review.c'
cfile.write_text(code, encoding='utf-8')
target = build / ('review.exe' if os.name == 'nt' else 'review')
subprocess.run([shutil.which('gcc') or 'gcc', '-std=c11', '-O2', '-Wall', '-Wextra', '-Werror', str(cfile), '-o', str(target)], check=True)
subprocess.run([str(target)], check=True, timeout=30)
