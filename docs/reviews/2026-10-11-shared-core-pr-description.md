# Draft PR: Share the P4 core across JC4880, JC1060 and M3

This file is a prepared description for a future review; no remote PR is created.

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

## Validation

Frozen clean software/package source:
`5ff76b6cb1474779c6e58ecd5b273dcee378a3ed`.

- Full host `-Qualification` PASS without optional SKIP; actual committed
  MP3/WAV/FLAC/PDB/ANLZ fixtures.
- Nine simulator presentations PASS with unchanged baselines; JC and M3
  production keylock PC policies each PASS 300 virtual seconds.
- All three fresh ordinary ESP-IDF 6.0.2 builds PASS with unchanged locks,
  one matching BSP, canonical component graph, clean identities and budgets.
- Local signed bundles/manifests and matching BIN/ELF/MAP/debug archives verify.
  Exact sizes/hashes: [qualification record](../validation/SHARED_P4_LOCAL_QUALIFICATION_20261011.md).
- Historical exact-image physical evidence is scoped independently. Installed
  Hercules M3 `e16ecfb5` completed 3600.016 s, 22 strict deltas zero, operator
  full-hour audio/visual PASS; 15 output-late events remain monitoring findings.

## Migration, risks and release boundary

Existing shared-identity M3 uses signed `main-deck-m3` OTA. Historical M3
`main-deck-p4` needs the preserving wired identity migration; the 16-MiB layout
and NVS remain intact. Legacy cue records are never automatically bound to an
unrelated export. Migration instructions and licence/provenance handoff are in
[delivery readiness](2026-10-11-shared-core-delivery-readiness.md).

Hosted CI is SKIPPED BY OPERATOR (quota), not PASS. New candidate physical
acceptance, JC hardware, fresh experiment builds and wider real-media/transport
coverage remain open. The operator clarified that the historical OTA restart
was associated with moving the cable; it is closed as a cable-intervention
event, not an outstanding firmware defect or a firmware-fix claim. S2 work remains
operator-closed; Hercules individual pad/FX/LED scope is partial. This review
does not reopen accepted controller campaigns or establish whole-core completion.
No existing release tag/asset or public OTA channel is changed.
