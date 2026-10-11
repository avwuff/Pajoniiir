"""Execute production queue reset with a preempted lower-priority producer."""
from pathlib import Path
import os
import shutil
import subprocess

root = Path(__file__).resolve().parents[2]
component = root/'firmware/p4-core/components/controller_usb_host'
source = (component/'controller_usb_host_routed.c').read_text(encoding='utf-8')
start = source.index('static BaseType_t routed_queue_reset(')
opening = source.index('{', start)
depth, end = 1, opening+1
while depth:
    depth += (source[end]=='{') - (source[end]=='}')
    end += 1
code = r'''
#include <assert.h>
#include "controller_midi_out_gate.h"
typedef int BaseType_t;
typedef void *QueueHandle_t;
#define pdFALSE 0
static controller_midi_out_gate_t s_output_gate;
static unsigned delays, resets;
/* The modeled producer is lower priority on the owner's core: it can retire
 * only while the owner is blocked, not while the owner remains runnable. */
static void vTaskDelay(unsigned ticks) {
    assert(ticks==1 && !controller_midi_out_gate_is_accepting(&s_output_gate));
    assert(controller_midi_out_gate_active_producers(&s_output_gate)==1);
    delays++; controller_midi_out_gate_end(&s_output_gate);
}
#define taskYIELD() assert(!"runnable high-priority owner starves the preempted producer")
static BaseType_t xQueueGenericReset(QueueHandle_t queue, int create) {
    assert(queue==(void*)1 && create==pdFALSE);
    assert(!controller_midi_out_gate_is_accepting(&s_output_gate));
    assert(controller_midi_out_gate_active_producers(&s_output_gate)==0);
    resets++;return 1;
}
'''
code += source[start:end]+r'''
int main(void) {
    controller_midi_out_gate_init(&s_output_gate);
    controller_midi_out_gate_start(&s_output_gate);
    assert(controller_midi_out_gate_begin(&s_output_gate, 0));
    assert(routed_queue_reset((void*)1)==1);
    assert(delays==1 && resets==1);
    assert(!controller_midi_out_gate_begin(&s_output_gate, 0));
    assert(routed_queue_reset((void*)1)==1);
    assert(delays==1 && resets==2);
}
'''
out = root/'.cache/routed_queue_reset_review'
out.mkdir(parents=True, exist_ok=True)
cfile = out/'test.c'; cfile.write_text(code, encoding='utf-8')
exe = out/('test.exe' if os.name=='nt' else 'test')
subprocess.run([shutil.which('gcc'), '-std=c11', '-Wall', '-Wextra', '-Werror',
    '-I'+str(component/'include'), str(cfile), str(component/'controller_midi_out_gate.c'), '-o', str(exe)], check=True)
subprocess.run([str(exe)], check=True)
print('PASS production queue reset: lower-priority producer retires while owner blocked, gate closed before drain')
