# Traktor Kontrol S2 MK3 profile preparation

## Performance candidate implementation (2026-10-08)

The [separate performance profile](../../controllers/ni_traktor_s2_mk3_full/README.md)
now contains 80 inputs and 26 outputs in a 1624-byte S3CP v4 binary. The basic
928-byte profile remains unchanged. Source-backed mixer, inverted pitch, jog,
shifted actions, Hot Cue LEDs and Note On meters use the common runtime.
Compiler/parser/storage validators support raw 10 with preserved v2/v3 bytes.

The shared USB audio runtime adds bounded descriptor-driven HS UAC2:
four-channel 48-kHz PCM, direct programmable clock RANGE/CUR/validity checks,
explicit feedback, fractional packetization, 24-in-32 packing and callback-owned
teardown. FS/FLX4 regressions remain mandatory. S2 headphone output, jog
sensitivity and newly mapped physical controls are NOT RUN on this candidate.

At the operator's request, physical checks are combined: one stopped-controls
batch, then one short dual-playback/transport/loop/MAIN/headphone batch. Only
failed or ambiguous checks are repeated. Final soak is last. Experimental
USB roles remain S2 USB3/HS, storage USB2/FS; production FLX4 defaults remain.
Prior basic-profile physical results below do not qualify this new image.

Software verification: the complete default P4 host suite exited 0, including
the new UAC2 regression and both S2 profile navigation fixtures. Focused profile
tests validate all 128 mixer/pitch values, replay exclusion for pitch, source
LED routing and malformed raw-10 rejection by both parser and upload validator.
Focused audio tests cover the captured configuration, clock RANGE/CUR/validity,
fractional feedback, PCM packing, stale feedback and ownership during teardown.
FLX4 golden parity remains unchanged. Profile regeneration is byte-identical;
documentation integrity and whitespace checks pass. M3 build/install and
combined physical acceptance are pending at this source checkpoint.

Candidate `40af1514925e60b5a17400e0e624c11737af32ec` built successfully
with IDF 6.0.2, unchanged lock and 2,581,648-byte image (SHA-256
`779d3f8ab3eebbad04b81087c04fa13cf1a46678b55784450b3fb15d358cadc9`).
Signed OTA returned HTTP 200 and boot 370 entered this image in ota_1, but
reset with PANIC before USB connection/startup readiness. Boot 371 rolled back
to the previous VALID af340dd8 image. The full profile was not uploaded.
This candidate is FAIL, not installed/accepted. Crash dump was disabled, so
the journal alone does not establish the panic cause.

A bench-only diagnostic option defers HS audio until ten seconds and OTA
validation, then quarantines it for a boot following PANIC. Flash crash dump
allows the Wi-Fi status API to expose the fault. Startup health remains intact;
FS/FLX4 behavior and production defaults are unchanged. This is an investigation
configuration, not a production workaround or headphone acceptance.

Diagnostic `949883fe325c6611c5f748ad2b92dc2641cecb9e` passed the M3
IDF 6.0.2 build and signed OTA, and is confirmed clean/VALID in ota_1.
Image: 2,599,088 bytes, SHA-256
`b3f7be2be8c94a529557ca3e469c158203c4d05b272c26f9d9215c053f22367c`;
bundle: 2,599,276 bytes, SHA-256
`d9458614ec5ba0dff442c8ade89ed0d38759415ddf1d7022e9038bf2ad076f85`;
ELF identity: `799e6641d087fd05bb2eb05188b5ae053790e2d6fc5e0a272c58fce85519fbb0`.
Local configuration enables flash crash dump (eight tasks, 2048-byte dump stack)
and the diagnostic HS deferral. Dependency lock is unchanged; no other board
was rebuilt. Boot 372 completed startup without a panic; no crash dump is present.

The 1624-byte performance profile was atomically uploaded with HTTP 200 over
SD id `ni_traktor_s2_mk3_basic` and became active. Repository basic bytes remain
unchanged. The deferred audio attempt succeeded: four channels, 48 kHz,
24-bit in four-byte subslots, HS explicit feedback. At the prepared stopped
snapshot, feedback_updates=148632 and feedback_invalid=0. This is negotiation
and traffic evidence only. An earlier idle/startup snapshot recorded two ISO
packet failures / 12 lost frames and pre-producer underflow; counters were not
reset. Take fresh baselines for each physical workload and do not claim zero loss
or clean headphones from these snapshots.

D1 Casanova Action and D2 Noćas su pederi u điru are READY/stopped, pitch centered,
loops/SYNC/MT off, stick mounted with 260 tracks, Wi-Fi available. Combined
physical control/listening acceptance is pending. The immediate-start panic
has not been reproduced or explained by this deferred configuration and remains
an open startup investigation. Final soak remains last, on an accepted image.
Evidence: `.cache/s2-mk3/20261008-m3-enumeration/performance-ota-install-949883fe325c.json`,
the matching installed status/profile snapshots and `combined-prepared-*.json`.

### Combined stopped-control window

On the exact diagnostic 949883fe image and performance profile, the first
combined window collected 279 status samples across 149.609 seconds. Both
decks remained stopped throughout; HTTP errors=0. MIDI packets increased by
6135 and semantic events by 5474. All observed new disconnect, MIDI pipe/output
submit error, queue failure, PCM underrun, output-late, UAC dropped block,
overflow/underflow and ISO packet/lost-frame deltas were zero. Feedback advanced
149606 times with no invalid feedback. Counters were not reset.

Both pitch controls traversed 0..16383 and finished at 8192; headphone mix also
traversed 0..16383 and finished at 8192. All declared channel mixer controls and
crossfader changed; the sampled window does not capture both endpoints of every
knob/fader, so it is not a full physical endpoint qualification. Both track
positions remained zero: touch-jog/shifted-search behavior requires the
operator's description before assigning a result. Operator confirmation and
the combined playback/listening block are pending. D2 volume, trim/EQ/filter
and crossfader were not all at neutral in the final sample; request restoration
before playback rather than silently overriding the physical controls.
Evidence: `combined-controls-20261008T200142Z.jsonl` and its `.summary.json`
under the same enumeration evidence directory.

The operator then reported loss of controller responsiveness while turning
the jogs. Subsequent live evidence on boot 372 shows HS/root 0 MIDI IN endpoint
0x85 (MPS 512) with three excessive-transaction events at uptime 469371 ms,
followed by a root disconnect at the same timestamp. The application reported
controller disconnect at 469377 ms. MIDI IN retry counts are 2 successful /
1 failed, fault operation IN_PIPE_CLEAR, code ESP_ERR_INVALID_STATE. No M3
reset occurred; Wi-Fi/API and FS storage remained available. Host recovery
requests did not advance, so this trace does not show an application-requested
power cycle preceding the disconnect. The cause of the bus loss is unproven;
do not label the complete physical controls batch PASS.

