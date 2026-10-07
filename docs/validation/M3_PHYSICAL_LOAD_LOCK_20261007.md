# M3 physical FLX4/touch LOAD LOCK — 2026-10-07

## Image and method

Clean installed source `082d2c0abda01f3406fef3f35f433415c69521bd`,
`M3-dev-g082d2c0abda0`, board m3 / main-deck-m3, VALID ota_1.
ELF SHA-256 `5b6148c686766314ef7aa983e87ca8572ff4107bd82c5265d75f07e30660c83a`.
No firmware, setting, cue record or controller profile was changed.

The operator selected a different library track without loading it. Each deck
was tested separately, with the other stopped. During a bounded 90-second
playing window, the operator attempted ordinary FLX4 LOAD for the playing
deck, followed by that deck's Library touch LOAD LOCK button. The helper used
the existing marked web transport endpoint only to start and finally stop;
it did not inject the physical LOAD actions. It observed status about once
per second and refused changed titles or unexpected transport/load states.

## Results: PASS for the exercised playing-deck refusals

| Deck | Playing track | Duration / samples | Physical result |
| --- | --- | --- | --- |
| D1 | France Gall — Ella, elle l'a | 90.197 s / 83 | Both attempts rejected; operator confirmed unchanged playback and clean sound |
| D2 | Jura Stublić & Film — Srce na cesti | 90.277 s / 83 | Both attempts rejected; operator separately confirmed clean, uninterrupted sound |

Every captured sample retained both titles, the target playing in PLAYING
state and the other deck stopped. These periodic observations do not measure
sub-second audio continuity; the operator listening confirmations are separate.
For each window, deltas were zero for both PCM underrun counters, UAC dropped
blocks, overflow frames, packet failures/lost frames, underflow frames,
service-log drops and output-late count. Existing idle underflow counts are
not represented as zero cumulative counts. Both final snapshots show both
decks stopped, with the original tracks retained.

Artifacts: `.cache/m3-migration/20261007-082d2c0a/physical-load-lock/deck{1,2}/`:
firmware, before/playing/final snapshots, status.jsonl, counter-deltas.json,
raw result.json and separate operator-verification.json. Raw helper results
retain their operator-confirmation-pending wording; the subsequent operator
records establish the physical observations without rewriting raw evidence.
Console logs: `D:/AI/m3-physical-load-lock-d{1,2}.log`.

## Limits

This closes the tested ordinary MIDI/touch LOAD refusal on each playing deck.
Stopped-deck physical LOAD success, Shift variants, lock OFF, simultaneous
physical LOAD/PLAY races and cold-power/reconnect Campaign A were not exercised.
This is not a new dual-deck waveform/audio soak, other-board qualification,
current-source CI PASS or production release. No push/public OTA was performed.
