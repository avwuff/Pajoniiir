# Share the P4 core across JC4880, JC1060 and M3

Description for [PR #46](https://github.com/dvucinozd/Pajoniiir/pull/46),
reconciled with the final implementation and authorized beta delivery.

## Problem and resulting behavior

M3 previously maintained a separate port with missing library, cue, transport,
OTA identity and controller behavior. JC1060 also depended on components inside
another board project. The three entrypoints now link one canonical P4
audio/deck/library/controller/UI/web/OTA implementation, with board providers
owning pins, peripherals, output policies and display/USB topology.

M3 retains its accepted DSI506/FT5426 geometry, PCM5102A MAIN, Wi-Fi default and
waveform scheduling policy. Ordinary M3 defaults remain controller USB2 FS and
storage USB3 HS. The separately accepted Hercules/S2 role configuration is an
experiment; its hardware evidence does not qualify the ordinary packages.

## Main changes

- Extract shared components/startup/CMake contracts into `firmware/p4-core`,
  add isolated providers under `firmware/boards`, and add `main-deck-m3`.
- Integrate M3 PCM ownership/FIR/cache/DSP fixes with common decoder seek/loop,
  LOAD LOCK and semantic MIDI/touch/web actions.
- Share PDB/ANLZ, playlist/artwork, memory/source/local cues and persistent media
  identity; preserve legacy cues for explicit export-bound offline migration.
- Isolate signed board/project/version identities and requested startup
  readiness; retain preserving factory/cue migration and debug/crash provenance.
- Share profile formats v2/v3/v4, USB lifecycle/epoch/recovery contracts,
  diagnostics and bounded experimental modules. Document controller limits.
- Add required fixtures, board isolation/budget/lock checks, simulators and
  three-target workflows. Fix missing shared idle source in simulator linking.
- Admit Hercules's 540-byte FS audio packet using the real controller-root
  FIFO policy, with captured JC4880 and M3-HS production stream regressions.
- Restore the USB spike's shared HAL dependencies, bus-trace source/include and
  wrapper links after its historical missing-header CI failure.
- Reconcile the English README while preserving all images and operational
  instructions; prepare board-specific beta assets and the JC1060 handoff.

## Validation

Latest completed clean software matrix:
`722bb9762bbfc1010b1faf437797bd33fe3e0360`.
The beta record identifies its own exact pushed release source and fresh builds.

- Full host `-Qualification` PASS without optional SKIP; actual committed
  MP3/WAV/FLAC/PDB/ANLZ fixtures.
- Earlier nine simulator configurations and both keylock policies passed on
  `5ff76b6c`; three product simulators passed on `95c1befb`. Their UI/adapter
  source is unchanged in `722bb976`; no new physical UI claim is made.
- Fresh ordinary ESP-IDF 6.0.2 builds passed on `95c1befb`, followed by corrected
  incremental `722bb976` builds and full required-fixture host qualification:
  unchanged locks, one matching BSP, canonical graph, clean identities/budgets,
  eight root/FIFO policies and both captured production stream regressions.
- Exact sizes/hashes and provenance:
  [final matrix](../validation/SHARED_P4_FINAL_BUILD_MATRIX_20261011.md).
- JC4880 `e22ff334` OTA/startup, 30-s active-controller smoke and 60-s mixed-rate
  MT/loop/seek/CUE/PFL block passed all 22 strict deltas with operator audio/UI
  confirmation. Its long soak is explicitly waived, NOT RUN rather than PASS.
- Historical exact-image physical evidence is scoped independently. Installed
  Hercules M3 `e16ecfb5` completed 3600.016 s, 22 strict deltas zero, operator
  full-hour audio/visual PASS; 15 output-late events remain monitoring findings.

## Migration, risks and release boundary

Existing shared-identity M3 uses signed `main-deck-m3` OTA. Historical M3
`main-deck-p4` needs the preserving wired identity migration; the 16-MiB layout
and NVS remain intact. Legacy cue records are never automatically bound to an
unrelated export. Migration instructions and licence/provenance handoff are in
[delivery readiness](2026-10-11-shared-core-delivery-readiness.md).

Current-source hosted CI is not claimed PASS; its earlier quota waiver and
historical spike failure remain distinct from local verification. JC1060
physical acceptance is NOT RUN, deferred to Kayrosen. New-image, experiment and
wider real-media/transport/controller qualification remain open.
The operator clarified that the historical OTA restart
was associated with moving the cable; it is closed as a cable-intervention
event, not an outstanding firmware defect or a firmware-fix claim. S2 work remains
operator-closed; Hercules individual pad/FX/LED scope is partial. This review
does not reopen accepted controller campaigns or establish whole-core completion.
The operator authorized push/merge and `shared-p4-beta.1` as a GitHub prerelease,
with ordinary board-specific signed packages and the JC1060 contributor kit.
No existing release tag/asset or public OTA channel is changed. Exact provenance
and publication results: [beta record](../validation/SHARED_P4_BETA_1_20261011.md).
