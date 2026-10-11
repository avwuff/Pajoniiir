# PR 46 startup, board and build review

Date: 2026-10-11. Local review baseline: `7fa8d3c0bfd01c3c344ee38a81165902bca1f87e`.
Comparison base: `9af99cd234e775521f3ec83c0104f5c6a0c72920`.

## Scope and service evidence

Six sequential CodeRabbit CLI scopes covered 51 changed files:

| Directory | Files | Findings | Service outcome |
| --- | ---: | ---: | --- |
| `firmware/p4-core/components/p4_app` | 4 | 0 | completed_with_warnings |
| `firmware/p4-core/components/board_adapter` | 11 | 2 | completed_with_warnings |
| `firmware/p4-core/components/board_capabilities` | 3 | 0 | completed |
| `firmware/boards` | 20 | 1 | completed_with_warnings |
| `firmware/p4-core/cmake` | 10 | 0 | completed_with_warnings |
| `.github/workflows` | 3 | 0 | completed |

Raw local records are `.cache/coderabbit-pr46-<scope>-7fa8d3c0.ndjson`.
The three findings were verified against production code and repaired locally.
Service warnings remain part of the evidence; these directory reviews are not
approval of the complete PR or service review of the subsequently added repairs.

## Repairs

1. **Incremental build identity (major).** Configure-time Git queries could
   retain an old SHA or cleanliness flag while an incremental build compiled
   changed sources. A shared CMake generator now refreshes the header and JSON
   before the identity component builds. Unchanged content preserves timestamps.
   Release builds also reject source changes made after initial configuration.
2. **SD DMA eligibility (minor).** An aligned internal pointer is not necessarily
   DMA-capable. The shared callback now rejects non-DMA-capable buffers, retaining
   the existing external-memory bounce policy and alignment check.
3. **M3 I2S diagnostic boundary (minor).** After successful clock reconfiguration,
   the last DMA timestamp is cleared before enabling output. This prevents the
   intentional disable/reconfigure interval being counted as an output DMA gap.
   Existing error propagation remains intact.
4. **Relocated build verification (manual).** The actual M3 review output exposed
   a verifier assumption that the build directory was under the entrypoint.
   Core source and lock paths now resolve from the verifier's checkout, supporting
   relocated build outputs and Docker-recorded paths. Private component copies,
   multiple BSPs and stale identities are still rejected.
5. **Startup board reporting (manual).** The shared startup log uses the selected
   board capability name instead of always identifying JC4880.

The host CI job now explicitly installs `cmake` and `ninja-build`, required by
the new incremental generation regression. Dependency versions, board defaults,
USB roles, display geometry and accepted waveform behavior were not changed.

## Verification and limits

The identity test builds the real production generator and identity component
in an isolated Git repository. It verifies a no-op build, dirty-source refresh,
new-commit refresh without reconfiguration, and release rejection after source
changes. Both cases pass on Windows with Ninja. Four relocated-verifier tests
pass, including negative isolation and stale-identity cases. The M3 reclock test
executes the production function with injected I2S failures and verifies the
timestamp boundary. The SD suite rejects an aligned non-DMA buffer.

Full host `-Qualification -KeepArtifacts` finishes with exit 0 and no optional
SKIP, including the new regressions and the previous storage mount regression.
The ordinary M3 ESP-IDF 6.0.2 build completes with exit 0. Its actual build
verifier passes: 2,599,504 bytes, isolated BSP, pinned LVGL 9.5.0 and compiled
application/source identity. Committed dependency locks remain unchanged.
These builds contain uncommitted repairs and are compilation evidence, not
clean release artifacts or physical acceptance.

A separate Ubuntu/WSL attempt at the identity regression could not start because
Ninja is absent there. Linux execution is **NOT RUN**; the explicit CI dependency
addresses runner provisioning, but no new GitHub Actions run is claimed.
JC4880/JC1060 builds and device tests were not repeated for this review batch.
Documentation integrity and `git diff --check` pass. The edited CI workflow
also parses successfully as YAML; this does not execute its jobs.

No OTA, push, release or public-channel update was performed. The installed
accepted Hercules image and frozen `5ff76b6c` artifacts remain unchanged.
Whole-PR coverage, current CI, JC physical gates, font permission and the
compatibility-sensitive CC14 pair-policy item retain their existing dispositions.
