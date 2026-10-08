"""Execute the production owner decision with recorded ownership side effects."""
from pathlib import Path
import os
import shutil
import subprocess

root = Path(__file__).resolve().parents[2]
source = (root / 'firmware/p4-core/components/controller_usb_host/controller_usb_host.c').read_text()
start = source.index('static bool handle_audio_fault(')
end = source.index('static esp_err_t probe_device(', start)
function = source[start:end]
code = r'''
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
typedef struct { bool opened, device_gone, audio_fault_isolated, claimed;
    struct { bool usb_audio_active; } identity; } controller_state_t;
typedef struct { bool faulted; unsigned fault_control_step;
    int32_t last_control_status, last_control_bytes; } controller_usb_audio_stream_stats_t;
static bool isolatable, cleanup_stuck;
static unsigned stops, closes, recoveries;
static bool controller_usb_audio_stream_needs_recovery(void) { return cleanup_stuck; }
static bool controller_usb_audio_stream_fault_isolatable(void) { return isolatable; }
static void controller_usb_audio_stream_request_stop(bool gone) { assert(!gone); ++stops; }
#define CONTROLLER_USB_FAULT_UAC_STREAM 5
#define ESP_FAIL -1
#define ESP_LOGW(...) ((void)0)
static void begin_controller_fault_recovery(controller_state_t *s, int f, const char *o, int e)
{ (void)s; (void)o; assert(f == 5 && e == -1); ++recoveries; }
static void close_step(controller_state_t *s) { ++closes; s->claimed = false; s->opened = false; }
'''
main = r'''
int main(void) {
    controller_state_t state = { .opened = true, .claimed = true, .identity.usb_audio_active = true };
    controller_usb_audio_stream_stats_t stats = { .faulted = true };
    isolatable = true;
    assert(!handle_audio_fault(&state, &stats));
    assert(state.opened && state.claimed && state.audio_fault_isolated);
    assert(!state.identity.usb_audio_active && stops == 1 && closes == 0 && recoveries == 0);
    // Subsequent turns preserve MIDI while callbacks finish audio cleanup.
    assert(!handle_audio_fault(&state, &stats) && stops == 1 && closes == 0);
    cleanup_stuck = true;
    assert(handle_audio_fault(&state, &stats) && closes == 1 && recoveries == 1);
    assert(!state.opened && !state.claimed);
    // Default FS and a live-stream fault use the established full recovery.
    cleanup_stuck = false; isolatable = false;
    state = (controller_state_t){ .opened = true, .claimed = true, .identity.usb_audio_active = true };
    assert(handle_audio_fault(&state, &stats) && closes == 2 && recoveries == 2);
    // A vanished device must not be kept alive by an audio-only fallback.
    isolatable = true;
    state = (controller_state_t){ .opened = true, .device_gone = true, .identity.usb_audio_active = true };
    assert(handle_audio_fault(&state, &stats) && closes == 3);
    assert(!handle_audio_fault(&state, &stats));
    return 0;
}
'''
build = Path(__file__).parent / ('build/audio-isolation-' + str(os.getpid()))
build.mkdir(parents=True, exist_ok=True)
c = build / 'owner.c'
exe = build / ('owner.exe' if os.name == 'nt' else 'owner')
c.write_text(code + function + main)
gcc = shutil.which('gcc')
assert gcc, 'gcc is required'
subprocess.run([gcc, '-std=c11', '-Wall', '-Wextra', '-Werror', str(c), '-o', str(exe)], check=True)
subprocess.run([str(exe)], check=True, timeout=10)
print('PASS production audio fault isolation: MIDI retained; stuck/FS/live/gone recovery preserved')
