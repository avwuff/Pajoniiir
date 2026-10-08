# Traktor Kontrol S2 MK3 profile preparation

Status: PC basic-input/navigation captures completed on 2026-10-08; see the
[profile capture revision](../../controllers/ni_traktor_s2_mk3_basic/README.md#pc-capture-navigation-revision-2026-10-08).
The updated profile is 928 bytes, 50 inputs, eight outputs, with USB audio off.
It separates LOAD (`13`) from Browse push (`18`) and maps both Browse encoders
(`B1/B3 19`, `41` / `3F`). Additional `91/93 29` notes remain unmapped.
Earlier evidence below is historical and does not describe the updated binary.
The basic profile remains experimental; complete
S2 MK3 support and its physical acceptance are **NOT RUN**.

## M3 enumeration investigation (2026-10-08)

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
