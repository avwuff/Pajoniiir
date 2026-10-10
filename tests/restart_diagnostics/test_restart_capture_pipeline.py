"""Synthetic raw-coredump pipeline tests using the actual built app BIN/ELF.

Run with ESP-IDF Python and --build <M3 build dir>. These exercise checksum,
identity, user-region lookup and ABI decoding; they are not physical PANIC tests.
"""
import argparse
import hashlib
import json
from pathlib import Path
import struct
import sys
import tempfile
import unittest
from elftools.elf.elffile import ELFFile
from esp_coredump.corefile.elf import ESPCoreDumpElfFile
from esp_coredump.corefile.loader import EspCoreDumpLoader

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / "tools"))
from decode_restart_capture import decode_dump

BUILD = None


class RawPipelineTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory(prefix="p4-restart-capture-test-")
        self.root = Path(self.tmp.name)
        description = json.loads((BUILD / "project_description.json").read_text())
        self.app_elf = BUILD / (description["project_name"] + ".elf")
        self.app_bin = BUILD / (description["project_name"] + ".bin")
        self.prefix = hashlib.sha256(self.app_elf.read_bytes()).hexdigest()[:9]
        with self.app_elf.open("rb") as stream:
            app = ELFFile(stream)
            symbol = app.get_section_by_name(".symtab").get_symbol_by_name("p4_restart_capture")[0]
            self.address = symbol["st_value"]
            self.assertEqual(app.get_section(symbol["st_shndx"]).name, ".noinit")
            self.assertEqual(symbol["st_size"], 256)
        # NOBITS has no initialized ELF payload. Model the production boot ABI;
        # the link contract separately verifies its init calls and dump bounds.
        self.payload = bytearray(256)
        for offset in (0, 128):
            struct.pack_into("<III", self.payload, offset, 0x50345244, 1, 128)
        # Simulate one captured frame with distinguishable trap/queue data.
        struct.pack_into("<I", self.payload, 3*4, 1)
        struct.pack_into("<I", self.payload, 8*4, 0x4ff0c634)
        struct.pack_into("<I", self.payload, 17*4, 0)
        struct.pack_into("<I", self.payload, 20*4, 0x4ff0c625)
        struct.pack_into("<I", self.payload, 23*4, 0x3ff100a0)

    def tearDown(self):
        self.tmp.cleanup()

    def raw(self, payload=None, prefix=None):
        core_path = self.root / "fixture-core.elf"
        core = ESPCoreDumpElfFile(e_machine=ESPCoreDumpElfFile.EM_RISCV)
        note = EspCoreDumpLoader._build_note_section("ESP_CORE_DUMP_INFO", 8266,
            struct.pack("<I", 0x120104) + (self.prefix if prefix is None else prefix).encode().ljust(64, b"\0"))
        core.add_segment(0, note, 4, 0)
        core.add_segment(self.address, bytes(self.payload if payload is None else payload), 1, 6)
        core.dump(str(core_path))
        content = core_path.read_bytes()
        body = struct.pack("<III", 12 + len(content) + 32, 0x120104, 103) + content
        raw_path = self.root / "fixture-raw.bin"
        raw_path.write_bytes(body + hashlib.sha256(body).digest())
        return raw_path

    def test_complete_checksum_identity_capture_pipeline(self):
        raw = self.raw()
        original = raw.read_bytes()
        result = decode_dump(raw, self.app_bin, self.app_elf)
        self.assertTrue(result["checksum_validated"])
        self.assertEqual(result["dump_elf_prefix"], self.prefix)
        self.assertEqual(result["records"][0]["frame"]["mepc"], "0x4ff0c634")
        self.assertEqual(result["records"][0]["store_queue"][0]["pc_raw"], "0x4ff0c625")
        self.assertFalse(result["records"][1]["captured"])
        self.assertEqual(raw.read_bytes(), original)

    def test_corrupt_embedded_checksum_rejected(self):
        raw = self.raw(); bad = bytearray(raw.read_bytes()); bad[-1] ^= 1
        raw.write_bytes(bad)
        with self.assertRaises(Exception): decode_dump(raw, self.app_bin, self.app_elf)

    def test_wrong_dump_identity_rejected(self):
        with self.assertRaisesRegex(ValueError, "prefix"):
            decode_dump(self.raw(prefix="0"*9), self.app_bin, self.app_elf)

    def test_incomplete_user_region_rejected(self):
        with self.assertRaisesRegex(ValueError, "complete diagnostic"):
            decode_dump(self.raw(payload=self.payload[:-4]), self.app_bin, self.app_elf)

    def test_wrong_capture_abi_rejected(self):
        struct.pack_into("<I", self.payload, 4, 2)
        with self.assertRaisesRegex(ValueError, "header/version"):
            decode_dump(self.raw(), self.app_bin, self.app_elf)


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--build", type=Path, required=True)
    args, rest = parser.parse_known_args()
    BUILD = args.build
    unittest.main(argv=[sys.argv[0]] + rest)
