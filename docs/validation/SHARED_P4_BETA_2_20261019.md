# Shared P4 beta 2 delivery record

Date:2026-10-19. Distribution: `shared-p4-beta.2`, a contributor prerelease.
Status: preparation in progress; publication has not yet been verified.

## Frozen board images

This distribution deliberately retains the already built images. It does not
rebuild physically accepted firmware merely to assign a common version. Each
board has its own embedded version, exact source SHA and independent evidence;
the distribution tag identifies the delivery record, not a common build SHA.
Production, test and tool sources are identical across the three build commits.

| Board | Project / embedded version | Source | Application bytes | Exact-image acceptance |
| --- | --- | --- | ---: | --- |
| M3 | `main-deck-m3` / `M3-dev-g1556e8d7dfcb` | `1556e8d7dfcb4f016c52ab1a372dd2e076dcaa1f` | 2,603,312 | Installed VALID ota_1; short loop/control/CUE checks and3600.286-s hour physically confirmed;24 late events/max12,988 us and sample-observed3/60-s cluster remain investigation findings |
| JC4880 | `main-deck-p4` / `M2.5-249-g0a33ec1f` | `0a33ec1fb16c0136dccaa85ca63a6fbc00a4d527` | 2,609,952 | Installed VALID ota_0;60.234-s mixed-rate MT/loop/CUE/control/audio/display smoke physically confirmed, all22 strict deltas/late0; long soak waived |
| JC1060 | `main-deck-jc1060` / `M2.5-246-g2f2b0555` | `2f2b05558ccb68238d7228f59e52cd6c7d06e12f` | 2,668,480 | Local software/build verified; physical installation, startup/network/storage/audio acceptance NOT RUN, deferred to Kayrosen |

[M3 hour and short scopes](M3_MT_FALLBACK_HANDOFF_20261018.md),
[JC4880 installation/smoke](M3_SOAK_CLOSEOUT_JC4880_OTA_20261019.md) and
[three-board software matrix](evidence/2026-10-18-shared-cache-builds/matrix.json)
retain the exact identities. Existing full host Qualification is on identical
production/test code. These are retained incremental IDF6.0.2 builds, not new
fresh clean builds. Dependency locks, BSP/project isolation, signatures, image
descriptors, ELF matches and0x380000 budgets must verify during assembly.
Hosted CI remains NOT RUN under the operator's quota waiver; source pushes use
`[skip ci]` rather than spending more Actions quota. No hosted-CI PASS is implied.

## Changes since beta 1

- Restore the FLX4 dedicated-headphone signal by removing the unintended12.041-dB
  transport attenuation, preserving legacy USB MAIN mirror behavior.
- Re-anchor Master Tempo to the actual consumer clock when ordinary-resampler
  fallback advances playback, preventing a reproduced stale-clock rewind.
- Bound task-context cache maintenance in ordinary P4 builds using the existing
 1-KiB policy, with range/alignment/error and DMA ownership semantics retained.
- Remove unbounded largest-block heap walks from ordinary diagnostic snapshots;
  unavailable fragmentation/walk fields remain explicit nulls.
- Include the reviewed shared controller/lifecycle and JC1060 integration
  repairs, including DDJ-400 loop halve/double mappings. New-image JC1060
  Ethernet/HTTP startup, SD warm-reset and USB MAIN reconnect gates remain open.

## Installation and qualification boundary

Use only the signed `.ddjota` matching the board project, with stopped decks and
stable power. M3 remains ordinary FLX4 USB2 FS/storage USB3 HS; experimental
S2/Hercules role-swap firmware is not included. M3's development identity has no
automatic pull discovery. Historical main-deck-p4 M3 firmware still requires
the preserving wired identity migration before main-deck-m3 OTA.

JC4880 uses FLX4 controller root1/storage root0, PCM5102A MAIN with board-selected
rate (44.1 kHz in its accepted smoke). JC1060 uses controller root0/storage
root1, controller MAIN/cue and Ethernet, without Wi-Fi or PCM5102A pin sharing.
The [JC1060 handoff](../JC1060_BETA_HANDOFF_20261011.md) remains applicable, with
the beta2 identity taking precedence over its historical package table.

Raw BIN/bootloader/partition/otadata artifacts are recovery/provisioning material,
not signed web-OTA payloads. Inspect the actual board/layout first; do not erase
the chip or overwrite partition settings indiscriminately. Separate matching
debug archives are required for crash analysis. Public-key fingerprints must
be compared with the trusted repository key; a key included in a ZIP alone does
not establish trust. No private signing material belongs in the distribution.

S2 stays operator-closed; no additional S2 tests are requested. Wider controller,
real-export/library, physical Campaign A, JC1060 and experimental feature gates
retain their partial/unrun dispositions. M3 input pressure specifically during
loop exit and exact historical stall attribution are not closed by the accepted
hour. Stable M2.5, existing beta1 assets and public OTA channels stay unchanged.

## Delivery verification

The final assets will contain per-board signed manifests, a signed release
manifest, SHA-256 inventories, controller profiles with provenance and matching
debug archives. Kit ZIP inventories, signatures and every published/downloaded
asset must verify before publication is recorded as complete. No contributor
message or additional device installation is part of this delivery.
