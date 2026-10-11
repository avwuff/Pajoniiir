# Shared P4 final development handoff

Date: 2026-10-11. This report supersedes previous next-action and current-bench
statements in dated handoffs. Their exact-image measurements remain valid history.
The integration is ready for PR reconciliation, not an unrestricted production
release or completion of every original physical acceptance criterion.

## Current device and artifacts

Canonical repository is `dvucinozd/Pajoniiir`, worktree
`D:/AI/Pajoniiir-shared-p4-core`, branch `codex/shared-p4-core-m3`.
The current reachable bench is **JC4880**, installed clean source
`e22ff334e01f7e9da6515970ea6ab8e9cef86073`, `M2.5-209-ge22ff334`,
project `main-deck-p4`, VALID `ota_1`, ELF SHA-256
`5d63acf5665091a82be61110769aed4b0a464b8c2a19dcdd40497df80f9e8558`.
Hercules is controller FS/root1, storage HS/root0, library 260 and Wi-Fi ON.
Both decks ended stopped after the accepted functional block. The
[JC4880 FIFO/acceptance record](../validation/JC4880_HERCULES_FIFO_20261011.md)
contains signed artifact hashes, local OTA and exact physical test scopes.

The last verified M3 installation remains source
`6166d93627304895857b626e719032de3b4f884d`,
`M3-dev-g6166d9362730`, project `main-deck-m3`, VALID `ota_1`.
ELF SHA-256 is `b0778c33f32757a80c59630cd9fd2426600da51167d0c4eb2678ff8f31aea448`.
Subsequent controller-root FIFO repairs were built and qualified locally;
their final `722bb976` images have not been flashed on either bench.

That M3 controller bench retains **Hercules USB3/root0, music stick USB2/root1**,
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
passes on clean `722bb9762bbfc1010b1faf437797bd33fe3e0360`: fresh ESP-IDF 6.0.2
builds on `95c1befb`, followed by incremental builds of the root-admission
repair on `722bb976`. One BSP, common-component isolation, unchanged locks,
source/project identity, linked DWC/SD wrappers, resampler checks and budgets
pass for JC4880, JC1060 and M3. Image sizes are
2,608,240 / 2,666,912 / 2,601,600 bytes.
Three product UI simulators pass 21/21/26 exact captures with no baseline edits.
The complete required-fixture host qualification on `722bb976` exits 0 without
SKIP, including JC4880 and M3-HS production stream policies and eight FIFO
configurations. UI source is unchanged from the freshly tested `95c1befb`.
Linux ASan/UBSan retains its preceding protocol-review source/scope; it was not
rerun for this FIFO follow-up. Earlier nine-variant simulator and
both keylock-policy qualifications retain their own source/configuration scope.
The last M3 controller bench build is separately verified; its generated config
header equals the accepted e16ecfb5 Hercules header byte-for-byte.

| Findings | Software state | Physical limits still relevant |
| --- | --- | --- |
| P01/P08 OTA and startup | Board/project isolation, signed packages, readiness and injected rollback checks implemented | Latest upload/VALID startup passed; negative/rollback history belongs to its tested images |
| P02/P04/P06 library/cues | Shared parsers, source/local merge, tombstone/restore, media identity and corrected real-export imports | Selected earlier cue/import lifecycle passed; wider exports/cross-media/legacy migration apply are separate |
| P03 LOAD LOCK | One semantic admission path and asynchronous recheck | Earlier FLX4/touch/Hercules refusal passed; simultaneous physical LOAD/PLAY race and lock OFF remain unrun |
| P05 phase/downbeat | PQTZ phase/downbeat regression coverage | Independent physical phase/downbeat review remains unrun |
| P07 transport/DSP | Shared seek/decoder loops/CUE/jog/DSP with production tests | Named earlier MT/manual-loop scopes passed; full EOF/seek/removal matrix is not implied |
| P09 qualification | Three-target workflows, required fixtures and final local matrix | Current hosted CI skipped by operator quota; JC4880 named short blocks accepted, JC1060 physical checks NOT RUN |
| P10 delivery/docs | Canonical docs, tools, provenance, signed artifacts and hashes | No push/merge/publication in this handoff; acceptance is configuration/image specific |

## Exact-image physical acceptance

