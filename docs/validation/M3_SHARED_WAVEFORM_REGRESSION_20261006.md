# Shared M3 dual-waveform regression, 2026-10-06

## Failed image and reproduction

Image `M3-dev-g11574f4eff16`, clean source
`11574f4eff162d74cdc9f2c17601fe2941139f3d`, M3 ota_1 VALID. Exact hashes are
in [the candidate record](M3_SHARED_CORE_11574_20261006.md).

The operator confirmed MT enabled on both decks and the required zoom setup.
D1 key 5 played a 44.1-kHz source at +5%, loop 30,000–32,400 ms; D2 key 139
played a 48-kHz source at -5%, loop 30,000–31,920 ms. PCM5102A output was 48 kHz,
FLX4 MIDI/UAC ready, USB3 catalog 324 tracks and Wi-Fi enabled.

The 60-minute TimingSoak attempt started at 20:48:03 Europe/Zagreb. The operator
reported both main waveforms looked as if moving through water, at approximately
eight visible beats. This is a hard physical failure under the campaign rules.
Playback was stopped after capturing status/resources/firmware and diagnostic
logs, and the monitor was interrupted. It captured 256 rows over 76.68 seconds,
254 with both decks playing. No completed monitor result or 60-minute PASS exists.
The two final stopped rows are the deliberate diagnostic stop.

At the failure snapshot, PCM underruns, UAC dropped/overflow frames, output-late
and service-log drops were all zero. The existing idle UAC underflow counter
remained 10,356,091 during active playback; this is not an absolute zero-underflow
claim. These counters do not override the operator's visual failure. No explicit
audio listening PASS was supplied.

The operator then confirmed:

- Solo D1 with the same MT/zoom: sharp and fluid.
- Dual playback with the continuous API monitor stopped: both waveforms still
  deform. Polling is therefore not a sufficient explanation.

The live web LOAD LOCK test also returned HTTP 409 on both playing decks and
preserved title/play state. That functional result is separate from visual failure.

Private raw evidence is under
`.cache/m3-migration/20261006-11574f4e/{timing-soak,operator-failure}`.
The earlier local delivery ZIP is an immutable preflight snapshot; this failure
record supersedes its physical NOT RUN/partial status without rewriting artifacts.

## Repair candidate

The common frame orchestrator already selects M3 waveform-first and dual
top-to-bottom policy. However, each deck's combined update performs added
status/loaded-track/artwork work before writing its waveform, then repeats the
same sequence for the other deck. Consequently those noncritical operations
still precede one or both PPA transfers despite the outer waveform-first flag.

The repair candidate uses one shared scheduler to execute both waveform phases
before either deck's chrome for boards requesting waveform-first. Other boards
retain their combined per-deck path. Interpolation runs once per deck per frame,
and chrome reuses that position. Dual redraw cadence and top-to-bottom order are
preserved. LVGL stack high-water scanning moves after frame/render work.

No DSI timing, lane, burst mode, pixel format, panel init, audio/DSP or waveform
geometry changes are included. The ordering correction is a candidate repair;
the precise physical timing cause and its effectiveness still require retest.

Software gates: production scheduler phase-order/budget/fairness tests, full
host qualification, reviewed simulator baselines and ESP-IDF builds. Physical
gates: solo/dual at all zooms, artwork/library/Wi-Fi load, clean MAIN/PFL audio,
then restart the full 60-minute soak on the new exact signed image.

The local full host qualification completed with exit 0 and no optional SKIP.
The extended production scheduler gate passed. M3, JC4880 and JC1060 simulator
presentations retained their existing screenshot hashes, including M3's five
zoom views. A local ESP-IDF 6.0.2 M3 compile and documentation integrity passed.
Clean committed builds, exact-source CI, signed packaging, installation and
operator retest are separate subsequent gates; no repair PASS is inferred here.

## Installed repair candidate

The new signed candidate is now installed and VALID in M3 `ota_0`:

