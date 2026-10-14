"""Reject missing initialization on either real linked startup path."""
from pathlib import Path
import sys
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / "tools"))
from check_restart_capture_elf import verify_startup_initialization


class StartupContract(unittest.TestCase):
    def setUp(self):
        self.hooks = ["__wrap_esp_restart", "__wrap_panic_arch_fill_info"]
        self.functions = {name: "jal 100 <p4_restart_capture_init>" for name in self.hooks}
        self.functions["pajoniiir_common_app_main_with_network"] = "jal 100 <p4_restart_capture_init>"
        self.functions["pajoniiir_common_app_main"] = "j 100 <pajoniiir_common_app_main_with_network>"
        self.functions["app_main"] = "jal 100 <pajoniiir_common_app_main>"

    def test_delegation(self):
        verify_startup_initialization(self.functions, self.hooks)

    def test_inlined_initialization(self):
        self.functions["pajoniiir_common_app_main"] = "jal 100 <p4_restart_capture_init>"
        verify_startup_initialization(self.functions, self.hooks)

    def test_no_provider_initialization(self):
        self.functions["pajoniiir_common_app_main_with_network"] = "ret"
        with self.assertRaises(ValueError):
            verify_startup_initialization(self.functions, self.hooks)

    def test_no_entry_delegation(self):
        self.functions["pajoniiir_common_app_main"] = "ret"
        with self.assertRaises(ValueError):
            verify_startup_initialization(self.functions, self.hooks)

    def test_hook_cannot_delegate_through_app(self):
        self.functions[self.hooks[0]] = "j 100 <pajoniiir_common_app_main_with_network>"
        with self.assertRaises(ValueError):
            verify_startup_initialization(self.functions, self.hooks)

    def test_provider_board_garbage_collects_unused_wrapper(self):
        del self.functions["pajoniiir_common_app_main"]
        self.functions["app_main"] = "jal 100 <pajoniiir_common_app_main_with_network>"
        verify_startup_initialization(self.functions, self.hooks)

    def test_missing_actual_board_call(self):
        self.functions["app_main"] = "ret"
        with self.assertRaises(ValueError):
            verify_startup_initialization(self.functions, self.hooks)


if __name__ == "__main__":
    unittest.main()
