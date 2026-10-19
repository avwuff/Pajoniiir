"""Audit every saved JC4880 smoke response and preserve local-OTA provenance."""
import gzip,hashlib,json,shutil
from pathlib import Path
ROOT=Path.cwd(); CACHE=ROOT/'.cache/jc4880-cache-20261019'
OUT=ROOT/'docs/validation/evidence/2026-10-19-jc4880-cache-ota'
OUT.mkdir(exist_ok=True)
SHA='0a33ec1fb16c0136dccaa85ca63a6fbc00a4d527'
ELF='078cb8c09cbf4b77622facc5d0a09fedef3382f0970a3cd18bbd5ec0357975bc'
FIRST=CACHE/'smoke-20261019T074338Z'; GOOD=CACHE/'smoke-20261019T074456Z'
def read(path):return json.loads(path.read_text(encoding='utf-8-sig'))
def save(path,data):path.write_text(json.dumps(data,indent=2,ensure_ascii=False)+'\n',encoding='utf-8')
def digest(data):return hashlib.sha256(data).hexdigest()
def at(obj,path):
 for key in path.split('.'):obj=obj[key]
 return obj
ota=read(CACHE/'ota/result.json');result=read(GOOD/'result.json');base=read(GOOD/'baseline.json');final=read(GOOD/'final-stopped.json')
assert ota['complete'] and ota['upload_attempts']==1 and ota['upload_http']==200
assert ota['final_firmware']['running_image_state']=='valid' and ota['final_firmware']['running_slot']=='ota_0'
assert ota['final_firmware']['source_sha']==SHA and ota['final_firmware']['image_elf_sha256']==ELF
assert result['complete'] and result['stopped'] and result['duration_s']>=60 and result['samples']==93 and not result['errors']
strict=result['strict_baseline'];assert len(strict)==22 and strict.keys()==result['strict_deltas'].keys()
assert all(type(v) is int and v==0 for v in result['strict_deltas'].values())
assert result['midi_packets_delta']==1902 and result['output_late_delta']==0
rows=[json.loads(line) for line in (GOOD/'status.jsonl').read_text().splitlines()];assert len(rows)==93
assert all(a['elapsed_s']<b['elapsed_s'] and a['status']['uptime_ms']<=b['status']['uptime_ms'] for a,b in zip(rows,rows[1:]))
assert rows[0]['status']['uptime_ms']>=base['uptime_ms'] and final['uptime_ms']>=rows[-1]['status']['uptime_ms']
allstatus=[('baseline',base),('final',final)]+[(str(r['elapsed_s']),r['status']) for r in rows]
for name in ('loops-on.json','loops-off.json','D1-cue-status.json','D1-resume-status.json','D2-cue-status.json','D2-resume-status.json'):
 allstatus.append((name,read(GOOD/name)))
for name,s in allstatus:
 assert s['source_sha']==SHA and s['source_dirty'] is False and s['board_id']=='jc4880',name
 assert s['capabilities']['fixed_output_sample_rate']==0 and s['main_sink']=='pcm5102a'
 assert s['capabilities']['controller_root']==1 and s['capabilities']['storage_root']==0
 assert s['controller']['present'] and s['controller']['active_profile']=='pioneer_ddj_flx4'
 u=s['uac_format'];assert u['streaming'] and not u['faulted'] and not u['high_speed'] and u['sample_rate']==44100 and u['bits']==16 and u['subslot_bytes']==2
 assert s['diagnostics']['output_sample_rate']==44100 and s['diagnostics']['deck_sample_rate1']==44100 and s['diagnostics']['deck_sample_rate2']==48000
 for key,value in strict.items():assert type(at(s,key)) is int and at(s,key)==value,(name,key)
 for d,pitch in ((1,5),(2,-5)):
  assert s[f'deck{d}']['master_tempo'] and s[f'deck{d}']['pitch_percent']==pitch and s[f'deck{d}']['title']==base[f'deck{d}']['title']
 assert s['diagnostics']['usb_headphones']['data_loss'] is False
assert all(all(r['status'][f'deck{d}']['playing'] for d in (1,2)) for r in rows)
assert all(not final[f'deck{d}']['playing'] and not final[f'deck{d}']['loop_active'] for d in (1,2))
assert read(GOOD/'loops-on.json')['deck1']['loop_active'] and read(GOOD/'loops-on.json')['deck2']['loop_active']
assert all(not read(GOOD/'loops-off.json')[f'deck{d}']['loop_active'] for d in (1,2))
for d in (1,2):
 assert read(GOOD/f'D{d}-cue-status.json')[f'deck{d}']['playing'] is False
 assert read(GOOD/f'D{d}-cue-status.json')[f'deck{3-d}']['playing'] is True
 assert read(GOOD/f'D{d}-resume-status.json')[f'deck{d}']['playing'] is True
