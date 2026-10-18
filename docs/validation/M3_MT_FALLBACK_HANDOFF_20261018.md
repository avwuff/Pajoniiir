# MT fallback must retain the actual consumer clock

## Frozen hardware findings

The [observer repair](M3_STATUS_OBSERVER_REPAIR_20261018.md) removes unsafe
diagnostic heap walks but does not close the loop-exit gate. Its isolated
mixed-rate MT test on `3459a97a` stops on UAC overflow/drop and one 139,045-us
mix segment. The operator hears/sees no disturbance; the strict FAIL remains.
Controller admission pressure in the separate active-control run also remains
an independent open gate. No new physical run precedes this software finding.

## Production-path reproduction

A deterministic host experiment withdraws future PCM from the canonical
timeline while MT is active, with a later refill. It uses the production MT,
timeline and resampler modules. Across 112 sampled combinations it observes
17 backward playhead movements. Source read work remains bounded (maximum 46
reads per output frame in the experiment); this does not reproduce or explain
the measured hardware wall-time stall.

The engine's production `ae_keylock_render_cb` restarts DSP on initialization
or timeline-generation change only. The production output mixer legitimately
falls back to ordinary resampling when MT lacks look-ahead, including after a
loop cut and near EOF. That fallback advances the real consumer cursor without
changing timeline generation. When the old MT clock resumes, its stale
`logical_seq` can overwrite the newer consumer position.

The new regression executes the actual callback extracted from `audio_engine.c`
against the real MT/timeline/resampler implementations. The old callback fails
at a 44.1/48-kHz, -5% handoff with one future frame retained: the actual cursor
moves from 7,151 back to 7,150 at output frame 513 after refill. This is a
source-clock ownership defect, independent of host timing or USB emulation.

## Minimal repair and scope

The output-owned callback compares the real timeline playhead with the MT
logical cursor before rendering. A mismatch re-anchors MT to the real consumer
position; ordinary playback with matching clocks follows its existing path.
Initialization and explicit decoder/user-seek generation handling are retained.
No producer cursor, queue policy, loss counter, gain, USB cadence, loop bounds
or decoder algorithm changes.

All 224 regression cases pass for both decks, 44.1/48-kHz source-to-output
ratios, +/-5% tempo, four hop phases, seven retained-runway sizes, starvation,
fallback, refill and explicit user-seek resets. The actual cursor never rewinds
across a rendered frame and its movement equals reported consumption. A linker
wrapper counts production DSP resets; the largest observed handoff count is
three per 1,024-frame window (the test caps it at four), rather than resetting
DSP on every unavailable source frame.

This fixes the reproduced stale-clock handoff. It is not proof that the earlier
PCM/UAC losses, controller admission failure or 139-ms hardware stall are solved.
Preserve their records and retain all strict acceptance criteria.

## Validation boundary

Focused production handoff regression is RED on the old callback and GREEN
after the repair. Full host `-Qualification`, ordinary M3 ESP-IDF 6.0.2
incremental reconfigure/build, board/budget isolation and signed development
packaging pass. The same 224-case harness also passes without the FIR cache.
Only M3 is built for this investigation; other-board
physical gates, the JC4880 soak waiver and operator-closed S2 scope are unchanged.

## Installed candidate and failed loop-exit retest

Clean `2fa2263142ed46a5a86a0a7cccdef6330134d526`,
`M3-dev-g2fa2263142ed`, was installed by signed local OTA (HTTP 200,
16.150252 s) and confirmed VALID on `ota_1`; the operator confirms normal GUI.
ELF SHA-256 is `b3b972750e38e6c1f9fa595099f82c224aa9ea7b2891f430574c64aabce378ae`.
BIN is 2,602,912 bytes, SHA-256
`9df466328bea61ef8b2f659d68ae9e61f6b9773776f283968d87c14889e74dcb`;
signed bundle is 2,603,100 bytes, SHA-256
`b98b4afcb489a6d289dc313abe3251104b52dbb11472d657dd1e13007406d57b`.
Effective configuration matches the ordinary M3 baseline. FLX4 USB2, storage
USB3, MAIN 48 kHz, FLX4 FS 44.1 kHz, Wi-Fi and library260 are preserved.

