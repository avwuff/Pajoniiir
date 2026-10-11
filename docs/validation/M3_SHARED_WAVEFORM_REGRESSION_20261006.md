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

## Retained-position candidate 1d3ee7b6: installed, physical retest pending

Clean source `1d3ee7b60ac5a28e78695009fea567b923d263bc`, version
`M3-dev-g1d3ee7b60ac5`, passed full host qualification, focused freshness/cache/
timing regressions, unchanged M3 product simulator hashes and the M3-only
ESP-IDF 6.0.2 build. The committed dependency lock did not change. Signature,
project/source identity and the 0x380000 image budget passed. Other board
builds, full CI, push and publication remain deferred.

Image size is 2,572,752 bytes, SHA-256
`5a8acf585c33d507592006d30cdacca84f2d1d803155d52f4a4a7f22728b6323`.
Bundle size is 2,572,940 bytes, SHA-256
`488fbf14cf52d994388d7b636013912a7a37e4373f659b8923c93e9122c8dcef`.
ELF SHA-256 is
`5281dbc698ac2a2125a38cef8e01b4ba7ff4eff55835900ed498d5982fa57154`.
Immutable software evidence is under `releases/m3-bench-1d3ee7b6/`.

Signed OTA upload returned HTTP 200. The host disconnected from Wi-Fi during
restart; reconnecting the existing Pajoniiir-M3 profile restored access. Exact
source/ELF, clean image, ota_0 VALID and OTA idle were observed at
`2026-10-07T08:49:29.361822+00:00`. This observation time is not a measured startup
duration. Install snapshots are under `.cache/m3-migration/20261007-1d3ee7b6/`.

The same key 5 44.1-kHz and key 139 48-kHz tracks, opposite 5% pitch and loops
are prepared; both decks remain stopped. Preparation observed MT off and zoom
12. The operator must set MT on both and zoom 8 before a comparable bounded
solo/dual measurement. GUI/listening acceptance, dual visual result and the
new timing soak remain **NOT RUN** at preparation. VALID is not physical acceptance.

## 1d3ee7b6 measured/operator retest: improved, residual stutter

The operator confirmed readiness and the API independently verified the exact
clean source/ELF, VALID ota_0, both MT flags on, zoom 8, opposite pitch,
the expected tracks/loops and both decks stopped. All three bounded windows
completed with unchanged playback state; both decks were stopped at the end.

| Measurement | Stopped | Solo D1 | Dual with loops | Dual without loops |
|---|---:|---:|---:|---:|
| Refresh interrupts | 607 | 610 | 621 | 622 |
| Coalesced notifications | 0 | 5 | 149 | 118 |
| Completed callbacks | 606 | 605 | 472 | 504 |
| D1/D2 retained samples | 1/1 | 45/47 | 222/223 | 183/184 |
| D1/D2 samples older than 120 ms | 0/0 | 0/0 | 38/39 | 0/0 |
| D1 cache mean, us | No redraw | 190.9 | 236.8 | 59.8 |
| D2 cache mean, us | No redraw | No redraw | 288.8 | 66.9 |
| D2 finish after refresh mean, us | No redraw | No redraw | 10,460 | 10,020 |
| Callback mean, us | Idle | 5,745.6 | 14,828.4 | 13,478.0 |
| Whole handler mean, us | Idle | 8,008.7 | 18,769.1 | 17,123.3 |

The operator reported much better visuals, but occasional slight stutter.
Without loops the slight stutter remained; MAIN and FLX4 headphones were
reported clean. This is improvement, **not a full fluidity PASS**. Captured
PCM/UAC loss deltas, output-late and service-log-drop deltas were zero. The
cumulative UAC idle underflow counter is nonzero and is not a zero-since-boot
claim. Listening confirmation covers these short windows only.

The no-loop comparison disabled the two active loops, measured continuous
dual playback, stopped both decks and restored each original loop start/end.
The final snapshot verifies restored loops and stopped playback. Raw records
are in the image's `mt-8-beat/` and `no-loop-comparison/` directories. No new
firmware was installed between the two measurements.

