# JC board preparation evidence

Frozen implementation: `5a005092a3bbe499d2d1c48a14c9d34414484916`, clean during
all four IDF6.0.2 builds. See the
[review record](../../../reviews/2026-10-19-jc-board-preparation.md) for scope.

- `checks.json` summarizes completed local gates and explicit NOT RUN scopes.
- `host-qualification.log` is the complete mandatory-fixture qualification run.
- `focused.log` and `ppa-red.log` retain patched PASS and unpatched FAIL evidence.
- `integration-build-*.json/log` record ordinary M3/JC4880/JC1060 results.
- `sd-experiment.json/log` record the fresh SDKCONFIG experimental JC1060 build.
- `sd-default-rejection.log` shows that the ordinary verifier rejects it.
- `ppa-provenance.json` seals generation from the real pinned IDF component.
- `select-positive.cmake/log` retain the real preparation/guard smoke, with
  registration stubs rather than a full JC8012 driver build.
- Build wrappers retain commands used; their original output paths are local
  ignored cache directories. They are evidence, not a new production runner.
- `SHA256SUMS` covers the retained inventory except itself.

Raw Windows logs preserve their bytes through local Git attributes. These are
software records; no hardware test, device control, installation or OTA ran.
