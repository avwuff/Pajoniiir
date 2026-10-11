# PR 46 profile, USB host and LED review, 2026-10-11

## Scope and source

CodeRabbit CLI 0.6.5 reviewed clean local source
`027545ed7ff55505cec23c7f85f444a746e30f1c` against base
`9af99cd234e775521f3ec83c0104f5c6a0c72920`, in five directory-scoped requests.
The full PR previously exceeded the service's 150-file limit. This record
does not claim whole-PR approval or merge readiness.

| Directory | Reviewed files | Findings | Service outcome |
| --- | ---: | --- | --- |
| controller_profile | 3 | 2 minor | completed_with_warnings |
| controller_profile_manager | 4 | 1 minor | completed_with_warnings |
| controller_profile_runtime | 3 | 1 minor | completed |
| controller_usb_host | 11 | 1 major | completed_with_warnings |
| controller_led_runtime | 5 | 1 minor | completed_with_warnings |

Raw NDJSON outputs are retained locally as
`.cache/coderabbit-pr46-<directory>-027545ed.ndjson`. Findings were checked
against current production code; service wording is not accepted as proof
of a hardware failure.

## Confirmed repairs

1. **Major: same-core MIDI OUT producer starvation during queue reset.**
   The USB owner closes admission and waits for active producers before
   resetting the queue. `taskYIELD()` leaves the high-priority owner runnable;
   a preempted lower-priority producer on that core cannot finish. The loop
   now blocks for one tick per iteration. Admission closure, producer ownership
   and reset ordering are preserved. This is teardown waiting, not a delay
   added to MIDI input, audio output or steady-state queue submission.
2. **Minor: malformed MIDI output entries admitted.** The profile parser,
   SD/upload storage validator and compiler now require a channel-voice status
   (`0x80`–`0xEF`) and seven-bit data1/off/on/blink bytes. Output kinds and the
   wire format remain unchanged. The compiler rejects malformed inputs instead
   of producing a CRC-valid bundle that the runtime subsequently rejects.
   Existing committed controller fixtures are unchanged. The zero status in a
   synthetic v4 upload fixture was corrected to a valid CC status.
3. **Minor: manual storage rescan omits reactivation.** A successful rescan of
   a present controller now requests the registered worker callback after
   releasing manager/SD locks and clearing storage busy. Failed scans and
   disconnected-controller scans do not request activation. The existing
   epoch revalidation remains responsible for rejecting stale worker results.
4. **Minor: activation log reads shared profile outside its lock.** Log VID,
   PID and input count from the private parsed allocation before freeing it.
   Profile publication and runtime-state changes remain protected by the mutex.
   The finding's title does not establish a separate deactivation bug; the
   manager already checks the connection binding under its mutex.
5. **Minor: LED header lacks C++ linkage guards.** The three public declarations
   now use conditional `extern "C"` guards, matching their C implementations.

## Retained 14-bit pair contract

The sixth finding proposes invalidating/resetting the stored LSB whenever an
MSB arrives. The reported intermediate value can occur: `(10,127)` followed by
MSB `11` emits `1535` before LSB `0` emits `1408`. However, the current mapper
deliberately maintains the last known halves after the first complete pair,
including subsequent MSB-only and LSB-only updates. The production FLX4
reference `update_14bit()` and `test_controller_profile.c` explicitly use this
contract; exhaustive FLX4 profile parity checks it.

The proposed change is therefore **deferred as a compatibility-sensitive
semantic change**, not silently applied or reported as repaired. A future
atomic-pair policy needs an explicit profile/ABI decision, supported-controller
message-order evidence and updated parity/physical qualification. No claim
is made that all controllers send complete pairs or that the intermediate
value is impossible. The service's broad MIDI-standard assertion is not used
to override the project contract without a normative source and compatibility
evidence. Existing S2/Hercules physical acceptance is not reopened.

## Validation

Focused profile, manager, USB-host and LED suites pass. Added compiler and
CRC-valid runtime/storage negative cases cover all output data fields and
status boundaries. The production rescan harness verifies connected,
disconnected, failed, in-flight and absent-callback paths, including callback
execution after lock/busy release. The production reset harness uses the real
MIDI OUT gate with a modeled preempted lower-priority producer: queue reset
occurs only after blocking permits producer retirement. This is software
scheduler-boundary evidence, not a physical reconnect campaign.

A C++ client links and executes all three LED functions compiled as C.
All six committed controller JSON profiles compile byte-identically to their
existing binaries with the tightened output validator.

Full host `-Qualification -KeepArtifacts` passes (exit 0, no optional SKIP),
including the new production rescan and queue-reset regressions. The ordinary
M3 ESP-IDF 6.0.2 build passes (exit 0); dependency locks are unchanged.
Documentation integrity passes (120 Markdown/HTML files, 134 retired paths)
and `git diff --check` passes. The repair build was made with uncommitted fixes;
it is compilation evidence, not a clean signed release image. JC builds and
physical tests were not run for these repairs.

No OTA, signed package, push, release or new physical acceptance is performed.
The installed accepted Hercules image and frozen `5ff76b6c` delivery artifacts
remain unchanged. Other PR areas, current CI and board-specific physical gates
remain separately open. Font permission/provenance remains open in the
[previous UI review](2026-10-11-pr46-ui-controller-review.md).
