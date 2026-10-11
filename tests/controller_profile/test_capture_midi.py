"""Offline capture integrity checks; these do not qualify controller hardware."""
import importlib.util
import json
from pathlib import Path
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[2]
spec = importlib.util.spec_from_file_location(
    "capture_midi", ROOT / "tools/controller_profile/capture_midi.py")
capture = importlib.util.module_from_spec(spec)
spec.loader.exec_module(capture)


class CaptureTests(unittest.TestCase):
    def test_short_message_lengths_and_channel(self):
        self.assertEqual(capture.decode_short(0x7F0191), {
            "bytes": [0x91, 1, 127], "status": "0x91", "channel": 2,
            "data1": 1, "data2": 127})
        self.assertEqual(capture.decode_short(0x7F05C3)["bytes"], [0xC3, 5])
        self.assertEqual(capture.decode_short(0xFFFFFFF8)["bytes"], [0xF8])
        self.assertEqual(capture.decode_short(0x0605F2)["bytes"], [0xF2, 5, 6])

    def test_rejects_corrupt_and_sysex(self):
        for packed in (0, 0x800191, 0x7F8091, 0xF0, 0xF7):
            with self.assertRaises(ValueError):
                capture.decode_short(packed)

    def test_requires_unambiguous_input(self):
        devices = [{"winmm_id": 0, "name": "S2 MIDI"},
                   {"winmm_id": 1, "name": "S2 MIDI second"}]
        self.assertEqual(capture.select_device(devices, 1, None), devices[1])
        for name in ("S2", "FLX4"):
            with self.assertRaises(ValueError):
                capture.select_device(devices, None, name)

    def test_incomplete_or_press_only_is_not_semantic_acceptance(self):
        records = [{"type": "metadata", "label": "D1 PLAY"},
                   {"type": "midi", **capture.decode_short(0x7F0191)},
                   {"type": "midi", **capture.decode_short(0x000191)}]
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp) / "capture.jsonl"
            def report(extra):
                path.write_text("\n".join(json.dumps(r) for r in records + extra))
                return capture.analyze(path)
            self.assertFalse(report([])["usable_capture"])
            self.assertFalse(report([{"type": "summary", "complete": False}])["usable_capture"])
            result = report([{"type": "summary", "complete": True}])
            self.assertTrue(result["usable_capture"])
            self.assertEqual(result["addresses"][0]["values"], [0, 127])
            self.assertFalse(result["physical_semantics_confirmed"])


if __name__ == "__main__":
    unittest.main()
