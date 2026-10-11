"""Execute the actual shared-host installer across board/diagnostic policies."""
import pathlib
import subprocess
import tempfile
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[2]


class InterruptPolicy(unittest.TestCase):
    def test_actual_installer(self):
        source = (ROOT / 'firmware/p4-core/components/usb_storage/usb_storage_shared.c').read_text()
        fn = source[source.index('static esp_err_t shared_storage_host_install('):
                    source.index('static esp_err_t shared_storage_handle_events(')]
        harness = r'''
#include <assert.h>
#include <stdbool.h>
#define ESP_INTR_FLAG_LEVEL1 (1<<1)
#define ESP_INTR_FLAG_LEVEL3 (1<<3)
#define USB_HOST_MANAGER_PERIPHERAL_MAP_DUAL 3
#define tskNO_AFFINITY -1
typedef int esp_err_t;
typedef struct {int unused;} usb_host_config_t;
typedef struct {int peripheral_map;bool root_port_unpowered,override_fs_phy_index;
 int fs_phy_index,intr_flags,daemon_stack_size,daemon_priority,daemon_core_id;} usb_host_manager_config_t;
static struct {int fs_phy_index;} board={1};
static void *unused;
#define board_capabilities_get() (&board)
static esp_err_t usb_host_manager_init(const usb_host_manager_config_t *c) {
#if CONFIG_PAJONIIIR_BOARD_M3 && CONFIG_CONTROLLER_USB_HS_AUDIO_DIAGNOSTIC
 assert(c->intr_flags==ESP_INTR_FLAG_LEVEL3);
#else
 assert(c->intr_flags==ESP_INTR_FLAG_LEVEL1);
#endif
#if CONFIG_CONTROLLER_USB_HS_AUDIO_DIAGNOSTIC
 assert(c->daemon_core_id==1);
#else
 assert(c->daemon_core_id==tskNO_AFFINITY);
#endif
 assert(c->peripheral_map==3 && c->root_port_unpowered && c->override_fs_phy_index);
 assert(c->fs_phy_index==1 && c->daemon_priority==4 && c->daemon_stack_size==4096);
 return 7; /* Installer must propagate manager allocation/IRQ errors. */
}
''' + fn + r'''
int main(void) {(void)unused;assert(shared_storage_host_install(0)==7);return 0;}
'''
        with tempfile.TemporaryDirectory(prefix='p4-usb-irq-') as d:
            src = pathlib.Path(d) / 'irq.c'
            src.write_text(harness)
            for m3, hs in ((0, 0), (1, 0), (0, 1), (1, 1)):
                exe = pathlib.Path(d) / f'irq{m3}{hs}.exe'
                subprocess.run(['gcc', '-std=c11', '-Wall', '-Wextra', '-Werror',
                                f'-DCONFIG_PAJONIIIR_BOARD_M3={m3}',
                                f'-DCONFIG_CONTROLLER_USB_HS_AUDIO_DIAGNOSTIC={hs}',
                                '-DCONFIG_USB_HOST_DIAGNOSTIC_HS_ONLY=0', str(src), '-o', str(exe)], check=True)
                subprocess.run([str(exe)], check=True)


if __name__ == '__main__':
    unittest.main()
