"""Prepare a board-private PPA overlay without modifying ESP-IDF or shared core.

Only the pinned IDF6.0.2 seven-file component is accepted (LF normalized).
The donor differs in one SRM workaround; no vendor panel/touch data is read.
This preparation tool does not make JC8012 a runnable or qualified target.
"""
import argparse
import hashlib
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SPEC = ROOT / 'firmware/boards/jc8012p4a1/ppa'


def digest(text):
    return hashlib.sha256(text.encode('utf-8')).hexdigest()


def prepare(idf, output, board):
    if board != 'jc8012p4a1':
        raise ValueError('PPA overlay is restricted to jc8012p4a1')
    idf, output = Path(idf).resolve(), Path(output).resolve()
    # Never overwrite an existing IDF, tracked component, or stale build tree.
    if output.exists() or output == idf or idf in output.parents:
        raise ValueError('output must be a new directory outside ESP-IDF')
    pins = json.loads((SPEC / 'pins.json').read_text(encoding='utf-8'))
    source = idf / 'components/esp_driver_ppa'
    files = {}
    for name, expected in pins['idf_files'].items():
        data = (source / name).read_text(encoding='utf-8')
        if digest(data) != expected:
            raise ValueError(f'IDF6.0.2 PPA source drift: {name}')
        files[name] = data
    before = (SPEC / 'workaround.before').read_text(encoding='utf-8')
    after = (SPEC / 'workaround.after').read_text(encoding='utf-8')
    if files['src/ppa_srm.c'].count(before) != 1:
        raise ValueError('SRM workaround anchor missing or duplicated')
    files['src/ppa_srm.c'] = files['src/ppa_srm.c'].replace(before, after)
    if digest(files['src/ppa_srm.c']) != pins['donor_srm_sha256']:
        raise ValueError('patched SRM differs from frozen donor')
    guard = ('if(NOT CONFIG_PAJONIIIR_BOARD_JC8012P4A1)\n'
             '    message(FATAL_ERROR "JC8012 private PPA selected by another board")\n'
             'endif()\n')
    files['CMakeLists.txt'] = guard + files['CMakeLists.txt']
    # Preserve the original licence and source copyright notices.
    files['LICENSE'] = (idf / 'LICENSE').read_text(encoding='utf-8')
    output.mkdir(parents=True)
    for name, data in files.items():
        path = output / name
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(data, encoding='utf-8', newline='\n')
    manifest = {'board': board, 'donor_sha': pins['donor_sha'],
                'files_sha256': {name: digest(data) for name, data in files.items()}}
    (output / 'provenance.json').write_text(json.dumps(manifest, indent=2) + '\n',
                                           encoding='utf-8')
    return manifest


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--idf', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--board', required=True)
    args = parser.parse_args()
    print(json.dumps(prepare(args.idf, args.output, args.board), indent=2))