The mixed-rate MT +/-5%, zoom8 loop/CUE test stops at 43.137 s / 44 saved
samples, during `wait-loop-False`, after clear intents at 35.419/36.060 s.
All 22 strict paths are present: 11 new UAC dropped blocks and 2,251 overflow
frames fail the gate; the other 20 deltas, including both PCM and ISO loss,
are zero. One output-late is 142,503 us. The same-boot journal records mix
141,867 us, with group15 taking 139,212 us. CUE/PLAY phases are not reached.
Both decks end stopped, loops OFF. All sample source identities match.

The operator reports no perceived disturbance and explicitly no control
movement. Readback nevertheless contains +40 MIDI packets, LEVEL 5,629..16,383,
MIX16,383 and PFL false/false; retain this discrepancy rather than claiming a
qualified physical PFL/CUE path. The strict telemetry FAIL remains regardless.
This repair fixes the demonstrated host clock defect but does not resolve the
repeated hardware mix stall. No new long soak or public publication follows.

Raw software, OTA, runner, failing response, final snapshots, operator context
and a 38-file SHA-256 inventory are retained in
[the evidence directory](evidence/2026-10-18-m3-mt-handoff/sha256.json).

## Next bounded observation

The follow-up adds two snapshots of the existing BSP I2S DMA counters around
the mix window and journals them only on the existing rate-limited late path.
No new ISR hook, heap walk or per-frame logging is added. Completion/overflow
counts are differences; maximum interrupt gap and highest sampled interrupted
task/priority provide context. A gap can straddle the observation boundary,
and the highest sampled task does not establish duration or causal ownership.
The measurement has overhead and does not prove a timing fix. Audio algorithms,
gain, USB cadence and strict criteria remain unchanged.

### DMA observer candidate f1c5ec9c

Full host `-Qualification`, documentation integrity, ordinary M3 ESP-IDF 6.0.2
incremental reconfigure/build, board/component/budget and signed development
packaging pass. Clean `f1c5ec9ca9e9e3eba89365384762efb95e426ff7`,
`M3-dev-gf1c5ec9ca9e9`, installs by local OTA (HTTP200, 15.053 s) and is confirmed
VALID `ota_0`, ELF `b964d5fc5018f43221361e9bc44e8c49407d9a0043b258a3df1974744e931db2`.
The operator confirms GUI/Wi-Fi. FLX4 MIDI/FS44.1-kHz UAC and library260 return.
BIN is 2,603,184 bytes, SHA-256
`5ae2cf5d10b8fa5adfe20cce4b21442d685ce7063ca1c0093a1c963e61bb112a`;
bundle is 2,603,372 bytes, SHA-256
`3d5e5bc485e99976c05a7d2ad16bb7554d3bc57b01e705c99f37b1acece93d37`.

Its first 60.267-s / 70-sample loop/CUE retest completes with all 22 strict
deltas zero and output-late zero. Both loop exits and each deck's independent
CUE park / PLAY resume execute; both decks end stopped, loops OFF. The operator
confirms clean MAIN/headphones, fluid waveforms and normal transitions. MIDI
delta is zero. PFL remains false/false, MIX/LEVEL16,383 in readback: this does
not add independent PFL routing or knob acceptance despite preparation intent.
No large mix outlier is reproduced, so no causal attribution or timing repair
is established. The observer itself is a timing perturbation. Earlier FAILs
and the independent active-control admission gate remain open.

Software, signed manifest, OTA, prepared workload and subsequent bounded-run
evidence are retained in
[the DMA observer evidence directory](evidence/2026-10-18-m3-mix-dma/sha256.json).
Only M3 is built; there is no push, release or public OTA change.

### Phase-sweep FAIL and interrupt observation

The next bounded 100-s observation plans three loop exits at different scheduled
phases. It instead stops on the first exit at 20.192 s / 14 samples. D1 begins
at 32,023 ms, D2 at 23,446 ms (different source positions from the preceding
15-second preparation). Clear intents are 11.545/13.297 s. All 22 paths are present:
D2 PCM underrun +1,546, UAC dropped blocks +13 and overflow frames +2,318; the
other 19 strict deltas are zero. MIDI delta is 0. One output-late is 142,477 us.
Both decks end stopped, loops OFF. The operator reports no disturbance or
control/cable movement. Retain FAIL; the earlier short PASS does not close it.

Same boot 430, journal 241,133 ms records mix 141,841 us, group 15 at 139,102 us.
The newly scoped DMA observation has only **one completion**, **141,430-us
maximum interrupt gap**, zero queue overflows and highest sampled priority 6
`ae_output`. This supplies evidence of a long interrupt-service gap, not simply
an expensive MT search. It does not identify the masking call or prove its
causal relationship to the earlier PCM underrun event at 240,978 ms. The DMA
gap can straddle the measurement boundary; task identity is context, not an
interrupted-PC capture or CPU-time measurement.

