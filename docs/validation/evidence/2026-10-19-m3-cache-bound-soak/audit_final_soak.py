import argparse, gzip, hashlib, importlib.util, json, shutil
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
parser=argparse.ArgumentParser(description='Offline audit; never contacts the device')
parser.add_argument('--run',type=Path,default=ROOT / '.cache/m3-cache-bound-20261018/final-soak-20261018T210936Z')
parser.add_argument('--out',type=Path,default=ROOT / 'docs/validation/evidence/2026-10-19-m3-cache-bound-soak')
args=parser.parse_args(); RUN=args.run.resolve(); OUT=args.out.resolve()
SHA = '1556e8d7dfcb4f016c52ab1a372dd2e076dcaa1f'
ELF = '4999176a6b7b1fe9c0f48e7c9687a830b45873d8752afc9a8e1a61d8e7992d93'
STRICT = '''uac_format.feedback_invalid service_log.dropped
p4_usb.host.recovery_requests p4_usb.host.recovery_failures p4_usb.host.recovery_queue_drops
p4_usb.controller.interface_claim_failures p4_usb.controller.transfer_alloc_failures
p4_usb.controller.midi_disconnects p4_usb.controller.midi_in_pipe_errors
p4_usb.controller.probe_event_drops p4_usb.controller.recovery_requests
p4_usb.controller.midi_in_submit_failures p4_usb.controller.midi_out_submit_failures
p4_usb.runtime.queue_failures p4_usb.storage.disconnect_events
diagnostics.pcm_underrun1 diagnostics.pcm_underrun2
diagnostics.usb_headphones.dropped_blocks diagnostics.usb_headphones.overflow_frames
diagnostics.usb_headphones.underflow_frames diagnostics.usb_headphones.packet_failures
diagnostics.usb_headphones.packet_lost_frames'''.split()
def read(name): return json.loads((RUN/name).read_text(encoding='utf-8-sig'))
def at(obj, path):
    for key in path.split('.'): obj = obj[key]
    return obj
def digest(data): return hashlib.sha256(data).hexdigest()
def save(path, obj): path.write_text(json.dumps(obj, indent=2, ensure_ascii=False)+'\n', encoding='utf-8')
baseline, result, final = map(read, ('baseline.json','result.json','final-stopped.json'))
assert len(STRICT)==22 and set(STRICT)==set(result['strict_deltas'])
assert result['complete'] and result['stopped'] and not result['errors'] and result['duration_s']>=3600
assert read('wrapper-exit.json')['exit_code']==0
for name in ('firmware.json','final-firmware.json'):
    fw=read(name)
    assert fw['source_sha']==SHA and fw['source_dirty'] is False
    assert fw['image_elf_sha256']==ELF and fw['running_image_state']=='valid' and fw['running_slot']=='ota_1'
    assert fw['board_id']=='m3' and fw['project']=='main-deck-m3'
for name in ('initial-ui-timing.json','final-ui-timing.json'): assert read(name)['zoom_beats']==8
spec=importlib.util.spec_from_file_location('late_policy',RUN/'s2_soak_late_policy.py')
policy=importlib.util.module_from_spec(spec); spec.loader.exec_module(policy)
late=policy.LateObservation(at(baseline,'diagnostics.output_late_count'),at(baseline,'diagnostics.output_late_max_us'))
resources=['heap_free','internal_free','internal_min_free','dma_free','dma_min_free','psram_free','psram_min_free']
ranges={k:[] for k in resources}; mixer_changes=[]; previous_mixer=None
last_elapsed=-1; previous_uptime=baseline['uptime_ms']; samples=0; max_gap=0; midi_changes=[]
previous_midi=at(baseline,'p4_usb.controller.midi_packets')
def verify(s, playing):
    assert s['source_sha']==SHA and s['source_dirty'] is False and s['board_id']=='m3' and s['project']=='main-deck-m3'
    for key in STRICT:
        assert type(at(s,key)) is int and type(at(baseline,key)) is int
        assert at(s,key)==at(baseline,key), key
    assert s['controller']['present'] and s['controller']['active_profile']=='pioneer_ddj_flx4'
    u=s['uac_format']
    assert u['streaming'] and not u['faulted'] and u['sample_rate']==44100 and u['bits']==16 and u['subslot_bytes']==2 and not u['high_speed']
    assert s['diagnostics']['output_sample_rate']==48000
    assert s['diagnostics']['deck_sample_rate1']==44100 and s['diagnostics']['deck_sample_rate2']==48000
    assert s['capabilities']['controller_root']==1 and s['capabilities']['storage_root']==0 and s['main_sink']=='pcm5102a'
    for d,pitch in ((1,4096),(2,12288)):
        deck=s[f'deck{d}']; base=baseline[f'deck{d}']
        assert deck['playing']==playing and deck['master_tempo'] and deck['loop_active'] and deck['raw_pitch']==pitch
        assert all(deck[k]==base[k] for k in ('title','loop_start_ms','loop_end_ms'))
