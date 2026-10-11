# S2 MK3 HS ISO loss: evidence and scheduling analysis

Date: 2026-10-10. Analysis only; no firmware, board configuration or device
state was changed. Source under analysis: clean
`fc4ed60878c99212033a5d2872de1d6395b3185c`, ESP-IDF 6.0.2, USB dependency
`cc65dc268f9fb6e89b8b3c6c9e94f5aa1dbb2ccb` plus project source transforms.

## Finding

The final run is a genuine strict packet-loss failure, not a host-monitor
disappearance. The class driver observed two packet descriptors with SKIPPED
status, zero actual bytes, and 80/96 requested bytes. The exact lower-level
reason those descriptors remained unexecuted is not recorded. Host-side ISO
scheduling/DMA completion visibility is the leading investigation area, but
the current evidence does not prove an interrupt deadline miss, cache fault,
FIFO fault, cable fault, or controller fault individually.

Internal descriptor and payload placement did not eliminate the failure.
Repeated short playback success is insufficient to close this rare-event gate.
The next firmware should improve causal capture before another tuning change.

## Exact final run

Evidence directory (relative to repository root):
`.cache/s2-mk3/next-gates-20261009/internal-payload-soak-durable-20261009T235253Z/`.

- Installed version `M3-dev-gfc4ed60878c9`, VALID `ota_0`, boot 407 POWERON.
- ELF SHA-256 `fb7ad3e0f8d758fc071730bd88bd3027b84b2f0770abb43738645ce0c99137d4`.
- Dual source rates 44100/48000 Hz, pitch +5/-5%, MT and four-beat loops,
  zoom 8, MAIN 48000 Hz, S2 UAC2 HS 44100 Hz, Wi-Fi normal 20 dBm.
- Runtime 529.275772 s, 783 status samples, wrapper exit 1, complete false.
- ISO packet failures +2; lost audio frames +11; other 20 strict deltas zero.
- Output-late +1, maximum in corresponding journal event 11031 us.
- Both decks stopped in final snapshot and live post-failure readback.
- Final firmware snapshot matches exact clean source/ELF and VALID slot.
- Operator explicitly noticed no audio, waveform or controller disturbance.

The host successfully saved result/final snapshots and exited. The one-time
Scheduled Task was removed after exit and heartbeat paused. This is FAIL,
not the earlier INCOMPLETE case where the observer disappeared.

### Fault reconstruction

At device uptime 818456 ms, journal records identify zero-based packet indices
1 and 2 of 56, class-driver transfer slot 0, status 6 (SKIPPED), actual bytes 0,
requested bytes 80 and 96, callback gap 7 RTOS ticks. With four channels and
four-byte subslots, the accounting is `(80 + 96) / 16 = 11` audio frames.
At 44100 Hz this is approximately 0.249 ms of scheduled audio data. This is
not an acoustic measurement of an audible gap; receiver buffering/concealment
and perception are not captured by these counters.

`_buffer_parse_isoc()` maps `USB_DWC_HAL_XFER_DESC_STS_NOT_EXECUTED` to SKIPPED.
The LL descriptor reader reports NOT_EXECUTED when the descriptor's active
bit remains set. This differs from an explicit packet error or buffer error.
The code can mark the whole URB COMPLETED while individual packets are SKIPPED;
the added per-packet counters are therefore necessary and must not be relaxed.
Raw descriptors are cleared during parsing, so they are unavailable afterward.

Only these two packets produced failure counts. That supports a localized
early-block problem, not loss of an entire URB. It does not prove packet 0
executed at its intended microframe, or rule out a near-boundary activation.

The output-late phase was at uptime 483192 ms, approximately 335.264 seconds
before the ISO fault. The journal's summary of that late event followed at
486078 ms. There is no temporal evidence that the same event caused both.

## Comparison of actual recorded runs

All paths below are under `.cache/s2-mk3/next-gates-20261009/`.

| Candidate | Relevant change | S2 rate | Observed run | Result |
|---|---|---:|---:|---|
| 00a1ca90 | 1-KiB task cache bound | 44100 | 514.981 s | 2 packets / 11 frames, late +2 |
| 17f75286 | IRQ priority and post-enable HFNUM capture | 44100 | 90.586 s | 2 packets / 11 frames, late +0 |
| 3f3605a7 | Internal ISO descriptor pool | 48000 | 600.408 s | Short telemetry/operator PASS |
| 3f3605a7 | Same image, independent long run | 48000 | 1336.174 s | 2 packets / 12 frames, late +0 |
| fc4ed608 | Internal ISO payloads as well | 44100 | 60.477 s | Short telemetry/operator PASS |
| fc4ed608 | Same image, independent long run | 44100 | 529.276 s | 2 packets / 11 frames, late +1 |

