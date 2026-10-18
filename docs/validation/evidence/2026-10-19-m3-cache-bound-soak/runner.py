import json, time, urllib.request, sys, os
from pathlib import Path
from datetime import datetime, timezone
from s2_soak_late_policy import LateObservation

EXPECTED = '1556e8d7dfcb4f016c52ab1a372dd2e076dcaa1f'
duration = int(sys.argv[1]) if len(sys.argv)>1 else 60
assert duration == 3600
op = urllib.request.build_opener(urllib.request.ProxyHandler({}))
base = 'http://192.168.4.1/api/'
out = Path(os.environ['M3_CACHE_SOAK_OUTPUT'])
out.mkdir(exist_ok=True)
def read(name='status'):
    with op.open(base + name, timeout=5) as r: return json.load(r)
def post(deck, action, value=None):
    url = base + f'control?deck={deck}&action={action}'
    if value is not None: url += f'&value={value}'
    req = urllib.request.Request(url, data=b'', method='POST', headers={'X-DDJ-Control':'1'})
    with op.open(req, timeout=5) as r: assert r.status == 200
def save(name, value): (out/name).write_text(json.dumps(value, indent=2), encoding='utf-8')
def counters(s):
    found = {}
    names = {'pcm_underrun1','pcm_underrun2','packet_failures','packet_lost_frames','underflow_frames','overflow_frames','dropped_blocks','feedback_invalid','midi_disconnects','interface_claim_failures','transfer_alloc_failures','midi_in_pipe_errors','midi_in_submit_failures','midi_out_submit_failures','queue_failures','recovery_requests','recovery_failures','recovery_queue_drops','probe_event_drops','disconnect_events','dropped'}
    def visit(v, prefix=''):
        if isinstance(v, dict):
            for k, val in v.items():
                path = prefix + '.' + k if prefix else k
                if k in names:
                    assert type(val) is int, 'Missing numeric counter ' + path
                    found[path] = val
                if isinstance(val,dict): visit(val,path)
    visit(s)
    return found
s = read(); fw = read('firmware'); ui = read('ui-timing')
save('firmware.json',fw); save('initial-ui-timing.json',ui)
assert fw['source_sha'] == EXPECTED and not fw['source_dirty'] and fw['running_image_state']=='valid'
assert fw['running_slot']=='ota_1'
assert fw['board_id']=='m3' and fw['project']=='main-deck-m3' and fw['image_elf_sha256']=='4999176a6b7b1fe9c0f48e7c9687a830b45873d8752afc9a8e1a61d8e7992d93'
assert s['uac_format']['sample_rate']==44100, 'Preserve 44100-Hz qualification scope'
assert not s['uac_format']['high_speed'] and s['uac_format']['bits']==16 and s['uac_format']['subslot_bytes']==2
assert s['source_sha']==EXPECTED and not s['source_dirty']
save('initial-usb-bus-trace.json',read('usb-bus-trace'))
assert s['controller']['present'] and s['controller']['active_profile']=='pioneer_ddj_flx4' and s['uac_format']['streaming'] and not s['uac_format']['faulted']
assert s['uac_format']['sample_rate'] in (44100,48000) and s['diagnostics']['output_sample_rate']==48000
assert s['diagnostics']['deck_sample_rate1']==44100 and s['diagnostics']['deck_sample_rate2']==48000
assert ui['zoom_beats'] in (4,8,12,16,24)
assert ui['zoom_beats']==8
for d, pitch in ((1,4096),(2,12288)):
    deck=s[f'deck{d}']
    assert not deck['playing'] and deck['state_text']=='READY' and deck['master_tempo'] and deck['loop_active'] and deck['raw_pitch']==pitch
    post(d,'seek',deck['loop_start_ms'])
