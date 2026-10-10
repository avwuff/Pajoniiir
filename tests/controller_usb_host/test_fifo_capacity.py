"""Execute production FIFO policy against actual controller packet requirements."""
import pathlib
import re
import subprocess
import tempfile
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[2]


class FifoCapacity(unittest.TestCase):
    def test_controller_and_storage_packet_limits(self):
        cmake = (ROOT / 'firmware/p4-core/cmake/apply_espressif_usb_fifo_patch.cmake').read_text()
        policy = re.search(r'set\(_pajoniiir_fifo_patched \[=\[\n(.*?)\]=\]\)', cmake, re.S)[1]
        harness = r'''
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
typedef struct { unsigned rx_fifo_lines, nptx_fifo_lines, ptx_fifo_lines; } fifo_t;
typedef struct { fifo_t fifo_config; } port_t;
typedef struct { struct { unsigned hsphy_type, fifo_size; } constant_config; } usb_dwc_hal_context_t;
static void calculate(port_t *port, const usb_dwc_hal_context_t *hal) {
''' + policy + r'''
}
int main(void) {
    port_t p = {0};
    usb_dwc_hal_context_t hal = {{1, 1024}};
    calculate(&p, &hal);
    assert(p.fifo_config.rx_fifo_lines + p.fifo_config.nptx_fifo_lines + p.fifo_config.ptx_fifo_lines == 1024);
    /* Pinned HAL limits: RX reserves two status quadlets, TX lines are bytes/4.
     * HS storage must still accept 512-byte bulk packets in both directions. */
    assert((p.fifo_config.rx_fifo_lines - 2) * 4 >= 512);
    assert(p.fifo_config.nptx_fifo_lines * 4 >= 512);
#if defined(CONFIG_PAJONIIIR_BOARD_JC1060) || CONFIG_CONTROLLER_USB_HS_AUDIO_DIAGNOSTIC
    assert(p.fifo_config.ptx_fifo_lines * 4 == 640);
    /* Captured Hercules OUT MPS is 540; parser-only widening was insufficient. */
    assert(p.fifo_config.ptx_fifo_lines * 4 >= 540);
#else
    assert(p.fifo_config.ptx_fifo_lines * 4 == 512);
#endif
    hal.constant_config.hsphy_type = 0; hal.constant_config.fifo_size = 256;
    calculate(&p, &hal);
    assert(p.fifo_config.rx_fifo_lines + p.fifo_config.nptx_fifo_lines + p.fifo_config.ptx_fifo_lines == 256);
    assert((p.fifo_config.rx_fifo_lines - 2) * 4 >= 64);
    assert(p.fifo_config.nptx_fifo_lines * 4 >= 64);
#ifdef CONFIG_PAJONIIIR_BOARD_JC1060
    assert(p.fifo_config.ptx_fifo_lines * 4 == 640);
#else
    /* The experiment changes only HS-root capacity; standard FLX4 stays 400. */
    assert(p.fifo_config.ptx_fifo_lines * 4 == 400);
#endif
    assert(p.fifo_config.ptx_fifo_lines * 4 >= 384);
    return 0;
}
'''
        with tempfile.TemporaryDirectory() as directory:
            source = pathlib.Path(directory) / 'fifo.c'
            source.write_text(harness)
            for jc1060, experiment in ((False, False), (False, True), (True, False), (True, True)):
                executable = pathlib.Path(directory) / f'fifo-{jc1060}-{experiment}.exe'
                args = ['gcc', '-std=c11', '-Wall', '-Wextra', '-Werror',
                        f'-DCONFIG_CONTROLLER_USB_HS_AUDIO_DIAGNOSTIC={int(experiment)}']
                if jc1060:
                    args.append('-DCONFIG_PAJONIIIR_BOARD_JC1060=1')
                subprocess.run(args + [str(source), '-o', str(executable)], check=True)
                subprocess.run([str(executable)], check=True)


if __name__ == '__main__':
    unittest.main()
