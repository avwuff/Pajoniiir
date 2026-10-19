"""Offline-only final-soak journal review, with explicit boot/time provenance."""
import collections, json, re, subprocess
from pathlib import Path

ROOT=Path(subprocess.check_output(['git','rev-parse','--show-toplevel'],text=True).strip())
EVIDENCE=ROOT/'docs/validation/evidence/2026-10-19-m3-cache-bound-soak'
OUT=ROOT/'.cache/m3-cache-bound-20261018/late-review'
OUT.mkdir(exist_ok=True)
def read(name): return json.loads((EVIDENCE/name).read_text(encoding='utf-8-sig'))
baseline=read('baseline.json'); final=read('final-stopped.json'); result=read('result.json')
text=(EVIDENCE/'final-diagnostic-log.txt').read_text(encoding='utf-8')
fw=re.findall(r'^schema=\d+ boot=(\d+) event=FIRMWARE_INFO fw=(\S+) partition=(\S+) reset=(\S+)',text,re.M)
assert fw[-1][1]=='M3-dev-g1556e8d7dfcb' and fw[-1][2]=='ota_1'
boot=int(fw[-1][0]); start=baseline['uptime_ms']; end=final['uptime_ms']
records=[]
pattern=re.compile(r'^seq=(\d+) boot=(\d+) ms=(\d+) level=(\S+) event=(\S+)(.*)$',re.M)
for match in pattern.finditer(text):
    seq,b,ms,level,event,tail=match.groups(); b=int(b); ms=int(ms)
    if b!=boot or not start<=ms<=end: continue
    fields={k:int(v) for k,v in re.findall(r'\b(a[0-3])=(\d+)',tail)}
    records.append({'seq':int(seq),'boot':b,'uptime_ms':ms,'event':event,'level':level,
                    **fields,'message':tail.split(' msg=',1)[1] if ' msg=' in tail else ''})
phase=[r for r in records if r['event']=='AUDIO_LATE_PHASE']
blocks=[r for r in phase if r['message'] in ('head','decode','mix','main','usb','codec','idle','tail')]
# Identify producer-labelled primary records by their following DMA context;
# retain the full raw selected records instead of silently dropping any label.
primary=[r for r in phase if r.get('a0',0)>=1000 and r.get('a1',0)>=1000 and r['message'] not in ('mix dma','i2s dma')]
dma=[r for r in phase if r['message'] in ('mix dma','i2s dma')]
context=[r for r in phase if r.get('a0',1000)<32 and r['message'] not in ('mix dma','i2s dma')]
counts=collections.Counter(r['event'] for r in records)
reported_late_count=sum(r.get('a0',0) for r in records if r['event']=='AUDIO_OUTPUT_LATE')
assert not any(counts[k] for k in ('AUDIO_BLOCK_OUTLIER','AUDIO_WDT_TRACE','CONTROLLER_DISCONNECTED'))
late=result['output_late_monitoring']
events=late['events']; intervals=[b['elapsed_s']-a['elapsed_s'] for a,b in zip(events,events[1:])]
initui=read('initial-ui-timing.json'); finalui=read('final-ui-timing.json')
summary={
 'scope':'Offline interpretation of unchanged final M3/FLX4 hour; no device calls or firmware changes',
 'source_sha':baseline['source_sha'],'boot':boot,'journal_window_uptime_ms':[start,end],
 'duration_s':result['duration_s'],'late_events':late['delta'],
 'late_events_per_hour':late['delta']*3600/result['duration_s'],
 'since_boot_max_us':late['final_since_boot_max_us'],
 'configured_threshold_us':baseline['diagnostics']['output_late_threshold_us'],
 'sample_observed_peak_events_in_60s':late['max_sample_observed_cluster_60s'],
 'investigation_triggers':late['investigation_triggers'],
 'minimum_observed_interevent_s':min(intervals),
 'journal_event_counts_in_window':dict(counts),
 'journal_aggregated_output_late_count':reported_late_count,
 'primary_phase_labels':dict(collections.Counter(r['message'] for r in primary)),
 'primary_block_max_us':max(r['a0'] for r in primary),
 'dominant_phase_max_us':max(r['a1'] for r in primary),
 'mix_group_max_us':max(r.get('a3',0) for r in primary),
 'dma_context_records':len(dma),
 'dma_observed_max_gap_us':max(r['a1'] for r in dma),
 'dma_queue_overflow_values':sorted(set(r['a2'] for r in dma)),
 'sampled_task_contexts':dict(collections.Counter(r['message'] for r in context)),
 'ui_refresh_delta':finalui['refresh_count']-initui['refresh_count'],
 'ui_coalesced_refresh_delta':finalui['coalesced_refreshes']-initui['coalesced_refreshes'],
 'selected_journal_records':records,
 'interpretation':[
   'Every strict counter remains unchanged and the operator accepts the full-hour sound/display.',
   'No >=50-ms block-outlier record appears in this same-boot/time window; historical 139-ms events are outside it.',
   'Late events are approximately twice the output block period; the rolling cluster remains an investigation finding.',
   'Diagnostic emission is rate limited to once per 2 seconds, so journal phase records are not a one-to-one census of late events.',
   'Sampled interrupted task names/priorities and mix wall-time include scheduling/interrupt effects; they do not prove Wi-Fi, DSP or any specific masking call caused a late block.',
   'The offline evidence supports retaining the accepted firmware, not a speculative new scheduling/DSP/cache change or a zero-late claim.',
   'UI counters are cumulative sample-boundary observations, not physical fluidity evidence; full-hour operator acceptance is separate.'
 ]}
assert late['delta']==reported_late_count==24 and late['final_since_boot_max_us']==12988
assert late['max_sample_observed_cluster_60s']==3 and summary['primary_block_max_us']<50000
(OUT/'late-review.json').write_text(json.dumps(summary,indent=2,ensure_ascii=False)+'\n',encoding='utf-8')
print(json.dumps({k:v for k,v in summary.items() if k!='selected_journal_records'},indent=2,ensure_ascii=False))
