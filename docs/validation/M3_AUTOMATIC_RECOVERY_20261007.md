# M3 automatic recovery supplement and Campaign A exclusions

## Scope authorized by the operator

On 2026-10-07 the operator requested:

> napravi campain a ali preskoči sve što uključuje moju intervenciju

All 30 prescribed Campaign A cycles require physical power or cable actions.
They were excluded by that instruction; this is a recorded operator waiver,
not a physical recovery PASS. No further operator action was requested.

| Physical Campaign A scenario | Planned | Executed | Status |
| --- | ---: | ---: | --- |
| Cold boot with both devices attached | 10 | 0 | SKIPPED BY OPERATOR |
| FLX4 unplug/replug, decks stopped | 10 | 0 | SKIPPED BY OPERATOR |
| USB3-first enumeration | 5 | 0 | SKIPPED BY OPERATOR |
| FLX4-first enumeration | 5 | 0 | SKIPPED BY OPERATOR |
| Physical LEDs, control gestures and listening after each cycle | per cycle | 0 | SKIPPED BY OPERATOR |

## Automated supplement

The existing guarded bodyless `POST /api/validation/reboot`, marked with
`X-DDJ-Control: 1`, was used with both decks stopped and both USB devices
attached. It invokes the firmware's software restart path. This does not
remove power from the M3, unplug a device or control physical enumeration order.
No firmware, OTA image, controller profile or persistent network configuration
was changed to run the test.

Exact installed identity: board `m3`, project `main-deck-m3`, clean source
`fb5cbca13f33d21dfb4e16e0ba667c340399eb0c`, version `M3-dev-gfb5cbca13f33`,
ota_0 VALID, ELF SHA-256
`e6382dff0c82d9db463e03b956f435ad2d6bad2f4fdee9f9d476cd152d1505a7`.
The same FLX4 remained on USB2 and the 324-track medium on USB3. Wi-Fi stayed
saved-enabled. The earlier exact-image 60-minute Campaign B remains separate.

The automatic helper records pre/post source/slot identity, advancing journal
boot epoch, `SW` reset reason, restored FLX4 MIDI In/Out/UAC and active profile,
mounted storage and exact catalog count, recovery accounting, strict counters
and ten stopped-state stability samples per cycle. It stops on failure rather
than requesting a physical reset or cable action. Idle UAC underflow history
and historical persisted WDT traces are not active playback loss claims.

The donor Group K harness was inspected but not run wholesale: it assumes
100 tracks, applies a zero-output-late rule and requires physical PLAY/listening
confirmation. Its reboot/log/accounting helpers were reused, preserving the
M3 monitoring policy and the operator's exclusions. This supplement does not
claim full Group K acceptance either.

Raw artifacts: `.cache/m3-migration/20261007-fb5cbca1/automatic-reboots/`.
Helper: `.cache/run_m3_fb5_automatic_reboots.ps1`.
Console log: `D:/AI/m3-fb5cbca1-automatic-reboots.log`.

## Timing interpretation

API/controller/library observation times are measured from the reboot request
and include firmware restart, Windows Wi-Fi association, DHCP and HTTP queries.
They are upper bounds on readiness, not isolated firmware initialization times.
The first cycle lost the host association; the agent requested reconnection to
the existing `Pajoniiir-M3` Windows profile without human input. No password or
profile configuration was changed. Subsequent host observation latency is
retained with the raw records; it must not be silently replaced by journal times.

Readiness observations above 15 seconds for the controller or 20 seconds for
the library remain investigation flags. Journal `CONTROLLER_CONNECTED`,
`PROFILE_TRANSFER_DONE`, `LIBRARY_LOADED` and `WIFI_STARTED` times help distinguish
device initialization from host visibility, but do not independently establish
physical LED correctness or fully ready UAC endpoints.

## Completed results

Ten software restarts were executed, journal boot epochs **334 through 344**.
All ten eventually showed the same clean source/ELF and VALID ota_0, FLX4
MIDI In/Out/UAC, active `pioneer_ddj_flx4` profile, mounted storage and all
324 tracks. Both decks remained stopped; no physical operator action was used.

Nine cycles have complete automated per-cycle telemetry checks. Cycle 6 has
verified eventual recovery (339 -> 340, `SW`) but incomplete first-readiness
timing. The initial helper compared post-reboot uptime only with the old
snapshot; that cannot identify a reboot when late host reconnection has already
exceeded the old uptime. The observation did not finish and the agent stopped
its own helper, preserving the first five results and cycle-6 recovery evidence.
The resumed helper accounts for elapsed time, uses bounded HTTP tasks and
requests host profile reconnection automatically. It executed only cycles 7-10;
cycle 6 was not repeated or silently converted into a timing PASS.

| Cycle | Boot transition | Host-observed controller ready (s) | Catalog ready (s) | Outcome |
| --- | --- | ---: | ---: | --- |
| 1 | 334 -> 335 | 76.52 | 76.60 | Recovered; host visibility investigation |
| 2 | 335 -> 336 | 16.91 | 17.01 | Recovered; controller visibility investigation |
| 3 | 336 -> 337 | 9.18 | 9.25 | Telemetry PASS |
| 4 | 337 -> 338 | 14.80 | 14.86 | Telemetry PASS |
| 5 | 338 -> 339 | 10.42 | 10.54 | Telemetry PASS |
| 6 | 339 -> 340 | unavailable | unavailable | Recovered; timing INCOMPLETE |
| 7 | 340 -> 341 | 12.91 | 13.00 | Telemetry PASS |
| 8 | 341 -> 342 | 11.06 | 11.10 | Telemetry PASS |
| 9 | 342 -> 343 | 12.94 | 13.03 | Telemetry PASS |
| 10 | 343 -> 344 | 11.01 | 11.11 | Telemetry PASS |

The fully measured cycles had zero post-boot PCM underruns, UAC dropped
blocks/overflow/packet losses, service-log drops, recovery failures/queue drops
and runtime queue failures, with balanced recovery accounting. Their output-late
counts were also zero while stopped. Cycle 6's recovered snapshot independently
shows zero PCM/UAC drop/overflow/service-log losses. This is idle/recovery
telemetry, not an active playback/listening test or a new zero-late soak.
The first cycle journal reports `WIFI_STARTED` at 3458 ms, catalog ready at
2565 ms and the last profile transfer at 2629 ms; its 76-second host observation
must not be reported as those firmware initialization times.

Combined supplement result: **INVESTIGATE_TIMING_COVERAGE**, with no observed
device hard failure, two host visibility threshold flags and one incomplete
timing window. The resumed four-cycle helper exited 0; the first helper was
stopped by the agent and did not produce a normal result. Its raw records and
console log remain separate from the resumed records and combined assessment.
`combined-result.json` links the evidence; `resume/result.json` covers only the
four resumed cycles and must not be presented as a ten-cycle PASS.

Final live status confirms both decks stopped, FLX4 MIDI/UAC active, 324 tracks,
Wi-Fi available, exact source/ELF and ota_0 VALID with OTA idle. Cold boots,
physical reconnect/order tests, LEDs, gestures and new listening checks remain
**SKIPPED BY OPERATOR**. Campaign A has no physical PASS. No firmware changes,
release publication or remote push were performed.
