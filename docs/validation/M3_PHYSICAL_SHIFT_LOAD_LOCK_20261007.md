# M3 physical Shift+Load protection — 2026-10-07

## Exact image and selected replacement

Clean installed source `082d2c0abda01f3406fef3f35f433415c69521bd`,
M3-dev-g082d2c0abda0, main-deck-m3 / m3, VALID ota_1;
ELF `5b6148c686766314ef7aa983e87ca8572ff4107bd82c5265d75f07e30660c83a`.
No firmware or settings change was needed for these attempts.

For the repeated tests, both loaded tracks were Combat Ready. The operator
explicitly named Comanchero as the different selected library row before
playback. Each deck was tested separately in a guarded 90-second window with
the other stopped. The operator held SHIFT, pressed the target FLX4 LOAD once
and released the controls while that target played.

## Repeated attempts

The operator confirmed on both D1 and D2 that Shift+Load was refused, the
loaded track remained Combat Ready and sound stayed clean and uninterrupted.
The D1 helper completed 90.203 s / 83 samples, retaining both titles and
target playback until automatic STOP. D2 completed 90.431 s / 83 samples with
the same title/state continuity. Both repeated attempts are PASS for the
exercised playing-deck refusal. Both final snapshots show both decks stopped.
Each window's deltas are zero for both PCM underrun counters, UAC dropped
blocks, overflow, packet failures/lost frames, underflow, service-log drops
and output-late count. Historical idle underflow counts are not cumulative
zero-loss claims. Raw helper results retain their pending-operator wording;
the separate subsequent operator-verification records complete the evidence.

## Initial disagreement preserved

In the earlier D1 attempt the operator reported a changed track. All captured
titles nevertheless remained Combat Ready on both decks; D1 position advanced
continuously while playing. The assistant then explicitly stopped playback
in response to the report. The helper's subsequent transport assertion failed
because playback was stopped; this raw FAIL is retained and is not attributed
to a firmware LOAD interruption. The report versus loaded-title discrepancy
was not resolved, so the initial physical attempt remains INCONCLUSIVE.
The repeated attempts have their own evidence and do not overwrite it.

Artifacts: `.cache/m3-migration/20261007-082d2c0a/physical-shift-load-lock/deck1/`
for the first attempt, including operator-failure snapshots and raw result;
`physical-shift-load-lock-repeat1/deck{1,2}/` for repeats, including firmware,
before/baseline/last/final snapshots, status.jsonl and operator verification.

## Limits

Periodic status proves only sampled title/state continuity; operator listening
is the separate physical evidence. This is not a dual-deck waveform/audio soak,
lock-OFF test, physical LOAD/PLAY race, cold-power/reconnect campaign or other
board/controller qualification. Current-source GitHub Actions remains skipped
by operator. No build/OTA, push or public release was performed.