Result directories: `iso-rearm-mt-loop-20261009T213958Z`,
`irq-priority-mt-loop-20261009T221436Z`,
`internal-isoc-mt-loop-20261009T222957Z`,
`internal-isoc-soak-durable-20261009T231506Z`,
`internal-payload-mt-loop-20261009T234926Z`,
`internal-payload-soak-durable-20261009T235253Z`.

The earlier 3f3605a7 attempt ending at 1032.793 s without a result remains
INCOMPLETE and is excluded from successful duration comparisons. Candidate
changes, clock changes and few observations prevent estimating comparative
failure rates. A longer time to first fault is not evidence that a candidate
is more reliable. The problem has been observed at both audio rates.

## What the compiled code actually does

The final HCD compiled by `build_s2_hs/build.ninja` is
`pajoniiir_usb_idle_recovery/hcd_dwc.c`, not the intermediate FIFO-patched
`pajoniiir_usb/hcd_dwc.c`. This distinction matters: the final transform already
moves completed IN/control payload cache maintenance outside the HCD lock
and splits owned URB maintenance into 256-byte calls. Repeating that fix would
not address the present failure.

The ISO OUT path is:

1. `prepare_and_submit()` reads PCM, packs four channels, fills 56 packet
   lengths and submits one of three class-driver URBs.
2. HCD fills one of two DMA descriptor lists. Each active packet gets a QTD;
   the last QTD has HOC, so hardware halts the channel at the block end.
3. An initially empty schedule chooses current HFNUM + 1 + 3 microframes.
   An already queued next block uses the previous block's next start index.
4. On completion, HCD marks the old block done and activates the next prepared
   block before parsing/refilling the old one.
5. HAL writes the list address/index and length, then enables the channel.
6. The class callback counts packet-level failures and refills its returned URB.

The 56-packet block covers 7 ms at one packet per 125-us HS microframe. Two
prepared HCD buffers help the producer keep up, but do not give software a
14-ms allowance to re-enable the channel at a block boundary. The next block
is scheduled contiguously. The three-microframe startup margin is not an
inserted gap at every block boundary. There are approximately 143 such
OUT block transitions per second.

At `CONFIG_FREERTOS_HZ=1000`, a callback gap of 7 ticks is normal for this
block size. It is measured in the client task, after hardware completion and
ISR processing. It cannot resolve a 125-us scheduling miss.

Internal RAM on ESP32-P4 still uses L1 cache. The current internal descriptor
and payload allocations retain cache synchronization; they do not make the
USB path cache-independent. The 1-KiB wrapper bounds selected task-context
operations by bytes, not their actual duration or all critical sections.
The final HCD still has a global `hcd_lock` shared across roots and performs
descriptor maintenance while handling channels. These are timing candidates,
not measured offenders in this run.

## Diagnostics: what they establish and what they miss

### Existing late-start trace is insufficient for exclusion

`usb_dwc_hal_compat.c` samples HFNUM after HAL activation and emits kind 3
only when `(start - (HFNUM & 63)) & 63` is zero or greater than 32.
Consequences:

- A lead of one microframe is never retained, even if almost no time remains
  in the current microframe or hardware needs earlier descriptor fetch.
- The full HFNUM is available only for emitted events; there is no retained
  ordinary activation record for the transfer that later fails.
- There is no timestamp before lock acquisition, activation sequence binding,
  raw QTD snapshot, periodic FIFO snapshot, or expected absolute microframe.
- Modulo-64 lead alone cannot distinguish every long-delay/wrap case.
- The trace retains eight exceptional events per root, not normal transitions.

The final HS trace has total 9 events, with the retained eight all startup port
events and no kind 3/channel-error event around the fault. This proves only
that the existing predicate did not report such an event. It is not proof
that channel activation met every hardware scheduling requirement.

### Logging and polling

The service log grew from 15 to 21 records, queue depth 0 at both ends, dropped
0. Two records concern the earlier output-late event and four describe the
packet fault itself. Those four are enqueued after detection and cannot be
the initiating cause of the first observed loss. The writer uses a bounded
queue and low-priority task; there was no observed log flood.

The monitor used `/api/status`, with median sample spacing 0.656894 s and
maximum 0.929100 s, approximately 1.5 samples/s. It did not request live heap
walks via `/api/resources`. Status still constructs JSON and uses allocations,
locks and Wi-Fi traffic: it is not zero-cost, and its timing effect has not
been isolated in this exact image. Fifteen-minute assistant checkpoints do not
mean the underlying status monitor samples only every fifteen minutes.

No new MIDI input packets, disconnects, recovery requests, BNA recoveries,
PCM underruns, UAC ring drops/overflow/underflow, or invalid feedback were
observed between baseline and final. Storage lifecycle counters do not measure
all bulk I/O, so unchanged mount counters cannot establish an idle USB bus.

