# PR 46 inherited DJ Link protocol review completion

Date: 2026-10-11. Baseline: `5fb2b2aae67f956e28f5cf8a9b24379c95c5299f`.
Method: operator-authorized manual review, continuing the
[focused protocol review](2026-10-11-pr46-dj-link-protocol-review.md).
CodeRabbit remains unavailable and was explicitly waived by the operator.
There is no completed service review for these inherited C modules.

## Completed scope

All 45 tracked files in the two directories were inspected, including full
implementation bodies, public contracts, build registration and provenance.
This finishes the requested remaining protocol-directory review; it is not a
proof that all defects are absent or that real equipment interoperates.

| Directory | Reviewed coverage | Files |
| --- | --- | ---: |
| `firmware/common/djlink` | All nine C implementations: packet, beat, claim, status, sync, mixer, media, DBServer and NFS; all ten public/aggregate headers; CMake, README, UPSTREAM and LICENSE | 23 |
| `firmware/common/dj_link_core` | All nine C implementations and nine headers: discovery, sync, DB, browse, cache, PDB, ANLZ, TCP and UDP; CMake, README and both licenses | 22 |

Inspection covered packet offsets and length checks, streaming completion,
bounded retries, source/connection epochs, master transitions, caller-owned
buffers, socket callback replacement, path decoding, cache completion ordering
and I/O isolation. Earlier repaired DBServer findings remain covered by their
production regressions. Licenses and protocol acknowledgements are retained.

## Confirmed additional findings

1. **P2: full-width wire names could overread memory.** Sync/fader/on-air
   builders pass fixed 20-byte name fields to `djlink_name_from_str()`, whose
   `strlen()` scanned beyond that field before truncating. Exact-size non-NUL
   allocations reproduce an ASan heap-buffer-overflow in the old helper.
   It now reads at most 20 bytes and preserves full-width names. Production
   codec regressions exercise the helper and all three affected builders.
   Red evidence: `.cache/pr46-protocol-completion-name-red.log` and the
   actual-builder overread in `.cache/pr46-protocol-completion-name-builder-red.log`.
2. **P2: ACK prematurely granted local master authority.** The old coordinator
   switched to local master immediately on the response. The
   [protocol handoff sequence](https://djl-analysis.deepsymmetry.org/djl-analysis/sync.html)
   requires the outgoing master's subsequent status announcement to name the
   new player. ACK now records agreement only. Grant requires the exact selected
   source epoch, a status received since this request, the requested destination
   and the existing three-second deadline. Production coordinator regressions
   cover ACK-only, stale/wrong-target status, source replacement, timeout and
   successful confirmation. Red evidence:
   `.cache/pr46-protocol-completion-handoff-red.log`.
3. **P2: a nonaffirmative response was parsed as agreement.** The response
   parser ignored its 32-bit acceptance field. It now requires value 1, matching
   the existing builder and wire contract. Codec and coordinator regressions
   reject zero; codec coverage also rejects another unsupported value (2).
   Both assertions fail when linked to the pre-repair response parser:
   `.cache/pr46-protocol-completion-ack-red.log`.
4. **P3: unbounded configurable NFS backoff could wrap.** The client accepted
   any uint32 base interval but shifted it left by up to six bits. Configuration
   now rejects values above `UINT32_MAX >> 6` before sending. The 250-ms default,
   retry count and existing 64x cap are preserved. The mock NFS regression
   failed on UINT32_MAX before the repair; post-repair checks also reject the
   first value above the documented maximum. Red evidence:
   `.cache/pr46-protocol-completion-nfs-red.log`.

README host commands now reference the actual canonical runners. Core README
provenance names `firmware/main-deck-jc1060/components/dj_link/dj_link_session.c`,
verified in the frozen donor's GitHub tree. The DBServer header also distinguishes
little-endian binary grid replies from big-endian ANLZ PQTZ entries. These
documentation corrections do not change licenses.
These findings are boundary/protocol defects, not a diagnosis of historical
bench symptoms or operator cable changes.

## Validation

All commands completed with exit code 0:

- Full Windows host qualification (`-Qualification -KeepArtifacts`), without
  optional SKIP: `.cache/pr46-protocol-completion-host.log`.
- Linux ASan/UBSan, all 13 codec/core/POSIX executables plus `deck_net_sync`
  and production network-lifecycle checks:
  `.cache/pr46-protocol-completion-sanitizers.log`.
- Ordinary M3 ESP-IDF **6.0.2** build:
  `.cache/pr46-protocol-completion-m3.log`. The board verifier confirms
  **2,601,280 bytes**, isolated M3 BSP, LVGL 9.5.0 and application identity.
  Dependency locks are unchanged. This is a build of the review working tree,
  not a new clean installed or packaged image.
- Documentation integrity (127 Markdown/HTML files and 134 retired paths):
  `.cache/pr46-protocol-completion-docs.log`; `git diff --check` also passes.

The focused Windows codec/sync/discovery checks also passed before the final
suite. The NFS mock tests remain Linux/POSIX checks. Red logs retain the failing
old implementations; they are evidence of reproduced defects, not failures of
the final source. Original and final evidence SHA-256 values are retained in
`.cache/pr46-protocol-completion-evidence.json`.

## Delivery boundary

No OTA, device controls, USB topology, experimental defaults, release artifacts,
public channel or remote branch was changed. The M3 build uses ordinary FLX4
defaults, not the physically accepted Hercules bench topology. Real Pro DJ Link
interoperability remains NOT RUN. JC4880/JC1060 builds are deferred to the final
cross-board review matrix; their physical gates and current CI remain open.
Installed Hercules and operator-closed S2 evidence keep their exact historical
image identities. Completing this manual review does not complete shared-core
acceptance.