### JC4880 current bench

Installed `e22ff334` passed a 30.516-s / 54-sample smoke with 2424 MIDI packets,
all 22 strict deltas zero and no output-late event. Operator confirmed GUI,
controls, MAIN and waveforms; headphone sound was subsequently confirmed after
enabling the previously off PFL.

The combined functional block completed 60.140 s / 94 samples with 546 MIDI
packets, all 22 strict deltas zero and no output-late event. Both decks used MT,
D1 -5% / D2 +5%, 44.1/48-kHz sources, initial four-beat loops and zoom 8.
PCM5102A MAIN/Hercules UAC1 output was 44.1 kHz. Automated loop exit and seek
were verified; samples directly record CUE stops and PLAY resumes on each deck.
The operator confirmed every named step, PFL/native Mix/Volume, both clean audio
paths and fluid waveforms. Both decks ended stopped. This is short functional
acceptance, not an hour soak, all transport cases or acceptance of `722bb976`.

### M3 last verified installation

The M3 `6166d936` image passed a 30.094-s / 51-sample dual mixed-rate smoke
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
| M3 Hercules 6166d936 | OTA/startup, GUI/touch and short audio/display plus active jog/Browse smoke PASS for named scope |
| Historical Hercules e16ecfb5 | Exact-image hour PASS, 15 output-late events / maximum 12,534 us; broader pad/FX/LED scope remains PARTIAL/NOT RUN |
| S2 a9f66bcd | Operator-closed with explicit waivers, accepted historical 48/44.1-kHz hours; no further S2 tests requested |
| Historical FLX4/M3 fb5 and later feature images | Recorded waveform/soak/cue/LOAD/wake acceptance retains its exact image/configuration scope |
| JC4880 e22ff334 / Hercules | OTA/startup, GUI/touch, short active-control/audio/display and 60-s MT/loop/seek/CUE/PFL scope PASS; long soak SKIPPED BY OPERATOR / NOT RUN; wider board acceptance open |
| JC1060 | Local software/build PASS; physical acceptance NOT RUN, deferred to contributor Kayrosen because the operator has no board available |
| Final 722bb976 matrix images | Local software/build PASS; not installed or physically accepted |
| Physical Campaign A | Operator-excluded cycles remain NOT RUN; software reboot supplement is not physical PASS |

First PLAY merely waking the screensaver is intentional; the operator confirmed
that was the reported behavior. Two earlier D1 underruns (518 frames total),
storage remounts and recovery history are preserved. The new smoke/supplement
did not increase them, but their cause remains unproven. Neither a causal repair
nor a zero-since-boot-loss claim is made. Historical OTA cable intervention is
closed as operator context, without claiming a firmware causal fix.

## Explicit remaining boundaries

- Hosted CI is SKIPPED BY OPERATOR (quota), not current CI PASS.
- JC4880 named short checks are accepted on `e22ff334`. On 2026-10-11 the
  operator explicitly waived its long soak: SKIPPED BY OPERATOR / NOT RUN,
  not PASS. Its wider topology matrix remains open.
- JC1060 hardware is unavailable to the operator. Physical checks are deferred
  to contributor Kayrosen and remain NOT RUN until exact-image evidence is
  recorded; no contact or testing by that contributor is implied here.
- Real nested playlist order, artwork stale-media, source memory-loop/PWV4,
  independent downbeat and cross-media variants require suitable exports.
  The [2026-10-12 library audit](../validation/SHARED_LIBRARY_EXPORT_AUDIT_20261012.md)
  adds real flat-playlist/PWV4/phase comparison and delayed-artwork/cross-export
  cue software regressions, while retaining missing/corrupt source files.
  The operator deferred the remaining physical/export-dependent checks.
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

The requested JC4880 short functional block and final three-target software
follow-up are complete for their named scopes. The operator subsequently
authorized README reconciliation, PR #46 push/merge, a JC1060 contributor kit
and GitHub prerelease `shared-p4-beta.1`. The
[beta delivery record](../validation/SHARED_P4_BETA_1_20261011.md) tracks fresh
builds, exact source/configuration, packaging and publication. Preserve the
experimental topology and all acceptance boundaries above. Beta publication
does not authorize changing stable public OTA channels or transfer historical
physical acceptance to newly built assets.
