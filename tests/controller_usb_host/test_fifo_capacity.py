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
        stream = (ROOT / 'firmware/p4-core/components/controller_usb_audio/controller_usb_audio_stream.c').read_text()
        admission = re.search(r'#if[^\n]+\n#define STREAM_CONTROLLER_FIFO_BYTES.*?#endif', stream, re.S)[0]
        harness = r'''
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
typedef struct { unsigned rx_fifo_lines, nptx_fifo_lines, ptx_fifo_lines; } fifo_t;
typedef struct { fifo_t fifo_config; } port_t;
typedef struct { struct { unsigned hsphy_type, fifo_size; } constant_config; } usb_dwc_hal_context_t;
''' + admission + r'''
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
    const unsigned hs_tx_bytes = p.fifo_config.ptx_fifo_lines * 4;
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
    /* M3 role swap uses the HS PHY FIFO even when the device runs at FS.
     * Diagnostic mode alone must not widen admission on the ordinary FS root. */
    assert(STREAM_CONTROLLER_FIFO_BYTES ==
           (CONFIG_BSP_M3_HS_CONTROLLER ? hs_tx_bytes : p.fifo_config.ptx_fifo_lines * 4));
#if CONFIG_BSP_M3_HS_CONTROLLER && CONFIG_CONTROLLER_USB_HS_AUDIO_DIAGNOSTIC
    assert(STREAM_CONTROLLER_FIFO_BYTES >= 540);
#endif
#if defined(CONFIG_PAJONIIIR_BOARD_JC4880) || defined(CONFIG_PAJONIIIR_BOARD_JC1060)
    assert(p.fifo_config.ptx_fifo_lines * 4 == 640);
    assert(p.fifo_config.ptx_fifo_lines * 4 >= 540);
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
            cases = ((board, experiment, hs_controller)
                     for board in ('M3', 'JC4880', 'JC1060')
                     for experiment in (False, True)
                     for hs_controller in ((False, True) if board == 'M3' else (False,)))
            for board, experiment, hs_controller in cases:
                executable = pathlib.Path(directory) / f'fifo-{board}-{experiment}-{hs_controller}.exe'
                args = ['gcc', '-std=c11', '-Wall', '-Wextra', '-Werror',
                        f'-DCONFIG_CONTROLLER_USB_HS_AUDIO_DIAGNOSTIC={int(experiment)}',
                        f'-DCONFIG_BSP_M3_HS_CONTROLLER={int(hs_controller)}']
                args.append(f'-DCONFIG_PAJONIIIR_BOARD_{board}=1')
                subprocess.run(args + [str(source), '-o', str(executable)], check=True)
                subprocess.run([str(executable)], check=True)


if __name__ == '__main__':
    unittest.main()
