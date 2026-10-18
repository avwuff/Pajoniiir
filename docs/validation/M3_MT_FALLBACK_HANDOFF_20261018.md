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
gain, USB cadence and strict criteria remain unchanged. This candidate still
requires software/build checks and an exact-image hardware observation.
