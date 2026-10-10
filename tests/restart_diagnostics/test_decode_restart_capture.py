import hashlib
from pathlib import Path
import struct
import sys
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / "tools"))
from decode_restart_capture import decode_records, validate_app_pair


def record(core, captured=True):
    w = [0] * 32
    w[:4] = [0x50345244, 1, 128, int(captured)]
    w[4:8] = [2, core, 100, 110]
    w[8:14] = [0x4ff0c624, 0x4ff00b70, 0x4ff33c80, 7, 0x3ff1009c, core]
    w[14:20] = [0x4ff0c634, 7, 0x3ff1009c, core, 256, 0]
    w[20:26] = [0x4ff0c625, 0, 0x4ff0c621, 0x3ff100a0, 0, 0x3ff1009c]
    return struct.pack("<32I", *w)


class CaptureDecodeTests(unittest.TestCase):
    def test_original_pc_and_raw_queue_pairs(self):
        data = decode_records(record(0) + record(1, False))
        self.assertEqual(data[0]["frame"]["mepc"], "0x4ff0c624")
        self.assertEqual(data[0]["csr"]["mepc"], "0x4ff0c634")
        self.assertEqual(data[0]["store_queue"][2]["address_raw"], "0x3ff1009c")
        self.assertEqual(data[0]["store_queue"][2]["pc_raw"], "0x4ff0c621")
        self.assertEqual(data[0]["store_queue"][2]["pc_address"], "0x4ff0c620")
        self.assertFalse(data[0]["store_queue"][1]["pc_valid"])
        self.assertFalse(data[1]["captured"])
        self.assertNotIn("frame", data[1])

    def test_truncated_missing_wrong_abi(self):
        good = record(0) + record(1)
        for bad in (b"", good[:-4], b"\0" * 256, good[:4] + struct.pack("<I", 2) + good[8:]):
            with self.subTest(length=len(bad)), self.assertRaises(ValueError):
                decode_records(bad)

    def test_wrong_core_flags_reserved_phase(self):
        good = bytearray(record(0) + record(1))
        for index, value in ((17, 1), (3, 2), (4, 3), (27, 1)):
            bad = bytearray(good)
            struct.pack_into("<I", bad, index*4, value)
            with self.subTest(index=index), self.assertRaises(ValueError):
                decode_records(bad)

    def test_app_pair_full_hash_not_prefix(self):
        elf = b"test matching application ELF"
        image = bytearray(208); image[0] = 0xe9
        struct.pack_into("<I", image, 32, 0xabcd5432)
        image[176:208] = hashlib.sha256(elf).digest()
        self.assertEqual(validate_app_pair(image, elf), hashlib.sha256(elf).hexdigest())
        with self.assertRaises(ValueError): validate_app_pair(image, elf+b"wrong")
        with self.assertRaises(ValueError): validate_app_pair(image[:207], elf)


if __name__ == "__main__":
    unittest.main()
