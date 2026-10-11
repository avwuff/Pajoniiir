# PR 46 diagnostics and media I/O review

Date: 2026-10-11. Service review source is the clean commit
`2d30984883c34985291cb383c989c19c39f01cc4`, compared with
`9af99cd234e775521f3ec83c0104f5c6a0c72920`.

## Service scope and limitations

CodeRabbit CLI 0.9.0 completed six sequential scoped requests. Raw NDJSON is
retained at `.cache/coderabbit-pr46-<scope>-2d309848.ndjson`.

| Directory | Candidate files | Issues | Service outcome |
| --- | ---: | ---: | --- |
| restart_diagnostics | 3 | 0 | completed_with_warnings |
| service_log | 5 | 3 | completed |
| media_io_gate | 3 | 3 | completed |
| sd_io_gate | 3 | 3 | completed |
| sd_idle_wait | 9 | 1 | completed |
| fatfs | 29 | 7 | completed_with_warnings; one unreviewed file |

There are 52 candidate files and 17 reported issues. All issues were checked
against production code and repaired locally. The FatFs response does not name
its unreviewed file or return a complete reviewed-file list, so this batch is
not a complete FatFs or whole-PR review PASS. The CLI emitted a completion
event for every scope; the wrapper's final exit code 1 accompanied the FatFs
findings, not a missing review result or a reported authentication/network error.
No usage credits were explicitly enabled.

## Repairs

1. The service-log writer peeks before consuming. When the card is unavailable,
   records and sync barriers stay in the existing bounded queue and the writer
   delays two seconds between attempts. A full queue still rejects/counts new
   producer records. No unbounded buffer, producer wait or audio-path NVS work
   was added. Batching stops when a rotation failure closes the active file.
2. Opening the log clears availability before any failure path. A failed
   rotation reopen cannot leave the journal reported as available.
3. Failed writer task creation deletes both sync semaphores and the queue,
   preventing resource leaks across initialization retries.
4. Both production I/O gates publish one mutex with atomic compare/exchange.
   Concurrent initializers discard their unused candidate and all callers use
   the published winner. The void blocking `begin()` functions fail closed via
   `ESP_ERROR_CHECK` if initialization/acquisition fails; they cannot return and
   permit unguarded I/O. Timed `try_begin()` still returns false on failure.
5. Standalone gate locks and availability/recorder flags use atomic access.
   The media host adapter additionally honors timed acquisition using a
   monotonic host clock and a one-millisecond sleep. The SD host adapter keeps
   its existing immediate try policy; it now guarantees exclusive acquisition.
   Initialization precedes concurrent host callers.
6. Busy SD idle polling yields before its first additional CMD13. IDF 6.0.2
   already issued the initial status command. Each additional probe follows a
   tick and is checked against the deadline. Immediate-ready and zero-budget
   paths remain immediate; SPI retains the original IDF implementation.
7. The exFAT override defaults to heap-backed LFN and hides no-LFN selection,
   avoiding a configuration that cannot compile with `FF_FS_EXFAT=1`.
8. FatFs directory seek error exits release their VFS lock. Full-disk positional
   writes restore the previous cursor and release the lock while reporting
   `ENOSPC`, rather than deadlocking later volume operations.
9. The contiguous-file query opens an existing file for reading, does not
   create missing files, and uses an `int` temporary before publishing a `bool`.
   It no longer writes four bytes through a one-byte caller output.
10. SD, raw-flash and wear-levelled-flash mount paths reject duplicate VFS
    registration. Failure cleanup only unregisters a VFS path owned by that
    attempt; it does not remove a pre-existing mount or pass an uninitialized
    filesystem to FatFs. Failed WL mounts release the new handle and clear the
    disk mapping; drive-exhaustion cleanup invalidates the returned WL handle.
11. Both format APIs propagate failed remount results instead of returning
    success after recycling mount resources. Original earlier failures retain
    precedence over later remount failures.

The FatFs defects are inherited in the copied ESP-IDF 5.5 VFS source; this does
not establish that the shared-core migration introduced them. Their local
delta is recorded in the component's `README-Pajoniiir.md` and original licence
headers remain intact. No destructive format operation was run on real media.

## Additional manually reproduced boundary

The shared service-log acknowledgement semaphore could wake a second sync
request when a previously timed-out request completed. A production-function
regression fails on the original code and passes after each queued barrier
receives an ID and the writer release-publishes its completed ID. Waiters verify
the ID after wakeup, with one fixed tick budget including stale wakeups and
tick-counter wrap. This preserves the dedicated semaphore instead of consuming
the caller's task notifications. It remains a bounded best-effort flush API;
filesystem errors remain visible through journal status.

While fixing SD format cleanup, two adjacent failure paths were corrected:
remount failure uses the public unmount cleanup to retire the context as well
as card resources; work-buffer allocation failure still attempts to restore
the prior mount and returns `ESP_ERR_NO_MEM`. These are separately validated
software boundaries, not hardware formatting acceptance.

## Verification and delivery boundary

Focused suites pass for restart capture, service-log schema, SD gate and idle
polling, enabled/disabled SPI passthrough and concurrent host media access.
New injected production tests cover two interleaved gate initializers and
allocation failure, late sync acknowledgement, retention before card mount,
writer initialization cleanup, failed reopen, VFS lock/boolean boundaries,
all three duplicate-mount paths, WL cleanup and both remount failure returns.
They pass on Windows and Ubuntu/WSL. Host media exclusion runs four threads;
SD gate exclusion runs two. These stubs do not emulate physical card timing.

ESP-IDF's installed Kconfig parser verifies the actual override's heap-LFN
default and hidden no-LFN choice. The final ordinary M3 ESP-IDF 6.0.2 build and
actual image verifier pass at 2,601,280 bytes with isolated BSP, LVGL 9.5.0 and
compiled app identity. Committed dependency locks are unchanged.

The complete `run_p4_host_tests.ps1 -Qualification -KeepArtifacts` run passes
with exit code 0 and no SKIP, including every new regression above. No JC build,
new signed package, installation, OTA, push, release or public-channel update
is part of this batch. Installed accepted firmware and frozen packages remain
unchanged and do not contain these new review repairs. Experimental modules,
current hosted CI and board-specific physical gates keep their existing scope.