Evidence: `*-bus-review-20261008T*.txt` captured by
`.cache/s2-mk3/jog-unresponsive-review.txt`; trace events identify uptime 469371 ms.
A temporary 1312-byte variant retains all 80 inputs and audio capability but
has zero LED outputs. It was atomically uploaded with HTTP 200 over the same
SD id. This isolates LED output during a short reconnect/jog/Browse check,
without changing firmware or USB topology. The full repository profile remains
unchanged and will be restored after isolation. Playback/listening remains pending.

The no-LED reconnect/jog/Browse check also reset M3 (operator-confirmed).
Boot 373 reports PANIC with a valid 13,312-byte flash core dump:
`assert failed: usbh_dev_close usbh.c:1334
(dev_obj->dynamic.num_ctrl_xfers_inflight == 0)`, task `controller_usb`.
PC `0x4ff0cbbc`, RA `0x4ff0c754`, SP `0x4ff93a60`, mcause 2.
This rules out LED output as the sole cause. The no-LED capture spans a reboot;
negative counter differences are invalid, not zero-loss/PASS evidence.
The diagnostic next-boot audio quarantine preserves API/crash access. Neither
this crash nor its dump establishes the cause of the earlier bus disconnect.

The pinned USBH EP0 handler publishes completed transfers (waking the client)
before subtracting their in-flight ownership. A client can therefore retire
its callback-owned transfers and close the device before the daemon updates
the counter. The common fail-closed dependency patch retires each dequeued
transfer under the USBH lock before publication, preserving close assertions.
Freeing remains deferred to the USBH process after the handler; callback
resubmission owns a new count. The executable regression reproduces the
upstream assertion with a client scheduled during publication and checks
single, batched, resubmitted and empty completion handling in the patched code.
Physical reconnect/jog and headphone acceptance remain open until retested.

Fix candidate `27d4abedb7fef87de5c273dc3251ed80ebdbd974` passed the
complete default host suite (exit 0), including the executable USBH race,
real CMake source transformation/fail-closed check, controller/UAC2 and FLX4
parity suites. The first host attempt overlapped a manual run of the new test
and hit a Windows executable file lock; process-specific build directories
remove that test-harness collision, and the full rerun passed. Documentation
integrity passed. Only M3 was built with ESP-IDF 6.0.2; locks remain unchanged.

Clean signed `M3-dev-g27d4abedb7fe` is installed and confirmed VALID in
ota_0 after HTTP 200 and startup health validation. Image 2,599,296 bytes:
`d8647b2a5e62fd04cd610dcd9874b6598806a5a1dc7e23ee19120cedcd116ebb`;
bundle 2,599,484 bytes:
`db35d7d9e2f937576eeaad3dd2372e20581baff935bef3ddba0afe38ca99191d`;
ELF `fbe68aacb1d42a33b1291c02fdaeec8a92b506c6a2448a0f7890ec3c6781584d`.
The initial read after the operator's power/cable change timed out before any
upload; a subsequent read confirmed the old VALID image before installing.
The full 1624-byte profile was restored atomically with HTTP 200. Both decks
are stopped, storage mounted and Wi-Fi available; S2 is absent at the installed
snapshot and requires MIDI-mode reconnect for the short jog/Browse retest.
The diagnostic audio deferral/crash dump remain enabled; physical root stability,
control acceptance, headphone listening and final soak are still open.
Evidence: `performance-ota-install-27d4abedb7fe.json`, matching installed API
snapshots, `control-retirement-host-suite-rerun.log` and M3 build/package logs.

