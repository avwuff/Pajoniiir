"""Execute the production status memory formatter for ordinary and HS builds."""
from pathlib import Path
import json
import os
import shutil
import subprocess

root = Path(__file__).resolve().parents[2]
source = (root / "firmware/p4-core/components/web_server/web_server.c").read_text(encoding="utf-8")
start = source.index('        "\\"heap_free\\":%u,"')
end = source.index('        "}"', start)
format_fragment = source[start:end]
args_start = source.index("        (unsigned)diagnostics.heap_free,", end)
args_end = source.index(";", args_start)
args = source[args_start:args_end].rstrip()
assert args.endswith(")")
args = args[:-1]
build = Path(__file__).parent / "build/status-json"
build.mkdir(parents=True, exist_ok=True)
gcc = shutil.which("gcc")
if not gcc:
    raise RuntimeError("gcc required")
fields = ("heap_free", "internal_free", "internal_min_free", "internal_largest_free",
          "dma_free", "dma_min_free", "dma_largest_free", "psram_free", "psram_min_free",
          "psram_largest_free")
code = '#include <stdio.h>\nstruct {\n'
code += "".join("unsigned " + name + ";\n" for name in fields)
code += 'unsigned heap_walk_max_us[3];\n} diagnostics;\nint main(void) {\n'
code += "".join(f"diagnostics.{name} = {i + 1}u;\n" for i, name in enumerate(fields))
code += 'diagnostics.heap_walk_max_us[0]=999;\n'
code += 'printf("{"\n' + format_fragment + ' "}",\n' + args + ');\n}\n'
c = build / "status.c"
c.write_text(code, encoding="utf-8")
for hs in (0, 1):
    exe = build / (f"status-{hs}.exe" if os.name == "nt" else f"status-{hs}")
    subprocess.run([gcc, "-std=c11", "-Wall", "-Wextra", "-Werror",
                    f"-DCONFIG_CONTROLLER_USB_HS_AUDIO_DIAGNOSTIC={hs}", str(c), "-o", str(exe)], check=True)
    result = json.loads(subprocess.check_output([str(exe)]))
    for i, name in enumerate(fields):
        assert result[name] == (None if "largest" in name else i + 1), (hs, name, result)
    assert result["heap_walk_max_us"] == [None, None, None]
print("PASS production status memory JSON: correct counters and explicit unavailable values on FS/HS")