## Shared bounded-cache candidate

Pinned IDF 6.0.2 `components/esp_mm/esp_cache_msync.c` holds a common interrupt-
masking spinlock around C2M writeback and M2C invalidation. The project wrapper
already divides valid task-context data maintenance into 1-KiB calls for the
experimental HS-audio configuration. Ordinary M3/FLX4 delegates a complete
operation instead: the HS controller selection gates a general
real-time cache policy. A USB ISR can also contend on that common cache lock.
This is a concrete unbounded-operation path and a plausible explanation of
the observed interrupt gap; the exact masking call has not been captured.

The production-wrapper regression is extended to require the same 1-KiB bound
in both ordinary and HS configurations, including a full 800x480 RGB888 scanout
span. It is RED on the ordinary wrapper (a 16-KiB request delegates once rather
than 16 times), GREEN after removing the HS-only guard. Both configurations
retain partial-tail coverage, aligned M2C, invalid direction/alignment
delegation, first-error return and unsplit ISR/outer-critical/uncacheable
delegation. DMA ownership remains with the synchronous caller until completion.
No controller, audio, panel timing, gain, loss criterion or global IDF mutex
configuration changes. This bounds bytes, not microseconds.

Clean `1556e8d7dfcb4f016c52ab1a372dd2e076dcaa1f`,
`M3-dev-g1556e8d7dfcb`, passes full host `-Qualification`, documentation integrity,
ordinary M3 ESP-IDF 6.0.2 incremental reconfigure/build, board/component/budget
and signed development packaging. Local OTA finishes HTTP200 in 15.197 s;
the image is VALID `ota_1`, ELF
`4999176a6b7b1fe9c0f48e7c9687a830b45873d8752afc9a8e1a61d8e7992d93`.
The failed phase-sweep source positions are restored while stopped. Exact-image
replay remains required; PCM continuation and controller admission stay open.
See [the cache-bound evidence](evidence/2026-10-18-m3-cache-bound/sha256.json)
for raw checks, signed manifest and artifact hashes. Other boards are not built
or requalified, and no public artifact is changed.

### Exact-image phase replay PASS, 2026-10-18

The first `1556e8d7` phase replay completes **100.264 s / 142 saved samples**
from D1 32,023 ms and D2 23,446 ms, mixed 44.1/48-kHz sources, +5/-5%, both MT,
zoom8, MAIN48-kHz and FLX4 FS44.1-kHz UAC. All 22 numeric strict paths remain
present and unchanged in every saved sample; final deltas and output-late are
zero. Initial stopped underflow3,290,918 is retained as a nonzero baseline.
MIDI delta0. Loop clears are confirmed at15.005,44.035,74.649 s; both decks end
stopped, loops OFF, with the exact clean source/ELF and VALID `ota_1` retained.

The operator confirms clean MAIN and FLX4 headphones, fluid waveforms and all
three exits throughout the entire window. Status retains PFL false/false and
MIX/LEVEL16,383; this does not independently qualify PFL or knob routing. No
same-boot outlier is recorded in the post-stop journal. Source positions match
the failed phase sweep; actual action timing differs because HTTP scheduling
is not a sample-accurate phase lock. This is one bounded replay PASS after the
cache policy correction, not proof of the exact earlier masking call or a
long reliability qualification. Earlier failures remain intact.

Raw runner, every response, operator confirmation, final snapshots and a
per-sample audit are in the cache-bound evidence directory, with a 37-file
SHA-256 inventory checked against staged Git bytes.

### Isolated active-control PASS

On the same clean installed image, a separate 30.145-s /45-sample dual-MT
window records 2,406 new MIDI packets while the operator turns both jog rims
and both Browse controls. All22 strict deltas, including runtime admission
failures, PCM/UAC/ISO loss, remain zero in every captured sample; output-late0.
The operator confirms both jogs/Browse responsive, clean MAIN and FLX4
headphones and fluid waveforms. Both decks end stopped, loops OFF. The post-stop
journal has no new same-boot audio outlier. Raw responses, runner, final
snapshots, separate operator confirmation and audit extend the evidence
inventory to 50 files. This qualifies isolated short input pressure; there are
no loop changes during this window. The earlier admission FAIL remains part of
history. Combined active controls with loop transitions and long reliability
on this image remain unrun; no threshold was weakened and no loss counter reset.