Metadata contention older than 120 ms is now observed during dual loop
playback, confirming the retained-input condition occurs on the device. It
does not establish every visual symptom as that defect. Without loops, all
lower-waveform finishes were below 12 ms, both caches were below 1 ms, yet
118 whole handlers exceeded 20 ms and 118 refreshes coalesced. Residual work
after waveform rendering, plus ordinary rendering spikes, still matters.

## Residual UI work revision

The next revision skips per-refresh PSRAM comparison of unchanged deck covers.
An LVGL-owned artwork content revision advances when a staged decode is
published, a remote thumbnail replaces a slot, or a cached entry is forgotten.
The Overview still copies pixels immediately on a changed source/revision and
retains fallback behavior. A production cache regression forces the same
pixel address to be reused for another full identity and verifies the revision
changes; reads leave the revision unchanged. Pixel pointers gain no extended
lifetime or cross-task ownership.

Library's active-deck indicator is styled only while Library is visible; its
state is rebuilt when that tab is updated. Async artwork and load/media events
continue to run while the tab is hidden. Additional fixed RAM timings separate
Overview chrome, UI work after Overview, and LVGL timers/rendering. These
measurements add no log formatting, UART or file/NVS writes. Waveform cadence,
transport/audio DSP, layout, panel timing and framebuffer ownership are unchanged.
Exact-image build/sign/install and operator verification remain required.

## UI work candidate bf2fdc47: installed, physical retest pending

Clean source `bf2fdc474ae7a1636a22245656681780f6bebe74`, version
`M3-dev-gbf2fdc474ae7`, passed full host qualification with mandatory fixtures,
unchanged M3 simulator hashes, documentation integrity and the M3-only
ESP-IDF 6.0.2 build. Dependency locks are unchanged. Signed manifest/bundle,
project/source identity and image budget checks passed. Other boards/full CI
and publication remain deferred.

Image size is 2,573,040 bytes, SHA-256
`3e63b888fa84caf3dd4dd1bbe55533b3fe6279d7cb9b71ba34970146b507a9bc`.
Bundle size is 2,573,228 bytes, SHA-256
`2f04f5a41d1cf6f963c20167ac0e561ec0e625aec15ca78c6c74aad07ec3a2bf`.
ELF SHA-256 is
`9151b6e923497817f25309863c1d26a3eb42fcfa9514dd00850f48f608fcfc15`.
Immutable M3 software evidence is under `releases/m3-bench-bf2fdc47/`.

Local signed upload returned HTTP 200. The exact clean source/ELF, ota_1 VALID
and OTA idle were observed at `2026-10-07T12:12:45.959143+00:00` within the
helper's polling deadline; this time is not a measured startup duration. Wi-Fi
remains on. The same tracks, opposite pitch and loops were prepared with both
decks stopped. Preparation read MT off and zoom 12; operator configuration is
required for the comparable MT/zoom-8 retest. New metrics `chrome`, `ui_tail`
and `lvgl_timers` are available. No listening/visual acceptance or new long
soak has been performed on this image. Install and preparation snapshots are
under `.cache/m3-migration/20261007-bf2fdc47/`.

### Comparable MT/zoom-8 measurement

After operator readiness, three continuous 12-second windows completed on the
exact bf2fdc47 image: stopped, solo D1 and dual playback. Both decks were stopped
afterwards. MT, zoom, transport and source remained consistent. Strict new
PCM/UAC loss checks passed; visual/listening confirmation is still pending.

Solo delivered 603 callbacks from 608 refreshes; dual delivered 484 from 631
(147 coalesced). Dual mean waveform D2 finish was 10,506.4 us, UI tail 3,032.2 us,
chrome 1,299.5 us and LVGL timers/render 2,298.0 us. Whole handler mean was
18,394.8 us with 134 samples over 20 ms. The cover/hidden-Library optimization
has not established the missing full fluidity. Raw snapshots are in
`20261007-bf2fdc47/mt-8-beat/`.

### Jog-mode polling correction

