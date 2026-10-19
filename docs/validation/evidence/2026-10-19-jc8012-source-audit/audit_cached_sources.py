"""Replay source comparisons and the syntax-only wrapper probe; never flash.

Input sources stay in the ignored cache. No vendor firmware is copied into
the repository, generated component or compiler input. See the source manifests.
"""
import argparse
import ctypes
import hashlib
import json
import os
from pathlib import Path
import re
import shlex
import subprocess

DONOR = '694ecd6a3f59caa99fd0811e11e0c29d77219411'


def digest(value):
    return hashlib.sha256(json.dumps(value, separators=(',', ':')).encode()).hexdigest()


def uncomment(text):
    return re.sub(r'/\*.*?\*/|//[^\n]*', '', text, flags=re.S)


def panel(text):
    number = r'(?:0[xX][0-9A-Fa-f]+|\d+)'
    pattern = (r'\{\s*('+number+r')\s*,\s*(?:\(\s*(?:const\s+)?uint8_t\s*'
               r'\[\s*\]\s*\)\s*\{([^}]*)\}|NULL)\s*,\s*('+number+r')\s*,\s*('
               +number+r')\s*\}')
    result = []
    for cmd, data, size, delay in re.findall(pattern, uncomment(text)):
        values = [int(v.strip(), 0) for v in data.split(',') if v.strip()] if data else []
        size = int(size, 0)
        if len(values) < size:
            raise ValueError('panel initializer has less data than declared')
        result.append([int(cmd, 0), values[:size], size, int(delay, 0)])
    return result


def firmware(text):
    body = re.search(r'GSLX680_FW\s*\[\s*\]\s*=\s*\{(.*?)\};', text, re.S).group(1)
    return [[int(a, 0), int(b, 0)] for a, b in re.findall(
        r'\{\s*(0x[\da-fA-F]+|\d+)\s*,\s*(0x[\da-fA-F]+|\d+)\s*\}', uncomment(body))]


def split_command(command):
    if os.name != 'nt':
        return shlex.split(command)
    count = ctypes.c_int()
    parse = ctypes.windll.shell32.CommandLineToArgvW
    parse.argtypes = [ctypes.c_wchar_p, ctypes.POINTER(ctypes.c_int)]
    parse.restype = ctypes.POINTER(ctypes.c_wchar_p)
    pointer = parse(command, ctypes.byref(count))
    if not pointer:
        raise ValueError('cannot parse compilation command')
    try:
        return [pointer[i] for i in range(count.value)]
    finally:
        ctypes.windll.kernel32.LocalFree(pointer)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--repo', type=Path, default=Path(__file__).resolve().parents[4])
    parser.add_argument('--cache', type=Path, required=True)
    args = parser.parse_args()
    repo, cache = args.repo.resolve(), args.cache.resolve()
    def donor(path):
        return subprocess.check_output(['git', '-C', str(repo), 'show', DONOR+':'+path]).decode()
    prefix = 'firmware/boards/jc8012p4a1/'
    public_panel = panel((cache/'hometiles-v2-panel.h').read_text())
    old_panel = panel((cache/'official-panel.c').read_text())
    factory_panel = panel(donor(prefix+'bsp_jc8012p4a1/jd9365_guition_init.inc'))
    published_fw = firmware((cache/'guition-published/private_include/gsl3680_firmware.h').read_text())
    donor_fw = firmware(donor(prefix+'esp_lcd_touch_gsl3680/include/esp_lcd_touch_gsl3680.h'))
    assert len(public_panel) == len(factory_panel) == 204 and len(old_panel) == 197
    assert public_panel == factory_panel and old_panel != factory_panel
    assert len(published_fw) == len(donor_fw) == 4356
    assert published_fw != donor_fw

    # Use the canonical base-touch compile command and its pinned IDF headers.
    build = repo/'firmware/main-deck-jc1060/build'
    commands = json.loads((build/'compile_commands.json').read_text())
    entry = next(c for c in commands if c['file'].replace('\\', '/').endswith('/esp_lcd_touch.c'))
    tokens = split_command(entry['command'])
    command, i = [], 0
    while i < len(tokens):
        token = tokens[i]
        if token in ('-o', '-MF', '-MT', '-MQ'):
            i += 2
            continue
        if token in ('-c', '-MD', '-MMD', '-MP') or token.replace('\\', '/').endswith('/esp_lcd_touch.c'):
            i += 1
            continue
        command.append(token)
        i += 1
    stub = cache/'compat_stub'
    stub.mkdir(exist_ok=True)
    (stub/'gsl3680_firmware.h').write_text('#pragma once\n#include <stdint.h>\n'
        'struct fw_data {char offset; uint32_t val;};\n'
        'static const struct fw_data GSLX680_FW[] = {{0, 0}};\n')
    command += ['-I'+str(stub), '-I'+str(cache/'guition-published/include'),
                '-fsyntax-only', '-H', str(cache/'guition-published/esp_lcd_touch_gsl3680.c')]
    run = subprocess.run(command, cwd=entry['directory'], capture_output=True, text=True)
    (cache/'replay-wrapper-syntax.log').write_text(run.stdout+run.stderr)
    assert run.returncode == 0, run.stderr
    assert 'compat_stub' in run.stderr and 'private_include' not in run.stderr
    print(json.dumps({'panel_entries': len(public_panel), 'panel_wire_sha256': digest(public_panel),
        'public_panel_equals_donor': True, 'official_old_entries': len(old_panel),
        'firmware_entries': len(published_fw), 'firmware_differing_pairs': sum(a != b for a,b in zip(published_fw, donor_fw)),
        'syntax_probe_exit': run.returncode, 'synthetic_firmware_only': True,
        'linked_image': False, 'hardware': 'NOT RUN'}, indent=2))


if __name__ == '__main__':
    main()
