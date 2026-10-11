"""Run the pinned USBH functions with a client scheduled inside publication.

The callback can run the client to completion before the daemon resumes.
Also cover batched completions and a callback submitting a new transfer.
This checks ownership ordering, not physical USB timing or S2 acceptance.
"""
from pathlib import Path
import os
import re
import shutil
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[2]
HERE = Path(__file__).resolve().parent
patch = (ROOT / 'firmware/p4-core/cmake/apply_espressif_usb_control_retirement_patch.cmake').read_text()
source = (HERE / 'fixtures/usbh_control_retirement.c').read_text()
source = source[source.index('static inline void handle_ep0_dequeue('):]
managed = ROOT / 'firmware/main-deck-m3/managed_components/usb/src/usbh.c'
if managed.exists():
    actual = managed.read_text()
    for function in source.strip().split('\n\n/* CLOSE */\n\n'):
        assert function.strip() in actual, 'Pinned USBH function drifted'

def block(name):
    return re.search(r'set\(' + name + r' \[=\[\n(.*?)\n\]=\]\)', patch, re.S).group(1) + '\n'

upstream, replacement, tail = [block(n) for n in ('_p4_ctrl_upstream', '_p4_ctrl_patched', '_p4_ctrl_tail')]
assert source.count(upstream) == source.count(tail) == 1
patched = source.replace('    int num_urbs = 0;\n', '').replace(upstream, replacement).replace(tail, '')
# Execute the real CMake transform and compare its output to the compiled body.
cmake = shutil.which('cmake')
if not cmake and os.name == 'nt':
    candidates = sorted(Path('C:/Espressif/tools/cmake').glob('*/bin/cmake.exe'))
    cmake = str(candidates[-1]) if candidates else None
assert cmake, 'cmake is required'
ninja = shutil.which('ninja')
if not ninja and os.name == 'nt':
    candidates = sorted(Path('C:/Espressif/tools/ninja').glob('*/ninja.exe'))
    ninja = str(candidates[-1]) if candidates else None
assert ninja, 'ninja is required'
with tempfile.TemporaryDirectory() as temp:
    folder = Path(temp)
    (folder / 'src').mkdir()
    (folder / 'src/usbh.c').write_text(source)
    hook = (ROOT / 'firmware/p4-core/cmake/apply_espressif_usb_control_retirement_patch.cmake').as_posix()
    (folder / 'CMakeLists.txt').write_text('''cmake_minimum_required(VERSION 3.16)
project(control_retirement NONE)
add_custom_target(usb_stub SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/usbh.c")
function(idf_component_get_property output component property)
    if(property STREQUAL "COMPONENT_DIR")
        set(${output} "${CMAKE_CURRENT_SOURCE_DIR}" PARENT_SCOPE)
    else()
        set(${output} usb_stub PARENT_SCOPE)
    endif()
endfunction()
include("''' + hook + '''")
get_target_property(sources usb_stub SOURCES)
file(WRITE "${CMAKE_BINARY_DIR}/selected-source.txt" "${sources}")
''')
    args = [cmake, '-G', 'Ninja', '-DCMAKE_MAKE_PROGRAM=' + ninja,
            '-S', str(folder), '-B', str(folder / 'build')]
    # No compiler or Ninja execution is needed for this source-selection check.
    result = subprocess.run(args, capture_output=True)
    assert result.returncode == 0, result.stderr.decode(errors='replace')
    generated = folder / 'build/pajoniiir_usb_control/usbh.c'
    assert generated.read_text() == patched
    assert Path((folder / 'build/selected-source.txt').read_text()) == generated
    (folder / 'src/usbh.c').write_text(source.replace('num_urbs++;', 'num_urbs += 1;'))
    result = subprocess.run(args, capture_output=True)
    assert result.returncode != 0 and b'EP0 retirement block changed' in result.stderr