Inspection of the measured UI tail found a blocking query that remained after
the zero-wait frame observation work: `ui_performance_deck_state()` called
`deck_core_get_deck_state()`, which obtains position under the audio decoder
mutex. The jog-mode label invokes this callback on every UI update, including
when Hot Cues is hidden. Its only consumers read `jog_cdj_mode`; position has a
separate callback. The revision uses `deck_core_get_deck_control_state()` for
this callback, preserving jog mode/transport control state without obtaining an
unused audio position. No DSP, panel, scanout policy or label behavior changes.
Software checks and exact-image physical measurement are required before
attributing the residual stutter to this wait.

### Repeated bf2fdc47 physical test

The operator missed the first measurement and requested a repeat. The same
MT-on-both, zoom-8, opposite-pitch, looped tracks were verified before the three
12-second windows. Stage announcements identified solo D1 and dual playback.
The operator reported slight stutter and confirmed clean MAIN/FLX4 headphones
sound. Full fluidity remains **FAIL**; listening confirmation covers this short
repeat only. Strict new PCM/UAC losses stayed zero and both decks ended stopped.

Dual delivered 481 callbacks from 630 refreshes (150 coalesced); mean D2 finish
was 10,561.2 us, UI tail 3,498.8 us and whole handler 18,581.6 us (131 over 20 ms).
Raw records are in `20261007-bf2fdc47/mt-8-beat-repeat/`; separate physical status
is `releases/m3-bench-bf2fdc47-physical-status.json`. Immutable bench evidence
is unchanged. Campaign B was not started.

## Jog-mode candidate fb5cbca1: installed, short visual gates and Campaign B passed

Clean source `fb5cbca13f33d21dfb4e16e0ba667c340399eb0c`, version
`M3-dev-gfb5cbca13f33`, passed host qualification with mandatory fixtures,
unchanged M3 simulator screenshots, documentation integrity, M3-only IDF 6.0.2
build, unchanged dependency locks, and signed package/source/identity/budget
checks. Immutable software evidence is under `releases/m3-bench-fb5cbca1/`.
JC4880/JC1060 builds, full CI and publication remain deferred.

Image: 2,573,040 bytes, SHA-256
`17a5c82ee5ac1944bf756918d1f4094a5f67bf9ab66f60dcb21ce4aa42f1015b`.
Bundle: 2,573,228 bytes, SHA-256
`1f6cff625d0db95e5be4617bfb1100aced4ee2dd2908ec06efa3618a94711cb3`.
ELF SHA-256:
`e6382dff0c82d9db463e03b956f435ad2d6bad2f4fdee9f9d476cd152d1505a7`.

Local signed upload returned HTTP 200; exact source/ELF, ota_0 VALID and OTA
idle were observed at `2026-10-07T12:28:58.178806+00:00`. This is observation
time, not measured startup duration. Same tracks/pitch/loops were prepared
stopped. Preparation read MT off and zoom 12; operator readiness is required
before the comparable retest. Wi-Fi remains on. Install/preparation records
are in `.cache/m3-migration/20261007-fb5cbca1/`. Listening/visual acceptance and
the long soak on this exact image remain NOT RUN.

### fb5cbca1 MT/zoom-8 measurement

Operator readiness was followed by the same three continuous 12-second windows
with stage announcements. Exact source/ELF/VALID state was checked before the
test. Both decks ended stopped with MT still on. Strict new PCM/UAC loss checks
passed. Physical visual/listening confirmation is pending; no long soak ran.

Solo delivered 604 callback samples from 610 refreshes, with six coalesced.
Dual delivered 607 from 641, with 35 coalesced. Small count differences reflect
the non-transactional timing counters sampled across callback boundaries.
Dual UI tail mean fell from 3,498.8 us in the bf2fdc47 repeat to 130.3 us; all
607 new tail samples were below 1 ms. Dual whole handler mean was 14,952.9 us
with 28 over 20 ms, compared with 18,581.6 us and 131 over 20 ms previously.
D2 waveform finish mean was 10,392.3 us, with 11 samples over 20 ms. The fixed
mutex wait is consistent with the reduced UI-tail time, but remaining deadline
misses preclude a zero-stutter claim from telemetry alone.

