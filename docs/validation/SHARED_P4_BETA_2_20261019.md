# Shared P4 beta 2 delivery record

Date:2026-10-19. Distribution: `shared-p4-beta.2`, a contributor prerelease.
Status: **published and independently verified** on2026-10-19 (Europe/Zagreb).

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

The final assets contain per-board signed manifests, a signed release
manifest, SHA-256 inventories, controller profiles with provenance and matching
debug archives. Kit ZIP inventories, signatures and every published/downloaded
asset verifies before publication is recorded as complete. No contributor
message or additional device installation is part of this delivery.

## Verified publication

[shared-p4-beta.2](https://github.com/dvucinozd/Pajoniiir/releases/tag/shared-p4-beta.2)
is published, non-draft, explicitly prerelease and `make_latest:false`.
Release ID:417044096; published2026-10-19T08:08:54Z.
The annotated tag object9ed56d5521fe7c818bb67fe5ec805755b226eff0 resolves to
distribution/documentation source `d259a71ae716217a6448c374bf52c2c798d67ce8`.
All three per-board build commits are ancestors of that pushed source; the
signed release manifest records their separate identities and common
firmware/test/tool/profile tree hashes. No firmware was rebuilt for publication.

Sixteen assets are public: three signed OTA bundles, three board kits, three
matching debug ZIPs, six-profile ZIP, public DER key, release manifest/signature,
release notes and SHA256SUMS/signature. Every ZIP's full inventory is checked,
222 entries in total. All six profile JSON files recompile byte-identically to
their supplied S3CP fixtures. Each kit retains module-specific notices and
recovery flash metadata, checked against the included recovery files.
Only complete PEM private-key payloads are rejected by the artifact scan;
standalone TLS parser delimiter strings in the binary are not private keys.

All16 draft assets were downloaded and verified before publication. All16
published assets were then independently downloaded without GitHub authentication
from their public URLs and matched byte-for-byte by SHA-256. Release-manifest,
SHA256SUMS, per-board manifest and bundle signatures verify against the trusted
repository key; each image/project/source/version/debug ELF match verifies.
No artifact was overwritten during assembly or after publication.

Stable latest remains M2.5, release ID404235364, with unchanged asset IDs,
sizes and digests. Beta1's existing asset IDs/sizes/digests also match the
before-publication snapshot. The public
`https://ota.pajoniiir.eu/latest.json` bytes match exactly before/after; no OTA
channel was written. Current hosted CI remains NOT RUN under the explicit quota
waiver; the `[skip ci]` push did not create a new run. This is not a CI PASS.
All20 original README image/embed references are retained. Local documentation
integrity passes147 Markdown/HTML files and134 retired-path checks before
publication; the final documentation bookkeeping receives a separate check.
README and user-manual source links are reconciled with beta2. A new Pages
deployment is NOT RUN under the Actions quota waiver; editing its source does
not claim that the live manual site has redeployed.

The [publication audit](evidence/2026-10-19-shared-p4-beta2/publication-audit.json)
and adjacent signed manifest/hash inventory retain the immutable identities,
public-download audit and stable/beta1/OTA snapshots. Full binaries and matching
debug files are the GitHub release assets; local originals also remain in
`.cache/shared-p4-beta.2/assembly-v3/assets/`. Earlier incomplete assembly
attempts remain local and were not uploaded. The temporary exact-source JC1060
packaging checkout is removed after its source/artifact checks; no contributor
message or new device installation occurs during publication.
