# M3 signed OTA rejection and startup rollback, 2026-10-07

## Scope and retained image

The operator authorized the autonomous OTA/startup tests (option 2) and excluded
GitHub Actions (option 3) because the account had reached its Actions quota.
No hosted workflow was dispatched. This is **SKIPPED BY OPERATOR**, not a CI PASS.
No physical reset, power/cable action or listening confirmation was requested.
Both decks stayed stopped throughout; saved Wi-Fi remained enabled.

The accepted running image before and after this work is:

| Field | Value |
| --- | --- |
| Board / project | `m3` / `main-deck-m3` |
| Version / clean source | `M3-dev-gfb5cbca13f33` / `fb5cbca13f33d21dfb4e16e0ba667c340399eb0c` |
| Running slot / image state | `ota_0` / VALID |
| Image size | 2,573,040 bytes |
| Image SHA-256 | `17a5c82ee5ac1944bf756918d1f4094a5f67bf9ab66f60dcb21ce4aa42f1015b` |
| ELF SHA-256 | `e6382dff0c82d9db463e03b956f435ad2d6bad2f4fdee9f9d476cd152d1505a7` |
| Bundle SHA-256 | `1f6cff625d0db95e5be4617bfb1100aced4ee2dd2908ec06efa3618a94711cb3` |

The immutable accepted package was not rebuilt or replaced. Runtime source/ELF,
slot and VALID state were checked after each negative case and both rollbacks;
the retained package hashes were rechecked at the end. This is API/journal and
artifact evidence, not a new full-flash readback. Earlier exact-image
[waveform and Campaign B acceptance](M3_SHARED_WAVEFORM_REGRESSION_20261006.md)
remains scoped to that same binary.

## Negative upload cases: PASS

Local tests used the existing marked `POST /api/ota/p4` path and trusted signing
key. The wrong-board package contained a real JC4880 application descriptor;
its signature and the deliberately wrong-chip package signature were verified
offline before testing. Invalid bundles were retained only as test artifacts.

| Input | Device response / observed outcome | Result |
| --- | --- | --- |
| Signed JC4880 `main-deck-p4` package | HTTP 400, `wrong manifest project` | PASS |
| Signed M3 package with wrong chip | HTTP 400, `wrong manifest chip` | PASS |
| Corrupted manifest signature | HTTP 403, `Invalid OTA manifest signature` | PASS |
| Raw application `.bin` | HTTP 400, `bad bundle magic` | PASS |
| Correctly signed manifest, corrupted image body | HTTP 400, `Firmware validation failed`; image SHA mismatch, transfer failed | PASS |
| Correct signed upload interrupted after 131,072 bundle bytes | 130,884 image bytes received; transfer failed with `HTTP upload interrupted` | PASS |

The first four cases sent a 1,024-byte prefix with the declared full bundle
length. The firmware rejected immediately, without changing OTA state, target
slot, expected size or received size. Together with the handler's validation
ordering, this verifies early rejection before `p4_ota_begin`/partition erase;
no direct inactive-slot flash readback was performed. Corrupt-body and interrupted
cases exercise writing the inactive slot and aborting before selecting it for
boot. They did not restart or alter the running accepted image. The resulting
failed transfer state was subsequently cleared by a successful diagnostic OTA.

## Isolated fault images

Only two M3 diagnostics were built, sequentially, in the detached
`D:/AI/Pajoniiir-m3-ota-diag` checkout of the exact accepted source. No production
source edits or dependency upgrades were made. Both ESP-IDF **v6.0.2** builds
exited 0, retained the committed dependency lock and passed the `0x380000` budget.
Source/build identity remained clean. No JC4880 or JC1060 rebuild was run here.

| Diagnostic | Fault setting | Image bytes | Image SHA-256 | ELF SHA-256 |
| --- | --- | ---: | --- | --- |
| Unconfirmed restart | `CONFIG_DDJ_OTA_FORCE_ROLLBACK_TEST=y` | 2,572,896 | `c54ba99f8bb8ce2bb7d6b711dd902eab697022cd6fbfa90ca903398834bafe52` | `c40d06ccaf64b8645341cfa1e6eadac4741fa41717dc74c65eb191fc6f047997` |
| Readiness timeout | `CONFIG_DDJ_OTA_STARTUP_TIMEOUT_TEST=y` | 2,573,072 | `e618d1dea3ad8fa84ad0ed5b99e82875aa1a7a8aedd63a9f8a3753619cce97a0` | `11bd951e6b50ba568672782a11d7223f837e16dcbf82f429c0ed8bf4a808788d` |

These images deliberately retain the same development version/source identity;
their different ELF hashes and saved fault configurations distinguish them.
Each enabled only its designated fault flag and retained bootloader rollback
support. Both ordinary production-packager attempts rejected them with
`OTA fault-injection images cannot be packaged as a production release` and
created no release output. Explicit test-only signed bundles remained under
ignored `.cache`, outside release/channel packaging.

