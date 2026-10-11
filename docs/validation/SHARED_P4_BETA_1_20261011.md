# Shared P4 beta 1 delivery record

Date: 2026-10-11. The operator authorized README reconciliation, PR #46
push/merge, board-specific contributor packaging and a GitHub beta release.
Distribution tag: `shared-p4-beta.1` (GitHub prerelease). Existing stable M2.5
tags/assets and public OTA channels are outside this publication and remain
unchanged. No device installation or additional physical campaign is requested.

## Prepared implementation and acceptance scope

One canonical core serves JC4880, JC1060 and M3, with isolated BSPs. The
[final handoff](../reviews/2026-10-11-shared-core-final-handoff.md) and
[software matrix](SHARED_P4_FINAL_BUILD_MATRIX_20261011.md) identify the latest
software-checked `722bb976` and the separately installed bench sources.
Full host required-fixture qualification and all three local product builds
passed. UI simulator provenance is unchanged-source equivalence, not a repeated
physical display campaign.

JC4880 short functional acceptance belongs to `e22ff334`; its long soak is
SKIPPED BY OPERATOR / NOT RUN. JC1060 physical acceptance is NOT RUN, deferred
to contributor Kayrosen using the [handoff](../JC1060_BETA_HANDOFF_20261011.md).
M3 controller campaigns retain their exact source/configuration/rate boundaries;
ordinary M3 assets do not use the Hercules/S2 experimental USB port swap.
S2 remains operator-closed. None of the newly packaged beta images inherits
physical acceptance, and wider real-media/transport/controller gates remain open.

## README and review reconciliation

The README remains English, retains all 20 original image/embed references and
the reference-build, electrical, wiring, enclosure, web/update and development
instructions. It now distinguishes stable and beta distribution, all three
targets, shared features, controller limits and board-specific USB/audio policies.
The PR description is reconciled with final source, validation and explicit
waivers rather than the earlier `5ff76b6c` packaging checkpoint.

Remote inspection also found a real historical CI failure in run 38148620977:
the USB spike component reused the production HAL adapter without declaring
`esp_timer`, its bus-trace source/include or all wrapper links. Its component
declaration now includes those shared dependencies, trace implementation and
wrapper links. Its fresh local ESP-IDF 6.0.2 configure/build exits 0, producing
270,640 bytes with unchanged dependency lock. Evidence is
`.cache/shared-p4-beta.1/spike-configure.log` and `spike-build.log`.
This is a build-only spike repair; current hosted CI remains distinct from local
verification and the operator's earlier quota waiver.

## Immutable release provenance

The release assets will record their exact pushed source commit, per-board
embedded versions/project/source/ELF, dependency locks, ordinary SDKCONFIGs,
BIN/bundle sizes, signed manifests and SHA-256 inventories. Fresh builds must
finish and package verification must pass before any asset is published.
The release contains board kits plus separate debug archives and compiled
controller profiles. No private signing material is included.

## Verified publication on 2026-10-12 (Europe/Zagreb)

[PR #46](https://github.com/dvucinozd/Pajoniiir/pull/46) was updated, marked ready
and merged. Exact merge/source commit:
`40e72fcbf403a1650185a4c64509dd11bba9f73f`. The annotated distribution tag
`shared-p4-beta.1` resolves to that commit; it is outside the M2/M3 ancestry-tag
patterns. The [published release](https://github.com/dvucinozd/Pajoniiir/releases/tag/shared-p4-beta.1)
is non-draft and explicitly prerelease, with `latest=false`.

All three ordinary products completed fresh ESP-IDF 6.0.2 configure/builds from
empty build directories and dedicated SDKCONFIGs on the clean, pushed merge
source. Dependency configuration/patching was serial; independent compilation
used four workers per board. Product source and tests are unchanged from the
qualified `722bb976` baseline; the only intervening firmware change is the
auxiliary USB spike component declaration. The historical full host/UI results
are therefore recorded with source equivalence, not described as newly rerun.

| Board / project | Embedded version | Image bytes | Remaining below 0x380000 |
| --- | --- | ---: | ---: |
| JC4880 / main-deck-p4 | M2.5-215-g40e72fcb | 2,607,712 | 1,062,304 |
| JC1060 / main-deck-jc1060 | M2.5-215-g40e72fcb | 2,666,368 | 1,003,648 |
| M3 / main-deck-m3 | M3-dev-g40e72fcbf403 | 2,600,960 | 1,069,056 |

All board/project/source/component/configuration checks, unchanged locks and
image budgets pass. Every signed bundle and board manifest verifies against
the committed release public key. Application descriptor versions/projects,
clean source SHA, BIN hashes and matching debug ELF hashes were independently
checked. Ordinary M3 has no HS-controller role swap. The release-level manifest
is also signed; each board kit carries its own signed OTA manifest.

Fifteen assets were published: three signed OTA bundles, three board kits,
three matching debug ZIPs, compiled profile ZIP, public verification key,
release manifest/signature, release notes and SHA256SUMS. Every ZIP inventory
was checked against its archived contents. All fifteen published assets were
downloaded independently and SHA-256 matched to their originals; the downloaded
release-manifest signature also verifies. `jc1060-beta-kit.zip` includes the
Kayrosen handoff; no message was sent to the contributor.

Stable latest remains the same M2.5 release ID and asset digests. The public
`https://ota.pajoniiir.eu/latest.json` before/after hashes match. No public OTA
channel was written, and no new image was installed. Exact beta hardware
acceptance remains NOT RUN; JC4880 soak waiver, deferred JC1060 testing and
other open gates retain their stated scope.

Evidence root: `.cache/shared-p4-beta.1/`, containing build/configuration/package
logs, `build-result.json`, `assets-audit.log`, per-board identities/configurations,
signed assets and debug archives, `remote-asset-audit.json`, downloaded copies,
stable-release/OTA before/after snapshots and packaging/audit scripts. The
published `release-manifest.json` and `SHA256SUMS.txt` contain immutable hashes;
GitHub source at the beta tag identifies the exact code used to build them.
