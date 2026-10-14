import hashlib
import json
import subprocess
from pathlib import Path

root = next(p for p in Path(__file__).resolve().parents if (p/'firmware/p4-core').is_dir())
records = root / '.cache/jc1060-review-20261014'
records.mkdir(parents=True, exist_ok=True)
tip = '00041a0dd1c704781af4d991b663c05cdb6bd4c3'
local = '78dc3b7d07966579b189f73700f9bff89c888d6b'
base = subprocess.check_output(['git', 'merge-base', local, tip], cwd=root, text=True).strip()
paths = subprocess.check_output(['git', 'diff', '--name-only', base, tip], cwd=root, text=True).splitlines()
hashes = {}
for name in paths:
    data = subprocess.check_output(['git', 'show', f'{tip}:{name}'], cwd=root)
    destination = records / 'contributor' / name
    destination.parent.mkdir(parents=True, exist_ok=True)
    destination.write_bytes(data)
    hashes[name] = {'bytes': len(data), 'sha256': hashlib.sha256(data).hexdigest()}
evidence = records / 'contributor/docs/validation/jc1060/evidence'
summaries = {}
for path in evidence.rglob('*result.json'):
    content = json.loads(path.read_text())
    summaries[str(path.relative_to(evidence))] = content
manifest = {'contributor_tip': tip, 'local_review_head': local, 'merge_base': base,
            'changed_files': len(paths), 'files': hashes}
(records / 'provenance.json').write_text(json.dumps(manifest, indent=2))
(records / 'result-summaries.json').write_text(json.dumps(summaries, indent=2))
print(json.dumps({'tip': tip, 'local': local, 'base': base, 'changed_files': len(paths),
                  'result_files': list(summaries)}, indent=2))
for name, summary in summaries.items():
    print(name, json.dumps(summary)[:3500])
