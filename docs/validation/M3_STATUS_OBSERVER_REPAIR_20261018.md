# Shared status observer repair after the M3 headphone smoke

## Problem and evidence boundary

The [headphone candidate smoke](M3_HEADPHONE_PACKAGE_20261018.md) on clean
`71ee5342` ends at 40.642 seconds on 3,387 new D2 PCM underrun counts during
the loop-clear window. The operator reports sufficient headphone volume and
no perceived disturbance. That result remains a telemetry FAIL, not a PASS.

One pre-fault snapshot raises the internal largest-block query wall-time
maximum to 436,751 us. This duration includes preemption and is not measured
IRQ-disabled time. It neither proves the cause of this PCM loss nor identifies
the decoder's exact state when the loss occurs. The failed response was not
appended by the old runner, whose validation ran before sample persistence.

Independent code inspection confirms a real observation-path defect: ordinary
builds invoke `heap_caps_get_largest_free_block` twice for every audio memory
snapshot. The HS diagnostic build already skips these internal/DMA queries.
ESP-IDF 6.0.2 implements largest-block lookup through `heap_caps_get_info` and
`multi_heap_get_info_impl`, walking the TLSF pool under allocator locks.
The cost depends on pool fragmentation and is unbounded by the audio deadline.
FS output and idle UAC silence also need bounded observation; this property
cannot depend on the HS experiment switch.

## Repair and regression

The shared memory snapshot now reads only per-heap free and lifetime low-water
counters. It does not perform a largest-block walk, query a timer or write a
journal. All internal/DMA/PSRAM largest-block fields and walk timings are
explicitly unavailable: zero placeholders in the C snapshot and JSON null in
status. Existing field names and measured free/low-water counters are retained.
Unavailable values do not satisfy the existing resource-budget thresholds;
the checker remains fail-closed, without inventing a fragmentation PASS.

The regression links the production memory module without any largest-block,
timer or service-log implementations. It fails to link against the previous
ordinary implementation (RED), then passes for ordinary and HS configurations.
It checks repeated snapshots and removal of stale values. A second regression
executes the production status formatter in both configurations, verifying
counter values, JSON nulls and argument alignment. The resource-budget test
also rejects explicit null fragmentation values.

No loop, decoder, transport, PCM loss counter, output cadence or headphone
level logic changes in this repair. It removes confirmed unsafe observer work;
resolution of the reported PCM2 loss still requires a targeted exact-image
loop-exit test. The original failure must remain preserved if that test passes.

## Validation and next hardware boundary

Focused ordinary/HS memory regressions, production JSON formatting and
resource-budget missing-evidence tests pass. Full `-Qualification` host suite,
mandatory fixture gate and documentation checks pass. A fresh ordinary M3
ESP-IDF 6.0.2 build, board/component isolation and image-budget verification,
and production signed bundle/manifest verification also pass.
Only M3 is built for this investigation; JC4880/JC1060 hardware acceptance,
the JC4880 soak waiver and the operator-closed S2 scope are unchanged.

The next bounded M3 runner must persist the failing sample before checking it,
record action intent before each loop-clear command and stop on any new strict
counter. It must retain exact image/rates and operator evidence separately.
No new long soak or public publication is authorized or inferred by this fix.

## Exact candidate and authorized OTA

Source `3459a97a5e2060223ebe24272879f0b731d21083` is clean and becomes
`M3-dev-g3459a97a5e20`, project `main-deck-m3`. Effective SDKCONFIG values are
identical to the preceding ordinary M3 build. The signed development bundle
remains local, with no automatic pull-update eligibility or public publication.

| Artifact | Bytes | SHA-256 |
| --- | ---: | --- |
| Application | 2,602,912 | `d60f454304e0a121083af676772583427a735ac1c60d5da2ab581b0731918bbf` |
| Signed OTA | 2,603,100 | `d95234def8ae5201390919c0a19f1d9f4f657de8d64ab1d8240c9303c6fe18df` |
| Matching ELF | retained with package | `8d9d74750a7a1c41aeb1c388f4d67ddc8d6d515561f28b455ea3a06078780880` |

