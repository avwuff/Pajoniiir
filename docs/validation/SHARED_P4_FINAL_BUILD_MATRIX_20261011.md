# Shared P4 final local build matrix

## Latest FIFO follow-up qualification

Date: 2026-10-11. Final checked clean source:
`722bb9762bbfc1010b1faf437797bd33fe3e0360`.

| Target | Project | Application bytes | Remaining below 0x380000 | Final build / board audit |
| --- | --- | ---: | ---: | --- |
| JC4880 | `main-deck-p4` | 2,608,240 | 1,061,776 | PASS |
| JC1060 | `main-deck-jc1060` | 2,666,912 | 1,003,104 | PASS |
| M3 | `main-deck-m3` | 2,601,600 | 1,068,416 | PASS |

All three ordinary targets first completed fresh ESP-IDF 6.0.2 builds on clean
`95c1befbaa90db27e7991d2868fb36caf6c7cb7d`, using empty build directories,
new dedicated SDKCONFIGs, committed defaults and unchanged locks. Configurations
were serialized to avoid shared dependency-cache locking. The actual produced
BIN/ELF/map/identity/bootloader/partition artifacts were frozen before reuse.

That verification found a controller-root admission regression in the M3
role-swap configuration: a Full Speed Hercules uses the HS root's periodic
FIFO. The subsequent `722bb976` repair restores admission according to physical
root capacity without changing the JC4880 640-byte allocation. The captured
production stream test failed before the repair and passes after it. Capacity
tests execute eight board/root/diagnostic configurations, including M3 with
and without HS-role swap. This is software evidence, not new M3 physical acceptance.

The three final builds above are **incremental follow-ups** in the freshly
verified build directories, with explicit reconfiguration and clean `722bb976`
source identity. Every final board verifier, linked DWC/SD-wrapper check,
resampler no-double-helper check, dependency lock comparison and image budget
passes. Final artifacts and hashes were archived separately.

`tests/run_p4_host_tests.ps1 -Qualification` exits 0 on `722bb976`, with required
fixtures and no SKIP. Its standalone production stream test runs both JC4880
and M3-HS controller policies. The earlier eight-case FIFO test and newly
captured M3-HS failure remain preserved as regression provenance.

Three product UI simulators passed on `95c1befb` without baseline changes.
No UI, board-adapter/capability or UI-simulator file changed between that source
and `722bb976`; this equivalence is recorded rather than repeating unchanged
screenshots. Their physical display scope is separate from simulation.

Evidence roots:

- `.cache/final-matrix-95c1befb-20261011/`: original fresh build/configure logs,
  matrix audit, three simulator logs and `frozen-95c1befb` artifacts per target.
- `.cache/final-matrix-722bb976-20261011/`: final configure/build logs, full
  qualification log, matrix audit/result, separate artifacts, before/after
  M3-HS regression and UI-source-equivalence record.

JC4880's actual installed and physically tested image remains `e22ff334`, with
the named 30-s smoke and 60-s MT/transport/PFL acceptance documented in the
[JC4880 record](JC4880_HERCULES_FIFO_20261011.md). None of the final matrix
images were flashed or published. JC1060 physical acceptance, current hosted CI
(operator quota waiver), long exact-image qualification and broader shared-core
gates remain open. S2 stays operator-closed and was not physically retested.

## Historical clean matrix before FIFO follow-up

Date: 2026-10-11. Exact tested source:
`b913818052de99af61c3a8c752b814185794b50e` (clean).
This matrix follows the completed manual protocol review. It verifies the
ordinary JC4880, JC1060 and M3 configurations, not controller bench experiments.

## Results

| Target | Project identity | Application bytes | Remaining below 0x380000 | Clean build / board audit | UI simulator |
| --- | --- | ---: | ---: | --- | --- |
| JC4880 | `main-deck-p4` | 2,608,224 | 1,061,792 | PASS | 800x480, 21 exact captures PASS |
| JC1060 | `main-deck-jc1060` | 2,666,912 | 1,003,104 | PASS | 1024x600, 21 exact captures PASS |
| M3 | `main-deck-m3` | 2,601,600 | 1,068,416 | PASS | 800x480, 26 exact captures, all five zooms PASS |

All three builds, the three simulator runs and the final audit returned exit
code 0. Each target used a new build directory and new dedicated SDKCONFIG,
ESP-IDF **6.0.2**, the committed defaults and dependency lock. Generated and
managed dependency caches were retained; no existing object files or bench
SDKCONFIG were reused. Commands were `idf.py ... reconfigure` followed by
`ninja -C <build> -j 4` against the freshly generated IDF build graph.

The board verifier validates one matching BSP, common production component
paths, board defaults and capability policies, application/OTA project identity,
LVGL 9.5.0, the fixed image budget and source identity embedded in the binary.
All three identities are the exact source above with `source_dirty: 0`.
Git blob comparison confirms all three committed dependency locks are unchanged.

Actual linked ELF symbols contain `__wrap_usb_dwc_hal_chan_decode_intr` and all
three SD wrappers (`sdmmc_wait_for_idle`, `sdmmc_read_sectors`,
`sdmmc_write_sectors`). The unwrapped DWC decoder is absent. All three production
`audio_resampler.c.obj` files are free of double-precision runtime helpers.
UI scripted navigation and exact screenshot hashes pass without changing any
baseline. This is simulation evidence, not a new physical display acceptance.

The full host qualification without SKIP and Linux ASan/UBSan evidence from
[protocol review completion](../reviews/2026-10-11-pr46-dj-link-protocol-completion.md)
remains applicable to the identical production/test files committed in this
source. Those suites were not repeated in this build-only matrix.

## Evidence and build observations

Local evidence root: `.cache/final-matrix-b913818-20261011/`.
Each project owns `build.log`, `sdkconfig`, `build/` and `verifier.log`.
`audit_matrix.py`, `audit.log` and `matrix-result.json` preserve the checks,
versions, image/ELF/map/bootloader/partition/source-identity SHA-256 hashes and
lock hashes. `ui-simulators.log` records the three layouts. Captures remain in
the corresponding `.cache/ui_simulator/screenshots-product-*` directories.

The first simultaneous M3 configure returned exit 2 because Component Manager's
shared Git cache was locked by another configure. Its original log is preserved
as `main-deck-m3/configure-first-attempt.log`. After the other dependency
resolution completed, M3 configuration/build was retried successfully. No lock
was manually deleted and no dependency version changed. Future matrix runners
should serialize dependency configuration before parallel compilation.

Build logs retain nonfatal upstream Kconfig diagnostics and the existing M3
CMake warning about the BSP's private audio-engine include path. This is not a
warning-free-build claim. No firmware repair was introduced during this matrix.
Documentation integrity and Git whitespace checks are required before committing
this record; the corresponding log is `documentation.log` in the evidence root.

## Acceptance and delivery boundary

This closes the final local ordinary three-target build/isolation/simulator
matrix for the exact source above. GitHub Actions was not run because the
operator deferred it after exhausting the account quota. Local checks do not
constitute a current CI PASS.

No board was flashed, no OTA bundle was installed or published, and no public
channel, release tag or remote branch changed. The accepted installed Hercules
image and frozen delivery artifacts retain their earlier identities. The new
ordinary M3 binary uses production FLX4 defaults, not the accepted Hercules
role-swap bench configuration. No physical acceptance is transferred to these
new binaries. JC4880/JC1060 physical gates, real Pro DJ Link interoperability,
remaining review dispositions and overall shared-core acceptance remain open;
operator-closed S2 tests are not reopened.