for name in ('firmware.json','final-firmware.json'):
 fw=read(GOOD/name);assert fw['source_sha']==SHA and fw['image_elf_sha256']==ELF and fw['running_slot']=='ota_0' and fw['running_image_state']=='valid' and not fw['source_dirty']
assert all(read(GOOD/name)['zoom_beats']==8 for name in ('initial-ui-timing.json','final-ui-timing.json'))
prev=base['p4_usb']['controller']['midi_packets'];timing=[]
for row in rows:
 n=row['status']['p4_usb']['controller']['midi_packets']
 if n!=prev:timing.append({'elapsed_s':row['elapsed_s'],'delta':n-prev})
 prev=n
original=read(FIRST/'result.json')
assert not original['complete'] and original['stopped'] and original['samples']==1 and all(v==0 for v in original['strict_deltas'].values())
for source,name in ((CACHE/'ota','ota'),(FIRST,'incomplete-monitor-attempt'),(GOOD,'smoke')):
 dest=OUT/name;dest.mkdir(exist_ok=True)
 for file in source.iterdir():
  if file.name=='status.jsonl':
   raw=file.read_bytes();compressed=gzip.compress(raw,mtime=0);assert gzip.decompress(compressed)==raw
   (dest/'status.jsonl.gz').write_bytes(compressed)
  elif file.is_file():shutil.copyfile(file,dest/file.name)
for name in ('install.py','prepare.py','smoke.py','prepare_runner.py','smoke-prepared.json','smoke-library.json','documentation-check.txt'):
 shutil.copyfile(CACHE/name,OUT/name)
shutil.copyfile(__file__,OUT/'archive.py')
save(OUT/'operator-confirmation.json',{'recorded_local_date':'2026-10-19','source':'Direct operator replies in this chat',
 'gui_and_host_network_reply':'DHCP obnovljen; GUI uredan','full_smoke_reply':'Sve navedeno provjereno i uredno',
 'scope':'Both jogs/Browse, PFL D1/D2, headphone MIX/LEVEL, loop/CUE transitions, clean MAIN/FLX4 headphones and fluid waveforms throughout the corrected60-s test',
 'physical_short_smoke':'PASS','source_sha':SHA,'image_elf_sha256':ELF,
 'limitations':'Short exact-image acceptance only; JC4880 long soak remains waived, no JC1060 physical qualification or publication'})
save(OUT/'audit.json',{'classification':'short_telemetry_and_operator_pass','source_sha':SHA,'image_elf_sha256':ELF,
 'duration_s':result['duration_s'],'samples':len(rows),'strict_counters':22,'all_saved_sample_and_action_deltas_zero':True,
 'output_late_delta':0,'midi_packets_delta':1902,'midi_observations':timing,
 'final_stopped_loops_off':True,'main_rate':44100,'flx4_rate':44100,'source_rates':[44100,48000],
 'startup_counter_context':'One recovery, MIDI disconnect/pipe error and stopped underflow3187384 already exist in baseline; unchanged throughout this test. No zero-since-boot claim.',
 'monitor_attempt_context':'First0.076-s attempt incomplete because its runner imposed M3 fixed48000 instead of JC4880 selected44100; raw result retained, all strict deltas0. Corrected runner changes board-rate assertion only, no loss threshold or firmware change.',
 'preflight_filename_context':'ota/preflight-* was refreshed after OTA and contains a later readback; ota/before-* preserves original admission state before the single upload.',
 'status_jsonl_raw_sha256':digest((GOOD/'status.jsonl').read_bytes()),
 'incomplete_status_jsonl_raw_sha256':digest((FIRST/'status.jsonl').read_bytes())})
(OUT/'.gitattributes').write_text('* -text whitespace=-blank-at-eol,-blank-at-eof,space-before-tab,cr-at-eol\n*.gz binary\n',encoding='utf-8')
save(OUT/'sha256.json',{str(p.relative_to(OUT)).replace('\\','/'):digest(p.read_bytes()) for p in sorted(OUT.rglob('*')) if p.is_file() and p.name!='sha256.json'})
print(json.dumps({'duration_s':result['duration_s'],'audited_status_snapshots':len(allstatus),'midi_observations':timing,'files':len(list(OUT.rglob('*')))},indent=2))
