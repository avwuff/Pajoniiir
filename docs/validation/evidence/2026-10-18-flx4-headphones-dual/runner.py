"""One 60-s JC4880/FLX4 mixed-rate MT smoke, with bounded semantic loop/CUE actions."""
import datetime
import json
from pathlib import Path
import time
import urllib.request

ROOT=Path(__file__).resolve().parent
OUT=ROOT/('smoke-'+datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%SZ'))
OUT.mkdir()
SOURCE='605bcd20d6dadf250ce1816d82353ee2ace21867'
op=urllib.request.build_opener(urllib.request.ProxyHandler({}))
def read(endpoint='status'):
    with op.open('http://192.168.4.1/api/'+endpoint,timeout=5) as response:return json.load(response)
def post(deck,action):
    request=urllib.request.Request(f'http://192.168.4.1/api/control?deck={deck}&action={action}',data=b'',method='POST',headers={'X-DDJ-Control':'1'})
    with op.open(request,timeout=6) as response:assert response.status==200
def save(name,data):(OUT/name).write_text(json.dumps(data,indent=2)+'\n',encoding='utf-8')
def counters(status):
    found={}
    names={'pcm_underrun1','pcm_underrun2','packet_failures','packet_lost_frames','underflow_frames','overflow_frames','dropped_blocks','feedback_invalid','midi_disconnects','interface_claim_failures','transfer_alloc_failures','midi_in_pipe_errors','midi_in_submit_failures','midi_out_submit_failures','queue_failures','recovery_requests','recovery_failures','recovery_queue_drops','probe_event_drops','disconnect_events','dropped'}
    def visit(obj,prefix=''):
        for key,value in obj.items():
            path=prefix+'.'+key if prefix else key
            if key in names:
                assert isinstance(value,(int,float)) and not isinstance(value,bool),'Invalid counter '+path
                found[path]=value
            if isinstance(value,dict):visit(value,path)
    visit(status);assert len(found)==22
    return found
baseline=read();fw=read('firmware');save('baseline.json',baseline);save('firmware.json',fw)
assert baseline['source_sha']==SOURCE and not baseline['source_dirty'] and fw['running_image_state']=='valid'
assert baseline['controller']['active_profile']=='pioneer_ddj_flx4' and baseline['uac_format']['streaming']
assert not baseline['uac_format']['faulted'] and not baseline['uac_format']['high_speed']
assert all(not baseline[f'deck{d}']['playing'] and baseline[f'deck{d}']['state_text']=='READY' for d in (1,2))
assert baseline['diagnostics']['deck_sample_rate1']==44100 and baseline['diagnostics']['deck_sample_rate2']==48000
assert baseline['board_id']=='jc4880' and fw['image_elf_sha256']=='f1c788368a7feff414909e248043df085e8e5e5e484d7394a5cf99c80fedfeb7'
assert baseline['deck1']['master_tempo'] and baseline['deck2']['master_tempo']
assert baseline['deck1']['pitch_percent']==5 and baseline['deck2']['pitch_percent']==-5
ui=read('ui-timing');save('initial-ui-timing.json',ui);assert ui['zoom_beats']==8
initial=counters(baseline)
result={'complete':False,'stopped':False,'samples':0,'duration_s':0,'strict_baseline':initial,
    'actions':[],'scope':'60-s JC4880 mixed-rate +/-5% MT/zoom8 FLX4/PCM MAIN, actor loop/CUE, operator PFL/MIX/LEVEL/jog/Browse, no soak',
    'operator_audio_visual_controls':'pending','errors':[]}
observed_mixer=[]
start=time.monotonic();last=baseline
def record_action(name):result['actions'].append({'elapsed_s':round(time.monotonic()-start,3),'action':name})
def validate(status):
    assert status['source_sha']==SOURCE and status['uptime_ms']>=baseline['uptime_ms'],'Identity/reset changed'
    assert status['controller']['present'] and status['controller']['active_profile']=='pioneer_ddj_flx4'
    assert status['uac_format']['streaming'] and not status['uac_format']['faulted'] and not status['uac_format']['high_speed']
    now=counters(status);assert now.keys()==initial.keys()
    for key,value in initial.items():assert now[key]==value,'New strict error '+key+' delta='+str(now[key]-value)
    assert status['diagnostics']['usb_headphones']['data_loss'] is False
    for d in (1,2):
        assert status[f'deck{d}']['title']==baseline[f'deck{d}']['title']
        assert status[f'deck{d}']['master_tempo'] and status[f'deck{d}']['pitch_percent']==baseline[f'deck{d}']['pitch_percent']
def wait_loop_state(active):
    until=time.monotonic()+2
    while time.monotonic()<until:
        status=read();validate(status)
        if all(status[f'deck{d}']['loop_active'] is active for d in (1,2)):return
        time.sleep(.05)
    raise RuntimeError('Loop state did not reach '+str(active))
try:
    # Establish transient CDJ cues at the prepared stopped heads, not Hot Cues.
    for d in (1,2):post(d,'cue')
    time.sleep(.2)
    for d in (1,2):post(d,'play_pause')
    start=time.monotonic()
    print('START: 0-20s PFL/MIX/LEVEL; 20-30s jogs/Browse; then audio/display',flush=True)
    with (OUT/'status.jsonl').open('x',encoding='utf-8') as file:
        loops=False;cleared=False;cue_done=set()
        while time.monotonic()-start<60:
            last=read();elapsed=time.monotonic()-start;validate(last)
            file.write(json.dumps({'elapsed_s':elapsed,'status':last})+'\n');file.flush();result['samples']+=1
            observed_mixer.append(last['mixer'])
            if elapsed>=25 and not loops:
                for d in (1,2):post(d,'loop_4')
                wait_loop_state(True)
                record_action('both loop_4 active');loops=True
            if elapsed>=35 and not cleared:
                for d in (1,2):post(d,'loop_clear')
                wait_loop_state(False)
                record_action('both loop_clear');cleared=True
            for d,when in ((1,43),(2,50)):
                if elapsed>=when and d not in cue_done:
                    post(d,'cue');time.sleep(.15);check=read();validate(check)
                    assert check[f'deck{d}']['playing'] is False,'CUE tap did not park deck'
                    assert check[f'deck{3-d}']['playing'] is True,'CUE affected other deck'
                    record_action(f'D{d} CUE tap parked only its deck')
                    post(d,'play_pause');check=read();validate(check)
                    assert check[f'deck{d}']['playing'] is True
                    record_action(f'D{d} PLAY after CUE');cue_done.add(d)
            time.sleep(.5)
    assert loops and cleared and len(cue_done)==2
    result['complete']=True
except Exception as error:
    result['errors'].append(str(error));print('Smoke stopped: '+str(error),flush=True)
finally:
    result['duration_s']=time.monotonic()-start
    for attempt in range(3):
        try:
            last=read()
            for d in (1,2):
                if last[f'deck{d}']['playing']:post(d,'play_pause')
            time.sleep(.2);last=read();save('final-stopped.json',last)
            result['stopped']=all(not last[f'deck{d}']['playing'] for d in (1,2))
            if result['stopped']:break
        except Exception as error:result['errors'].append('Stop: '+str(error))
    final=counters(last)
    result['strict_deltas']={key:final[key]-value for key,value in initial.items()}
    result['midi_packets_delta']=last['p4_usb']['controller']['midi_packets']-baseline['p4_usb']['controller']['midi_packets']
    result['output_late_delta']=last['diagnostics']['output_late_count']-baseline['diagnostics']['output_late_count']
    result['output_late_max_us']=last['diagnostics']['output_late_max_us']
    result['classification']='short_telemetry_pass' if result['complete'] and result['stopped'] and all(value==0 for value in result['strict_deltas'].values()) else 'failed_or_incomplete'
    result['pfl_states']=[list(x) for x in sorted({(m['pfl1'],m['pfl2']) for m in observed_mixer})]
    for field in ('headphone_mix','headphone_level'):
        result[field+'_min_max']=[min(m[field] for m in observed_mixer),max(m[field] for m in observed_mixer)] if observed_mixer else None
    save('result.json',result)
    print(str(OUT));print(json.dumps(result,indent=2),flush=True)
    for endpoint in ('firmware','ui-timing','usb-bus-trace'):
        try:save('final-'+endpoint+'.json',read(endpoint))
        except Exception as error:print('Final evidence: '+str(error),flush=True)
raise SystemExit(0 if result['classification']=='short_telemetry_pass' else 1)