stub = r'''
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
typedef int esp_err_t;
typedef void *usb_device_handle_t;
typedef struct { int id; } urb_t;
typedef struct {
    struct { void *default_pipe; } constant;
    struct { int num_ctrl_xfers_inflight, open_count;
        struct { bool enum_lock, is_gone, waiting_free; } flags; } dynamic;
} device_t;
typedef struct { int event; struct { usb_device_handle_t dev_hdl; urb_t *urb; } ctrl_xfer_data; } usbh_event_data_t;
static int depth, published, queued, mode, frees_requested;
static urb_t urbs[3];
static device_t device;
#define USBH_ENTER_CRITICAL() (++depth)
#define USBH_EXIT_CRITICAL() (--depth)
#define USBH_CHECK(c, e) do { if (!(c)) return (e); } while (0)
#define USBH_CHECK_FROM_CRIT(c, e) do { if (!(c)) { --depth; return (e); } } while (0)
#define ESP_OK 0
#define ESP_ERR_INVALID_ARG 1
#define ESP_ERR_NOT_ALLOWED 2
#define USBH_EVENT_CTRL_XFER 1
#define DEV_ACTION_FREE 1
#define USB_PROC_REQ_SOURCE_USBH 1
#define ESP_LOGD(...) ((void)0)
static bool _dev_set_actions(device_t *d, int action) { (void)d; assert(action == 1); ++frees_requested; return true; }
static void request(int a, bool b, void *c) { (void)a; (void)b; (void)c; assert(depth == 0); }
static urb_t *hcd_urb_dequeue(void *pipe) { (void)pipe; return queued ? &urbs[--queued] : NULL; }
static void completion(usbh_event_data_t *, void *);
static struct { struct { void (*event_cb)(usbh_event_data_t *, void *); void *event_cb_arg;
    void (*proc_req_cb)(int, bool, void *); void *proc_req_cb_arg; } constant; } host = { .constant = { .event_cb = completion, .proc_req_cb = request } };
static __typeof__(host) *p_usbh_obj = &host;
'''
main = r'''
static void completion(usbh_event_data_t *e, void *arg) {
    (void)arg; assert(e->ctrl_xfer_data.dev_hdl == &device); assert(depth == 0);
    ++published;
    if (mode == 2 && published == 1) {
        // An internal or client callback can submit a subsequent control stage.
        assert(device.dynamic.num_ctrl_xfers_inflight == 0);
        ++device.dynamic.num_ctrl_xfers_inflight; ++queued;
    } else if (queued == 0) {
        assert(usbh_dev_close(&device) == ESP_OK);
        assert(device.dynamic.open_count == 0);
    } else {
        assert(device.dynamic.num_ctrl_xfers_inflight == queued);
    }
}
int main(int argc, char **argv) {
    assert(argc == 2); mode = atoi(argv[1]);
    queued = mode == 1 ? 3 : (mode == 3 ? 0 : 1);
    device.dynamic.num_ctrl_xfers_inflight = queued;
    device.dynamic.open_count = 1; device.dynamic.flags.is_gone = true;
    handle_ep0_dequeue(&device);
    assert(device.dynamic.num_ctrl_xfers_inflight == 0 && depth == 0);
    assert(published == (mode == 1 ? 3 : (mode == 2 ? 2 : (mode == 3 ? 0 : 1))));
    assert(frees_requested == (mode == 3 ? 0 : 1));
    return 0;
}
'''
build = HERE / ('build/control-retirement-' + str(os.getpid()))
build.mkdir(parents=True, exist_ok=True)
gcc = shutil.which('gcc')
assert gcc, 'gcc is required'
for label, body in [('upstream', source), ('patched', patched)]:
    c = build / (label + '.c')
    exe = build / (label + ('.exe' if os.name == 'nt' else ''))
    c.write_text(stub + body + main)
    subprocess.run([gcc, '-std=gnu11', '-Wall', '-Wextra', '-Werror', str(c), '-o', str(exe)], check=True)
    for case in range(4):
        result = subprocess.run([str(exe), str(case)], capture_output=True, timeout=10)
        if label == 'patched' or case == 3:
            assert result.returncode == 0, result.stderr.decode(errors='replace')
        else:
            assert result.returncode != 0 and b'Assertion' in result.stderr, 'Upstream race was not reproduced'
print('USBH control retirement: upstream race reproduced; single/batch/resubmit/empty PASS')