time.sleep(.5)
s = read(); baseline=s; save('baseline.json',s); initial=counters(s)
assert len(initial)==22, 'Strict counter schema changed'
late=LateObservation(s['diagnostics']['output_late_count'],s['diagnostics']['output_late_max_us'])
result={'complete':False,'duration_s':0,'samples':0,'errors':[],'stopped':False,'classification':'incomplete','uac_sample_rate':s['uac_format']['sample_rate'],'started_utc':datetime.now(timezone.utc).isoformat()}
begin=time.monotonic()
try:
    for d in (1,2):
        if not s['mixer'][f'pfl{d}']: post(d,'pfl')
    for d in (1,2): post(d,'play_pause')
    s=read();save('initial-playing.json',s)
    assert all(s[f'deck{d}']['playing'] for d in (1,2)), 'Both decks must play before measuring the full hour'
    assert counters(s)==initial, 'Strict startup counter changed'
    begin=time.monotonic()
    print(f"START: {duration} s dual MT/loop, +5/-5%, zoom {ui['zoom_beats']}; observe audio/display, leave controls unchanged",flush=True)
    with (out/'status.jsonl').open('x',encoding='utf-8') as f:
        while time.monotonic()-begin < duration:
            s=read(); elapsed=time.monotonic()-begin
            f.write(json.dumps({'elapsed_s':elapsed,'status':s})+'\n'); f.flush(); result['samples']+=1
            assert s['source_sha']==EXPECTED and not s['source_dirty'] and s['uptime_ms']>=baseline['uptime_ms'], 'Image/reboot changed'
            assert all(s[f'deck{d}']['title']==baseline[f'deck{d}']['title'] for d in (1,2)), 'Loaded track changed'
            assert s['controller']['present'] and s['controller']['active_profile']=='pioneer_ddj_flx4' and s['uac_format']['streaming'] and not s['uac_format']['faulted'], 'Controller/audio fault'
            assert s['uac_format']['sample_rate']==baseline['uac_format']['sample_rate'], 'UAC rate changed'
            assert not s['uac_format']['high_speed'] and s['uac_format']['bits']==16 and s['uac_format']['subslot_bytes']==2, 'UAC format changed'
            assert s['diagnostics']['output_sample_rate']==48000 and s['diagnostics']['deck_sample_rate1']==44100 and s['diagnostics']['deck_sample_rate2']==48000,'Sample rates changed'
            now=counters(s)
            assert now.keys()==initial.keys(), 'Counter schema changed'
            for k,v in initial.items(): assert now[k]==v, 'New strict counter: '+k+' delta='+str(now[k]-v)
            late.update(s['diagnostics']['output_late_count'],s['diagnostics']['output_late_max_us'],elapsed)
            assert all(s[f'deck{d}']['playing'] for d in (1,2)), 'Playback stopped early'
            assert all(s[f'deck{d}']['master_tempo'] and s[f'deck{d}']['loop_active'] and s[f'deck{d}']['raw_pitch']==baseline[f'deck{d}']['raw_pitch'] for d in (1,2)), 'Workload changed'
            time.sleep(.5)
    result['complete']=True; result['classification']='telemetry_pass'
except Exception as e:
    result['errors'].append(str(e)); result['classification']='workload_changed' if str(e)=='Workload changed' else 'failed_or_incomplete'
    print('INTERRUPTED: '+str(e),flush=True)
finally:
    result['duration_s']=time.monotonic()-begin
    for attempt in range(3):
        try:
            s=read()
            for d in (1,2):
                if s[f'deck{d}']['playing']: post(d,'play_pause')
            time.sleep(.3); s=read(); save('final-stopped.json',s)
            result['stopped']=all(not s[f'deck{d}']['playing'] for d in (1,2))
            if result['stopped']: break
        except Exception as e: result['errors'].append('stop: '+str(e))
    final_counters=counters(s)
    if final_counters.keys()!=initial.keys():result['errors'].append('Final counter schema changed')
    result['strict_deltas']={k:final_counters[k]-v if k in final_counters else None for k,v in initial.items()}
    if any(v!=0 for v in result['strict_deltas'].values()):result['errors'].append('Final strict counter changed')
    if not result['stopped']:result['errors'].append('Final stop not confirmed')
    try: late.update(s['diagnostics']['output_late_count'],s['diagnostics']['output_late_max_us'],result['duration_s'])
    except Exception as e: result['errors'].append('final late counter: '+str(e))
    result['output_late_delta']=s['diagnostics']['output_late_count']-baseline['diagnostics']['output_late_count']
    result['output_late_monitoring']=late.summary()
    if result['complete'] and late.findings:result['classification']='complete_with_output_late_investigation'
    result['midi_packets_delta']=s['p4_usb']['controller']['midi_packets']-baseline['p4_usb']['controller']['midi_packets']
    try: save('final-ui-timing.json',read('ui-timing')); save('final-firmware.json',read('firmware'))
    except Exception as e: result['errors'].append('final snapshot: '+str(e))
    for endpoint in ('usb-bus-trace','diagnostic-log'):
        if not result['stopped']: break
        try:
            if endpoint=='diagnostic-log':
                with op.open(base+endpoint,timeout=5) as r: (out/'final-diagnostic-log.txt').write_bytes(r.read())
            else: save('final-'+endpoint+'.json',read(endpoint))
        except Exception as e: result['errors'].append('capture snapshot: '+str(e))
    if result['errors']:result['complete']=False;result['classification']='failed_or_incomplete'
    if result['complete'] and result['duration_s']<duration:
        result['complete']=False;result['classification']='incomplete_duration'
    result['capture_note']='Compare original initial/final bus capture; historical first-fault contents are not automatically a new fault.'
    save('result.json',result); print(json.dumps({'path':str(out),**result}),flush=True)
sys.exit(0 if result['complete'] and result['stopped'] else 1)

