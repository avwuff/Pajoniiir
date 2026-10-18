"""One 30-second D1 headphone comparison; always stop any started deck."""
import ast
import json
from pathlib import Path
import time
import urllib.request

ROOT = Path(__file__).resolve().parent
OUT = ROOT / 'listening-test'
OUT.mkdir(exist_ok=False)
op = urllib.request.build_opener(urllib.request.ProxyHandler({}))
tree = ast.parse((ROOT.parent / 'm3-ota-c0ee8152-20261018/smoke.py').read_text())
node = next(n for n in tree.body if isinstance(n, ast.FunctionDef) and n.name == 'counters')
scope = {}
exec(compile(ast.Module(body=[node], type_ignores=[]), '<pure counters>', 'exec'), scope)
counters = scope['counters']

def get(ep='status'):
    with op.open('http://192.168.4.1/api/' + ep, timeout=5) as r:
        return json.load(r)

def post(deck):
    q = urllib.request.Request(f'http://192.168.4.1/api/control?deck={deck}&action=play_pause', data=b'', method='POST', headers={'X-DDJ-Control': '1'})
    with op.open(q, timeout=6) as r:
        assert r.status == 200

def save(name, data):
    (OUT / name).write_text(json.dumps(data, indent=2) + '\n')

b = get()
assert b['source_sha'] == '605bcd20d6dadf250ce1816d82353ee2ace21867'
assert not b['deck1']['playing'] and not b['deck2']['playing']
assert b['mixer']['headphone_level'] == 0 and b['mixer']['headphone_mix'] == 0 and b['mixer']['pfl1']
initial = counters(b)
save('baseline.json', b)
result = {'complete': False, 'stopped': False, 'errors': [], 'operator_listening': 'pending', 'scope': '30-s solo D1 headphone LEVEL comparison, no soak'}
started = time.monotonic()
samples = []
try:
    post(1)
    started = time.monotonic()
    with (OUT / 'status.jsonl').open('x') as f:
        while time.monotonic() - started < 30:
            s = get()
            assert s['source_sha'] == b['source_sha'] and s['uptime_ms'] >= b['uptime_ms']
            assert s['controller']['present'] and s['uac_format']['streaming'] and not s['uac_format']['faulted']
            assert counters(s) == initial and not s['diagnostics']['usb_headphones']['data_loss']
            assert s['mixer']['headphone_mix'] == 0 and s['mixer']['pfl1']
            assert not s['deck2']['playing']
            samples.append(s)
            f.write(json.dumps({'elapsed_s': time.monotonic() - started, 'status': s}) + '\n')
            f.flush()
            time.sleep(.5)
    result['complete'] = True
except Exception as e:
    result['errors'].append(str(e))
finally:
    result['duration_s'] = time.monotonic() - started
    for attempt in range(3):
        try:
            s = get()
            for d in (1, 2):
                if s['deck' + str(d)]['playing']:
                    post(d)
            time.sleep(.15)
            s = get()
            result['stopped'] = not s['deck1']['playing'] and not s['deck2']['playing']
            save('final-stopped.json', s)
            if result['stopped']:
                break
        except Exception as e:
            result['errors'].append('Stop: ' + str(e))
    result['strict_deltas'] = {k: counters(s)[k] - v for k, v in initial.items()}
    result['samples'] = len(samples)
    result['level_raw_min_max'] = [min(x['mixer']['headphone_level'] for x in samples), max(x['mixer']['headphone_level'] for x in samples)] if samples else None
    result['midi_packets_delta'] = s['p4_usb']['controller']['midi_packets'] - b['p4_usb']['controller']['midi_packets']
    result['output_late_delta'] = s['diagnostics']['output_late_count'] - b['diagnostics']['output_late_count']
    result['telemetry_pass'] = result['complete'] and result['stopped'] and not result['errors'] and all(v == 0 for v in result['strict_deltas'].values())
    save('result.json', result)
    save('final-firmware.json', get('firmware'))
    print(json.dumps(result, indent=2), flush=True)
raise SystemExit(0 if result['telemetry_pass'] else 1)