## Ranked hypotheses and discriminating evidence

| Priority | Hypothesis | Current support | Missing discriminator |
|---|---|---|---|
| 1 | Boundary activation / periodic scheduler timing | Repeated early-block SKIPPED packets; block rearm required | Matched absolute HFNUM and phase before/after enable and at completion |
| 2 | DMA/QTD visibility, ownership or hardware fetch interaction | NOT_EXECUTED active bits; internal RAM remains cached | Raw first QTDs before clearing, list address, sequence, cache timing |
| 3 | IRQ/critical-section interference from other work | Shared HCD/cache locks and Wi-Fi/display coexist | Measured lock/ISR latency correlated to failed block |
| 4 | Periodic FIFO/resource timing | Hardware fetch cannot be inferred from class callback | FIFO and channel registers at first packet fault |
| 5 | Physical bus/controller/power | Not completely excluded without bus measurements | Bus-level transaction evidence; present run has no reset/disconnect |

The S2 packet counter alone does not implicate the PCM5102A MAIN path, which
has zero PCM underrun deltas and operator-confirmed clean sound. Neither does
this evidence justify blaming MT, changing the accepted waveform scheduler,
reducing Wi-Fi power again, or changing USB ports.

The remaining cached DMA headroom (13995 B after track load, minimum 13087 B)
is a separate lifecycle concern. No allocation failure or recovery occurred in
the failed playback window, so it is not established as this loss's cause.

## Recommended next implementation

1. Add a bounded first-fault capture to the existing HCD transform. Bind the
   activation sequence, root/channel, actual QTD-list address and URB/block
   identity; retain HFNUM including remaining-time bits before/after channel
   enable. Use a small fixed internal structure with documented ownership.
2. At completion, after required M2C synchronization and before destructive
   QTD parsing, freeze the relevant raw descriptors, expected start index,
   HCD buffer/queue state, channel status and periodic FIFO state. Correlate
   to the exact activation. Record at most the first fault; no ISR JSON,
   heap allocation, file/NVS writes, or console logging.
3. Measure entry-to-lock and lock-to-enable timing with a minimal bounded
   mechanism, and verify the observer's cost. A snapshot must distinguish
   stale descriptor visibility from an actually late/scheduler-skipped slot.
4. Host-test the capture's wrap arithmetic, zero-based packet mapping,
   ownership, first-fault freeze, reset/epoch handling, and completeness.
   Verify the final generated HCD and linked image, not only an intermediate
   source or a model of expected behavior. Build only M3 at this stage.
5. Perform one prepared reproduction with the unchanged workload/rate and a
   strict stop at the first fault. If it reproduces, inspect the frozen record
   before any tuning. A short non-reproduction does not close the gate.
6. Fix the measured path. Do not simply add a permanent gap between blocks:
   that changes the supplied audio rate and can create a new underflow.
   A continuous/ring descriptor scheduler is a larger possible redesign only
   if the evidence shows the stop/re-enable architecture cannot meet timing.
7. After a causal fix, run host regressions, M3 build/package verification,
   short physical check and then the final 60-minute exact-image soak.
   Keep 44100/48000-Hz qualification distinct, followed by lifecycle tests.

The OTA-restart PANIC/quarantine remains a separate issue. No new soak,
firmware installation or acceptance claim is part of this analysis.

## Source pointers

- `firmware/p4-core/components/controller_usb_audio/controller_usb_audio_stream.c`:
  block size, packet construction, callback fault accounting.
- `firmware/p4-core/components/usb_storage/usb_dwc_hal_compat.c`:
  cache wrapper and late-start trace predicate.
- `firmware/p4-core/components/usb_storage/usb_bus_trace.c` and include header:
  bounded exceptional-event retention.
- `firmware/p4-core/cmake/apply_espressif_usb_idle_recovery_patch.cmake`:
  final owned-URB cache transform; actual final HCD source identified above.
- `firmware/p4-core/cmake/apply_espressif_usb_isoc_descriptor_patch.cmake` and
  `apply_espressif_usb_isoc_payload_patch.cmake`: current internal allocations.
- Local IDF `components/esp_hal_usb/usb_dwc_hal.c`,
  `esp32p4/include/hal/usb_dwc_ll.h`, `components/esp_mm/esp_cache_msync.c`:
  activation, QTD status decoding and cache serialization.
- [Espressif v6.0.2 DWC host maintainer notes](https://raw.githubusercontent.com/espressif/esp-idf/v6.0.2/docs/en/api-reference/peripherals/usb_host/usb_host_notes_dwc_otg.rst):
  DMA/QTD model, ISO per-packet checking and lack of retry.
