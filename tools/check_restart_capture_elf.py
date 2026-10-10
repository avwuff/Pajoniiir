"""Check actual IDF link placement and interception, not source spelling."""
import argparse
import json
from pathlib import Path
import re
import subprocess
from elftools.elf.elffile import ELFFile


def verify(build):
    description = json.loads((build / "project_description.json").read_text())
    elf_path = build / (description["project_name"] + ".elf")
    with elf_path.open("rb") as stream:
        elf = ELFFile(stream)
        table = elf.get_section_by_name(".symtab")

        def symbol(name):
            items = table.get_symbol_by_name(name) or []
            if len(items) != 1:
                raise ValueError(f"missing or ambiguous symbol {name}")
            return items[0]

        wrapper_names = ["__wrap_panic_arch_fill_info", "__wrap_esp_restart", "__wrap_esp_restart_noos"]
        addresses = {}
        for name in wrapper_names:
            item = symbol(name)
            if elf.get_section(item["st_shndx"]).name != ".iram0.text":
                raise ValueError(f"{name} is not in internal IRAM")
            addresses[name] = hex(item["st_value"])
        init = symbol("p4_restart_capture_init")
        if elf.get_section(init["st_shndx"]).name != ".iram0.text":
            raise ValueError("capture initialization is not in internal IRAM")
        record = symbol("p4_restart_capture")
        start = symbol("_coredump_noinit_start")["st_value"]
        end = symbol("_coredump_noinit_end")["st_value"]
        address = record["st_value"]
        if not (record["st_size"] == 256 and start <= address and address + 256 <= end):
            raise ValueError("record is not fully included in IDF user coredump noinit")
        if elf.get_section(record["st_shndx"]).name != ".noinit":
            raise ValueError("capture is not in internal DRAM")

    cache = (build / "CMakeCache.txt").read_text()
    match = re.search(r"^CMAKE_OBJDUMP:FILEPATH=(.+)$", cache, re.M)
    if not match:
        raise ValueError("missing configured target objdump")
    disassembly = subprocess.check_output([match[1].strip(), "-d", str(elf_path)], text=True)
    blocks = re.split(r"(?m)^[0-9a-f]+ <([^>]+)>:\n", disassembly)
    functions = dict(zip(blocks[1::2], blocks[2::2]))
    callers = {}
    init_call = r"\b(?:jal|jalr|j)\b[^\n]*<p4_restart_capture_init>"
    for caller in wrapper_names + ["pajoniiir_common_app_main"]:
        if not re.search(init_call, functions.get(caller, "")):
            raise ValueError(f"{caller} does not initialize the capture region")
    init_targets = re.findall(r"\b(?:jal|jalr|j)\b[^\n]*<([^>]+)>",
                              functions.get("p4_restart_capture_init", ""))
    if any(target.split("+")[0] != "p4_restart_capture_init" for target in init_targets):
        raise ValueError("capture initialization calls an external helper")
    for wrapped in wrapper_names:
        originals = wrapped.replace("__wrap_", "")
        body = functions.get(wrapped, "")
        if not re.search(r"\b(?:jal|jalr|j)\b[^\n]*<" + re.escape(originals) + r">", body):
            raise ValueError(f"{wrapped} does not delegate to original IDF {originals}")
        found = [name for name, code in functions.items() if name != wrapped and
                 re.search(r"\b(?:jal|jalr|j)\b[^\n]*<" + re.escape(wrapped) + r">", code)]
        if not found:
            raise ValueError(f"no real linked caller intercepts {wrapped}")
        callers[wrapped] = found
    panic = functions["__wrap_panic_arch_fill_info"]
    for csr in ("0x7e1", "0x7c5", "0xbf0", "0xbf1", "0xbf2", "0xbf8", "0xbf9", "0xbfa"):
        if csr not in panic:
            raise ValueError(f"panic hook omits CSR {csr}")
    result = {"wrapper_iram_addresses": addresses, "actual_callers": callers,
              "capture_address": hex(address), "capture_bytes": 256,
              "coredump_noinit_bounds": [hex(start), hex(end)],
              "initialization_iram_address": hex(init["st_value"]),
              "scope": "linked placement/interception only; physical panic export NOT RUN"}
    print(json.dumps(result, indent=2))
    return result


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--build", required=True, type=Path)
    parser.add_argument("--output", type=Path)
    args = parser.parse_args()
    result = verify(args.build)
    if args.output:
        args.output.write_text(json.dumps(result, indent=2) + "\n", encoding="utf-8")
