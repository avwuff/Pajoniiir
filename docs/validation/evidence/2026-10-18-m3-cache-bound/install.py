import hashlib,json,time,urllib.request
from pathlib import Path
root=Path.cwd(); out=Path(__file__).resolve().parent/'ota'; out.mkdir(exist_ok=True)
op=urllib.request.build_opener(urllib.request.ProxyHandler({}))
def read(ep):
    with op.open('http://192.168.4.1/api/'+ep,timeout=8) as r: return json.load(r)
def save(n,v): (out/n).write_text(json.dumps(v,indent=2)+'\n',encoding='utf-8')
status=read('status'); fw=read('firmware'); save('before-status.json',status);save('before-firmware.json',fw)
assert status['board_id']=='m3' and fw['project']=='main-deck-m3'
assert status['source_sha']=='f1c5ec9ca9e9e3eba89365384762efb95e426ff7'
assert not status['source_dirty'] and fw['running_image_state']=='valid'
assert all(not status[f'deck{d}']['playing'] for d in (1,2))
bundle=root/'releases/m3-cache-bound-1556e8d7-20261018/pajoniiir-m3-M3-dev-g1556e8d7dfcb/main-deck-m3.ddjota'
data=bundle.read_bytes(); save('upload-input.json',{'path':str(bundle),'bytes':len(data),'sha256':hashlib.sha256(data).hexdigest()})
assert hashlib.sha256(data).hexdigest()=='e8362bb55d66118e5ccc5b0aa4eaa818d3d371bbc00f6d3f5f0001a5842c9505'
request=urllib.request.Request('http://192.168.4.1/api/ota/p4',data=data,method='POST',headers={'Content-Type':'application/octet-stream','X-DDJ-Control':'1','X-DDJ-OTA':'p4'})
t=time.monotonic()
with op.open(request,timeout=100) as r:
    raw=r.read(); (out/'upload-response.json').write_bytes(raw)
    save('upload-result.json',{'http_status':r.status,'elapsed_s':time.monotonic()-t,'headers':dict(r.headers)})
    assert r.status==200 and json.loads(raw).get('ok') is True
print('OTA HTTP 200; elapsed_s',round(time.monotonic()-t,3))
