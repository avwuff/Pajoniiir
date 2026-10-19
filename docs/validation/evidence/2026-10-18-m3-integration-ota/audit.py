"""Offline independent audit of the completed ordinary M3 smoke (no device I/O)."""
import ast
import hashlib
import json
from pathlib import Path
import shutil

ROOT = Path(__file__).resolve().parent
REPO = ROOT.parents[1]
RUN = ROOT / 'smoke-20261018T152620Z'
DEST = REPO / 'docs/validation/evidence/2026-10-18-m3-integration-ota'
SOURCE = 'c0ee81527b4dbd5d83599140aaa106ebcd27772a'
ELF = '282250f0665266e2fb4aa8a348dbf5c0e619401d75df5c686f229fb652267340'
PACKAGE = ROOT / 'package/pajoniiir-m3-M3-dev-gc0ee81527b4d'

def read(path):
    return json.loads(path.read_text(encoding='utf-8-sig'))

def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()

# Extract only the pure counter function; never import/execute the monitor.
tree = ast.parse((ROOT / 'smoke.py').read_text())
node = next(n for n in tree.body if isinstance(n, ast.FunctionDef) and n.name == 'counters')
space = {}
exec(compile(ast.Module(body=[node], type_ignores=[]), '<pure counters>', 'exec'), space)
counters = space['counters']
baseline = read(RUN / 'baseline.json')
final = read(RUN / 'final-stopped.json')
result = read(RUN / 'result.json')
install = read(ROOT / 'result.json')
first = counters(baseline)
rows = [json.loads(line) for line in (RUN / 'status.jsonl').read_text().splitlines()]
assert len(rows) == result['samples'] == 101
assert result['complete'] and result['stopped'] and result['duration_s'] >= 60
previous_elapsed = -1
previous_uptime = baseline['uptime_ms']
for row in rows:
    status = row['status']
    assert row['elapsed_s'] > previous_elapsed
    assert status['uptime_ms'] >= previous_uptime
    previous_elapsed, previous_uptime = row['elapsed_s'], status['uptime_ms']
    assert status['source_sha'] == SOURCE and status['source_dirty'] is False
    assert status['board_id'] == 'm3' and status['project'] == 'main-deck-m3'
    assert counters(status) == first
    assert status['controller']['present'] and status['controller']['midi_in'] and status['controller']['midi_out']
    assert status['controller']['active_profile'] == 'pioneer_ddj_flx4'
    uac = status['uac_format']
    assert uac['streaming'] and not uac['faulted'] and not uac['high_speed']
    assert (uac['sample_rate'], uac['bits'], uac['subslot_bytes']) == (44100, 16, 2)
    assert status['main_sink'] == 'pcm5102a'
    assert status['diagnostics']['output_sample_rate'] == 48000
    assert not status['diagnostics']['usb_headphones']['data_loss']
    assert status['capabilities']['storage_root'] == 0 and status['capabilities']['controller_root'] == 1
    assert status['p4_usb']['storage']['mounted']
    for deck in ('deck1', 'deck2'):
        assert status[deck]['title'] == baseline[deck]['title']
        assert not status[deck]['master_tempo'] and status[deck]['pitch_percent'] == 0
assert counters(final) == first
assert not final['deck1']['playing'] and not final['deck2']['playing']
assert not final['deck1']['loop_active'] and not final['deck2']['loop_active']
assert len(result['strict_deltas']) == 22 and set(result['strict_deltas'].values()) == {0}
assert final['p4_usb']['controller']['midi_packets'] - baseline['p4_usb']['controller']['midi_packets'] == result['midi_packets_delta'] == 2100
assert final['diagnostics']['output_late_count'] - baseline['diagnostics']['output_late_count'] == result['output_late_delta'] == 0
assert len(result['actions']) == 6
fw = read(RUN / 'final-firmware.json')
assert fw['source_sha'] == SOURCE and not fw['source_dirty']
assert fw['image_elf_sha256'] == ELF and fw['running_slot'] == 'ota_0' and fw['running_image_state'] == 'valid'
assert install['complete'] and install['upload_http'] == 200 and install['upload_attempts'] == 1
assert install['crash_before'] == install['crash_after'] == final['crash_dump']
for name, size, sha in (
    ('main-deck-m3.bin', 2603200, install['image_sha256']),
    ('main-deck-m3.ddjota', 2603388, install['bundle_sha256']),
):
    assert (PACKAGE / name).stat().st_size == size and digest(PACKAGE / name) == sha

DEST.mkdir(parents=True, exist_ok=True)
(DEST / '.gitattributes').write_text('* -text whitespace=blank-at-eol,blank-at-eof,space-before-tab,cr-at-eol\n*.sig binary\n')
shutil.copyfile(ROOT / 'result.json', DEST / 'installation-result.json')
shutil.copyfile(ROOT / 'after-library.json', DEST / 'installed-library.json')
for name in ('smoke.py', 'audit.py'):
    shutil.copyfile(ROOT / name, DEST / name)
for path in RUN.iterdir():
    if path.suffix in ('.json', '.jsonl'):
        shutil.copyfile(path, DEST / path.name)
for name in ('manifest.json', 'manifest.sig'):
    shutil.copyfile(PACKAGE / name, DEST / name)
audit = {
    'offline_audit': 'pass', 'samples': len(rows), 'strict_paths': len(first),
    'source_sha': SOURCE, 'image_elf_sha256': ELF,
    'duration_s': result['duration_s'], 'midi_packets_delta': result['midi_packets_delta'],
    'output_late_delta': result['output_late_delta'], 'final_stopped': True,
    'all_samples_same_identity_and_healthy_uac': True,
    'pitch_percent': [0, 0], 'master_tempo': [False, False],
    'historical_crash_summary_unchanged': True,
    'saved_historical_crash_dump_sha256': digest(ROOT / 'preflight/previous-crash-dump.bin'),
    'operator_confirmation': 'recorded separately when received',
    'scope': 'Short ordinary M3/FLX4 mixed-rate smoke; not a soak or full hardware qualification',
}
(DEST / 'audit.json').write_text(json.dumps(audit, indent=2) + '\n')
hashes = {p.name: digest(p) for p in sorted(DEST.iterdir()) if p.is_file() and p.name != 'sha256.json'}
(DEST / 'sha256.json').write_text(json.dumps(hashes, indent=2) + '\n')
print(json.dumps(audit, indent=2))