After live board/stopped checks, local OTA completes HTTP 200 in 16.075 s.
The returning API confirms exact source/version/ELF, VALID `ota_0`, idle OTA
service and empty error. FLX4 MIDI and non-faulted 44.1-kHz/16-bit FS UAC,
260 tracks and ordinary USB roles are available. The historical crash summary
is unchanged. Live memory fields retain measured free counters and explicitly
return null for largest blocks and all walk timings. The operator confirms GUI.

The [software and installation evidence](evidence/2026-10-18-m3-status-observer/software.json)
retains RED/GREEN logs, complete host qualification, board identity, signed
manifest, package verification, unchanged configuration and live snapshots.

## Active-control retest: separate queue-admission FAIL

An operator-ready MT/zoom-8 dual-deck retest stops at **25.968 s**, with 2,437
MIDI packets and **19 new local runtime queue failures**. All other 21 strict
counter deltas, including PCM/UAC loss, are zero. One output-late event reaches
10,954 us. The failing response is preserved before validation. Both loops
are cleared and both decks are stopped during cleanup. The test does not reach
loop exit or CUE/PLAY phases.

The operator confirms responsive controls, clean audio/display and sufficient
headphone volume. `local_semantic_callback` counts unsuccessful downstream
admission attempts; `controller_runtime_dispatch_pending` retains rejected work
for retry. This is not proof of 19 permanently lost commands. It remains a
strict gate failure; no counter or acceptance threshold is weakened. Existing
status does not expose pending/coalesced/dropped runtime-buffer diagnostics,
so eventual complete delivery is not established by these snapshots.

The [active-control audit](evidence/2026-10-18-m3-status-observer/active-controls/audit.json)
preserves all 34 samples, action intents, original runner and operator answer.
A preceding preflight rejected default software MIX=MASTER before any PLAY;
the corrected preflight requires minimum LEVEL while MIX is exercised during
the test. That preflight is not a failed playback window.

## Loop-only retest: strict UAC loss remains without heap walks

To reach the original loop-exit window without high-rate MIDI input, a separate
bounded run uses the same image, tracks, MT, pitch and zoom. During preparation,
reloading the same tracks reset MT; rate/MT preflight checks abort before PLAY.
The operator then explicitly re-enables MT on both decks. No further load or
configuration change precedes the actual loop-only run.

This run stops at **42.186 s / 44 samples** on **13 new UAC dropped blocks and
2,320 overflow frames**. All other 20 strict deltas are zero, including PCM1/2,
ISO packet loss and runtime queue failures. The prior queue count of 19 is
retained as baseline, not erased. There are zero new MIDI packets. One new
output-late event reaches 142,441 us; its journal identifies 141,856 us in mix,
with one coarse mix segment (13) taking 139,045 us wall time. This includes
possible preemption and does not identify a specific callback or instruction.

Both loops become active at 27.306 s; clear intents are recorded at 35.221 and
35.704 s. The first subsequent wait-loop sample contains the loss, both loops
OFF, and explicitly unavailable heap walks. Both decks end stopped on the same
VALID image. The operator reports no perceived disturbance. CUE/PLAY phases
are not reached and the intended 60 seconds are not completed.

The [loop-only audit](evidence/2026-10-18-m3-status-observer/loop-only/audit.json)
and raw journal preserve the fault. This proves that removing unsafe heap walks
does **not** close the loop-exit gate. The earlier PCM2 count itself is not
reproduced; a different strict UAC failure still prevents acceptance. Keep the
observer repair, but investigate output-mix callback work versus preemption
during loop exit before another decoder/DSP change. No further firmware change,
speculative workaround, long soak or repeated unattended playback is performed.