Status: PC basic-input/navigation captures completed on 2026-10-08; see the
[profile capture revision](../../controllers/ni_traktor_s2_mk3_basic/README.md#pc-capture-navigation-revision-2026-10-08).
The updated profile is 928 bytes, 50 inputs, eight outputs, with USB audio off.
It separates LOAD (`13`) from Browse push (`18`) and maps both Browse encoders
(`B1/B3 19`, `41` / `3F`). Additional `91/93 29` notes remain unmapped.
Earlier evidence below is historical and does not describe the updated binary.
The basic profile remains experimental; complete
S2 MK3 support and its physical acceptance are **NOT RUN**.

## M3 enumeration investigation (2026-10-08)

### Latest result: Browse traffic survives after adapter swap and power-on

On unchanged clean `af340dd8266ba65921cdafc3409205e5b1b4bf39`, VALID in
`ota_0`, the operator reported swapping the USB-C adapters and subsequently
reconnecting S2 in MIDI mode. The journal also records a new M3 `POWERON`
boot 369; this is not an adapter-only controlled comparison. The working-PC
comparison used USB-A directly, without a USB-C adapter.

At uptime 48915 ms, S2 was connected on HS/root 0 with the basic profile
active and zero MIDI packets. After the requested approximately 20-second
dual-Browse exercise, uptime 90882 ms showed 210 packets / 840 bytes,
203 total semantic events, one connection and zero disconnects. IN pipe
errors, retries, retry failures, IN/OUT submit failures, runtime queue
failures and controller recovery requests were all zero. Both roots retained
only their four enumeration trace events, with no new channel errors or
disconnect. The FS/root 1 stick remained mounted; boot library count was 260.

This short input-traffic check passed telemetry and exceeds the earlier
approximately three-second failing attachment. It does not establish the
root cause, complete controller acceptance, or prove visible Browse routing
and LED behavior. Physical navigation confirmation and reconnect regression
remain open. No firmware change or audio test was performed in this check.

Evidence: `*-bus-review-20261008T185358Z.txt` and
`*-bus-review-20261008T185440Z.txt` under
`.cache/s2-mk3/20261008-m3-enumeration/`.

The operator subsequently confirmed that both Browse encoders moved the
visible selection. One requested unplug/replug cycle also passed on the
same boot 369, without resetting M3: controller disconnect at 182213 ms,
reconnect at 194146 ms, and profile reactivation at 194178 ms. At uptime
219174 ms, counters showed two connects, one expected disconnect, 472 MIDI
packets / 1888 bytes and 441 semantic events. All MIDI pipe error/retry,
submit-failure and runtime queue-failure counters remained zero, with no
controller recovery request. Storage remained mounted with no disconnect.
The operator confirmed both Browse encoders worked after reconnect.
Evidence: `*-bus-review-20261008T185648Z.txt` in the same directory.
This qualifies one stopped-deck reconnect and basic Browse operation only;
LOAD/transport/LED checks, extended reconnect qualification and audio remain
open. Adapter causality is still unproven.

The subsequent operator-confirmed D1 load produced `TRACK_LOAD_DONE` for
track 35, "Casanova Action {320}" by Latin Lover at 331898 ms. At uptime
352490 ms D1 was READY at position zero, D2 still had No Track, and both
were stopped. MIDI packet count was 494, with no new disconnect, pipe error,
retry or submit failure. The preceding operator instruction incorrectly
called the Browse encoder press LOAD; the profile maps separate LOAD
buttons (`91/93 13`) and Browse push (`91/93 18`) to different actions.
The snapshot confirms the resulting deck state; the exact physical button
used in this step was not independently captured. Further LOAD checks must
explicitly use the dedicated LOAD buttons.

The dedicated D2 LOAD check was operator-confirmed. At uptime 420900 ms,
D2 contained track 26, "Kuzma & Shaka Zulu - Noćas su pederi u điru", while
D1 retained "Casanova Action {320}". Both decks remained READY, stopped at
position zero. The journal completed the load at 404051 ms. Counters reached
608 MIDI packets / 2432 bytes with no new disconnect or MIDI fault; storage
remained mounted. Evidence: `*-bus-review-20261008T190010Z.txt`.

The operator confirmed a short D1 PLAY/start-stop check: only D1 played,
its waveform and PCM5102A MAIN audio were clean, and the PLAY LED followed
the playback state. At uptime 498303 ms D1 was stopped at 8703 ms and D2
was still stopped at zero, with both loaded titles unchanged. The four new
MIDI packets matched the requested two button press/release cycles; all
pipe-error, retry, submit-failure and queue-failure counters remained zero.
S2 remained connected and storage mounted. Evidence:
`*-bus-review-20261008T190127Z.txt`. This is a short D1 transport/PLAY LED
and MAIN smoke result, not dual-deck, S2 headphone or soak qualification.

The same short PLAY/start-stop, PLAY LED, waveform and clean MAIN check
was operator-confirmed for D2. At uptime 551002 ms D2 was stopped at
7557 ms, D1 remained stopped at 8703 ms, and loaded titles were unchanged.
Four additional MIDI packets brought the total to 616 / 2464 bytes, with
473 semantic events. No new disconnect, pipe error, retry, submit failure
or queue failure occurred; storage remained mounted. Evidence:
`*-bus-review-20261008T190220Z.txt`. Both short solo transport checks passed;
this does not yet qualify simultaneous playback or S2 headphone audio.

The operator then confirmed CUE behavior on both decks: CUE during playback
stopped and returned to the initial cue; a held CUE preview played only while
held and returned to the cue on release. MAIN audio and CUE LEDs were reported
clean/correct. At uptime 658296 ms both decks were stopped at position zero,
with titles unchanged. There were 12 additional MIDI packets (628 total,
2512 bytes) and 485 semantic events, no new disconnect or MIDI/queue fault,
and storage remained mounted. Evidence: `*-bus-review-20261008T190407Z.txt`.
CUE-to-PLAY handoff and simultaneous playback were still open at this point.

The operator subsequently confirmed CUE-to-PLAY handoff on both decks:
PLAY while holding CUE committed playback, releasing CUE did not stop it,
and the final CUE returned to the cue with clean MAIN audio. At uptime
742984 ms both decks were stopped at zero with unchanged titles. Counters
showed 640 MIDI packets and 497 semantic events, with no new disconnect,
pipe error, retry, submit failure or queue failure. PCM underrun counters
for both decks, output-late count and service-log drops were all zero at this
snapshot. Both loaded sources were 44100 Hz, with 48000 Hz PCM5102A output.
Evidence: `*-bus-review-20261008T190532Z.txt`. Simultaneous playback remains
open; S2 USB headphone audio remains unavailable by profile configuration.

The operator confirmed the requested approximately 20-second simultaneous
PLAY D1/D2 check, followed by CUE on each deck: both waveforms were fluid,
MAIN audio clean and controller response normal. At uptime 835460 ms both
decks were stopped at zero with unchanged titles. MIDI totals were 648
packets and 505 semantic events; no extra disconnect, pipe error, retry,
submit failure or runtime queue failure occurred. PCM underrun counters,
output-late count and service-log drops remained zero, and the FS stick
remained mounted. Evidence: `*-bus-review-20261008T190705Z.txt`.
This is a short operator-confirmed dual-deck MAIN/UI smoke at 44100/44100 Hz,
zero pitch and MT off; it is not the planned worst-case mixed-rate/MT soak.

The stopped-deck SYNC-on check was confirmed by the operator for both the
screen indicators and physical LEDs. At uptime 984748 ms both decks had
`sync_enabled=true` and remained stopped; D2 reported +1.22 percent pitch
after alignment. MIDI totals were 654 packets and 511 semantic events,
with no new disconnect or pipe/submit/queue error, and storage mounted.
Evidence: `*-bus-review-20261008T190934Z.txt`. SYNC-off and acoustic/phase
behavior under playback remain separate checks.

The operator confirmed SYNC indicators/LEDs off and both PFL indicators/LEDs
on. At uptime 1069508 ms API states were `sync_enabled=false` on both decks
and `pfl1=true`, `pfl2=true`, with both decks stopped. MIDI totals were 662
packets / 2648 bytes and 519 semantic events; no new disconnect or MIDI/queue
fault occurred. Evidence: `*-bus-review-20261008T191059Z.txt`. This qualifies
PFL control/LED state only, not S2 headphone sound (USB audio is disabled).

To prepare loop encoder checks, the shared `loop_4` web action was issued
once per stopped deck. Both loops became active without starting playback:
D1 1829..3653 ms (1824 ms), D2 1031..2879 ms (1848 ms). Evidence:
`loop-prepare-20261008T191138Z.json` in the same evidence directory.

One clockwise LOOP encoder step per deck was operator-confirmed to display
eight beats. At uptime 1176506 ms, D1 loop was 1829..5477 ms (3648 ms) and
D2 1031..4727 ms (3696 ms), exactly twice their prepared lengths, with both
decks stopped. MIDI totals were 672 packets and 523 semantic events, with
no new disconnect, pipe/retry/submit/queue fault and storage mounted.
Evidence: `*-bus-review-20261008T191246Z.txt`. Reverse-size and loop encoder
press exit/reloop behavior remain open.

The operator confirmed one counterclockwise step returned each loop display
to four beats, then one encoder press per deck exited the loop. At uptime
1251837 ms both decks were stopped with `loop_active=false`; the inactive
status intentionally published zero loop bounds, so the intermediate four-
beat length is operator evidence, not a post-exit length measurement. MIDI
totals were 684 packets and 529 semantic events; no new disconnect or MIDI/
queue fault occurred and storage stayed mounted. Evidence:
`*-bus-review-20261008T191401Z.txt`. Remembered-loop re-entry remains open.

The operator confirmed a further encoder press re-entered each remembered
four-beat loop. At uptime 1297168 ms, the active bounds exactly matched
the prepared loops: D1 1829..3653 ms and D2 1031..2879 ms. Both decks
remained stopped. MIDI totals were 692 packets and 533 semantic events,
with no new disconnect or MIDI/queue fault; PCM underrun counters, output-
late count and service-log drops remained zero, and storage stayed mounted.
Evidence: `*-bus-review-20261008T191446Z.txt`. Stopped-deck loop doubling,
halving, exit and remembered re-entry are accepted on both decks. Resizing
and exiting loops during playback remain separate gates.

Remaining basic-profile acceptance groups:

1. Hot Cue pad recall and shifted clear, preserving existing local cue data.
2. Dedicated D1 LOAD recheck, LOAD LOCK, Browse push, screensaver wake and
   PFL-off, using the S2 semantic path rather than inherited FLX4 acceptance.
3. Loop resize/exit/re-entry and SYNC behavior during playback.
4. Further USB lifecycle qualification and final exact-image timing soak.

Mixer/tempo scaling, jog/touch behavior and S2 USB headphone audio are
additional implementation work outside this limited basic profile. Do not
interpret completion of these four acceptance groups as full S2 support.

### Latest control: HS stick and native S2 reconnect

On restored clean `7c86016265e6`, VALID in `ota_1`, a known stick
`18A5:0302` successfully enumerated on USB3/root 0 at high speed. Its
bulk endpoints have 512-byte maximum packets. Storage mounting was correctly
rejected there because this experimental image assigns storage to USB2/root 1.
This confirms HS enumeration for that stick; it does not qualify S2.

The operator then returned the stick to USB2 and connected S2 to USB3 without
holding FLX (native mode). A fresh live snapshot at uptime 366374 ms still
showed only two topology observations, the last probe belonging to the stick
on root 0, zero S2 MIDI connections, and storage unmounted. Neither the
reported stick return nor the S2 attachment produced a new observed device
event. The descriptor endpoint explicitly reports `last_probe_not_live_attachment`;
its stick descriptor must not be attributed to S2. Native-mode S2 enumeration
is therefore unconfirmed, not a MIDI-parser rejection. Host lifecycle state
and physical connection remain unresolved; cold power restart is the next
isolation step before another firmware change.

Evidence: `status-known-stick-usb3-control.json`,
`usb-controller-descriptors-known-stick-usb3-control.json`,
`status-s2-native-control.json`, and timestamped `*-native-fresh-*` snapshots
in `.cache/s2-mk3/20261008-m3-enumeration/`. Both decks remained stopped.

A subsequent operator-confirmed cold power restart produced journal boot 367
(`POWERON`) with the same VALID image. At uptime 36569 ms, the only newly
enumerated device was again stick `18A5:0302`, address 1, speed 2, direct
root 0. Storage remained unmounted (`ESP_ERR_NOT_SUPPORTED`); no S2 probe
occurred. This is fresh boot evidence, not the previous last-probe snapshot.
The reported physical placement and observed root disagree, so port placement
must be established by a single-device removal before interpreting S2 mode
or modifying firmware. Timestamped `*-native-cold-*` snapshots retain this
observation.

The subsequent code review corrected the removal-counter interpretation:
the MSC driver only publishes `MSC_DEVICE_DISCONNECTED` for an installed
device handle. A route-rejected stick has no such handle. Therefore unchanged
storage disconnect counters do **not** establish that the operator did not
remove it, nor do they establish a stalled USB daemon. The topology client
does receive `DEV_REMOVED`, but this installed status API does not expose
its removal counter. Last-probe information remains historical.

The controlled comparison then moved only the stick into the receptacle
previously occupied by S2, with S2 disconnected. It enumerated at full speed,
root 1, address 2, and mounted successfully. Keeping the stick there and
connecting S2 with left FLX into the other receptacle reproduced the original
transfer failure on boot 367: S2 `17CC:1710`, root 0, configuration 318,
profile active, 39 MIDI packets/156 bytes, three IN errors, two successful
pipe clears, then clear failure 259 and disconnect. Storage remained mounted;
host recovery counts stayed at the preceding one startup recovery. Evidence:
`*-stick-in-s2-port-*` and `s2-corrected-ports-20261008T115511Z.jsonl`.
This restores a reproducible transfer-level failure with verified root routing.

### Opt-in bus fault trace

`CONFIG_USB_HOST_DIAGNOSTIC_BUS_TRACE` (default off) records only port events
and channel errors into an eight-entry RAM ring per USB root. No logs, NVS,
allocation, reset, or recovery action occurs in the recorder. Normal channel
completions are not recorded. The port decoder delegates to the pinned HAL;
the existing channel decoder retains its established error behavior.

Read-only `GET /api/usb-bus-trace` returns bounded coherent snapshots with
boot-relative milliseconds, raw interrupt/HPRT/HCCHAR/HCTSIZ values and
decoded HAL event. Kind 1 is a channel error; kind 2 is a port event. Event
numbers follow ESP-IDF 6.0.2 `usb_dwc_hal.h`. HPRT bits 0/2/4/7/8/12 indicate
connection/enabled/overcurrent/suspend/reset/power. Snapshot availability is
explicit; unavailable is not evidence of zero faults. The ring is diagnostic
history, not a continuous USB traffic capture. Production defaults leave
recording off. Focused host tests exercise ring wrap, per-root isolation and
concurrent reader/writer coherence. Full host suite, M3 diagnostic build and
physical fault capture are tracked separately; this is not a fix/PASS claim.

Clean source `af340dd8266ba65921cdafc3409205e5b1b4bf39` passed the full default
host suite, documentation integrity and the M3-only IDF 6.0.2 experimental
build. Dependency locks remained unchanged. The local HS-role SDKCONFIG enables
the bus trace and retains both hosts; HS-only isolation is off. The signed
package passed board identity, budget and signature checks. Image size is
2,577,984 bytes, SHA-256
`6e30e9a82d31b451656111148b005ffa87b7ea1536e087c895541d67346031b9`;
bundle size is 2,578,172 bytes, SHA-256
`aed26a58e07b997a266bf5727b523bd9a5ad3669bddad1cefe932e6a90ec24f2`.

Local OTA returned HTTP 200. Boot 368 reports `M3-dev-gaf340dd8266b`, clean,
VALID in `ota_0`, matching ELF
`07bc1f9e3fc7567635dfb3cd43cd0e6c7d8714971d8bd142d2e784eab9fa19b6`.
Both decks are stopped, Wi-Fi remains enabled, and storage is mounted on FS
root 1 with 260 tracks. Initial trace correctly contains root-1 connection,
reset/enable transitions, with no root-0 event before the requested S2
reconnect. Exact installation evidence is `bus-trace-ota-install.json`;
subsequent raw snapshots use `*-bus-review-*` in the enumeration directory.
S2 fault capture on this diagnostic image remains pending the operator's
MIDI-mode reconnect. No transfer fix or new physical support PASS is claimed.

The operator subsequently completed the MIDI-mode reconnect/Browse test on
boot 368. The trace now identifies the hardware event preceding application
teardown. Root 0 connected at 6,405,805 ms, completed its enumeration resets
at 6,406,144 ms, and the controller/profile activated at 6,406,185/6,406,217 ms.
At 6,408,932 ms it recorded three channel errors on address 2, IN endpoint
`0x85`, MPS 512: HCINT `0x1002` (excessive transaction error + halted),
HPRT `0x1005` (connected/enabled/power), HCTSIZ `0x400000ff`. In the same
millisecond a port DISCONN event followed with GINTSTS `0x2500002d` and
HPRT `0x1008` (connection/enabled cleared, power still set; no suspend/reset
or overcurrent). Application disconnect followed at 6,408,938 ms.

59 MIDI packets / 236 bytes arrived before the failure. Two pipe retries
succeeded; the third clear returned 259 after the port lost connection.
Controller recovery requests stayed zero and total host recovery remained
the single earlier storage-startup recovery. FS storage stayed mounted with
no new root-1 events. Evidence:
`usb-bus-trace-bus-review-20261008T153635Z.txt` and sibling snapshots.
Thus application teardown follows a hardware-reported link disconnect;
increasing retry count does not address its cause. This is not proof of
physical unplugging, inadequate power, a cable defect, or a device/PHY defect.
The HPRT power bit is not a VBUS voltage measurement. The operator reports
M3 powered by a separate 67-W adapter; its actual 5-V supply path remains
unmeasured. A compatible independent S2 supply comparison was requested.

The operator has no S2 DC supply, so that comparison is NOT RUN. A normal-mode
S2 reconnect on the same confirmed HS receptacle produced no new hardware
connect event (`*-bus-review-20261008T154115Z.txt`); it is not a successful
native-mode control. To test whether the host was stranded after the preceding
disconnect, the operator removed S2 and moved the known stick from FS to that
same HS receptacle, without resetting M3. Root 0 immediately recorded connect
at 6,834,468 ms, completed resets at 6,834,810 ms and enumerated `18A5:0302`,
address 3, high speed. Evidence: `*-bus-review-20261008T154335Z.txt`.
This demonstrates post-fault HS host recovery for the known stick. It does
not distinguish the S2 cable/adapter, device behavior, supply path or HS
interoperability. The exact cable/adapter combination used for the successful
PC test is the next comparison. Current stick placement is HS, so storage is
intentionally unmounted under the experimental FS-storage role policy.

On installed clean `af4ca59007e3`, both decks stopped, the captured 928-byte
profile installed with HTTP 201 and was listed valid as `17CC:1710`.
It did not activate: the USB probe saw S2 but returned `ESP_ERR_NOT_FOUND`
at the MIDI-descriptor stage. Operator-confirmed left-FLX/green-flash reconnect
reproduced this on USB2 (direct root 1, configuration length 221).
A temporary USB3 check saw the same VID/PID on root 0 with configuration length
318 and the same rejection. These lengths differ; neither observation alone
establishes a full-speed restriction or a malformed MIDI descriptor.
The operator restored S2 to USB2 and storage to USB3. Raw before/upload/
reconnect/USB3 status evidence is retained in
`.cache/s2-mk3/20261008-m3-enumeration/`. No controller actions or audio were
qualified, and the temporary USB3 connection is not an accepted topology.

The investigation adds a read-only `/api/usb-controller-descriptors` last-probe
snapshot: raw device descriptor, bounded configuration bytes (max 1024),
captured/total lengths and explicit truncation, speed/address/root information.
The scope is explicitly the last probe, not current attachment or all device
configurations. Snapshot publication happens only on USB probes. Atomic byte
publication and a three-attempt reader prevent torn reads without unbounded
waiting; the HTTP response uses heap buffers, not a large HTTP stack frame.
It sends no USB requests and adds no periodic logging or audio/UI loop work.
Focused USB codec, snapshot bounds/overwrite and audio/gate tests pass.
The clean M3-only ESP-IDF 6.0.2 build, full default host suite, unchanged M3
dependency lock, signed packaging and documentation checks passed. Diagnostic
source `6438c351ea2de9072d74d809b0107071bb31c8a0`, version
`M3-dev-g6438c351ea2d`, installed with HTTP 200 and reached VALID on `ota_0`.
Image: 2,575,472 bytes, SHA-256
`6a6bad305f9464319804067ff442b250db4ae6b416888fbb24410097939b8235`.
Bundle: 2,575,660 bytes, SHA-256
`cb80347aa5bad4333cb6fc2bc85bd2c2c266be41b3ff513c721eedd8d35036d0`.
ELF: `9f4f1783ca9f3acf76e4d47bec9b35ae8a9a924f05d3cf1ae41c2f08e6239656`.
The first post-boot snapshot was the USB stick, not S2; its VID/PID and length
must not be attributed to the controller. S2 reconnect and full descriptor
interpretation remain pending. This diagnostic image has no new audio/UI soak
or production acceptance. Other board builds and hosted CI were not run.

### Descriptor result and selected experiment

Fresh diagnostic reconnect snapshots identify `17CC:1710`, `bcdDevice=0037`,
one configuration. FS has audio and HID, no MIDI interface. HS exposes Audio
class / MIDIStreaming subclass on interface 6, IN `85` (512-byte MPS), OUT `05`
(64-byte MPS), followed by eight zero bytes inside its declared 318-byte length.
The existing parser rejected those zeros after otherwise valid MIDI endpoints.
Actual 221/318-byte captures are preserved in the host fixture header;
acceptance of the HS bytes and rejection of nonzero padding/missing IN pass.

The operator explicitly chose S2 USB3 + storage USB2 for a separate experimental
M3 build. `BSP_M3_HS_CONTROLLER` defaults off; when enabled, only M3 board USB
roles swap. Existing routing and recovery continue using board capabilities.
The variant uses an isolated SDKCONFIG/build directory, keeps PCM5102A MAIN and
USB audio disabled in the S2 profile. Prior production default and its physical
acceptance are retained, not reassigned to this experiment.

### Experimental variant installation

Clean source `1fea1fd16d8ba35441671f5511cfaee09b779019`, version
`M3-dev-g1fea1fd16d8b`, passed the full default host suite (including the
swapped M3 board contract), isolated ESP-IDF 6.0.2 `build_s2_hs`, unchanged
dependency lock, signed packaging and documentation integrity checks.
The isolated configuration enables `BSP_M3_HS_CONTROLLER`; production defaults
remain unchanged. Local signed OTA returned HTTP 200 and startup health
confirmed VALID on `ota_1` with matching clean source and ELF identity.

- Image: 2,575,648 bytes, SHA-256
  `8ae01dade85e072e3e4656e885081477360a45355e589cbf5d3dbdb4efc0c0e9`.
- Bundle: 2,575,836 bytes, SHA-256
  `ecfbaa3a4d4a4c2c1a0b3d26d27946051e88716404b02dad3f4ea5bbecddc3bc`.
- ELF: `aedb1ef17c3ecef49d749bdcb5d51e0962c20c509e1b47e7f16ac36a12548980`.
- Runtime capabilities: storage root 1, controller root 0, PCM5102A MAIN,
  fixed 48-kHz output capability, Wi-Fi enabled. Storage mounted successfully
  on root 1; the current `/api/library` snapshot contains 260 tracks at
  generation 1. This is the observed count, not confirmation that this stick
  contains the previously audited 324-track export. Both decks are stopped;
  no playback output rate or storage-throughput acceptance is inferred.

Evidence: `.cache/s2-mk3/20261008-m3-enumeration/experimental-ota-install.json`
and `*-s2-hs-installed.json` / `*-s2-hs-ready.json`; package under
`releases/m3-s2-hs-1fea1fd1/`. S2 was not present in the first post-OTA
snapshot; an operator MIDI-mode reconnect was requested. Controller binding,
physical controls/LEDs, FS-storage playback performance and reconnect remain
**NOT RUN**. USB audio remains disabled. No soak or release qualification is
inferred from successful installation. Other board builds and hosted CI were
not run for this experiment.

## Bench isolation

### First experimental S2 reconnect outcome

On the same clean `1fea1fd16d8b` image, boot 359, the operator's reconnect
enumerated `17CC:1710` at high speed on direct root 0. The probe reached
`ready` / `ESP_OK`, with no interface-claim or transfer-allocation failure.
The journal records `CONTROLLER_CONNECTED` at 36,526 ms and activation of
`ni_traktor_s2_mk3_basic` at 36,558 ms. At 44,575 ms it records disconnection.
The snapshot has one MIDI connect, one disconnect, one fault-recovery epoch
and zero MIDI packets/bytes. Storage remains mounted on root 1 with 260 tracks;
both decks remain stopped. No allocation failures were reported.

Descriptor parsing, interface claim and initial profile activation are now
observed successes, but sustained controller connection **FAILS** in this
attempt. The transfer operation/status that triggered recovery is not exposed
by the current HTTP snapshot; the eight-second interval alone does not prove
an idle timeout. An immediate Browse-input reconnect comparison was requested.
Evidence is preserved in `*-s2-replug-*.json`,
`diagnostic-log-s2-fault.txt` and `resources-s2-fault.txt` in the enumeration
evidence directory. Physical controls, LED behavior and playback acceptance
remain open; this is not an S2 support PASS.

The second operator reconnect with immediate Browse rotation received 24 MIDI
packets (96 bytes), with runtime semantic-event count 25 and no queue failures.
Journal connection/profile activation occurred at 223,660/223,691 ms and
disconnection at 226,431 ms on boot 359. The fault epoch advanced to 2 while
storage remained mounted. Thus active MIDI input works briefly; disconnection
is not limited to the zero-input attempt. Read-only status now adds
`last_fault_operation` (0 none, 1 IN submit, 2 OUT submit, 3 IN transfer,
4 OUT transfer, 5 UAC stream, 6 IN pipe clear), `last_fault_code` and existing submit-failure
counters. Transfer codes are raw `usb_transfer_status_t`; submit/UAC codes are
`esp_err_t`. Publication happens only on first fault of a recovery epoch, with
no new periodic logging or writes.

Clean diagnostic source `c45cab13e0238d42811e9275cde95bbc4428cb4d`
passed full default host tests, M3-only IDF 6.0.2 experimental build,
unchanged dependency lock, documentation integrity and signed packaging.
Local OTA HTTP 200 installed `M3-dev-gc45cab13e023`, VALID on `ota_0`,
with matching ELF `884599bb4ca7c485d3ed3ab5a7f32ee0de876fe9281d060ca8705d4b3332659c`.
Image: 2,575,872 bytes, SHA-256
`fa4189218f62853dfa9fae5fd1a3c7e075810865cb4449e35f109efd492099fe`.
Bundle: 2,576,060 bytes, SHA-256
`94e2e66f303ecdaef435723e7a192c7a2fef57adb75892d98938550492cbc96c`.
The post-startup status exposes the new fault fields, initially zero; storage
is mounted and S2 requires another MIDI-mode reconnect. Fault diagnosis/retest
remains pending. Evidence: `fault-diagnostic-ota-install.json` and
`*-s2-fault-installed.json`. Package: `releases/m3-s2-fault-c45cab13/`.

The operator repeated the MIDI-mode reconnect/Browse input on this diagnostic
image. `status-c45-browse-reconnect.json` reports 19 packets / 76 bytes,
16 semantic events, one connect/disconnect and fault epoch 1. The captured
first fault is operation 3 (MIDI IN transfer), raw USB status 1
(`USB_TRANSFER_STATUS_ERROR`), with IN/OUT submit-failure counters both zero.
The pinned HCD maps this generic transfer error to an excessive transaction
error; it is not STALL, OVERFLOW or the separate timeout status. This does
not distinguish electrical/no-response/CRC causes or prove a driver defect.
Storage remains mounted. A USB1/CH340 serial connection was requested to
capture driver context before choosing a recovery or HAL change; the host
currently exposes no serial ports. No further firmware fix or acceptance
claim is made from this snapshot.

### Input-only isolation prepared

USB1/CH340 remained unavailable after the operator's cable/power reconnect;
boot 361 is a POWERON boot of the same VALID `c45cab13` image. Investigation
continues over Wi-Fi. Before changing HAL/recovery behavior, a temporary
input-only profile was compiled from the checked-in JSON: identical inputs,
zero outputs, `led_feedback=false`, `usb_audio=false`. The 832-byte binary
has SHA-256 `714b7cb70bf158b87a513a97273ab3092abd577aa47829a07c167face83d7e0f`.
Atomic replacement of `ni_traktor_s2_mk3_basic` returned HTTP 200 and the
profile list confirms valid size 832. Both decks were stopped and the
controller disconnected at replacement. No firmware change/build/OTA was
needed. Evidence is in `.cache/s2-mk3/20261008-input-only/`.

The operator was asked to repeat MIDI-mode reconnect and Browse rotation.
Outcome remains pending. LED acceptance is not possible with this temporary
profile; restore the checked-in 928-byte profile (SHA above) after the isolation.
This test distinguishes operation without MIDI output from the earlier
bidirectional setup; it does not itself establish an electrical or HAL cause.

The operator completed this test on boot 363: the 832-byte upload at 153,571 ms
preceded profile activation at 184,623 ms; disconnection followed at 187,303 ms.
The read-only observation captured a connected profile and additional MIDI
packets before the same IN ERROR/fault epoch increment. Thus the observed
failure also occurs with zero profile output bindings. Original 928-byte
profile restoration returned HTTP 200 (`restore.json`). No USB audio was used.

The next candidate bounds HS generic IN-error recovery to three host-pipe
clear/resubmit attempts per attachment, without resetting that budget after
successful packets. It preserves the endpoint PID, uses the existing retired
single IN URB, clears outside the callback and never consumes failed-URB bytes.
FS controllers and terminal/STALL/OVERFLOW statuses retain the existing close
path. Clear/submit failure or exhausted budget closes the connection as before.
New `midi_in_pipe_errors`, `midi_in_pipe_retries` and
`midi_in_pipe_retry_failures` counters retain visibility of all attempts;
successful resubmission does not establish lossless MIDI or fix a link cause.
Acceptance remains open pending build, installation and physical retest.

Clean retry source `7c86016265e60b04d45e211f21cb75c88d1ece85` passed focused
controller runtime/recovery tests (including the production retry budget),
full default host suite, M3-only IDF 6.0.2 experimental build, unchanged lock,
documentation and signed packaging checks. OTA HTTP 200 installed
`M3-dev-g7c86016265e6`, VALID on `ota_1`, matching ELF
`54f7568cbadbf7e9415e72255b4146b8abe2576d06158c99c648125a45a4d779`.
Image: 2,576,240 bytes, SHA-256
`b5be6955f51c325d8b9ad9484700bde78200f5255d289e3b53aacfe709820cec`.
Original 928-byte S2 profile is installed; USB audio remains disabled.

Operator-completed reconnect/Browse test **FAILS sustained connection**.
Boot 364 connection/activation at 22,315/22,347 ms, disconnect at 24,899 ms.
Status: three MIDI packets / 12 bytes, two IN pipe errors, one successful pipe
clear, one clear failure, one fault epoch; final operation 6 / code 259
(`ESP_ERR_INVALID_STATE`). IN/OUT submit failures and runtime queue failures
are zero; storage remains mounted. Retrying the endpoint is insufficient;
the budget was not exhausted and will not be increased to hide this result.
Pinned HCD clear requires a halted pipe and enabled port; the returned code
alone does not identify which precondition was lost. No physical disconnect,
power/cable cause or HAL defect is asserted. Another data cable comparison
was requested, with the same image/profile/topology, before further changes.
Evidence: `in-retry-ota-install.json`, `status-7c86-browse.json`,
`journal-7c86-browse.txt`; package `releases/m3-s2-in-retry-7c860162/`.
No full audio/control/LED/reconnect acceptance or soak PASS applies to this
experimental candidate. Other board builds and current hosted CI were not run.

The operator has no alternative USB cable; that comparison is **NOT RUN**.
With the stick removed and S2 reconnected, the same image still fails:
`status-s2-without-stick-final.json` shows mounted=false, 42 cumulative MIDI
packets, five cumulative IN errors, three successful clears and two failed
clears, final operation 6 / 259. Both decks are stopped. The observation
also shows nine host recovery requests/successes, while controller recovery
requests remain zero: the empty storage root keeps its recovery cadence.
Root routing review confirms storage root 1 and controller root 0; no incorrect
indexed route was found. The second FS host remains installed in this test.

An opt-in HS-only diagnostic installs peripheral map 1, disables the FS PHY
override and suppresses storage recovery requests. Controller root remains 0
and the original S2 profile remains unchanged; FS storage is intentionally
unavailable. `USB_HOST_DIAGNOSTIC_HS_ONLY` defaults off. The M3 CMake option
`PAJONIIIR_S2_HS_ONLY_DIAGNOSTIC` requires the HS-role experiment and a separate
SDKCONFIG. This isolates the second host's activity; it is not a product build
or playback qualification. Build/installation/retest remain pending.

HS-only clean source `378825576fcab75248096b8341d754fbd6d4390d` passes the
full default host suite, clean isolated `build_s2_hs_only` IDF 6.0.2 build,
unchanged dependency lock and signed packaging. The preceding host attempt
on `5f50ec77` stopped on a literal FS PHY-default lint; explicit default
initializers were restored and the complete suite rerun successfully.
Local OTA HTTP 200 installed `M3-dev-g378825576fca`, VALID on `ota_0`,
matching ELF `1e7cb2c72503a03cba24e193cd11334872126a0c00c806b56ff0212f2fcfc4f7`.
Image: 2,576,224 bytes, SHA-256
`985410515e89261f11035653db8ec89fa94bad07a861b99fdee28d5a4b026076`.
Bundle: 2,576,412 bytes, SHA-256
`e7cb49ee98c790d131df29a3be9d9e3d1bf4eabaefc68729787b3a26056b877d`.
Runtime confirms peripheral map 1, requested power mask 1, FS PHY override
false, host ready, zero recovery requests and storage unmounted. No devices
had enumerated in the first snapshot; descriptor API HTTP 503 is expected
without a last probe and does not invalidate the separately verified OTA.
The install wrapper exited on that optional follow-up request after saving
exact-image VALID evidence. Reconnect/physical outcome remains pending.
Evidence: `hs-only-ota-install.json`, `status-s2-hs-only-installed.json`;
package `releases/m3-s2-hs-only-37882557/`. The operator was also asked for
the M3 5-V source/current specification; no power diagnosis is inferred yet.

The operator subsequently reported connection and a repeat. On boot 365,
HS-only snapshots still show zero topology observations, zero probes and zero
daemon iterations; descriptor API remains 503. Thus no IN-transfer comparison
was obtained. Known-stick USB3 control enumeration was requested, but the
repeat message did not establish which device/port was used. Do not attribute
this absence to the S2 or claim a valid isolation result. The HS-only setup
remains unqualified and its enumeration issue is separate from the original
dual-host IN ERROR.

Without a new build, the previously verified signed `7c860162` dual-host package
was restored by local OTA HTTP 200 and reached exact clean VALID on `ota_1`.
Runtime confirms map 3; the first post-boot snapshot enumerates `18A5:0302`
on FS root 1 and mounts storage. It is the USB stick, not S2. Original valid
928-byte S2 profile remains installed. A fresh S2 USB3 MIDI-mode reconnect
with green-flash confirmation was requested. Evidence:
`*-hs-only-connected-*.txt`, `*-hs-only-repeat-*.txt`,
`restore-dual-host-ota-install.json`, `*-dual-host-restored.json`.
No new firmware fix or S2 support acceptance is claimed by restoration.

The M3 timing soak uses the exact clean `fb5cbca13f33d21dfb4e16e0ba667c340399eb0c`
image with FLX4 on USB2, storage on USB3, PCM5102A MAIN and FLX4 headphones.
During that soak, keep the S2 on a separate PC USB port. Do not change the M3
image, active profile, controller, USB wiring or playback workload.

On 2026-10-07 the operator reported connecting the borrowed S2 to the PC in
MIDI mode. The first present-device inventory found no `VID_17CC`/Traktor
device, and WinMM listed no MIDI inputs. This is a connection observation,
not evidence that the S2 MIDI protocol is unsupported. Enumeration and raw
input capture remain **NOT RUN** until the device appears.

## Protocol and provenance

Use the built-in MIDI mode: hold the **left FLX** button while connecting USB;
the pads briefly flash green. Disconnecting exits this mode. Native
Instruments specifies current controller firmware for this feature. Check
the borrowed device's version; do not update its firmware automatically.
See the [NI instructions](https://support.native-instruments.com/support/solutions/articles/69000879412-traktor-switching-your-kontrol-s2-mk3-to-midi-mode).

The [Mixxx S2 MK3 implementation](https://manual.mixxx.org/2.5/en/hardware/controllers/native_instruments_traktor_kontrol_s2_mk3.html)
uses HID. Its report offsets and native Traktor TSI control IDs are not MIDI
status/data1 addresses. No HID adapter is required for the initial MIDI path.

The checked-in [basic profile](../../controllers/ni_traktor_s2_mk3_basic/README.md)
comes from the pinned djay factory MIDI mapping (235 source records), with
VID/PID `17CC:1710`. It contains S3CP v2, 46 expanded inputs and eight LED
outputs. These remain source-derived until confirmed on this physical unit.
The historical October 1 profile upload in its README was on a donor bench;
it does not establish installation or acceptance on the current M3 image.

## PC capture workflow

`tools/controller_profile/capture_midi.py` uses Windows WinMM and Python's
standard library. It opens **only a MIDI input**, never sends output, captures
short messages only (no SysEx or HID), and refuses to overwrite evidence.
WinMM indexes and driver manufacturer/product fields are not USB VID/PID.
Capture device presence/USB IDs separately before and after reconnect.

From the canonical repository root:

```powershell
./tools/controller_profile/inventory_usb.ps1 -VendorId 17CC -OutFile .cache/s2-mk3/usb-before.json
python tools/controller_profile/capture_midi.py list
python tools/controller_profile/capture_midi.py record --name "Traktor" --label "D1 PLAY press-release x3" --seconds 15 --out .cache/s2-mk3/d1-play.jsonl
python tools/controller_profile/capture_midi.py analyze .cache/s2-mk3/d1-play.jsonl
```

Use the exact listed `--device-id` if the name is ambiguous; list again after
reconnecting because indexes can change. Record one named control per file,
both press and release, in a documented order. Start with D1 PLAY, CUE, SYNC,
PFL and browse press, then repeat on D2. The source predicts D1 PLAY status/data1
`91 01` and D2 `93 01`. The fixture treats nonzero velocity as press and zero
as release; capture establishes the actual velocities, including whether
release uses Note Off instead.

The JSONL stores metadata, driver timestamps, MIDI bytes and a final summary.
Missing summary, invalid messages, queue overflow, driver error or driver
backlog make capture incomplete. A file with no messages returns exit code 2.
The analyzer groups observed values; it does not infer physical semantics,
invert controls, generate a new profile or declare hardware acceptance.
For callback behavior see [Microsoft WinMM documentation](https://learn.microsoft.com/en-us/windows/win32/multimedia/mim-moredata).

## Implementation and acceptance matrix

| Area | Existing profile / required work | Acceptance evidence |
|---|---|---|
| USB identity and mode | Verify actual MIDI-mode VID/PID and MIDI interfaces; do not assume native-mode identity is unchanged | PC enumeration, cold reconnect, then correct M3 USB2 root after soak |
| Transport | PLAY/CUE/SYNC/PFL/LOAD/reloop already source-mapped | D1/D2 press/release, held CUE/PLAY handoff, LOAD LOCK, no stuck state after reconnect |
| Pads and SHIFT | Fixed eight Hot Cues and shifted clear; mode buttons omitted | All eight pads both decks, source/local cues, SHIFT release/reconnect; establish mode messages before implementing |
| Loop and browse | Loop size rel64 included; browse direction marked flipped in source | Slow/fast clockwise and counterclockwise capture, loop size/exit and Browse/Back/LOAD behavior |
| Mixer | Seven-bit CCs omitted | Capture min/center/max for channel gain/EQ/filter/faders/crossfader; `cc7_to14` v3 scale 0/64/127 correctly, reconcile held values after reconnect |
| Tempo | Seven-bit and flipped in source; input inversion absent in current ABI | Capture both endpoints/center/direction; design backward-compatible inversion through compiler/parser/storage/runtime before enabling |
| Jogs | Source encoding differs from FLX4; omitted | Capture slow/fast both directions, top/rim/touch/release and SHIFT; establish scaling and scratch/bend ownership before enabling |
| MT/FX/modes | Need semantic-action audit; do not guess omitted firmware actions | Known messages plus shared semantic-action implementation and focused production-module regressions |
| LEDs | PLAY/CUE/SYNC/PFL source values on=127/off=20; pads/RGB/meters omitted | Bounded output, physical LED behavior and reconnect state; native initialization/custom djay class remains unknown |
| Audio | `usb_audio=false` retained | Full USB descriptors: UAC version, alternate settings, channels/rates/sample width, endpoints and FS bandwidth; then physical cue test. Windows audio visibility is insufficient |

Input inversion and jog scaling are explicit shared-runtime design gaps, not
reasons to create an S2-specific copy of the controller runtime. Preserve
S3CP v2/v3/v4 compatibility and FLX4 behavior when extending the contract.
Do not silently change the basic fixture or enable audio from source metadata.

## Ordered next gates

1. Resolve PC enumeration; capture basic control edges and verify source
   channel numbering against the physical S2.
2. Capture the missing controls and full USB descriptors; create an address
   table with evidence files and distinguish source-only from observed values.
3. Specify shared inversion/jog contracts and test compiler, parser, upload
   validator, runtime and reconnect fencing together. Add mixer/tempo first,
   then jogs/modes/LEDs with independently reviewable profile revisions.
4. After the M3 soak has finished, store the experimental basic profile
   atomically and test S2 on USB2 with PCM5102A MAIN unchanged. First qualify
   MIDI/LED/reconnect; keep USB audio disabled until descriptor and listening
   gates pass. Do not inherit FLX4 audio or waveform acceptance automatically.
5. Record exact image/profile hashes, control coverage, omitted functions,
   MAIN/headphone listening and reconnect evidence before advertising support.

No firmware build, OTA, profile upload, controller output or M3 USB swap is
part of this preparation. Mandatory new-device physical gates remain open.

## Software preparation checks

- Four offline capture tests pass: short-message sizes, malformed data/SysEx
  rejection, unambiguous input selection and incomplete evidence handling.
- The checked-in S2 binary recompiles byte-for-byte: 864 bytes, SHA-256
  `7c83a50674eeba41e94af256e33560b52d669afd9240bf09eda6c17438f0b555`.
- The host suite's new S2 fixture executes the production parser/matcher:
  both deck PLAY edges, all shifted/unshifted pads, exact-channel rejection,
  LED values and audio-disabled capability. A physical Note Off release is
  deliberately not treated as an alias of the declared Note On input.
- Controller format/FLX4 parity and controller runtime/recovery suites pass.
  Documentation integrity and `git diff --check` pass.
- Live MIDI recording, full descriptors, controller output and S2 physical
  control/audio acceptance: **NOT RUN**. Firmware builds are unnecessary for
  these tools/tests/docs changes; the installed soak image remains unchanged.