## Unconfirmed-image restart: PASS

The signed diagnostic upload returned HTTP 200. Boot journal transitions were
**344 -> 345 (ota_1, SW) -> 346 (ota_0, SW)**. The diagnostic restarts before
`esp_ota_mark_app_valid_cancel_rollback`; the bootloader then returns to the
previous VALID image. Exact accepted source/ELF/ota_0 returned, followed by
FLX4 MIDI In/Out/UAC and all 324 catalog tracks. Five stopped-state stability
checks passed. No operator reset was needed.

Host recovery was observed at 41.22 seconds from upload start, including upload,
Windows Wi-Fi reassociation and HTTP observation. The brief pending diagnostic
was not directly observed through HTTP; its boot in ota_1 and subsequent return
are recorded in the journal. Do not label host observation latency as firmware
startup duration. The helper exited 0.

## Startup readiness deadline: PASS with retained helper error

The signed timeout diagnostic upload returned HTTP 200. The actual AP/API stayed
alive while the existing injection forced the health gate's network readiness
input false. HTTP captured **45** diagnostic-ELF snapshots in ota_1
`pending_verify`, at device uptimes **8,635 through 59,624 ms**. Transfer state
was `idle` during these observations; it did not imply image acceptance.

The journal then recorded:

```text
seq=15 boot=347 ms=60193 level=E event=P4_OTA_FAILED a0=2 a1=60193 msg=startup readiness rejected
```

`service_log_event` takes argument count first, followed by the recorded values:
`a0=2` is `P4_STARTUP_TIMEOUT`, and `a1=60193` is the rejection uptime. The
deadline therefore rejected at **60.193 seconds**. Boot transitions were
**346 -> 347 (ota_1, SW) -> 348 (ota_0, SW)**. The exact accepted ELF returned
VALID in ota_0 automatically. Host observation was 88.82 seconds from upload
start; it includes upload and reassociation and is not the gate deadline.

The initial helper exited 1 because its parser incorrectly expected a third
journal argument. Its raw `timeout/result.json` remains **FAIL**, with the exact
assertion and all observations preserved. The actual rejection event was already
present. A separate read-only assessment used the correct formatter/API contract,
verified the event, pending snapshots, boot transitions and final runtime, and
recorded **PASS_DEVICE_OTA_GATES**. The test was not repeated and no raw outcome
was overwritten. This is a harness error, not an observed firmware rollback
failure. The test simulates false readiness; it does not establish behavior
under an actual Wi-Fi hardware failure.

## Final state, checks and limits

Final live assessment at 2026-10-07 15:01:27 UTC confirmed exact accepted
source/ELF, ota_0 VALID, OTA idle with empty error, both decks stopped,
PCM5102A configured as MAIN at fixed 48 kHz, FLX4 MIDI In/Out/UAC and active
`pioneer_ddj_flx4` profile, mounted storage and 324 tracks. `/api/resources`
reported startup `ready`, zero allocation failures and zero critical failures.
PCM underruns, UAC dropped/overflow/packet losses and service-log drops were
zero; the current WDT flag was false. Idle UAC underflow was nonzero and is not
active playback loss evidence. No new listening/visual acceptance was requested.
No persistent settings write or full-chip erase was performed.

Focused production-module host suites `p4_ota_policy`, `ota_manifest`,
`p4_startup_gate` and `firmware_health` passed, runner exit 0. The earlier full
host/software gate remains separate; this run did not repeat it or claim a new
full qualification.

Raw artifacts and scripts:

- `.cache/m3-migration/20261007-fb5cbca1/ota-negative/`: requests, before/after snapshots and six-case PASS.
- `.cache/m3-migration/20261007-fb5cbca1/ota-health/`: diagnostic configuration/packages, upload responses, observations, journals, raw results, production-packager rejection, final snapshots and `assessment.json`.
- `.cache/test_m3_ota_negative.py`, `.cache/test_m3_ota_health.py`, `.cache/assess_m3_ota_health.py`.
- `D:/AI/m3-fb5-ota-{negative,rollback-test,timeout-test,rollback-build,timeout-build,host-tests}.log`.

This closes the autonomous signed-upload rejection/interruption and injected
unconfirmed-restart/readiness-timeout rollback gates for the shared M3 runtime.
Physical Campaign A remains **SKIPPED BY OPERATOR**; GitHub Actions for this
source remains **SKIPPED BY OPERATOR / quota**, not PASS. Other-board/controller
hardware, full per-zoom dwell and remaining library/control integration gates
retain their existing open status. This record does not establish a production
release or authorize a remote push, release tag or public OTA channel update.
