from pathlib import Path
root=Path.cwd()
source=root/'.cache/jc4880-headphone-level-20261018/dual-smoke/smoke.py'
target=root/'.cache/jc4880-cache-20261019/smoke.py'
text=source.read_text(encoding='utf-8')
def replace(old,new):
 global text
 assert old in text,old
 text=text.replace(old,new)
replace("SOURCE='605bcd20d6dadf250ce1816d82353ee2ace21867'","SOURCE='0a33ec1fb16c0136dccaa85ca63a6fbc00a4d527'")
replace('f1c788368a7feff414909e248043df085e8e5e5e484d7394a5cf99c80fedfeb7','078cb8c09cbf4b77622facc5d0a09fedef3382f0970a3cd18bbd5ec0357975bc')
replace("isinstance(value,(int,float)) and not isinstance(value,bool)","type(value) is int")
replace("assert baseline['deck1']['master_tempo']", "assert fw['board_id']=='jc4880' and fw['project']=='main-deck-p4' and fw['running_slot']=='ota_0' and not fw['source_dirty']\nassert baseline['mixer']['headphone_level']==0, 'Require operator LEVEL minimum before playback'\nassert baseline['deck1']['master_tempo']")
replace("assert status['source_sha']==SOURCE and status['uptime_ms']>=baseline['uptime_ms']", "assert status['source_sha']==SOURCE and not status['source_dirty'] and status['board_id']=='jc4880' and status['uptime_ms']>=baseline['uptime_ms']")
replace("assert status['diagnostics']['usb_headphones']['data_loss'] is False", "assert status['diagnostics']['usb_headphones']['data_loss'] is False\n    assert status['uac_format']['sample_rate']==44100 and status['uac_format']['bits']==16 and status['uac_format']['subslot_bytes']==2\n    assert status['diagnostics']['output_sample_rate']==48000\n    assert status['diagnostics']['deck_sample_rate1']==44100 and status['diagnostics']['deck_sample_rate2']==48000")
replace("assert status['diagnostics']['output_sample_rate']==48000", "assert status['diagnostics']['output_sample_rate']==44100, 'JC4880 FLX4-selected MAIN rate changed'")
replace("initial=counters(baseline)", "assert baseline['capabilities']['fixed_output_sample_rate']==0 and baseline['diagnostics']['output_sample_rate']==44100, 'JC4880 ordinary rate policy'\ninitial=counters(baseline)")
replace("'scope':'60-s JC4880 mixed-rate +/-5% MT/zoom8 FLX4/PCM MAIN,", "'scope':'60-s JC4880 mixed-rate +/-5% MT/zoom8 FLX4/PCM MAIN44100, fix_output_rate0,")
replace("last=read();elapsed=time.monotonic()-start;validate(last)\n            file.write", "last=read();elapsed=time.monotonic()-start\n            file.write")
replace("result['samples']+=1\n            observed_mixer", "result['samples']+=1\n            validate(last)\n            assert all(last[f'deck{d}']['playing'] for d in (1,2)), 'Unexpected playback stop'\n            observed_mixer")
replace("post(d,'cue');time.sleep(.15);check=read();validate(check)", "post(d,'cue');time.sleep(.15);check=read();save(f'D{d}-cue-status.json',check);validate(check)")
replace("post(d,'play_pause');check=read();validate(check)", "post(d,'play_pause');check=read();save(f'D{d}-resume-status.json',check);validate(check)")
replace("if all(status[f'deck{d}']['loop_active'] is active for d in (1,2)):return", "if all(status[f'deck{d}']['loop_active'] is active for d in (1,2)):\n            save('loops-on.json' if active else 'loops-off.json',status);return")
replace("try:save('final-'+endpoint+'.json',read(endpoint))", "try:save('final-'+endpoint+'.json',read(endpoint))")
replace("raise SystemExit(0 if result['classification']=='short_telemetry_pass' else 1)", "if result['stopped']:\n        try:\n            with op.open('http://192.168.4.1/api/diagnostic-log',timeout=5) as response:(OUT/'final-diagnostic-log.txt').write_bytes(response.read())\n        except Exception as error:print('Final diagnostic evidence: '+str(error),flush=True)\nraise SystemExit(0 if result['classification']=='short_telemetry_pass' else 1)")
target.write_text(text,encoding='utf-8')
compile(text,str(target),'exec')
print('Prepared one 60-s runner; no device calls or playback')
