# Shared P4 final development handoff

Date: 2026-10-11. This report supersedes previous next-action and current-bench
statements in dated handoffs. Their exact-image measurements remain valid history.
The integration is ready for PR reconciliation, not an unrestricted production
release or completion of every original physical acceptance criterion.

## Current device and artifacts

Canonical repository is `dvucinozd/Pajoniiir`, worktree
`D:/AI/Pajoniiir-shared-p4-core`, branch `codex/shared-p4-core-m3`.
Installed clean firmware source is `6166d93627304895857b626e719032de3b4f884d`,
`M3-dev-g6166d9362730`, project `main-deck-m3`, VALID `ota_1`.
ELF SHA-256 is `b0778c33f32757a80c59630cd9fd2426600da51167d0c4eb2678ff8f31aea448`.
Subsequent handoff commits are documentation only.

The controller bench retains **Hercules USB3/root0, music stick USB2/root1**,
normal Wi-Fi, PCM5102A MAIN 48 kHz and Hercules UAC1 FS 44.1 kHz packed-24.
This configuration differs from ordinary FLX4 USB2/controller and USB3/storage.
The catalog has 260 tracks. Both decks are stopped after testing.

The [candidate and OTA record](../validation/M3_FINAL_CONTROLLER_CANDIDATE_20261011.md)
contains the immutable BIN/bundle/configuration hashes, signature checks and
debug archive. Application: 2,605,008 bytes; signed bundle: 2,605,196 bytes,
SHA-256 `c1e9861778a83dc7c7fb427f45685631ff72b17523ab56174c8d207b3d6e2ce4`.
The bundle is a local development candidate, not a public pull-update release.
Both deterministic controller profile binaries are included separately; firmware
OTA did not replace installed profiles. Existing public tags/assets are unchanged.

## What is implemented and verified in software

All three targets use common production components in `firmware/p4-core`,
board providers in `firmware/boards`, and thin entrypoints. Audio/deck state,
library parsers, persistent cues, semantic actions, controller profiles, UI/web,
OTA and diagnostics have canonical implementations. M3 preserves its accepted
BSP/display/touch/audio/USB policies through board contracts.

The [phase/P01-P10 review](2026-10-11-shared-core-hercules-closeout.md) describes
the implementation and intentional limits; the
[delivery ledger](2026-10-11-shared-core-delivery-readiness.md) records provenance,
migration and licensing. The subsequent manual protocol review completed all
45 tracked common protocol files and repaired four reproduced defects with
failing-before/passing-after regressions. CodeRabbit service review was blocked;
the operator explicitly authorized manual review. Deferred review dispositions
remain documented and are not silently converted to completed repairs.

The [latest ordinary matrix](../validation/SHARED_P4_FINAL_BUILD_MATRIX_20261011.md)
passes on clean `b913818052de99af61c3a8c752b814185794b50e`: clean ESP-IDF 6.0.2
builds, one BSP, common-component isolation, unchanged locks, source/project
identity, linked DWC/SD wrappers, resampler checks and budgets for JC4880,
JC1060 and M3. Image sizes are 2,608,224 / 2,666,912 / 2,601,600 bytes.
Three product UI simulators pass 21/21/26 exact captures with no baseline edits.
Complete required-fixture host qualification without SKIP and Linux ASan/UBSan
passed in the preceding protocol review. Earlier nine-variant simulator and
both keylock-policy qualifications retain their own source/configuration scope.
The current controller bench build is separately verified; its generated config
header equals the accepted e16ecfb5 Hercules header byte-for-byte.

| Findings | Software state | Physical limits still relevant |
| --- | --- | --- |
| P01/P08 OTA and startup | Board/project isolation, signed packages, readiness and injected rollback checks implemented | Latest upload/VALID startup passed; negative/rollback history belongs to its tested images |
| P02/P04/P06 library/cues | Shared parsers, source/local merge, tombstone/restore, media identity and corrected real-export imports | Selected earlier cue/import lifecycle passed; wider exports/cross-media/legacy migration apply are separate |
| P03 LOAD LOCK | One semantic admission path and asynchronous recheck | Earlier FLX4/touch/Hercules refusal passed; simultaneous physical LOAD/PLAY race and lock OFF remain unrun |
| P05 phase/downbeat | PQTZ phase/downbeat regression coverage | Independent physical phase/downbeat review remains unrun |
| P07 transport/DSP | Shared seek/decoder loops/CUE/jog/DSP with production tests | Named earlier MT/manual-loop scopes passed; full EOF/seek/removal matrix is not implied |
| P09 qualification | Three-target workflows, required fixtures and local matrix | Current hosted CI skipped by operator quota; JC physical targets not available |
| P10 delivery/docs | Canonical docs, tools, provenance, signed artifacts and hashes | No push/merge/publication in this handoff; acceptance is configuration/image specific |

