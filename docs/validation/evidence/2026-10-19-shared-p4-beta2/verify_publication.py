import argparse,hashlib,json,subprocess
from pathlib import Path
HERE=Path(__file__).resolve().parent
parser=argparse.ArgumentParser();parser.add_argument('--draft',action='store_true');args=parser.parse_args()
def read(name):return json.loads((HERE/name).read_text(encoding='utf-8-sig'))
release=read('draft-release.json' if args.draft else 'published-release.json')
expected=read('final-assets.json')
assert release['id']==417044096 and release['tag_name']=='shared-p4-beta.2' and release['prerelease'] is True
assert release['draft']==args.draft
remote={r['name']:r for r in release['assets']};assert set(remote)=={r['file'] for r in expected}
for r in expected:
 actual=remote[r['file']];assert actual['size']==r['size'] and actual['digest']=='sha256:'+r['sha256'] and actual['state']=='uploaded',r['file']
resolved=subprocess.check_output(['git','rev-parse','shared-p4-beta.2^{}'],text=True).strip()
assert resolved=='d259a71ae716217a6448c374bf52c2c798d67ce8'
if not args.draft:
 before,after=read('stable-release-before.json'),read('stable-release-after.json')
 def view(r):return (r['id'],r['tag_name'],r['draft'],r['prerelease'],sorted((a['id'],a['name'],a['size'],a['digest']) for a in r['assets']))
 assert view(before)==view(after) and after['tag_name']=='M2.5','Stable latest changed'
 assert view(read('beta1-before.json'))==view(read('beta1-after.json')),'Beta1 assets changed'
 assert (HERE/'stable-ota-before.json').read_bytes()==(HERE/'stable-ota-after.json').read_bytes(),'Public OTA changed'
 print('PASS published prerelease16 assets, immutable tag, stable M2.5/beta1 asset identities and public OTA bytes unchanged')
else:
 assert read('downloaded-draft-audit.json')['complete']
 assert 'passed' in (HERE/'documentation-prepublication.log').read_text()
 (HERE/'publish-request.json').write_text(json.dumps({'tag_name':'shared-p4-beta.2','target_commitish':resolved,'draft':False,'prerelease':True,'make_latest':'false'})+'\n',encoding='utf-8')
 print('PASS draft metadata/server hashes/downloaded signatures/docs; prepared publication request for exact release ID')