Retained position observations remain present: D1/D2 fresh 238/235, retained
369/372, and older-than-120-ms 67/69; maximum streak since boot 22/22. Removing
the tail's blocking query makes refreshes more frequent while metadata can still
be busy. These counters measure observation age, not clock-publication age.
Raw records are in `20261007-fb5cbca1/mt-8-beat/`.

The operator subsequently confirmed fluid dual waveforms and clean MAIN/FLX4
headphones sound on this exact fb5cbca1 MT/zoom-8 retest. This is a **PASS for
the short zoom-8 visual/listening gate**, not full hardware qualification or a
zero-deadline-miss claim. Other zooms, Campaign A/B, startup fault/rollback and
remaining integrated acceptance still require their own exact-image evidence.

### Short five-zoom operator review

The operator confirmed fluid dual waveforms on 4, 8, 12, 16 and 24 beats with
clean MAIN/FLX4 headphones sound. The bounded scan lasted about 121 seconds
including API/file overhead, captured all five zooms and ended with both decks
stopped. New PCM underrun, UAC drop/overflow/underflow/packet-loss and service-log
drop deltas were zero. This is **PASS for the short operator review**.

The helper exited 1 on its conservative minimum-four-samples-per-zoom coverage
check: counts were 11/3/2/1/13 for 4/8/12/16/24. Intermediate zooms were changed
quickly and visited more than once. This is incomplete bounded per-zoom dwell
coverage, not an audio/data-loss failure; do not describe it as a software
coverage PASS or a 15-second-per-zoom acceptance. The earlier dedicated 12-second
zoom-8 window remains separate valid evidence. Raw records and the explicit
coverage failure are under `20261007-fb5cbca1/all-zoom/`.

Four new output-late events were observed; since-boot maximum was 11,654 us,
below 15,000 us. Three events were observed in the last approximately 16 seconds,
crossing the existing clustering investigation trigger. The operator reported
no physical consequence and strict loss deltas remained zero. Retain this as
monitoring/investigation evidence, not a zero-late claim or automatic release
failure. No firmware change is justified solely by this cluster. Campaign B
remains required on the exact installed image.

### Cross-board software checks after M3 visual confirmation

After the operator's five-zoom confirmation, JC4880 and JC1060 local IDF 6.0.2
reconfigure/builds passed on clean `fb5cbca13f33d21dfb4e16e0ba667c340399eb0c`.
M3's already-verified build was reused; no redundant M3 rebuild or reinstall
was performed. All three dependency locks are unchanged. Board identity,
component isolation and budget checks passed. These are local software gates,
not clean CI qualification or other-board physical acceptance.

| Project | Image bytes | SHA-256 |
| --- | ---: | --- |
| main-deck-p4 | 2,588,640 | `73a0d2d7a37d84c40187200b2011a367896f952175ea6a61138d72bba261f3be` |
| main-deck-jc1060 | 2,646,960 | `2b1c73d0c98630c15abec0e1920e4159b325a0cea56f84e7c26b49852e34a017` |
| main-deck-m3 | 2,573,040 | `17a5c82ee5ac1944bf756918d1f4094a5f67bf9ab66f60dcb21ce4aa42f1015b` |

The first parallel JC4880 configure encountered the dependency manager's shared
Git cache `index.lock`; JC1060 completed, then sequential JC4880 retry passed.
No dependency version or board setting changed. The original failure log is
retained separately. Only this task's documentation changes were temporarily
stashed for exact clean-source build identity, then restored. Historical user
stashes were preserved. Evidence is `20261007-fb5cbca1/cross-board-builds.json`.

### Completed exact-image Campaign B timing soak

The continuous run completed on 2026-10-07 with **telemetry PASS and operator
visual/listening PASS**. Playback samples cover 14:41:26.561 through
15:41:26.529 Europe/Zagreb; the monitor's elapsed counter reached 3600.163 seconds.
Initial metadata precedes playback sampling (14:41:10.458), and final diagnostic
collection finished at 15:41:41.012. These timestamps are not startup durations.

