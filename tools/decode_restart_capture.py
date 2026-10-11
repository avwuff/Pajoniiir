"""Decode a checksum-validated P4 raw dump using its retained app BIN/ELF.

Requires the ESP-IDF Python environment (esp_coredump and pyelftools).
Never substitute a running/new ELF for the crashed app. The dump identity
may contain only a prefix; the BIN/ELF pair is checked by the full ELF hash.
"""
import argparse
import hashlib
import json
from pathlib import Path
import struct

MAGIC = 0x50345244
RECORD_BYTES = 128
CORES = 2


def decode_records(data):
    if len(data) != RECORD_BYTES * CORES:
        raise ValueError("missing or truncated 256-byte restart capture")
    records = []
    for core in range(CORES):
        w = struct.unpack_from("<32I", data, core * RECORD_BYTES)
        if w[0:3] != (MAGIC, 1, RECORD_BYTES) or w[3] not in (0, 1):
            raise ValueError("invalid restart capture header/version/flags")
        if w[4] not in (0, 1, 2) or any(w[27:]):
            raise ValueError("invalid restart phase or reserved data")
        captured = w[3] == 1
        if captured and w[17] != core:
            raise ValueError("capture CSR core does not match its slot")
        entry = {"slot": core, "captured": captured, "restart_phase": w[4],
                 "restart_core": w[5], "restart_cycle": w[6], "noos_cycle": w[7]}
        if captured:
            entry.update(
                frame=dict(zip(("mepc", "ra", "sp", "mcause", "mtval", "mhartid"),
                               (hex(v) for v in w[8:14]))),
                csr=dict(zip(("mepc", "mcause", "mtval", "mhartid", "mexstatus", "mhint"),
                             (hex(v) for v in w[14:20]))),
                store_queue=[{"index": i, "pc_raw": hex(w[20+i]),
                              "pc_valid": bool(w[20+i] & 1),
                              "pc_address": hex(w[20+i] & ~1),
                              "address_raw": hex(w[23+i])} for i in range(3)],
                capture_cycle=w[26],
                frame_core_matches_reader=(w[13] == w[17]),
                sampling="sequential on one core; not an atomic bus snapshot")
        records.append(entry)
    return records


def validate_app_pair(bin_bytes, elf_bytes):
    if len(bin_bytes) < 208 or bin_bytes[0] != 0xE9:
        raise ValueError("invalid application BIN")
    if struct.unpack_from("<I", bin_bytes, 32)[0] != 0xABCD5432:
        raise ValueError("missing application descriptor")
    elf_hash = hashlib.sha256(elf_bytes).hexdigest()
    if bin_bytes[176:208].hex() != elf_hash:
        raise ValueError("application BIN and ELF full SHA-256 differ")
    return elf_hash


def decode_dump(raw_path, app_bin, app_elf):
    from io import BytesIO
    from esp_coredump.corefile.loader import ESPCoreDumpFileLoader
    from esp_coredump.corefile.elf import ESPCoreDumpElfFile
    from elftools.elf.elffile import ELFFile

    elf_bytes = app_elf.read_bytes()
    app_hash = validate_app_pair(app_bin.read_bytes(), elf_bytes)
    raw = raw_path.read_bytes()
    loader = ESPCoreDumpFileLoader(str(raw_path), is_b64=False)
    try:
        if loader.header.tot_len != len(raw):
            raise ValueError("raw dump extent differs from declared length")
        # This verifies the embedded dump checksum before reading any records.
        loader.create_corefile(exe_name=None, e_machine=ESPCoreDumpElfFile.EM_RISCV)
        with open(loader.core_elf_file, "rb") as stream:
            core = ELFFile(stream)
            prefixes = []
            for segment in core.iter_segments():
                if segment["p_type"] == "PT_NOTE":
                    for note in segment.iter_notes():
                        if note["n_name"] == "ESP_CORE_DUMP_INFO":
                            prefixes.append(note["n_desc"][4:68].split(b"\0", 1)[0].decode("ascii"))
            if len(prefixes) != 1 or not (1 <= len(prefixes[0]) <= 64):
                raise ValueError("missing/ambiguous dump ELF identity")
            prefix = prefixes[0].lower()
            if any(c not in "0123456789abcdef" for c in prefix) or not app_hash.startswith(prefix):
                raise ValueError("dump ELF prefix does not match supplied app pair")
            app = ELFFile(BytesIO(elf_bytes))
            if app.elfclass != 32 or app["e_machine"] != "EM_RISCV":
                raise ValueError("supplied app ELF is not RV32")
            symbols = app.get_section_by_name(".symtab")
            matches = [] if symbols is None else symbols.get_symbol_by_name("p4_restart_capture") or []
            if len(matches) != 1 or matches[0]["st_size"] != 256:
                raise ValueError("supplied ELF lacks the restart capture ABI")
            address = matches[0]["st_value"]
            payloads = []
            for segment in core.iter_segments():
                if segment["p_type"] == "PT_LOAD":
                    offset = address - segment["p_vaddr"]
                    if 0 <= offset and offset + 256 <= segment["p_filesz"]:
                        payloads.append(segment.data()[offset:offset+256])
            if len(payloads) != 1:
                raise ValueError("dump does not contain the complete diagnostic user memory region")
            return {"raw_sha256": hashlib.sha256(raw).hexdigest(), "raw_bytes": len(raw),
                    "checksum_validated": True, "app_elf_sha256": app_hash,
                    "dump_elf_prefix": prefix, "identity_scope": f"{len(prefix)} hex characters in dump; full BIN/ELF pair verified",
                    "capture_address": hex(address), "records": decode_records(payloads[0])}
    finally:
        # esp_coredump 1.14 owns temporary filenames, but exposes no cleanup().
        # Never remove raw input or any retained app artifact.
        inputs = {raw_path.resolve(), app_bin.resolve(), app_elf.resolve()}
        for temporary in loader.temp_files:
            path = Path(temporary)
            if path.resolve() not in inputs:
                path.unlink(missing_ok=True)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--raw", required=True, type=Path)
    parser.add_argument("--app-bin", required=True, type=Path)
    parser.add_argument("--app-elf", required=True, type=Path)
    parser.add_argument("--output", type=Path)
    args = parser.parse_args()
    result = decode_dump(args.raw, args.app_bin, args.app_elf)
    rendered = json.dumps(result, indent=2)
    if args.output:
        if args.output.resolve() in (args.raw.resolve(), args.app_bin.resolve(), args.app_elf.resolve()):
            raise ValueError("output must not overwrite an input artifact")
        if args.output.exists():
            raise ValueError("output already exists; preserve evidence")
        with args.output.open("x", encoding="utf-8") as output:
            output.write(rendered + "\n")
    print(rendered)


if __name__ == "__main__":
    main()
