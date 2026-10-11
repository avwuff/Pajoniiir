# PR 46 web, OTA and library/storage review, 2026-10-11

## Scope

CodeRabbit CLI 0.6.5 reviewed eight component directories independently at clean
local source `393468a5ad4940c8505b202fd317505a8c4f5331`, against base
`9af99cd234e775521f3ec83c0104f5c6a0c72920`. The Linux checkout is isolated
from the Windows worktree. This continues scoped PR 46 review; the service's
150-file limit prevents a single unrestricted review of the 707-file PR.
Raw outputs remain under `.cache/coderabbit-pr46-<scope>-393468a5.ndjson`.

| Directory | Files | Findings | Service outcome |
| --- | ---: | ---: | --- |
| web_server | 16 | 1 minor | completed_with_warnings |
| p4_ota | 5 | 1 major, 1 minor | completed |
| p4_ota_pull | 3 | 2 minor | completed |
| p4_ota_pull_core | 8 | 0 | completed_with_warnings |
| library | 16 | 1 major, 3 minor | completed_with_warnings |
| media_catalog | 3 | 0 | completed_with_warnings |
| media_identity | 3 | 1 minor | completed |
| usb_storage | 17 | 1 minor | completed_with_warnings |

All 11 findings in 71 files were verified against current code and addressed
locally. The service labels warning outcomes as unverified findings, including
two zero-finding scopes; these are not recast as unrestricted service approval.

## Confirmed repairs

1. Optional mDNS failure no longer stops HTTP and its OTA recovery endpoint.
   Partial mDNS setup is cleaned up only if this component initialized it;
   a failed init does not free another component's instance. Successful setup
   remains owned until web teardown.
2. OTA begin rejects both RECEIVING and READY_TO_REBOOT. After successful
   activation, a second upload cannot erase the selected next-boot slot.
   A late duplicate abort also preserves READY_TO_REBOOT admission fencing.
3. Flash/hash write failures release the OTA handle and PSA hash operation
   immediately. A subsequent begin defensively closes leftover resources.
   Existing callers' explicit abort remains safe after error cleanup.
4. Both channel and bundle downloads classify only HTTP 404 as NOT_FOUND.
   Authentication/server errors become INVALID_RESPONSE and log the status,
   instead of misleadingly reporting that no update was published.
5. Pull-OTA API comments now describe actual admission, version, configuration,
   allocation and worker errors; they do not claim a Wi-Fi-state check.
6. Cache reads build paths without mkdir. A cache miss performs no directory
   creation, including with LIBRARY_ANLZ_CACHE_WRITE disabled. Explicit saves
   retain directory creation. This fixes an unintended write path; it does
   not establish that any historical reset was caused by that write.
7. Cache publication checks fclose/flush completion before deleting the old
   entry. A failed close discards the temporary file and preserves the old one.
   The existing FAT unlink/rename replacement sequence is unchanged; arbitrary
   power loss during replacement is not claimed to be atomic.
8. The cache accepts the parser's full uint16_t beat-count range, including
   65,535 beats, instead of rejecting valid long beatgrids above 16,384.
9. The Windows host default now matches Kconfig's disabled cache-write default.
   A separate enabled suite retains explicit coverage of writes/write failure.
10. Failed persistent-ID derivation clears its output, preventing reused output
    from remaining valid for the previous track. Valid digest/output aliasing
    is preserved by clearing only failure paths.
11. A fast storage disconnect/reconnect now publishes the unmounted callback
    before releasing the previous mount. The application observes true/false/
    true even if the storage owner did not sample the disconnected interval.
    Failed replacement emits false once and retries without a duplicate edge.

## Validation and boundaries

The mDNS harness executes production setup/teardown with failures at each
setup step, checking HTTP preservation, ownership cleanup and duplicate stop.
The OTA harness executes production begin/write/finish/abort with flash and
hash failures, verifies retry cleanup and rejects re-entry after activation,
including late abort and duplicate finish. It also exercises HTTP status
classification for success, 404, redirects, authentication and server failures.

The cache suite exercises read-only miss, injected fclose failure preserving
the previous cue/cache entry, and a 65,535-beat round trip. Both library policy
suites pass 228 checks each. Persistent-ID tests cover every rejected input
category and valid aliased derivation. These are host software boundaries,
not new device or electrical acceptance.

The storage reconciliation harness executes production mount/unmount/commit
functions with the real session implementation and verifies both successful
rapid reconnect and failed mount/retry ordering. It was run separately after
the final full host run started; its host-runner registration is included for
future runs. That distinction prevents claiming the running suite had already
parsed a newly added test entry.

Full host `-Qualification -KeepArtifacts` completes with exit 0 and no optional
SKIP; both library cache policies and new mDNS/OTA regressions are included.
The final ordinary M3 ESP-IDF 6.0.2 build, including storage reconciliation,
completes with exit 0. Dependency locks are unchanged. Builds were made with
uncommitted repairs and provide compilation evidence, not clean release
artifacts or physical acceptance. JC4880/JC1060 builds were not repeated.
Documentation integrity passes (121 Markdown/HTML files, 134 retired paths),
and `git diff --check` passes.

No OTA, push, release, public-channel update or new physical gate
was performed by this review. The accepted installed Hercules image and
frozen `5ff76b6c` delivery artifacts remain unchanged. Font permission and
the prior CC14 pair-policy item remain open/deferred in their respective
review reports; whole-PR, current CI and JC physical acceptance are separate.
