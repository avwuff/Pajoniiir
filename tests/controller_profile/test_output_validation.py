"""Compiler MIDI output limits match runtime and storage admission."""
import importlib.util
from pathlib import Path
import unittest

path = Path(__file__).resolve().parents[2] / 'tools/controller_profile/compile_profile.py'
spec = importlib.util.spec_from_file_location('compiler', path)
compiler = importlib.util.module_from_spec(spec)
spec.loader.exec_module(compiler)

class OutputValidation(unittest.TestCase):
    def test_channel_voice_status_boundaries(self):
        for status in (0, 0x7f, 0xf0, 0xff):
            with self.subTest(status=status), self.assertRaises(ValueError):
                compiler.Output(1, 0, 0, status, 1)
        for status in (0x80, 0x90, 0xb0, 0xef):
            self.assertEqual(compiler.Output(1, 0, 0, status, 1).pack()[3], status)

    def test_all_output_data_bytes(self):
        for field in ('data1', 'off', 'on', 'blink'):
            for value in (-1, 128, 255):
                args = dict(led_id=1, deck=0, kind=0, status=0x90, data1=1)
                args[field] = value
                with self.subTest(field=field, value=value), self.assertRaises(ValueError):
                    compiler.Output(**args)
        self.assertEqual(compiler.Output(1, 0, 0, 0x90, 127, 127, 127, 127).pack()[4:8], bytes([127]*4))

if __name__ == '__main__':
    unittest.main()
