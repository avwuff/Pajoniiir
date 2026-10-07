# M3 stopped-deck physical LOAD routing — 2026-10-07

## Exact image and scope

Clean installed source `082d2c0abda01f3406fef3f35f433415c69521bd`,
M3-dev-g082d2c0abda0, main-deck-m3 / m3, VALID ota_1;
ELF `5b6148c686766314ef7aa983e87ca8572ff4107bd82c5265d75f07e30660c83a`.
Both decks stayed stopped. The operator selected library rows and issued the
physical LOAD actions. Read-only API snapshots verified READY, changed target
title and unchanged other-deck title. No web LOAD was used in this test.

## Exercised cases: PASS

| Action | Before D1 / D2 | After D1 / D2 |
| --- | --- | --- |
| Ordinary FLX4 LOAD D1 | France Gall / Jura Stublić | TAINTED DUB - CLIP / Jura Stublić |
| Touch LOAD DECK 2 | TAINTED DUB - CLIP / Jura Stublić | TAINTED DUB - CLIP / TAINTED DUB - CLIP |
| FLX4 SHIFT + LOAD D1, repeated attempt | TAINTED DUB - CLIP / TAINTED DUB - CLIP | Star Eater / TAINTED DUB - CLIP |
| FLX4 SHIFT + LOAD D2 | Star Eater / TAINTED DUB - CLIP | Star Eater / Star Eater |

The operator confirmed each targeted change and preservation of the other
deck. Final state is both READY/stopped with Star Eater. Resources report
startup ready, allocation failures 0 and critical allocation failures 0.
The first Shift+Load D1 observation retained both original titles despite the
operator confirmation. It is INCONCLUSIVE, not a passing change or a proven
firmware failure; the selected row name was not captured. The raw snapshot
was preserved. A repeated physical attempt changed only D1 as shown above.

Artifacts: `.cache/m3-migration/20261007-082d2c0a/physical-load-routing/`:
initial-status.json, firmware.json, per-action status/verification records,
both Shift D1 snapshots, and final-resources.json.

## Limits

Ordinary stopped FLX4 LOAD D2 and stopped touch LOAD D1 were not exercised
in this series. Shift+Load refusal during playback, explicit contrary browse
target selection, lock OFF and simultaneous physical LOAD/PLAY races remain
NOT RUN. Ordinary playing-deck refusals on both FLX4/touch paths are recorded
separately in [the LOAD LOCK test](M3_PHYSICAL_LOAD_LOCK_20261007.md).
No audio listening, waveform soak, power/reconnect campaign, other-board or
new-controller qualification is inferred. No firmware build/OTA, push or
public release was performed.