The installed clean source/ELF above remained unchanged, ota_0 VALID, with
324 library tracks, both decks playing, MT and loops active, 44.1/48-kHz sources,
+5/-5 percent pitch, fixed 48-kHz MAIN output, FLX4 MIDI/UAC and Wi-Fi polling
at the requested 250-ms cadence. There were 9,759 status samples and zero
workload violations. Checkpoint and final UI snapshots confirmed zoom 8.

| Observation | Result |
| --- | --- |
| New PCM underruns, D1/D2 | 0 / 0 |
| New UAC dropped blocks / overflow / underflow | 0 / 0 / 0 |
| UAC underflow since-boot baseline and final | 996,009 / 996,009 (not a lifetime-zero claim) |
| New service-log drops | 0 |
| HTTP status / library / firmware errors | 0 / 0 / 0 |
| Allocation / critical allocation failures | 0 / 0 |
| Minimum sampled free internal / PSRAM bytes | 61,647 / 21,836,872 |
| Minimum reported task stack headroom | 2,200 bytes |
| New output-late events | 11 (count 4 to 15) |
| Maximum output-late since boot | 12,212 us |
| Monitor hard failures / investigation triggers | none / none during this run |

Maximum HTTP status/library/firmware latency was 1,599.424 / 2,162.856 /
926.145 ms. No query failed; this does not imply uniformly low web latency.
The preceding short all-zoom cluster remains separate investigation evidence;
this completed soak does not erase it or establish zero-late behavior.

After completion the operator explicitly confirmed that both waveforms stayed
fluid, the display stable, and MAIN and FLX4 headphones clean throughout the
test. Both decks were stopped by the wrapper and the final snapshot retained
MT/loops, the active FLX4 profile and exact source identity. A subsequent live
read verified both stopped and the same source/ELF/VALID slot. The recurring
15-minute checkpoint automation was paused after completion.

Raw monitor `result.json` remains unchanged with its operator field `NOT RUN`:
the later human acceptance is stored separately in
`timing-soak/operator-acceptance.json` and
`releases/m3-bench-fb5cbca1-physical-status.json`. Immutable image/bundle/software
evidence was not rewritten. Raw records are under
`.cache/m3-migration/20261007-fb5cbca1/timing-soak/`.

This closes the 60-minute Campaign B timing and operator audio/visual gate for
this exact M3 image. Campaign A, startup fault/timeout/rollback, current-source
CI, complete bounded per-zoom dwell coverage, other-board/controller hardware
gates and remaining integration acceptance are still open. No release tag,
remote push or public OTA channel was changed.

### Subsequent Campaign A scope waiver

After this soak the operator excluded all Campaign A work requiring human
intervention. All 30 physical cycles and operator LED/control/listening checks
were SKIPPED BY OPERATOR. Ten software restarts were run separately, with
verified eventual recovery and nine complete timing windows; host visibility
flags and cycle-6 timing remain investigation/incomplete evidence. See the
[automatic recovery supplement](M3_AUTOMATIC_RECOVERY_20261007.md). This changes
the Campaign A scope record and does not extend the soak's physical PASS to
cold boot, cable reconnect or other controllers.

### Subsequent autonomous OTA qualification and CI waiver

The operator then authorized option 2 (negative OTA/startup rollback) and skipped
option 3 (GitHub Actions) because of quota. Six negative upload cases and both
injected startup rollback scenarios passed on-device; see the
[OTA evidence record](M3_OTA_NEGATIVE_ROLLBACK_20261007.md). Timeout rejection was
recorded at 60.193 seconds. Both temporary diagnostics were rejected by ordinary
production packaging and returned automatically to the same clean fb5cbca1
ELF, ota_0 VALID. The timeout helper's parser error and raw FAIL remain preserved;
the separate read-only assessment establishes the actual device PASS without a
repeat. Both decks remain stopped, Wi-Fi/FLX4/catalog available. Current-source
hosted CI is SKIPPED BY OPERATOR, not PASS. This does not add a new image or extend
the earlier listening/visual acceptance to the diagnostic configurations.