- Version `M3-dev-gc698fa672bc1`.
- Clean source `c698fa672bc1122224b130e35721f8fdb350768b`.
- ELF SHA-256 `73f34062b4d791de891fa80191fa6cd2ca38915aa62ddb3f7957d056602e3029`.
- Image 2,566,160 B, SHA-256 `ae36858df995823d4c1046abd0a513e3ce4f219ae5518838961cc3453adc407d`.
- Bundle 2,566,348 B, SHA-256 `6e717a89e1859cd2a34e171c667d21a6aca4acce70cdb0cb9219259e2aeac2ad`.

All three clean-source ESP-IDF 6.0.2 ordinary builds and unchanged dependency
locks passed. All three signed packages passed candidate verification. The
exact-source [12-job matrix](https://github.com/dvucinozd/Pajoniiir/actions/runs/37516104001),
[USB gate](https://github.com/dvucinozd/Pajoniiir/actions/runs/37516104138) and
[documentation gate](https://github.com/dvucinozd/Pajoniiir/actions/runs/37516104057)
passed. A separate GitHub Advanced Security run on the preceding documentation
commit failed with HTTP 402/monthly quota; it did not complete a security review.

Signed OTA returned HTTP 200 and runtime confirmed the exact board/project/source/
ELF, VALID health and idle service. The 324-track catalog remounted and both
selected tracks again reached READY at 44.1/48 kHz. Opposing pitches and loops
were restored while stopped. MT reset on reboot and must be enabled by the
operator before the comparable retest. Wi-Fi remains enabled.

The operator enabled MT on both decks and selected eight visible beats. Both
decks were started with the same opposing pitches and 44.1/48-kHz loops, without
continuous API polling. The operator reported the watery deformation was still
present and that the image now also stuttered. Audio did not stutter according
to the operator. This candidate therefore also fails the physical visual gate.
Status/resources/firmware and diagnostic logs were captured before deliberately
stopping both decks. The new full 60-minute soak was NOT RUN.

At the failure snapshot, PCM underruns, output-late, UAC dropped/overflow frames
were zero and active UAC data-loss was false. The idle UAC underflow count was
447,174; it is not an absolute-zero claim. Allocation and critical allocation
failures were zero. These measurements do not overrule the visual failure.

The ordering correction alone is insufficient. The next candidate adds bounded
RAM-only scanout timing measurements exposed by read-only `/api/ui-timing`:
refresh wake latency, coalesced refreshes, frame intervals, Overview entry,
callback/handler duration and per-deck cache/blit/finish timing. No panel timing,
cadence or rendering policy is changed for this measurement step. Histograms
use upper bounds 1/2/4/8/12/20 ms and a final >20-ms bucket; deltas between
snapshots isolate each playback window from startup/load outliers.

Installation and software verification are not a physical repair PASS.
Raw installation and preflight evidence is private under
`.cache/m3-migration/20261006-c698fa67`. No public release/channel was modified.

## Installed measurement candidate and measured failure

Signed candidate `M3-dev-g2fa8a6373847`, clean source
`2fa8a6373847decad971a815e1bf6d817a8c5ad1`, is VALID in `ota_1` after HTTP 200 OTA.
ELF SHA-256 is `1cb10ca022ceec0e4b6cbc17d372af699a69682c3fa5296bb324796b944f58d3`.
Image: 2,568,176 B, SHA-256
`ba61d232b8dda816ff7fe45a314f6bb628da165cf7134e3c0d15e3ee8a75feae`.
Bundle: 2,568,364 B, SHA-256
`42e5e0a2f99b3d72487908ea0b5f324f0749f2f07b8b4a69725329ef63b6f535`.

All three clean-source ordinary builds, unchanged locks, signature/project/budget
checks and local full host qualification passed. Exact-source CI passed:
[12-job matrix](https://github.com/dvucinozd/Pajoniiir/actions/runs/37518777918),
[USB gates](https://github.com/dvucinozd/Pajoniiir/actions/runs/37518777839),
[documentation](https://github.com/dvucinozd/Pajoniiir/actions/runs/37518777872).
The separate [Advanced Security run](https://github.com/dvucinozd/Pajoniiir/actions/runs/37518788650)
failed with HTTP 402/monthly quota; no security review completed.

The operator re-enabled MT on both decks and eight-beat zoom. Three approximately
12-second windows used the same sources, pitches and loops; only initial/final
API snapshots were taken, without continuous polling. Both decks were stopped
automatically at completion. Raw records are private under
`.cache/m3-migration/20261006-2fa8a637/mt-8-beat`.

| Measurement | Stopped | Solo D1 | Dual |
|---|---:|---:|---:|
| Refresh interrupts | 608 | 607 | 606 |
| Coalesced refreshes | 0 | 0 | 57 |
| Frame interval mean, us | 19,999.1 | 19,999.1 | 22,063.2 |
| Wake latency mean, us | 12.5 | 188.6 | 2,631.9 |
| Overview entry after refresh mean, us | 96.5 | 415.3 | 3,827.6 |
| Callback duration mean, us | 475.3 | 5,652.7 | 17,165.7 |
| D1 cache / blit mean, us | No redraw | 186.9 / 3,841.5 | 211.9 / 4,173.8 |
| D2 cache / blit mean, us | No redraw | No redraw | 237.4 / 4,565.5 |
| D1 finish after refresh mean, us | No redraw | 4,623.4 | 8,636.6 |
| D2 finish after refresh mean, us | No redraw | No redraw | 15,521.3 |

There were 549 dual redraws; 42 D1 finishes and 113 D2 finishes exceeded 20 ms.
The operator confirmed solo D1 sharp, dual deformed, audio clean. PCM underrun,
UAC dropped/overflow and output-late counters stayed zero in this short test.
This is a visual FAIL and diagnostic measurement, not timing-soak acceptance.

The next focused repair removes the wrapper's duplicate whole-strip source
cache sync: the pinned IDF 6.0.2 PPA SRM driver already writes back its complete
input row window and invalidates the destination before DMA. Cache maintenance
is then owned by the driver and included in PPA duration. It also rebuilds the
waveform-first frame context after Library only when UI track/ANLZ publication or target/tab
changes. A changed load or media clear still refreshes immediately; unchanged
playback avoids a second round of audio/file-mutex-backed reads. Other board
policies retain their existing refreshed-context behavior. Panel timing,
two-deck redraw order, cadence and geometry remain unchanged. Quantitative and
operator retests must establish whether this repair is sufficient.

## Installed cache/context repair, 2026-10-07

The live runtime now confirms `M3-dev-g180b008c4f19` in VALID `ota_0`:

- Clean source `180b008c4f191456b98645aaf55f324d2a33a6ae`.
- ELF SHA-256 `791667086cc29dd7fb53119c9d89ce3fc8d8eca3f7a29b94b5913a409b8341cf`.
- Image 2,568,304 B, SHA-256 `3ce734e567dda38abe2a103b6f5133d922325b0231aa723c4168e650c87b0728`.
- Bundle 2,568,492 B, SHA-256 `4a56b5c0d27cb94b8d4db854813fe5f8cb33666976122905577c561ab9b29f3d`.

All three clean-source ordinary ESP-IDF 6.0.2 builds, unchanged locks, signed
packages and candidate verification passed. Full local host qualification and
all three product simulator presentations passed without baseline changes.
Exact-source CI passed: [12-job matrix](https://github.com/dvucinozd/Pajoniiir/actions/runs/37521456636),
[USB](https://github.com/dvucinozd/Pajoniiir/actions/runs/37521456864),
[documentation](https://github.com/dvucinozd/Pajoniiir/actions/runs/37521456739).
The separate [Advanced Security review](https://github.com/dvucinozd/Pajoniiir/actions/runs/37521478181)
failed with HTTP 402/monthly quota; no security review completed.

The upload returned HTTP 200. The installation script's 100-second polling
deadline then expired without confirming the new image. The later live check
confirmed exact source/ELF/board/project, VALID and idle. This distinguishes
successful current installation from an unestablished earlier startup timing;
it does not prove that the image became VALID within the polling deadline.
Raw upload evidence remains under `.cache/m3-migration/20261006-180b008c`;
later runtime verification and retest evidence are under
`.cache/m3-migration/20261007-180b008c`.

The 324-track catalog and FLX4 returned. D1 44.1 kHz / +5% and D2 48 kHz / -5%
were loaded, with the same 30-second loop starts. Both decks remain stopped
until the operator enables MT and eight-beat zoom for the comparable retest.
The operator enabled MT on both decks and eight-beat zoom. The same three
approximately 12-second windows completed and automatically stopped both decks.

| Measurement | Stopped | Solo D1 | Dual |
|---|---:|---:|---:|
| Refresh interrupts | 605 | 605 | 603 |
| Coalesced refreshes | 0 | 0 | 55 |
| Frame interval mean, us | 19,999.1 | 19,999.1 | 22,006.3 |
| Wake latency mean, us | 45.5 | 149.9 | 2,702.1 |
| Overview entry after refresh mean, us | 131.7 | 391.9 | 4,079.3 |
| Callback duration mean, us | 412.7 | 5,599.0 | 16,613.2 |
| D1 cache / blit mean, us | No redraw | 188.4 / 3,821.9 | 208.5 / 4,170.1 |
| D2 cache / blit mean, us | No redraw | No redraw | 238.0 / 4,542.4 |
| D1 finish after refresh mean, us | No redraw | 4,587.3 | 8,823.2 |
| D2 finish after refresh mean, us | No redraw | No redraw | 15,365.6 |

There were 549 dual redraws; 51 D1 and 109 D2 finishes exceeded 20 ms.
The operator again confirmed solo sharp, dual deformed, clean MAIN/headphones
sound. PCM underrun, UAC dropped/overflow and output-late counters remained zero
in the short test. This candidate fails the physical visual gate; the small
timing differences do not establish a material repair. The new 60-minute soak
was NOT RUN. No public release/channel was modified.

## Nonblocking display observation candidate

The frame still called the blocking decoder-mutex status/position APIs before
both waveform writes: two deck-position reads, two duration queries and an
active-deck loading query. Deck chrome repeated status and LOAD LOCK queries.
The historical M3 duration helper used metadata directly; integration added
decoded-duration selection but also repeated these blocking status reads.
This is a demonstrated blocking code path, not proof that it explains every
cache or scanout outlier.

The next candidate adds a zero-wait production status getter that returns
TIMEOUT without changing its caller-owned snapshot when decode owns the mutex.
One UI-owned observation per deck feeds position, session-checked decoded
duration, loading/error chrome and library progress. Track publication/clear
invalidates the retained observation. The existing position interpolator bridges
ordinary missed observations. Control state and the atomic playing flag are
sampled separately without querying decoder position; LOAD LOCK needs only
that playing flag. Authoritative transport/status APIs remain available for
LOAD/seek/CUE decisions. Scratch position uses the same audible-head calculation
in both status paths, without a nested decoder-mutex acquisition.

Tests exercise the actual production API with a decoder mutex held by another
thread, both decks, unchanged timeout payloads, invalid arguments and recovery
to identical authoritative status. Product simulators now execute the same
display-observation path and check retained duration while busy plus refresh
after release. Full host qualification, clean target builds, exact-source CI,
signed packaging and measured/operator retest are required before acceptance.
Panel timing, geometry, PPA order, redraw cadence and audio DSP remain unchanged.

## Installed nonblocking status candidate, 2026-10-07

Clean source `e4536de2ec6fc9fb25a6e8bbdb17e4a4b4143d9a` passed full host
qualification, unchanged M3/compact/wide product simulator baselines, all three
full-clean IDF 6.0.2 builds with unchanged dependency locks, and signed packaging.
Exact-source CI passed: [12-job matrix](https://github.com/dvucinozd/Pajoniiir/actions/runs/37544368074),
[USB](https://github.com/dvucinozd/Pajoniiir/actions/runs/37544368082), and
[documentation](https://github.com/dvucinozd/Pajoniiir/actions/runs/37544368122).
The separate Advanced Security review failed on monthly quota, not a completed
security assessment.

The signed upload returned HTTP 200 and the helper observed exact clean
source/ELF, VALID `ota_1` and idle within its polling deadline. Exact package
hashes and the diagnostic-overhead audit are in the
[e4536de2 review](M3_DIAGNOSTIC_OVERHEAD_REVIEW_20261007.md).
The comparable MT/eight-beat mixed-rate retest measured 65 coalesced refreshes
and a 16,247.8-us mean D2 finish, with 121 finishes beyond 20 ms in 542 callbacks.
The operator again confirmed solo sharp, dual deformed and clean sound.
PCM/UAC active-window loss, output-late and service-log drop deltas were zero;
the service writer added no records in any measured window. This remains a
physical visual FAIL, not a timing repair. Both decks were stopped; the full
new soak was NOT RUN.

The remaining waveform-phase loop-display queries still acquire the decoder
mutex, including a second query in the armed-loop overlay. The next focused
repair must observe loop state without waiting and reuse it throughout the
frame, preserving authoritative transport APIs and loaded-session fencing.
Diagnostic sampling/UART risks are tracked separately in the audit.

## Nonblocking loop observation repair

The candidate extends the existing coherent zero-wait audio status with active
loop bounds. A short control-shadow merge supplies armed/manual IN state without
querying audio again. UI session fencing rejects observations from a replaced
track; the frame carries one loop display value per deck, reused for both cache
invalidation and the armed-loop burn/blit/restore path. No decoder mutex is
acquired in either waveform-phase loop query. The authoritative loop getter
remains available for transport/controller decisions.

Production status tests cover observed loop set/clear in addition to existing
held-decoder contention and recovery checks. Deck tests cover manual IN shadow
and a supplied observation that deliberately disagrees with the fake backend.
The installed e4536de2 visual FAIL is not superseded by source edits.

## Installed M3-only loop-observation bench candidate, 2026-10-07

Clean source `ee54f0d11cb6a46d487c56c9fcbcdda3a640a270` passed full host
qualification, unchanged M3/compact/wide product simulator baselines and the M3
ESP-IDF 6.0.2 build with its committed dependency lock unchanged. Signed M3
package verification passed. The exact image is installed VALID in ota_0 with
OTA idle; source, clean identity and ELF matched within the install polling
deadline. This proves installation, not panel/audio acceptance.

| Artifact | Bytes | SHA-256 |
|---|---:|---|
| M3 image | 2,568,944 | `87fb8287514d492dba03ddcb3110c95f413bc0a4ead8ca98bbff3788dd98ad95` |
| Signed bundle | 2,569,132 | `185fc281150637eb1aca5dff5040d53fcdc2009054c9737a187a2433063689ff` |
| ELF identity | - | `712be75a1e8e56193c337adf95beba99c80f6021f5c5f1816a072f5418a85360` |

Artifacts and immutable `bench-evidence.json` are under
`releases/m3-bench-ee54f0d1/pajoniiir-m3-M3-dev-gee54f0d11cb6/`.
The manifest records M3 software checks as passed, but full integration software
qualification, hardware acceptance and release remain false.

The operator changed the iteration policy: while the M3 visual defect remains,
build and retest M3 only; perform other target builds/full CI after M3 physical
confirmation. The already-finished JC4880 build is retained as evidence, but
JC1060 configure was interrupted and the push/PR matrix runs were cancelled.
Those deferred gates are not a software failure or a PASS. No new cross-board
build or CI run is needed to diagnose this bench candidate.

The operator reported: "sad jos vise zapinje". This candidate is not visually
accepted. A subsequent API observation found both decks stopped, MT off and
zoom 8. An exploratory non-MT window later changed zoom from 8 to 4 during dual
playback, so it is not a controlled eight-beat comparison. Raw observations are
preserved under `.cache/m3-migration/20261007-ee54f0d1/`; measured windows and
operator reports must retain their actual conditions. Campaign B remains NOT
RUN and no public release/channel has changed.

After the operator corrected zoom, API checks confirmed both MT flags on, both
decks stopped and zoom 8 before the final measurement. All three windows
retained MT and zoom 8. The runner stopped both decks afterwards.

| Measurement | Stopped | Solo D1 | Dual |
|---|---:|---:|---:|
| Refresh interrupts | 602 | 609 | 607 |
| Coalesced refreshes | 0 | 0 | 31 |
| Wake latency mean, us | 1,164.9 | 179.3 | 1,255.5 |
| Overview entry after refresh mean, us | 1,255.1 | 291.7 | 1,365.0 |
| Callback duration mean, us | 238.3 | 5,133.8 | 13,774.1 |
| D1 finish after refresh mean, us | No redraw | 4,425.0 | 5,790.5 |
| D2 finish after refresh mean, us | No redraw | No redraw | 10,836.3 |

There were 578 D2 redraw observations; 38 D2 finishes exceeded 20 ms. Count
boundaries can differ by one because counters are captured while rendering.
PCM underrun, output-late, UAC drop/overflow/underflow/packet-loss and service-log
drop/write deltas were zero across all three short windows. UAC's absolute
underflow count was already nonzero before the windows; this is an active-window
delta statement, not an absolute zero-loss claim.

The operator confirmed: "Solo uredan; dual deformiran/trza; zvuk cist".
Physical visual result: **FAIL**. Audio was reported clean for this short test,
without qualifying a long audio soak. The smaller coalesced count than e4536de2
does not establish repaired fluidity, nor prove that every prior candidate was
quantitatively worse. Keep the operator's reported worsening and the measured
timing differences as separate evidence.

## Retained-position regression mechanism to isolate

A host probe executing the unchanged production `ui_position_interpolator.c`
with a deliberately retained 30,000-ms position, +5% speed and 20-ms frame
cadence reproduced four backwards resets in 500 ms. At 100 ms the display
prediction reached 30,105 ms; at 120 ms it rebased to the unchanged 30,000-ms
sample. The new zero-wait frame path retains the last sample on decoder-mutex
contention but does not tell the interpolator whether that sample is fresh.

This is a reproducible stale-input mechanism that can explain added positional
jerks if observations remain stale long enough. It is not proof that this
condition occurred during the device test: the installed image does not count
observation age or timeout streaks. It also does not explain away the separately
measured scanout deadline misses. Before another repair, isolate position
freshness from scanout using the accepted M3 path as an A/B reference; avoid
layering another speculative optimization onto the failed candidates.

## Completed-frame presentation repair

The direct scanout path modifies the same framebuffer that DPI DMA reads.
Missing its scanout window exposes pixels from different waveform positions in
one panel frame. The previous timing-only repairs reduced some measured delays
but still failed the operator's visual gate.

The new M3 board policy uses two RGB888 framebuffers. Both waveforms retain the
existing top-to-bottom order and layout, but all PPA and LVGL writes target the
inactive buffer. Only after both waveform phases and LVGL chrome finish does
the task select that completed buffer for the next DMA transfer. A late render
keeps the previous complete picture on the panel rather than modifying its
active scanout. No new panel timing, DSI lane setting or audio DSP change is made.

IDF 6.0.2 `mipi_dsi_dma_trans_done_cb` on the configured pre-v3 P4 path selects
`cur_fb_index`, restarts DMA, then invokes `on_refresh_done`. The task seals a
submitted buffer and waits for a refresh generation observed after submission
returns. Only then can the previous front be reused. A callback during submission
cannot release a buffer early. A v3-only build is rejected because its bridge
VSYNC notification does not provide this same retirement contract.

Before reuse, the retired buffer receives only the previous frame's dirty row
spans. This preserves stopped waveforms, unchanged widgets, screen transitions,
and partial LVGL redraws without a mandatory full-screen copy. CPU-copied rows
are written back before PPA invalidates overlapping destination cache lines.
The extra M3 framebuffer occupies 1,152,000 bytes of PSRAM. Other board policies
retain one framebuffer; their ESP builds/CI remain deferred during this bench
iteration. Status reports the board's `display_framebuffers` capability, and
RAM timing adds `framebuffer_sync` and `present` costs for the physical retest.

The production framebuffer-state helper passed 2,000 frames against a complete
pixel reference in RGB565/RGB888, including disjoint/overlapping partial writes,
sealed submission, late notifications, wrapped generations, stale notifications,
and unchanged frames. Existing M3 product simulator baselines pass unchanged.
Full host qualification and the exact clean-source M3 build/sign/install and
operator retest are required before calling this a repaired physical image.
The retained-position mechanism is deliberately not changed in this candidate;
its occurrence on the device remains an independent hypothesis.

## Completed-frame candidate 6d55ce2a: failed bench test

Clean source `6d55ce2a05a187379d8927ab5f053777ce054f59`, version
`M3-dev-g6d55ce2a05a1`, was installed locally with signed OTA and observed
VALID in `ota_1`. Status confirmed `display_framebuffers=2`. Full host
qualification, unchanged M3 simulator baselines, the M3 ESP-IDF 6.0.2 build,
signature/project identity and image budget passed. Other board builds and
full CI remain deferred under the M3-first iteration policy.

The image is 2,571,184 bytes, SHA-256
`58c0b9443674ae9f8635e68f09e104a562d8c5dd99e1ad103f095894c5d5d71a`;
the signed bundle is 2,571,372 bytes, SHA-256
`792e0634542f7f4f71a17034e3340876bb7fbc556cbded9eff8109f84ef1d517`.
The immutable software evidence is in
`releases/m3-bench-6d55ce2a/pajoniiir-m3-M3-dev-g6d55ce2a05a1/`.

On 2026-10-07, API checks confirmed both MT flags on, zoom 8, both decks
stopped, and the expected source before measurement. The stopped and solo D1
windows retained MT and zoom 8. Their twelve-second observations were:

| Measurement | Stopped | Solo D1 |
|---|---:|---:|
| Refresh interrupts | 602 | 602 |
| Coalesced refreshes | 0 | 13 |
| Framebuffer synchronization mean, us | No copy | 6,038.9 |
| Callback duration mean, us | 395.3 | 6,669.0 |
| Handler duration mean, us | 395.5 | 14,142.7 |
| D1 finish after refresh mean, us | No redraw | 11,423.4 |

PCM underrun, output-late, UAC drop/overflow/underflow and service-log drop
deltas were zero in those two captured windows. One service-log record was
written during solo; this is not a no-write window. These observations do not
qualify dual playback or listening.

No dual before/after records or final-stopped record were produced. Subsequent
HTTP requests timed out or were reset while Wi-Fi remained connected and ping
responded. Neither a completed dual test nor automatic final stop is confirmed.
The operator reported `sve je lose` (everything is bad). Physical acceptance
is **FAIL**, without assigning a more specific visual/audio symptom to this
report. A reset and USB1 serial diagnosis were requested. No new long soak,
cross-board build, push, or public OTA publication follows this failure.

Raw measurements are preserved in
`.cache/m3-migration/20261007-6d55ce2a/mt-8-beat/`. The measured synchronization
cost is an additional performance problem to address; it does not by itself
establish the cause of the HTTP failure.

After the operator reset, API access returned and the same image was observed
VALID with both decks stopped. The retained-RAM previous journals were invalid
and the SD journal recorded a POWERON reset; this is not a recorded WDT reset.
The persistent SD `system.log` identifies the failed image as boot 327 and
contains new AUDIO_UNDERRUN, UAC_DATA_LOSS, AUDIO_OUTPUT_LATE and
AUDIO_BLOCK_OUTLIER records after the solo window ended at uptime 723,329 ms.
At 730,994 ms output-late maximum was 144,367 us. Several mix groups took about
100 ms; an I2S stall record also names the interrupted `ae_decode` task. These
are confirmed audio/service failures, not isolated harmless output-late events,
but do not establish whether framebuffer traffic, decoder work or another
blocking path initiated the failure. The raw log and post-reset snapshots are
in the same image's evidence directory. No UART port was present on the host.

For recovery, the previous signed ee54f0d1 bundle was verified and uploaded
locally (HTTP 200), then observed VALID in ota_0 with the exact expected clean
source/ELF at `2026-10-07T08:08:36.618435+00:00`. This restores the previous bench
baseline; it does not change that image's visual FAIL.

## Deferred preservation revision

The next revision preserves inactive-buffer ownership but moves old-pixel
preservation after all waveform and LVGL writes. A 48,000-byte PSRAM bitmap
marks exact new pixel coverage on 800x480 M3. For each prior dirty row span,
only pixels not written in the current frame are copied from the front. Fully
redrawn strips therefore copy no old pixels. This also preserves a paused
waveform and gaps between disjoint LVGL rectangles; a conservative bounding
span alone would incorrectly discard pixels in those gaps. Copied CPU rows
are written back after PPA rendering and before submitting the complete frame.

Retirement and the sealed-buffer contract stay unchanged. There are no panel,
audio, interpolation or waveform geometry changes. The production helper
passes 2,000 randomized reference frames plus full-overwrite zero-copy and
explicit disjoint-gap cases in RGB565/RGB888; M3 simulator screenshot hashes
remain unchanged. Full host qualification passed with no optional qualification
SKIP, and documentation integrity passed. The exact clean-source M3 build,
packaging and physical retest are separate gates. The preceding failure has
not been resolved by these software tests.

## Deferred-preservation candidate b0c66744: failed visual retest

Clean source `b0c66744b252aa383009959986011393c9308247`, version
`M3-dev-gb0c66744b252`, was installed with signed local OTA (HTTP 200) and
observed VALID in ota_1 at `2026-10-07T08:21:26.422033+00:00`.
Image size is 2,572,096 bytes with SHA-256
`abb2231be28c0d6888e4b5bd35bcd15a576d18251e93174d0f12b83319e593b2`.
The 2,572,284-byte bundle SHA-256 is
`bfb5d9915e05a0b764ebffb909f8a7843f9ec5569992fd9e678e0c5cffc8e1e6`;
ELF SHA-256 is
`b72e44755ecbc16cc2974bb2e52a17e1eeadb6fa2899bddc9b921d3c4fa8fb5d`.
Immutable software evidence is under `releases/m3-bench-b0c66744/`.
Full host qualification, unchanged M3 simulator hashes, the exact M3 build,
signature/project identity and budget passed. Other boards/CI remain deferred.

MT was on both decks and zoom remained 8. The solo twelve-second window had
614 refreshes, six coalesced notifications and a mean framebuffer-sync cost of
798 us, down from 6,039 us on 6d55ce2a. Mean handler cost was 8,008 us. The
operator stopped playback during the dual interval because of severe stutter,
and confirmed it began only when both decks played. Physical visual result is
**FAIL**. Both decks were observed stopped at the midpoint and end; therefore
the dual summary mixes playing/stopped intervals and cannot qualify continuous
dual operation. Raw records remain in
`.cache/m3-migration/20261007-b0c66744/mt-8-beat/`.
Captured PCM/UAC loss, output-late and service-log-drop deltas were zero; this
does not establish listening acceptance. No new long soak followed the failure.
The measurement runner now rejects a changed playback state at midpoint/end
and records an interrupted window instead of reporting it as completed.

## Retained-position correction and observation-age measurement

The shared zero-wait UI observer retained its last coherent status when the
decoder metadata mutex was busy. It passed that old position into the production
interpolator without freshness information. A retained sample could exceed the
120-ms correction threshold and reset a smoothly advancing display backwards.
The production waveform cache then treats the backwards motion as a large
change and rebuilds its strip.

A regression now executes both production modules with a 500-ms retained
observation, +5% speed and 20-ms frame cadence. The previous contract produces
four backwards resets and five full cache redraws. With an explicit retained
flag it produces zero backwards resets and only the initial full redraw.
Separate cases cover -5%, pause/play handoff, fresh seek/CUE/loop wrap, and
scratch-authoritative zero-speed motion. A retained scratch observation holds
the last displayed coordinate until the next coherent observation.

The UI now carries freshness only for a matching loaded session. A timed-out
observation cannot authorize a rebase. Fresh large changes remain authoritative.
RAM counters expose successful/retained observations, retained streak maxima
and age histograms, including samples older than 120 ms. They perform no UART,
NVS or file writes. No decoder, mixer, panel timing or geometry change is made.

The host reproduction proves a defect in the retained-input path, not its
occurrence or primacy on the device. A successful mutex read can also observe
an output position whose deferred bookkeeping has not advanced yet; these
counters measure metadata-observation age, not audio-clock publication age.
Device measurements and operator confirmation must decide whether this repair
resolves the dual symptom. The new candidate has no physical PASS in advance.