verify(read('initial-playing.json'),True)
with (RUN/'status.jsonl').open(encoding='utf-8-sig') as f:
    for line in f:
        row=json.loads(line); elapsed=row['elapsed_s']; s=row['status']; verify(s,True)
        assert elapsed>last_elapsed and s['uptime_ms']>=previous_uptime
        if last_elapsed>=0: max_gap=max(max_gap,elapsed-last_elapsed)
        last_elapsed=elapsed; previous_uptime=s['uptime_ms']; samples+=1
        late.update(at(s,'diagnostics.output_late_count'),at(s,'diagnostics.output_late_max_us'),elapsed)
        for key in resources:
            value=s['diagnostics'][key]; assert type(value) is int; ranges[key].append(value)
        for key in ('internal_largest_free','dma_largest_free','psram_largest_free'):
            assert s['diagnostics'][key] is None
        assert s['diagnostics']['heap_walk_max_us']==[None,None,None]
        mixer=s['mixer']
        if mixer!=previous_mixer:
            changes=mixer if previous_mixer is None else {k:v for k,v in mixer.items() if previous_mixer.get(k)!=v}
            mixer_changes.append({'elapsed_s':elapsed,'changes':changes}); previous_mixer=mixer
        midi=at(s,'p4_usb.controller.midi_packets')
        assert midi>=previous_midi
        if midi!=previous_midi: midi_changes.append({'elapsed_s':elapsed,'delta':midi-previous_midi})
        previous_midi=midi
verify(final,False)
assert final['uptime_ms']>=previous_uptime
late.update(at(final,'diagnostics.output_late_count'),at(final,'diagnostics.output_late_max_us'),result['duration_s'])
assert late.summary()==result['output_late_monitoring']
assert samples==result['samples']==5301
# The runner checks the deadline between polls; its final monotonic duration
# includes the final 0.5-s sleep, whereas the last status precedes that sleep.
assert 0 <= result['duration_s']-last_elapsed <= 1
assert all(v==0 for v in result['strict_deltas'].values())
assert at(final,'p4_usb.controller.midi_packets')-at(baseline,'p4_usb.controller.midi_packets')==result['midi_packets_delta']==74
initial_bus=read('initial-usb-bus-trace.json'); final_bus=read('final-usb-bus-trace.json')
audit={'telemetry_audit':'complete_with_output_late_investigation','operator_full_hour':'pending',
       'source_sha':SHA,'image_elf_sha256':ELF,'duration_s':result['duration_s'],'samples':samples,
       'last_sample_elapsed_s':last_elapsed,'max_sample_gap_s':max_gap,'strict_counters_present':22,
       'all_sample_strict_deltas_zero':True,'all_sample_workload_and_identity_verified':True,
       'final_stopped':True,'output_late_monitoring_replay_matches':True,
       'resources':{k:{'min':min(v),'max':max(v),'baseline':baseline['diagnostics'][k],'final':final['diagnostics'][k]} for k,v in ranges.items()},
       'fragmentation_and_walk_timings':'Unavailable/null; not converted to zero',
       'midi_packets_delta':74,'midi_observations':midi_changes,'mixer_observations':mixer_changes,
       'bus_capture_equal':initial_bus==final_bus,'bus_capture_scope':'Disabled/unavailable observer; not proof of absence of errors',
       'physical_scope':'Pending separate operator confirmation; MIDI/mixer observations do not qualify active-control pressure',
       'status_jsonl_raw_sha256':digest((RUN/'status.jsonl').read_bytes())}
OUT.mkdir(parents=True,exist_ok=True)
for file in RUN.iterdir():
    if file.is_file() and file.name!='status.jsonl': shutil.copyfile(file,OUT/file.name)
raw=(RUN/'status.jsonl').read_bytes(); compressed=gzip.compress(raw,compresslevel=9,mtime=0)
assert gzip.decompress(compressed)==raw
(OUT/'status.jsonl.gz').write_bytes(compressed)
if Path(__file__).resolve()!=OUT/'audit_final_soak.py': shutil.copyfile(__file__,OUT/'audit_final_soak.py')
save(OUT/'audit.json',audit)
save(OUT/'sha256.json',{p.name:digest(p.read_bytes()) for p in sorted(OUT.iterdir()) if p.is_file() and p.name!='sha256.json'})
print(json.dumps({k:v for k,v in audit.items() if k not in ('midi_observations','mixer_observations')},indent=2))
print(json.dumps({'midi_observations':midi_changes,'mixer_changes':mixer_changes,'compressed_bytes':len(compressed),'evidence_files':len(list(OUT.iterdir()))},indent=2))
