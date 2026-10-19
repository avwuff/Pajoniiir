# Branch integration evidence

See the [integration record](../../../reviews/2026-10-19-branch-integration.md).
Final implementation/build source is clean
`abffb203bfc4807b9ef622f520b02a9335afc415`. These are software artifacts,
not signed packages, installation evidence or new physical acceptance.

| Files | Scope |
| --- | --- |
| `integration-matrix.json` | All three ordinary IDF6.0.2 builds, image sizes/hashes, embedded versions, exact clean source, unchanged locks, absent `ui_next` and selected stock IDF PPA. |
| `integration-checks.json` | Final host qualification exit 0, no optional fixture SKIP, runner step counts, focused test boundaries, eight demo render names and unchanged existing profile hashes. |
| `integration-final-host-qualification.log` | Complete final `-Qualification` output: 142 functional build steps and 203 run steps. Step counts are not independent test counts. |
| `numark-pfl-red-isolated.log`, `numark-pfl-green.log` | Production actor idempotent per-deck ON/OFF regression before/after repair, with explicitly reset test state. |
| `numark-pfl-disconnect-red.log`, `numark-pfl-disconnect-green.log` | Production actor lifecycle cleanup regression, including retained legacy-toggle behavior; final ordinary and scratch variants pass. |
| `numark-profile-green.log`, `numark-runtime-green.log`, `numark-uac-green.log` | Registered compiled-profile/storage checks, bounded delivery/epochs and production USB stream harness. Numark AS fixture is reconstructed, not a complete raw capture. |
| `integration-build-*.log` | Final ordinary builds and board verifiers. M3 explicitly reconfigures first to refresh the development version. Existing build directories are reused. |
| `ui-next-sim-*.log`, `ui-render-hashes.json` | Headless demo compile and eight Perform frames across six geometries. Output directory is created before rendering. No touch/core binding or production baseline acceptance is inferred. |
| `perform_800x480_night.png`, `perform_1024x600_night.png` | Lossless PNG conversion of the two visually inspected demo PPM frames. No UI baseline is changed. |
| `shared-master-merged.json`, `master-*-ci.json` | PR51 merge and successful automatic CI for baseline master `d878c066`; not a CI claim for the later Numark source. |
| `component-discovery-scope.json` | Frozen CMake excerpts correct the earlier review: JC4880/M3 scan the shared component root; JC1060 has an explicit allow-list. |
| `collect_integration.py`, `build_integration.ps1` | Local verification orchestration. Reproduction needs checkout of the exact implementation SHA, configured IDF/toolchain and the retained local build outputs. |
| `SHA256SUMS.txt` | Exact archive file hashes. Directory-local attributes preserve bytes across Git checkout line-ending settings. |

Build output paths in logs are local evidence, not instructions to flash.
The final M3 image hash is the one in `integration-matrix.json`, after explicit
reconfigure; earlier cached-version preliminary images are not this final
artifact. Published beta2 and installed firmware remain unchanged. The source
commits and donor merge parents retain attribution/history. No branch or
worktree is removed.
