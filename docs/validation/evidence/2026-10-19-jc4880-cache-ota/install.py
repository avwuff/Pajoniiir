"""One marked signed local upload; never retries an ambiguous mutation."""
import hashlib,json,sys,time,urllib.request
from datetime import datetime,timezone
from pathlib import Path
ROOT=Path.cwd(); OUT=Path(__file__).resolve().parent/'ota'; OUT.mkdir(exist_ok=True)
PACKAGE=ROOT/'releases/jc4880-cache-bound-0a33ec1f-20261019/pajoniiir-M2.5-249-g0a33ec1f'
EXPECTED='0a33ec1fb16c0136dccaa85ca63a6fbc00a4d527'
op=urllib.request.build_opener(urllib.request.ProxyHandler({}))
def get(ep):
    with op.open('http://192.168.4.1/api/'+ep,timeout=5) as r: return json.load(r)
def save(name,data): (OUT/name).write_text(json.dumps(data,indent=2)+'\n',encoding='utf-8')
def stopped(s): return all(s[f'deck{d}']['playing'] is False and s[f'deck{d}']['state_text'] in ('IDLE','READY') for d in (1,2))
mode=sys.argv[1] if len(sys.argv)>1 else 'preflight'
result={'complete':False,'stage':'preflight','source_sha':EXPECTED,'upload_attempts':0}
try:
    fw=get('firmware'); s=get('status')
    assert fw['board_id']=='jc4880' and fw['project']=='main-deck-p4' and s['board_id']=='jc4880'
    assert fw['state']=='idle' and fw['running_image_state']=='valid' and not fw['source_dirty']
    assert fw['source_sha']==s['source_sha'] and not s['source_dirty'] and stopped(s)
    if mode=='preflight':
        save('preflight-firmware.json',fw);save('preflight-status.json',s)
        print(json.dumps({'firmware':fw,'controller':s['controller'],'mixer':s['mixer'],'uac_format':s['uac_format'],'decks_stopped':True},indent=2));sys.exit(0)
    assert mode=='upload'
    assert not (OUT/'upload-started.json').exists(),'Upload already attempted; inspect device instead of retrying'
    old=json.loads((OUT/'preflight-firmware.json').read_text())
    assert fw['source_sha']==old['source_sha'] and fw['running_slot']==old['running_slot']
    save('before-firmware.json',fw);save('before-status.json',s)
    manifest=json.loads((PACKAGE/'manifest.json').read_text(encoding='utf-8-sig')); target,=manifest['targets']
    assert manifest['source_sha']==EXPECTED and target['project']=='main-deck-p4'
    data=(PACKAGE/target['ota_bundle']).read_bytes()
    assert len(data)==target['bundle_size'] and hashlib.sha256(data).hexdigest()==target['bundle_sha256']
    assert hashlib.sha256((PACKAGE/target['file']).read_bytes()).hexdigest()==target['sha256']
    expected_slot={'ota_0':'ota_1','ota_1':'ota_0'}[fw['running_slot']]
    result.update(stage='upload_started',upload_attempts=1,old_source=fw['source_sha'],old_slot=fw['running_slot'],expected_slot=expected_slot,
                  bundle_sha256=target['bundle_sha256'],image_elf_sha256=target['image_elf_sha256'],started_utc=datetime.now(timezone.utc).isoformat())
    save('upload-started.json',result);save('result.json',result)
    request=urllib.request.Request('http://192.168.4.1/api/ota/p4',data=data,method='POST',headers={'Content-Type':'application/octet-stream','X-DDJ-Control':'1','X-DDJ-OTA':'p4'})
    start=time.monotonic()
    with op.open(request,timeout=100) as response:
        raw=response.read(); (OUT/'upload-response.json').write_bytes(raw)
        result.update(upload_http=response.status,upload_elapsed_s=time.monotonic()-start)
        assert response.status==200 and json.loads(raw)['ok'] is True
    result['stage']='waiting_startup';save('result.json',result)
    print('Upload HTTP200; awaiting exact JC4880 startup. No retry upload.',flush=True)
    deadline=time.monotonic()+70
    while time.monotonic()<deadline:
        row={'utc':datetime.now(timezone.utc).isoformat()}
        try:
            fw=get('firmware');row['firmware']=fw
            if fw['source_sha']==EXPECTED:
                assert fw['board_id']=='jc4880' and fw['project']=='main-deck-p4' and not fw['source_dirty']
                assert fw['image_elf_sha256']==target['image_elf_sha256'] and fw['running_slot']==expected_slot
                if fw['running_image_state']=='valid' and fw['state']=='idle':
                    s=get('status');save('after-firmware.json',fw);save('after-status.json',s)
                    assert stopped(s) and s['source_sha']==EXPECTED and not s['source_dirty']
                    result.update(complete=True,stage='valid_stopped',final_firmware=fw)
                    break
        except (OSError,ValueError) as e:row['error']=str(e)
        with (OUT/'startup.jsonl').open('a',encoding='utf-8') as f:f.write(json.dumps(row)+'\n')
        time.sleep(3)
    if not result['complete']:raise RuntimeError('Upload finished; startup not confirmed. Reconnect host network and inspect, never retry upload automatically.')
except Exception as e:result['error']=str(e)
finally:
    if mode!='preflight':save('result.json',result);print(json.dumps(result,indent=2),flush=True)
sys.exit(0 if result['complete'] else 1)
