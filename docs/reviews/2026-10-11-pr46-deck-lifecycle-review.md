# PR 46 deck state and USB/Wi-Fi lifecycle review

Date: 2026-10-11. Clean service review source:
`e3943a1f39ecf42671fea0ddbf9144055cdaa167`, compared with
`9af99cd234e775521f3ec83c0104f5c6a0c72920`.

## Scope

Nine sequential CodeRabbit CLI requests reviewed 48 changed files.
Raw evidence: `.cache/coderabbit-pr46-<scope>-e3943a1f.ndjson`.

| Directory | Files | Findings | Outcome |
| --- | ---: | ---: | --- |
| deck_core | 11 | 3 | completed |
| hot_cue_store | 3 | 2 | completed |
| app_settings | 3 | 2 | completed_with_warnings |
| beat_jump | 3 | 0 | completed |
| control_link | 5 | 0 | completed_with_warnings |
| p4_local_controller | 3 | 2 | completed |
| usb_host_manager | 8 | 1 | completed_with_warnings |
| wifi_link | 9 | 2 | completed |
| wifi_transition_lease | 3 | 1 | completed |

Twelve findings received local repairs; the lease-token proposal is deferred
below. A further Beat Jump overflow was reproduced manually and repaired.
Warnings and directory scope remain explicit; this is not whole-PR approval.

## Dispositions

1. A LOAD reservation retries up to 50 yields when an actor event owns the deck.
   Another replacement token refuses immediately. Playback admission is still
   checked after reservation against the actual transport, preserving LOAD LOCK.
2. Snapshot publication adds a release fence after the odd sequence and an
   acquire fence before the reader's second sequence check. This addresses
   ordering on the P4 cores; host tests alone do not demonstrate weak-memory
   hardware behavior or establish a formal C memory-model proof.
3. Network snap seeks use the loaded track's measured duration where available
   and clamp to its last valid millisecond before audio transport admission.
4. Clearing a cue bank validates both namespaces and removes v2 before v3,
   preventing a cleared v3 overlay from exposing legacy cues again. Standalone
   and production NVS branches both remove the legacy bank.
5. Corrupt cue payloads can be replaced or cleared when their full persistent
   identity is readable and matches. This deliberately differs from simply
   ignoring load errors: truncated identities, oversized unreadable records,
   short-key collisions and NVS I/O errors still fail closed. Load continues to
   reject bad CRC/size/slot data. Two-namespace deletion is not claimed to be one
   atomic flash transaction; failed writes/commits are reported.
6. Loaded, immediate and debounced backlight values enforce the documented
   10..100 range, preventing a stored zero from starting with a dark display.
7. Successful OTA settings blob migration erases the three legacy plaintext
   keys and commits cleanup. Startup also retires keys left behind on devices
   that already have a valid blob. Failed blob migration preserves legacy values.
   Cleanup errors are reported in the existing migration diagnostic; no NVS
   encryption or secure physical erasure is claimed.
8. A transient controller runtime initialization failure retries with the
   existing bootstrap delay instead of permanently deleting the bootstrap task.
9. Disconnect advances the local profile epoch, making queued work from the
   preceding connection ineligible during reactivation/reconnect.
10. USB startup and topology registration use separate persistent completion
    semaphores rather than notifications on the initiating tasks. The original
    five-second waits remain bounded. A late daemon completion cannot consume
    or leave a notification in the caller's unrelated task wait. A still-running
    install is not restarted concurrently; a permanently hung driver is not
    converted into automatic reinstallation or an unbounded startup wait.
11. Probe admission rechecks active/AP state after taking the radio lease, so
    a completed disable between precheck and acquisition cannot start a probe.
12. Retry exhaustion resets the worker-local budget before continuing. A new
    enable request arriving before worker exit receives the full retry budget.
13. Manually reproduced Beat Jump integer overflow uses a widened index sum
    before edge clamping. INT_MAX and INT_MIN now select the correct grid edge.

## Deferred lease-token proposal

The service correctly describes a limitation of the owner-only API: a caller
that releases, reacquires as the same owner, then repeats its old release could
clear the new lease. Current CONTROL, PROBE and OTA call sites were audited.
Their success, task-creation failure and precondition failure branches release
once per acquisition; no reachable repeated old release was established.
Acquisition-token API migration is therefore deferred hardening, not a claimed
repair or proof that the API is safe against arbitrary future callers.

## Validation

Focused production tests pass for both deck configurations, Beat Jump, settings
and both cue adapters. The new LOAD fixture releases transient ownership after
three yields, rejects another LOAD without yielding and bounds permanent actor
contention. The network fixture reproduces a forward alignment beyond EOF and
verifies the last-valid-millisecond clamp. The Beat Jump boundary test failed on
the original code and passes with the widened sum.

NVS tests cover owned corrupt/truncated payload recovery, unreadable identity
rejection, full-ID collision protection, removal of both namespaces, failure
propagation and handle cleanup. Settings tests verify low brightness through
startup/immediate/debounce paths and preservation of legacy keys on migration
failure. Existing controller runtime/profile regressions remain applicable;
the local bootstrap/epoch repairs receive compilation and full-suite evidence,
not a new physical reconnect acceptance claim.

New fault-injection harnesses execute production Wi-Fi probe/worker and USB
manager initializer/daemon functions. They verify the disable/acquire race,
same-worker retry renewal, timeout followed by late host completion, and
isolation from caller notifications. Both pass on Windows and Ubuntu/WSL.

The full host qualification runner completed with exit code 0 and no SKIP.
The final settings cleanup refinement was made after that suite's settings
execution; its focused rerun passes all 91 checks, including cleanup with an
already-valid blob. The final ordinary M3 ESP-IDF 6.0.2 build and actual image
verifier pass: 2,600,528 bytes, isolated BSP, LVGL 9.5.0 and compiled source
identity. Dependency locks are unchanged.
No JC build, device action, OTA, push, release
or public-channel update is part of this batch. Accepted installed firmware and
frozen delivery artifacts remain unchanged; current CI and physical gates are
separate from these local software checks.
