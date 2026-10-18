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
resource-budget missing-evidence tests pass. Full host qualification and the
ordinary M3 ESP-IDF 6.0.2 build are required before candidate delivery.
Only M3 is built for this investigation; JC4880/JC1060 hardware acceptance,
the JC4880 soak waiver and the operator-closed S2 scope are unchanged.

The next bounded M3 runner must persist the failing sample before checking it,
record action intent before each loop-clear command and stop on any new strict
counter. It must retain exact image/rates and operator evidence separately.
No new long soak or public publication is authorized or inferred by this fix.
