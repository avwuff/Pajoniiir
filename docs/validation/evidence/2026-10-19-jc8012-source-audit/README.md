# JC8012 source audit evidence

See the [research record](../../../reviews/2026-10-19-jc8012-source-audit.md).
This inventory contains source metadata, notices, comparison results and
compiler diagnostics. It contains **no controller firmware table, downloaded
component archive or factory/published panel payload**.

`touch-source-manifest.json` pins the examined repository files by Git blob and
SHA-256. `official-panel-manifest.json` pins the manufacturer source/datasheet.
`registry-archive-audit.json` records the published archive digest and confirms
its code/notice/firmware match the pinned repository; its manifest differs by
serialization and registry-injected repository metadata. Source bodies remain
in the ignored research cache.

`panel-table-comparison.json` compares complete ordered commands, transmitted
data and delays. `touch-firmware-comparison.json` records only counts/hashes and
the number of differences, not the firmware content.

`touch-compat-result.json` and `touch-idf602-syntax.log` record an unmodified MIT
wrapper syntax/type check with canonical IDF6.0.2 headers and touch1.2.1. A
one-entry synthetic firmware stub replaces the excluded payload. This produces
no linked image and cannot qualify touch behavior, firmware or JC8012 hardware.

`audit_cached_sources.py` replays the comparisons/probe after the pinned files
have been fetched into `.cache/jc8012-source-research` using the manifests.
It uses the existing JC1060 base-touch compilation command; build that ordinary
target on the same host first. Example from the repository root:

```powershell
python docs/validation/evidence/2026-10-19-jc8012-source-audit/audit_cached_sources.py `
  --cache .cache/jc8012-source-research
```

`pr54-*` and `merge-*` retain the exact integration and CI snapshots.
`SHA256SUMS` covers the inventory except itself. Raw diagnostics retain their
original bytes. No physical test, OTA, release or public channel was changed.
