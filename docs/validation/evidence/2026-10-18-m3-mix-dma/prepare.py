from pathlib import Path
import json
import time
import urllib.request

out=Path(__file__).resolve().parent
op=urllib.request.build_opener(urllib.request.ProxyHandler({}))
source='f1c5ec9ca9e9e3eba89365384762efb95e426ff7'
def read(endpoint='status'):
    with op.open('http://192.168.4.1/api/'+endpoint,timeout=8) as response:return json.load(response)
def post(endpoint):
    request=urllib.request.Request('http://192.168.4.1/api/'+endpoint,data=b'',method='POST',headers={'X-DDJ-Control':'1'})
    with op.open(request,timeout=12) as response:assert response.status==200
def stopped(status):return all(status[f'deck{d}']['playing'] is False for d in (1,2))
status=read();assert status['source_sha']==source and stopped(status)
assert status['controller']['active_profile']=='pioneer_ddj_flx4'
assert status['uac_format']['streaming'] and not status['uac_format']['high_speed'] and not status['uac_format']['faulted']
library=read('library');(out/'smoke-library.json').write_text(json.dumps(library,indent=2),encoding='utf-8')
for deck,title in ((1,'Casanova Action {320}'),(2,'Dino Dvornik - Ritam moj')):
    matches=[track for track in library['tracks'] if track['title']==title];assert len(matches)==1
    status=read()
    rate=44100 if deck==1 else 48000
    if status[f"deck{deck}"]["title"]!=title or status[f"deck{deck}"]["state_text"]!="READY" or status["diagnostics"][f"deck_sample_rate{deck}"]!=rate:
        post(f"load?deck={deck}&track_key={matches[0]['track_key']}&generation={library['generation']}")
    until=time.monotonic()+30
    while time.monotonic()<until:
        status=read();assert stopped(status)
        if status[f'deck{deck}']['title']==title and status[f'deck{deck}']['state_text']=='READY' and status['diagnostics'][f'deck_sample_rate{deck}']==rate:break
        time.sleep(.5)
    else:raise RuntimeError('Load did not reach READY: '+title)
    post(f'control?deck={deck}&action=seek&value=15000')
for deck,value in ((1,4096),(2,12288)):
    post(f'control?deck={deck}&action=pitch&value={value}')
time.sleep(.2)
status=read();assert stopped(status)
assert status['diagnostics']['deck_sample_rate1']==44100 and status['diagnostics']['deck_sample_rate2']==48000
(out/'smoke-prepared.json').write_text(json.dumps(status,indent=2),encoding='utf-8')
print(json.dumps({'source':source,'tracks':len(library['tracks']),'deck1':status['deck1'],
    'deck2':status['deck2'],'uac':status['uac_format'],'stopped':True},indent=2))
