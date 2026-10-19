"""Download final public assets without GitHub authentication; compare sealed bytes."""
import concurrent.futures,hashlib,json,subprocess
from pathlib import Path
HERE=Path(__file__).resolve().parent;OUT=HERE/'downloaded-public';OUT.mkdir(exist_ok=False)
release=json.loads((HERE/'published-release.json').read_text(encoding='utf-8-sig'))
expected={r['file']:r for r in json.loads((HERE/'final-assets.json').read_text())}
def fetch(asset):
 name=asset['name'];record=expected[name];url=asset['browser_download_url']
 assert url.startswith('https://github.com/dvucinozd/Pajoniiir/releases/download/shared-p4-beta.2/')
 target=OUT/name
 subprocess.run(['curl.exe','--fail','--location','--silent','--show-error','--max-time','60','--retry','1','--output',str(target),url],check=True)
 data=target.read_bytes();digest=hashlib.sha256(data).hexdigest()
 assert len(data)==record['size'] and digest==record['sha256'],name
 return {'file':name,'bytes':len(data),'sha256':digest,'url':url,'authenticated':False}
with concurrent.futures.ThreadPoolExecutor(max_workers=3) as pool:records=list(pool.map(fetch,release['assets']))
(HERE/'public-download-audit.json').write_text(json.dumps({'complete':True,'assets':records},indent=2)+'\n',encoding='utf-8')
print('PASS',len(records),'unauthenticated public downloads exactly match sealed originals')
