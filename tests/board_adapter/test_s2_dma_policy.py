"""Exercise actual configure and packaging-verifier DMA admission contracts."""
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tools"))
from check_board_build import verify_usb_dma_policy

DMA = "USB_HOST_DWC_DMA_CAP_MEMORY_IN_PSRAM"
BOUNCE = "PAJONIIIR_SD_INTERNAL_BOUNCE"
M3 = {DMA, BOUNCE, "PAJONIIIR_BOARD_M3", "BSP_M3_HS_CONTROLLER", "CONTROLLER_USB_HS_AUDIO_DIAGNOSTIC"}
def config(flags): return "\n".join(f"#define CONFIG_{f} 1" for f in flags)

class PolicyTests(unittest.TestCase):
    def test_production_stays_internal(self):
        for project in ("main-deck-p4", "main-deck-jc1060", "main-deck-m3"):
            verify_usb_dma_policy("", project)
            with self.assertRaises(AssertionError): verify_usb_dma_policy(config(M3), project)

    def test_explicit_s2_and_each_required_capability(self):
        verify_usb_dma_policy(config(M3), "main-deck-m3", experimental_s2=True)
        for flag in M3:
            with self.assertRaises(AssertionError):
                verify_usb_dma_policy(config(M3 - {flag}), "main-deck-m3", experimental_s2=True)
        for project in ("main-deck-p4", "main-deck-jc1060"):
            with self.assertRaises(AssertionError): verify_usb_dma_policy(config(M3), project, experimental_s2=True)

    def test_existing_jc1060_experiment(self):
        verify_usb_dma_policy(config({DMA, BOUNCE}), "main-deck-jc1060", experimental_recorder=True)
        with self.assertRaises(AssertionError):
            verify_usb_dma_policy(config({DMA, BOUNCE}), "main-deck-p4", experimental_recorder=True)

    def test_real_cmake_configure_guard(self):
        cmake = shutil.which("cmake")
        if not cmake:
            cmake = str(sorted(Path("C:/Espressif/tools/cmake").glob("*/bin/cmake.exe"))[-1])
        guard = (ROOT / "firmware/p4-core/components/board_adapter/CMakeLists.txt").read_text().split("idf_component_register(")[0]
        cases = [(set(), set(), True), (M3, {"PAJONIIIR_S2_HS_EXPERIMENT"}, True),
                 ({DMA, BOUNCE, "PAJONIIIR_BOARD_JC1060"}, {"PAJONIIIR_STORAGE_EXPERIMENT"}, True),
                 (M3, set(), False)]
        cases += [(M3 - {f}, {"PAJONIIIR_S2_HS_EXPERIMENT"}, False)
                  for f in M3 - {DMA}]
        with tempfile.TemporaryDirectory() as folder:
            path = Path(folder) / "guard.cmake"
            for flags, options, allowed in cases:
                path.write_text("\n".join(f"set(CONFIG_{f} ON)" for f in flags) + "\n" +
                    "\n".join(f"set({f} ON)" for f in options) + "\n" + guard)
                result = subprocess.run([cmake, "-P", str(path)], capture_output=True)
                self.assertEqual(result.returncode == 0, allowed, result.stderr.decode(errors="replace"))

if __name__ == "__main__": unittest.main()
