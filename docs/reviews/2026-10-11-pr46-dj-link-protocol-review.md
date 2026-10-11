# PR 46 inherited DJ Link protocol review

Date: 2026-10-11. Working source: `eb92aa8503b67768430d38b90be44e7bffbc1471`
plus the repairs described below. Review method: manually authorized by the
operator after CodeRabbit rejected the request.

## Scope and service limitation

The intended service scopes were `firmware/common/djlink` and
`firmware/common/dj_link_core`, using base
`6503e6fa476a05023f8db55001ae4edac38da1b0`, before the inherited protocol import.
This produces 45 candidate files, including the inherited implementation;
the earlier base only exposed documentation changes in these directories.

CodeRabbit returned `rate_limit` before completing the first scope: included
reviews were exhausted and the Git provider account had no assigned seat.
The second scope was not submitted. The preserved first-scope NDJSON is
`.cache/coderabbit-pr46-protocol-djlink-eb92aa85.ndjson`, SHA-256
`656cbb1d3c55c793d5ad8faf36e5b1a05a9c2f2b80eb8f20d0da0fae8296e321`.
There is no completed CodeRabbit review for this batch. The operator explicitly
authorized continuing manually; the findings below are manual findings.

Manual inspection focused on DBServer framing/streaming and completion,
NFS bounds/retries, discovery identity/epochs, sync authority/clock handling,
cache publication, path/PDB bounds and TCP/UDP ownership. The 45-file candidate
list is not a claim of exhaustive line-by-line review of every codec, header
or adapter. Sanitizer coverage complements this inspection; it does not prove
protocol interoperability or absence of all defects.

## Confirmed findings and repairs

1. **P2: streamed analysis accepted contradictory argument types.**
   `blob_head()` accepted any integer field before the binary body, regardless
   of its declared argument tag. It discarded trailing type declarations, and
   `blob_step()` similarly accepted any integer type afterward. The ordinary
   message codec rejected the same malformed reply. A byte-fragmented production
   regression reproduced the discrepancy: the old client published the reply
   instead of entering FAILED. The repair maps integer declarations to actual
   field types, rejects unknown/non-integer declarations and retains at most
   `DJLINK_DB_MAX_ARGS` trailing field types until consumption. No blob callback
   is published for a malformed response. The destination may already contain
   streamed bytes when a trailing mismatch is detected; ownership still requires
   a successful completion callback before using those bytes.
   The existing vynull no-data convention (a declared but omitted binary body,
   with an integer placeholder for empty extended cues) is preserved.

2. **P2: file-path render completion ignored the requested row count.**
   `PATH_WAIT_RENDER` could publish a path after a footer even when the peer
   supplied fewer rows than advertised. The old implementation failed the new
   incomplete-response regression. The repair counts validated rows, rejects
   malformed/excess items and requires the exact requested count before the
   path callback. A track-info menu above the 64-row bound now fails explicitly
   instead of silently truncating the request. Unknown IDs retain their empty
   answer behavior; invalid/oversized UTF-16 paths still decode to an empty path.

The production client regression includes contradictory/unknown leading and
trailing tags, byte-wise fragmentation, valid mixed INT8/INT16/INT32 trailing
fields, incomplete/excess path menus and a complete positive path response.
The first tag discrepancy and incomplete path case were observed failing before
their respective repairs. Other rejection cases extend the repaired gate.

## Validation and remaining boundaries

Baseline Linux ASan/UBSan protocol suites passed before the repairs. Focused
Windows DBServer client regressions pass after repair. The ordinary M3
ESP-IDF 6.0.2 build and board verifier pass: 2,601,280 bytes, isolated M3 BSP,
LVGL 9.5.0 and correct application identity. Dependency locks are unchanged.

Final full host qualification passed with exit code 0 and no SKIP in
`.cache/pr46-protocol-qualification.log`. The final focused client run also
passed, including the added positive mixed-type tail case, in
`.cache/pr46-protocol-final-client.log`. The repaired Linux ASan/UBSan runner
passed all 13 protocol executables plus deck network-sync and production
network-lifecycle checks with exit code 0, in
`.cache/pr46-protocol-sanitizers-final.log`. Its baseline log is
`.cache/pr46-protocol-sanitizers-baseline.log`.

Documentation integrity passed (126 Markdown/HTML files and 134 retired paths)
and `git diff --check` passed. Build evidence is
`.cache/pr46-protocol-m3-build.log`; the ordinary build uses the production
FLX4 USB defaults and does not reproduce the accepted Hercules bench topology.

No device, OTA package, public channel or experimental default was changed.
Real Pro DJ Link interoperability is NOT RUN. JC4880/JC1060 builds were not
repeated in this batch; their physical gates and current GitHub CI remain open.
Installed Hercules acceptance and operator-closed S2 evidence retain their
original image identities. This batch does not close shared-core acceptance.