## Exact-image physical acceptance

The new installed image passed a 30.094-s / 51-sample dual mixed-rate smoke
with all 22 strict deltas zero and output-late delta zero. Operator confirmed
normal GUI/touch, clean MAIN/headphones and fluid waveforms. Controls were not
rotated during that window and were initially NOT RUN.

The authorized supplement completed **20.531 s / 35 samples**, with **2421 MIDI
packets**, all 22 strict deltas present and zero, output-late delta zero and both
decks finally stopped. The operator confirmed both jogs/Browse responded as
expected, both audio outputs were clean and waveforms were normal. This closes
the short active-control smoke gap. Workload was pitch zero without MT/loops;
this is not a full new-image MT/long-soak/profile qualification.

Evidence roots are `.cache/final-m3-candidate-6166d936/ota-20261011T183237Z/`,
`smoke-30s-20261011T202456Z/` and `controls-20s-20261011T202900Z/`.
The supplement retains all samples, exact firmware, final stopped/UI/USB
snapshots, runner, raw diagnostic log, operator confirmation and SHA-256 inventory.

| Hardware/campaign | Disposition |
| --- | --- |
| Current Hercules 6166d936 | OTA/startup, GUI/touch and short audio/display plus active jog/Browse smoke PASS for named scope |
| Historical Hercules e16ecfb5 | Exact-image hour PASS, 15 output-late events / maximum 12,534 us; broader pad/FX/LED scope remains PARTIAL/NOT RUN |
| S2 a9f66bcd | Operator-closed with explicit waivers, accepted historical 48/44.1-kHz hours; no further S2 tests requested |
| Historical FLX4/M3 fb5 and later feature images | Recorded waveform/soak/cue/LOAD/wake acceptance retains its exact image/configuration scope |
| JC4880/JC1060 new shared images | Local software/build PASS; physical acceptance NOT RUN |
| Physical Campaign A | Operator-excluded cycles remain NOT RUN; software reboot supplement is not physical PASS |

First PLAY merely waking the screensaver is intentional; the operator confirmed
that was the reported behavior. Two earlier D1 underruns (518 frames total),
storage remounts and recovery history are preserved. The new smoke/supplement
did not increase them, but their cause remains unproven. Neither a causal repair
nor a zero-since-boot-loss claim is made. Historical OTA cable intervention is
closed as operator context, without claiming a firmware causal fix.

## Explicit remaining boundaries

- Hosted CI is SKIPPED BY OPERATOR (quota), not current CI PASS.
- JC physical topology/display/audio checks need their hardware and operator.
- Real nested playlist order, artwork stale-media, source memory-loop/PWV4,
  independent downbeat and cross-media variants require suitable exports.
- Broader transport/active-removal/race/FAT/throughput and individual controller
  LED/held-state/pad/FX checks retain the partial or unrun ledger dispositions.
- Recorder, alternate DJ UI, M3 Ethernet and Pro DJ Link stay experimental or
  disabled in production defaults; software does not prove physical interoperability.
- No new hour soak was performed on 6166d936. Earlier hours do not transfer.
- S2 waivers are NOT RUN, not unrestricted controller production acceptance.
- Legacy cue migration applies only when explicitly needed with the selected old
  export; ambiguous records remain archived. No new migration was performed.

These boundaries prevent declaring the original complete shared-core acceptance
criterion met. They do not require repeating already accepted historical blocks
without changed assumptions or a reproduced defect.

## Next delivery step

Reconcile the PR title/body with the final implementation and this handoff;
preserve the deliberate experimental topology, exact hashes and open gates.
Push/merge requires the user's next authorized scope. Public release/channel
publication requires separate authorization and the relevant board acceptance.
This report and its accompanying document updates are locally committed only.
