import hashlib,json,time,urllib.request
from pathlib import Path
root=Path.cwd(); out=Path(__file__).resolve().parent/'ota'; out.mkdir(exist_ok=True)
op=urllib.request.build_opener(urllib.request.ProxyHandler({}))
def read(ep):
    with op.open('http://192.168.4.1/api/'+ep,timeout=8) as r: return json.load(r)
def save(n,v): (out/n).write_text(json.dumps(v,indent=2)+'\n',encoding='utf-8')
status=read('status'); fw=read('firmware'); save('before-status.json',status);save('before-firmware.json',fw)
assert status['board_id']=='m3' and fw['project']=='main-deck-m3'
assert status['source_sha']=='2fa2263142ed46a5a86a0a7cccdef6330134d526'
assert all(not status[f'deck{d}']['playing'] for d in (1,2))
bundle=root/'releases/m3-mix-dma-f1c5ec9c-20261018/pajoniiir-m3-M3-dev-gf1c5ec9ca9e9/main-deck-m3.ddjota'
data=bundle.read_bytes(); save('upload-input.json',{'path':str(bundle),'bytes':len(data),'sha256':hashlib.sha256(data).hexdigest()})
request=urllib.request.Request('http://192.168.4.1/api/ota/p4',data=data,method='POST',headers={'Content-Type':'application/octet-stream','X-DDJ-Control':'1','X-DDJ-OTA':'p4'})
t=time.monotonic()
with op.open(request,timeout=100) as r:
    raw=r.read(); (out/'upload-response.json').write_bytes(raw)
    save('upload-result.json',{'http_status':r.status,'elapsed_s':time.monotonic()-t,'headers':dict(r.headers)})
    assert r.status==200 and json.loads(raw).get('ok') is True
print('OTA HTTP 200; elapsed_s',round(time.monotonic()-t,3))
