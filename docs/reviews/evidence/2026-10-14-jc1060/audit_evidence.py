import hashlib, importlib.util, json, pathlib, struct, sys
ROOT=next(p for p in pathlib.Path(__file__).resolve().parents if (p/'firmware/p4-core').is_dir()); OUT=ROOT/'.cache/jc1060-review-20261014'
E=OUT/'contributor/docs/validation/jc1060/evidence'
audit={}
for path in sorted(E.rglob('*_result.json')):
 d=json.loads(path.read_text()); baseline=d.get('baseline',{}); final=d.get('final',{})
 calculated={k:final[k]-baseline[k] for k in baseline if k in final}
 declared=d.get('delta',{})
 assert calculated==declared
 audit[str(path.relative_to(E))]={'result':d.get('result'),'mode':d.get('mode'), 'duration':d.get('elapsed_seconds'), 'samples':d.get('status_samples'),'identity':d.get('identity'), 'baseline':baseline,'final':final,'calculated_deltas':calculated, 'declared_deltas':declared, 'physical_operator_acceptance':d.get('physical_operator_acceptance'), 'timing_workload':d.get('timing_workload')}
 print(str(path.relative_to(E)), json.dumps(audit[str(path.relative_to(E))]))
d=json.loads((E/'D-audio/d7_uac_transition_run4_cue_fix_pass.json').read_text())
samples=d['samples']; assert len(samples)==182
keys=['underflow_frames','overflow_frames','dropped_blocks','packet_failures','packet_lost_frames','pcm1','pcm2','output_late']
delta={k:d['final'][k]-d['baseline'][k] for k in keys}
assert all(v==0 for v in delta.values())
assert all(s['deck1_playing'] and s['deck2_playing'] and not s['uac_data_loss'] and s['uac_flags']==0 and all(s[k]==d['baseline'][k] for k in keys) for s in samples)
audit['transition_stress']={'top_level_keys':list(d), 'result':d['result'],'requested_cycles':d['requested_cycles'],'completed_operations':d['completed_operations'],'actual_seconds':d['actual_seconds'],'samples':len(samples),'calculated_audio_deltas':delta,'failures':d.get('failures')}
print('transition',json.dumps(audit['transition_stress']))
rows=[json.loads(l) for l in (E/'F-link/f8_peer_loss_1hz.jsonl').read_text().splitlines() if l.strip()]
valid=[r for r in rows if 'deck1' in r and 'deck2' in r]
audit['peer_loss']={'total_rows':len(rows),'status_rows':len(valid),'duration_wall_s':valid[-1]['t']-valid[0]['t'], 'playing_both':sum(r['deck1']['playing'] and r['deck2']['playing'] for r in valid), 'counter_deltas':{key:valid[-1][key]-valid[0][key] for key in ['pcm_underrun1','pcm_underrun2','output_late_count','af','sderr']},'max_position_minus_uptime_step_ms':{deck:max(abs((b[deck]['position_ms']-a[deck]['position_ms'])-(b['up']-a['up'])) for a,b in zip(valid,valid[1:])) for deck in ['deck1','deck2']}}
print('peer loss',json.dumps(audit['peer_loss']))
from idf_component_tools.hash_tools import calculate
from idf_component_tools.manager import ManifestManager
original=calculate.hash_file
locks={}
for name in ['usb','espressif__usb_host_msc']:
 path=ROOT/'firmware/main-deck-jc1060/managed_components'/name
 if not path.exists(): path=ROOT/'firmware/main-deck-m3/managed_components'/name
 manifest=ManifestManager(str(path/'idf_component.yml'),name=name).load()
 kwargs={'use_gitignore':manifest.use_gitignore,'include':manifest.include_set,'exclude':manifest.exclude_set}
 hashes={}
 for mode in ['original','LF','CRLF']:
  def normalized_hash(p):
   b=pathlib.Path(p).read_bytes()
   try: b.decode('utf-8')
   except UnicodeDecodeError: return hashlib.sha256(b).hexdigest()
   if b'\0' not in b:
    b=b.replace(b'\r\n',b'\n')
    if mode=='CRLF':b=b.replace(b'\n',b'\r\n')
   return hashlib.sha256(b).hexdigest()
  calculate.hash_file=original if mode=='original' else normalized_hash
  hashes[mode]=calculate.hash_dir(path,**kwargs)
 calculate.hash_file=original
 locks[name]={'path':str(path),'hashes':hashes,'component_hash_file':(path/'.component_hash').read_text().strip()}
 print('lock hash',name,json.dumps(locks[name]))
audit['lock_line_endings']=locks
(OUT/'evidence-audit.json').write_text(json.dumps(audit,indent=2))
