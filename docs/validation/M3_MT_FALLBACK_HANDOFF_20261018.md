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
after the repair. Full host `-Qualification`, ordinary M3 ESP-IDF 6.0.2 build,
board/budget isolation and signed development packaging remain required before
installing a candidate. Only M3 is built for this investigation; other-board
physical gates, the JC4880 soak waiver and operator-closed S2 scope are unchanged.

The next exact-image bounded smoke must exercise loop exit and CUE/PLAY under
the same mixed-rate MT workload, keep pre-validation failure capture, and
retain separate operator audio/display/control evidence. No long soak or
public publication follows from this software correction alone.
