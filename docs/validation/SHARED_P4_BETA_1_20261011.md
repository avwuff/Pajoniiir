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

Final build/package/publication results are appended after verification.
