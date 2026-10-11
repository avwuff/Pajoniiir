# M3 on-device Hot Cue lifecycle — 2026-10-07

## Exact image and scope

Installed source: `082d2c0abda01f3406fef3f35f433415c69521bd`, clean,
`M3-dev-g082d2c0abda0`, `main-deck-m3`, board `m3`, VALID `ota_1`.
ELF SHA-256: `5b6148c686766314ef7aa983e87ca8572ff4107bd82c5265d75f07e30660c83a`.
The unchanged medium contains 324 tracks. Wi-Fi remains enabled.

The operator explicitly confirmed that D1's France Gall track had no local
edits to preserve and authorized the lifecycle test ending with HOLD RESTORE.
All mutations use the production touch Hot Cue path; the status API cannot
mutate or enumerate Hot Cue slots. Both decks remain stopped. D1 is track 7,
France Gall — Ella, elle l'a; D2 is track 317, Jura Stublić & Film — Srce na cesti.

## Observations

| Stage | Outcome and evidence |
| --- | --- |
| Source recall | Operator confirmed A–D recall; stopped API position 123347 ms matches source D from EXT PCOB. |
| Local set and reboot | PASS for persistence: operator set a local D while retaining A–C, then confirmed recall after guarded reboot/reload. Two API snapshots reported 31058 ms. |
| Exact set position | NOT VERIFIED: preparation was 30000 ms but no snapshot exists at the physical set event. Operator clarified D was always about 31 s. Do not claim an exact 30000 ms round trip or infer a seek defect from this discrepancy. |
| Deletion and reboot | PASS: operator deleted D, then confirmed it remained empty after guarded reboot/reload, with A–C retained. Empty D was not pressed to create a replacement. |
| Restore source and reboot | PASS: operator confirmed D returned after HOLD RESTORE and remained available after reboot/reload. Final stopped API position is exactly 123347 ms; source A–C were retained. |

The reboot helpers verified the same source/ELF, VALID slot, OTA idle,
controller MIDI In/Out/UAC, library count, stopped reloads and startup ready.
All three phase helpers exited 0, with zero allocation failures.
Their `PASS_REBOOT_RELOAD` result alone does not establish cue persistence;
the separate operator observations above provide slot evidence.

## Evidence and remaining limits

Artifacts: `.cache/m3-migration/20261007-082d2c0a/cue-lifecycle/`, including
per-phase `result.json`, firmware/status/resources captures, and separate
operator verification records. Override captures retain both 31058 ms readings
and the later explicit seek-to-zero preparation; raw records are not replaced.
Final `restore/recall-status.json` and `restore/final-firmware.json` confirm
both decks stopped and the same clean source in VALID ota_1. HOLD RESTORE
removed the test track's local edits; its bank now uses the original source
cues. No local test override or deletion tombstone remains on this track.

Physical power loss during an NVS write, two real export namespaces, physical
Campaign A and MIDI/touch playing-deck LOAD LOCK remain unrun. Soft reboots do
not count as cold-power/reconnect cycles. This test does not transfer the old
fb5 waveform/audio soak to this image or qualify other boards/controllers.
No firmware modification, remote push, release or public OTA publication is
part of this lifecycle check.
