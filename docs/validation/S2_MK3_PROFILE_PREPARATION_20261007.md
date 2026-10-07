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
acceptance are retained, not reassigned to this experiment. Variant installation
and physical gates are pending.

## Bench isolation

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
