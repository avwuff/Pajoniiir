# Hercules Inpulse 500 preparation, 2026-10-10

Status: matching firmware/profile installed; combined physical 60-second smoke
passed on `04f0cfb6`; full Hercules qualification remains open.
The latest installed candidate is `ac9287af`; its stopped normal restart is
recorded below, separately from the earlier physical playback evidence.
The initial Windows/offline preparation below did not alter the then-running
`3f3605a7` S2 soak, controller profile, firmware, USB connections or workload.
The later software qualification and installation are recorded separately below.

## Verified Windows identity

The connected device reports USB `06F8:B12B`, product `DJControl Inpulse 500`.
Present composite, MEDIA and speaker/line endpoints report OK.
MEDIA interface: `USB\VID_06F8&PID_B12B&MI_00\7&5A05388&0&0000`.
Driver service `usbaudio`, provider Microsoft, version `10.0.26100.9457`.
WinMM input index 0 is named `DJControl Inpulse 500`.
WinMM manufacturer/product numbers are driver identifiers, not USB VID/PID.
Windows enumeration does not establish M3 root, speed or USB-audio support.

## Offline software evidence

The committed JSON reproduces the committed 5700-byte S3CP v2 binary exactly:
SHA-256 `4eb737e7dd6c96432878e273b1d0466c19ff07b295fc372940c42e3c0db5aa70`.
Header declares 274 expanded input entries and 107 output entries.
The local staged copy and manifest are in
`.cache/hercules-inpulse-500/preparation-20261010/`; nothing is installed.

Checks on 2026-10-10 completed with exit 0:

- Controller converter: 11 regressions.
- Profile runtime/format, FLX4 parity, MIDI capture integrity and S2 fixture suite.
- Production profile-manager host tests, including Hercules exact VID/PID
  registry match, rejection of B105, atomic install and interrupted-swap recovery.

These checks exercise software; they do not prove all Hercules MIDI controls,
LED colours, audio or lifecycle behavior physically. No firmware build is
needed for this documentation/staging work, and no new firmware is installed.

## Source audit and activation blockers

Sources re-read on 2026-10-10:

- [Manufacturer MIDI commands](https://ts.hercules.com/download/sound/manuals/DJC_Inpulse500/DJControlInpulse500_MIDI_Commands.pdf).
- [Manufacturer manual](https://ts.hercules.com/download/sound/manuals/DJC_Inpulse500/DJControl_Inpulse_500_User_Manual_EN.pdf), audio specification on printed page 5 and native controls on printed page 4.
- [Mixxx mapping author notes](https://github.com/mixxxdj/mixxx/wiki/Hercules-DJControl-Inpulse-500), explaining omission of native hardware output controls to avoid applying them twice.

1. Audio: manufacturer specifies 24-bit 44100 Hz, MAIN 1/2 and headphones 3/4.
   Existing JSON says preferred 48000 Hz, but this entire descriptive `audio`
   object is not serialized by `compile_profile.py`. It neither selects a
   format nor overrides descriptor validation. Record actual M3 descriptors.
2. Filter: current omitted `channel_filter_requires_smart_cfx` defaults true.
   Use the shared v4 always-filter capability in the audited candidate rather
   than making Hercules depend on an FLX4 enable button it does not have.
3. Hardware output controls: decide the intentional PCM5102A master behavior
   and native Hercules headphone blend/level policy before mirroring events.
   Existing software mirrors can duplicate hardware attenuation/blending.
4. Jog touch (resolved by Windows capture): manufacturer table's touch row prints CC statuses while the
   existing profile uses Note On. CC `B1 08` is also the pitch MSB. Do not
   remap it speculatively. The focused Windows capture below supports the
   existing Note On mapping.

The staged baseline is a reference, not approved for unconditional activation.
No speculative SysEx, sample-rate override or USB role change is made.
The combined post-soak test order is in the controller README.

## Focused jog-touch capture

The user agreed to the announced PC capture, with D1 then D2 touch/release,
then the corresponding shifted gestures. At 00:55:54 local a 45-second
read-only WinMM recording captured 34 valid short messages with no queue
drops, driver pressure/errors or invalid messages. No MIDI output or SysEx
was sent. Raw evidence:
`.cache/hercules-inpulse-500/preparation-20261010/jog-touch.jsonl`.

| Address | Messages | Values | Existing mapping |
| --- | ---: | --- | --- |
| `91 08` | 6 | 0, 127 | D1 jog touch |
| `92 08` | 4 | 0, 127 | D2 jog touch |
| `94 08` | 4 | 0, 127 | Shift+D1 jog touch |
| `95 08` | 6 | 0, 127 | Shift+D2 jog touch |
| `91 04`, `92 04` | 2 each | 0, 127 | Both Shift buttons |

Incidental rotation messages (`B2 0A`, `B5 09`, `B5 0A`) were also present;
they do not establish calibrated jog direction/sensitivity. All four touch
addresses match the committed profile. The table's CC touch notation should
not replace these measured Note On addresses. This is PC address evidence,
not M3 scratch/playback or lifecycle acceptance.

## Native-headphone implementation candidate

The historical v2 baseline above is superseded by a 5592-byte S3CP v4 fixture
with 268 inputs and 106 outputs, SHA-256
`570b59ecca6baa7319ef85ca92d7d3a11fbe11ea1f9c9c580a8779031a40f549`. Shared capability bit 5 assigns final
headphone blend/level to hardware; bit 4 enables independent filters. Parser,
storage admission, compiler, web converter, profile runtime and production
audio mixer share the contract. No Hercules VID check is added to playback.
The compiler forbids software headphone mix/level/master-cue input mirrors.
MASTER deliberately still adjusts PCM5102A MAIN; RCA MAIN is not qualified.
USB 3/4 carry the selected mono pre-fader PFL bus at unity; the controller
performs the native blend with MAIN 1/2 and headphone attenuation. Physical
blend endpoints remain NOT RUN. The descriptive preferred rate is corrected
to 44100 Hz without overriding USB descriptors. Measured jog-touch notes remain.

Install matching firmware before uploading this fixture: older v4 firmware
including `0b08fb90` ignores the new bit. S2 work remains closed; the controller
port remains experimental USB3/root0, with storage USB2/root1.

Focused checks pass: production parser/storage/runtime fixture tests, native
control isolation and activation/clear/non-native replacement; production PCM
mixer verifies cue unity independent of software headphone mute/mode and that
MAIN is preserved; converter passes 12 regressions. Full host qualification,
M3 build/package/install and all Hercules M3 physical checks are pending.

## M3 software qualification, OTA and profile installation

Clean source `eafdb7a5d296d188abf8cb3c0bbc6ce705608ef4` passes the full
`run_p4_host_tests.ps1 -Qualification` suite, exit 0 with no SKIP. The single
M3 ESP-IDF 6.0.2 reconfigure/build passes, including linked restart capture and
five raw coredump pipeline fixtures. Board isolation/identity, unchanged lock,
`0x380000` image budget and documentation checks pass. JC4880/JC1060 were not
rebuilt for this Hercules candidate. Physical checks remain separate.

| Artifact | Verified value |
| --- | --- |
| Installed version | `M3-dev-geafdb7a5d296` |
| Project / board / slot | `main-deck-m3` / `m3` / VALID `ota_0` |
| Application | 2,601,952 bytes |
| BIN SHA-256 | `872a783f5a85313d890ec1d058a8fc8188fb31cb2cda23bbaccd92e344c98b18` |
| Matching ELF SHA-256 | `99c0ced2ce4473789f870b52aad75bd325feb8e93a63ca259a1c829eb51d3e39` |
| Signed bundle | 2,602,140 bytes |
| Bundle SHA-256 | `106d27a713d227167b72a3bfb56935c36258cdb52866b5279f90f3545f08af7c` |

Package and matching debug archive:
`.cache/hercules-native-candidate-eafdb7a5/pajoniiir-m3-M3-dev-geafdb7a5d296/`.
Its verification manifest hashes seven retained build/test/profile artifacts;
bundle and manifest signatures verify against the trusted release public key.
One marked local OTA returned HTTP 200; exact clean source/ELF reached VALID,
API idle and both decks stopped. Installation evidence:
`.cache/hercules-native-wireless/20261010T215446Z/`. The 7456-byte historical
dump is still byte-identical with SHA-256
`75afa985b81eea00e253a8e2410ec55364960bcc65c873a4db547b65b8e0c839`.
No new PANIC or intentional fault was observed; original evidence is retained.

After verifying matching firmware, a marked overwrite replaced only the old
Hercules profile. HTTP 200 and the fresh registry confirm valid `06F8:B12B`,
5592 bytes. FLX4 and S2 registry entries are unchanged. Evidence:
`.cache/hercules-inpulse-500/native-profile-install-20261010T215517Z/`.
The server validates write/readback CRC and metadata; the API has no binary
hash-readback endpoint, so the uploaded SHA is not claimed as an independent
client readback SHA. The controller was absent and audio was not streaming
during installation. GUI confirmation, actual descriptors, profile activation,
MIDI/LEDs, cue/master hardware blend and audio on M3 are pending.

The next physical action is ordinary Hercules connection on USB3, storage
remaining USB2, with stopped decks. No S2 MIDI-mode FLX gesture is needed.
Combine stopped controls and then short monitored audio checks when the
operator is ready; no S2 retest, push, release or public OTA is authorized.

## Initial physical activation and UAC1 feedback finding

The operator confirmed the `eafdb7a5` GUI returned normally and connected
Hercules to USB3 with storage on USB2. Current status reports `06F8:B12B`,
product `DJControl Inpulse 500`, MIDI IN/OUT and active
`hercules_djcontrol_inpulse_500`. Both decks remain IDLE. This proves binding
and profile activation, not an input sweep or audible headphone output.

The matching last-probe descriptor contains all 270 configuration bytes,
untruncated, full-speed (`speed:1`), direct root. AudioStreaming interface 1,
alt 1 is four-channel packed 24-bit PCM, discrete 44100 Hz only, asynchronous
OUT `0x01`, max packet 540, interval 1, linked to explicit feedback IN `0x81`.
Feedback max packet is 3, interval 1, refresh exponent 5; no endpoint sampling
frequency control is advertised. Actual audio status is not streaming; the
existing generic UAC1 policy rejects asynchronous feedback and its M3 packet
bound is below 540. The descriptive JSON rate did not cause this rejection.
Evidence: `.cache/hercules-inpulse-500/activation-20261010/`, actual descriptor
`usb-controller-descriptors.json`, status, bus trace and diagnostic TEXT.
An earlier request used an incorrect descriptor URI and returned the HTML
fallback; `usb-probe-descriptor.json` is not descriptor evidence.

The [USB Audio 1.0 specification](https://www.usb.org/sites/default/files/audio10.pdf),
section 3.7.2.2, defines three-byte 10.14 feedback as samples per USB frame and
the endpoint association. The new shared driver candidate validates the explicit
pipe in the same alternate setting, converts samples/ms to its Q16 accumulator,
bounds feedback against rate and endpoint capacity, and preserves fractional
packet cadence. The experimental controller-root packet bound admits 540 bytes;
qualified FLX4 selection still rejects asynchronous profiles. MAIN stays 48 kHz
while the existing USB resampler targets 44.1 kHz. Zero-byte feedback between
refreshes is not a new clock value; stale estimates stop the stream with bounded
cleanup. The existing UAC2 feedback decoder and clock negotiations are retained.

A captured fixture and actual stream-owner host regression pass endpoint
association, malformed width/refresh, missing feedback, 10.14 conversion,
capacity rejection, fractional 44/45-frame packets, stale stop, teardown and
FLX4 isolation. Existing UAC2 clock/feedback/30-reconnect host cases pass.
Full qualification, build/package/install of this feedback candidate and
physical Hercules audio remain pending. No S2 physical test is reopened.

## Feedback candidate installation and claim rejection

Clean `90fba6906338c68c7d38e322f5f4209aa0273d3f` passes the complete host
qualification without SKIP and the single M3 ESP-IDF 6.0.2 build, board/budget
and restart capture gates. Application: 2,602,208 bytes, SHA-256
`804f69abadec4600410eb2672997cf8643e669a03d4af0afa0cf8697f8e05c47`.
ELF: `38c63799653e616c0f707ddda7433b70423681aa5c8d1c86dd7463a950315387`.
Signed bundle: 2,602,396 bytes, SHA-256
`c14465d7319dd8882973ec40974ec2e8b3465c2e3de6ca91f57744cf7b4f3d23`.
Matching debug and verification files are retained under
`.cache/hercules-feedback-candidate-90fba690/pajoniiir-m3-M3-dev-g90fba6906338/`.

One local signed upload returned HTTP 200. The 90-second observer then timed
out while the PC was disconnected from the AP; original result/polls remain
unchanged. The operator reported a normal GUI. Reconnection to `Pajoniiir-M3`
confirmed exact clean source/ELF, VALID `ota_1`, API idle, boot 415/SW and both
decks IDLE. No repeat upload or reset was needed. Evidence, including separate
`recovery-verification.json`, is retained at
`.cache/hercules-feedback-wireless/20261010T220549Z/`.

Hercules MIDI IN/OUT and the v4 profile bind successfully. Audio selects packed
24-bit/44100 Hz, but remains non-streaming with no control completion or prime
fault reported. In this production startup path that places the rejection at
`usb_host_interface_claim`, before asynchronous configuration. The return code
is not exposed by this image; FIFO, HCD endpoint allocation and memory causes
are not established. The old crash summary still refers to the preserved
7456-byte historical image, not a new PANIC. No playback test was started.

The next diagnostic preserves claim rejection as prime stage 5, with interface
number and exact `esp_err_t`, while retaining the existing MIDI/ownership and
recovery behavior. Production stream tests exercise unsupported/no-memory
claims, zero leaked ownership/allocations, preserved evidence after cleanup
and successful following UAC2 startup. Focused tests pass; this diagnostic's
full build/install is pending. Hercules physical audio/native blend and controls
remain NOT RUN. S2 testing stays closed.

The generated production HCD exposed the missing integration constraint:
its HS-root periodic TX FIFO remains 128 quadlets (512 bytes), below the
recorded Hercules 540-byte MPS. The pinned HAL rejects such an endpoint during
claim; widening only the stream selector does not widen the HCD. The next
candidate uses 160 quadlets (640 bytes) on HS only when the diagnostic controller
experiment is enabled, as already used by JC1060. HS RX and bulk OUT retain
at least 512-byte capacity, and ordinary M3/JC4880 defaults and M3 FS FIFO
remain unchanged. An executable gate compiles the actual CMake replacement
block for all four board/experiment combinations and checks packet admission,
bulk/MIDI capacity, total FIFO bounds and unchanged standard layouts. It passes.
The claim-code diagnostic is retained to distinguish any further startup error.

## FIFO correction installed and combined physical smoke

Clean source `04f0cfb6dba310e29192f784ac5c06f84dd20c86` is installed as
`M3-dev-g04f0cfb6dba3`, VALID `ota_0`, matching ELF
`43ff2a28650883994261fcd8dfcc6072bb9591660d6271ea1653ce1bdba5542c`.
BIN: 2,602,208 bytes, SHA-256
`8ae0b087143f17231e8e88632b343c401707e0bc8a1979d73929a2e54c8a56a6`.
Signed bundle: 2,602,396 bytes, SHA-256
`56db91057350e4aa3cb3fb68b3749465ee1e2916ec7e7627a030dbf8bd884b71`.
The M3 ESP-IDF 6.0.2 build, board/budget, linked capture/raw gates and signature
checks pass. The full host qualification of the claim diagnostic passed exit 0,
no SKIP; the subsequent FIFO-only change passes the additional four-configuration
production-policy executable gate. No JC build was run. Matching artifacts and
verification scope are retained at
`.cache/hercules-fifo-candidate-04f0cfb6/pajoniiir-m3-M3-dev-g04f0cfb6dba3/`.

One marked local OTA returned HTTP 200 and the new image reached VALID/idle.
Hercules audio starts without claim/configuration/prime error: full-speed,
44100 Hz, packed 24-bit, subslot 3. Feedback updates grow, invalid count remains
zero. Both decks were stopped. Installation evidence:
`.cache/hercules-fifo-wireless/20261010T221928Z/`.

The outgoing `90fba690` restart also produced a natural 7776-byte panic in
`ota_reboot`. It was exported and checksum/identity-validated against the exact
outgoing ELF; the captured register sequence resolves to `Cache_WriteBack_All`.
This is not a clean-restart PASS. See the separate
[natural restart evidence](RESTART_BUS_CAPTURE_20261010.md#first-natural-captured-ota-restart-fault).
The historical 7456-byte original is preserved, not overwritten in the host
archive. No further reset or intentional fault was introduced.

The combined 60.078-s test used Casanova Action (44100 Hz, +5%) and Ritam moj
(48000 Hz, -5%), both MT and 4-beat loops, operator-confirmed zoom 8, normal
Wi-Fi, PCM5102A MAIN 48000 Hz and Hercules headphones 44100 Hz. The operator
rotated both jogs/Browse and checked PFL D1/D2 plus native headphone Mix/Volume.
All controls, both audio outputs and waveforms were confirmed normal.

All 85 samples pass the independent audit: exact source, active healthy
44100-Hz stream, both MT/loops/playing, all 22 strict counters unchanged.
There were 649 new MIDI packets and zero new output-late events. The idle
underflow baseline 4,709,681 predates this test; its playback delta is zero,
not a claim that the since-boot counter was zero. Both decks were safely stopped.
Evidence: `.cache/hercules-inpulse-500/combined-60s-20261010T222234Z/`, including
original result, final status/firmware/UI/descriptor/bus trace, separate operator
confirmation/sample audit and SHA-256 manifest.

This closes the combined short smoke only. Full pad/FX/LED/load/screensaver
coverage, reconnect/power and long Hercules qualification remain NOT RUN;
RCA MAIN is not the PCM5102A reference. Restart fault root cause also remains
open. No S2 gate is reopened, and no push, release or public OTA is authorized.

## Later cache restart candidate: `ac9287af`

Clean `ac9287af4a4c07d826474ed11f0688e81135e515` is now installed as
`M3-dev-gac9287af4a4c`, VALID `ota_1`, with the same Hercules profile and USB
roles. The complete qualification host suite, exact M3 ESP-IDF 6.0.2 build,
linked restart gates, board/budget checks and signed package pass.
Its shared normal restart joins the IDF cache-sync lock before resetting the
peer core; architecture and SoC PANIC paths bypass the wait.

One stopped software restart with Hercules idle audio streaming passes:
boot 418 -> 419, reset `SW`, exact image remains VALID, full raw crash hash
unchanged, Hercules MIDI/UAC1 44100-Hz stream restored. Both decks remain
stopped. This is not a USB cable reconnect, cold power cycle, listening test,
or new-image playback qualification. The operator-confirmed 60-second smoke
above still belongs to `04f0cfb6`; it is not transferred to this candidate.
On 2026-10-11 the operator confirmed: "Gui je uredan". This closes new-image
GUI restoration only; historical fault causality, wider restart reliability
and new-image playback/audio qualification remain open. Full artifact identities and records
are in [the restart observation](RESTART_BUS_CAPTURE_20261010.md#exact-candidate-and-stopped-restart-observation).

## Unattended unloaded idle observation, 2026-10-11

On exact clean `ac9287af` / ELF
`53808ea215b37addd063c783afbd995b219b4a3c000b9d8b332cc86230da9113`,
VALID `ota_1`, a read-only 180.172-second observation completed with 171 samples.
Both decks were unloaded IDLE throughout, the PCM codec remained closed, and
Hercules MIDI/profile/UAC1 FS/44100-Hz packed-24 readiness and the USB2 storage
mount remained present. Uptime increased from 5,376,170 to 5,556,653 ms; no
reboot occurred inside this observed window. The preflight uptime alone is
not evidence of an earlier continuously observed run.

All 21 explicitly enumerated connection/loss counters have zero deltas,
including ISO packet failures/lost frames, PCM underruns, dropped/overflow
frames, invalid feedback, MIDI/queue errors, USB disconnect/recovery and
service-log drops. Feedback updates increased by 45,121; output-late delta
was zero. Every saved sample and both firmware endpoints passed an independent
audit against exact identity, counter schema and unloaded/no-producer scope.

The idle ring underflow counter increased by 7,960,226 frames and is reported
separately, not hidden or treated as zero. Production `prepare_transfer()`
calls `controller_audio_ring_read(..., true)`: an empty ring zero-fills the
requested packets and counts unavailable frames. Here submitted frames/blocks
and queued frames remained zero, with no loaded deck or active PCM producer.
This narrowly scoped idle observation therefore does not apply the playback
underflow gate. The existing 22-counter playback gate remains unchanged;
this is neither audible silence verification nor a playback/soak PASS.

Original baseline, JSONL, final status/firmware/bus trace, profile registry,
runner, independent audit and SHA-256 manifest are retained at
`.cache/hercules-inpulse-500/unloaded-idle-20261011T093801Z/`.
Preflight and deterministic local profile regeneration are at
`.cache/hercules-inpulse-500/unattended-ac9287af/`. Both runner and audit exit 0.
There were no POST requests, playback, firmware/profile changes, resets or
intentional physical changes in this observation. No monitor remains running.

## Remaining combined operator blocks

Prepared on 2026-10-11 while the operator was away. No playback, control,
profile, firmware, USB, Wi-Fi or power change is needed for this preparation.
The unchanged JSON recompiles byte-for-byte to the committed 5592-byte fixture,
SHA-256 `570b59ecca6baa7319ef85ca92d7d3a11fbe11ea1f9c9c580a8779031a40f549`.
The live registry reports valid exact VID/PID, active profile and matching size;
it does not expose a raw installed-binary hash. Local regeneration is not a
byte-for-byte readback of NVS.

The existing host evidence covers compiler/semantic parity, representative
transport/Shift/loop/pad and LED mappings, registry/atomic replacement, and the
production native-headphone ownership path. The Hercules-specific test checks
ignored native knobs/buttons, intentional PCM master volume, both jog-touch
releases, v2/v3 capability rejection, missing-UAC rejection and activation/clear.
These tests do not prove physical pad colours, knob endpoints, release behavior
or audible routing. Key/Pitch Play pads, Vinyl/Slip, stutter-play, guide controls
and physical modes 7/8 remain deliberately deferred; do not invent a PASS for
them or expand the profile solely to finish a checklist.

| Combined block | Checks and completion evidence | Current-image state |
| --- | --- | --- |
| Navigation, protection and transport | Both LOAD/Shift+LOAD routes with stopped decks, LOAD LOCK on each playing deck, CUE hold/release and PLAY handoff, Sync/Shift+Sync, Quantize/range, loop rotary/In/Out/exit; exact titles and other-deck preservation | NOT RUN on `ac9287af` |
| Mixer, pads/FX and visible feedback | Both pitch/fader/trim/EQ/filter endpoints, crossfader and intentional PCM MASTER; each implemented pad bank on both decks, press/release and Shift behavior; mode/transport/PFL/loop LEDs, pad colours and VU. Use existing source cue recall; local cue writes/deletion require an explicitly disposable test track | NOT RUN on `ac9287af` |
| Dual audio and controls | One short monitored mixed-rate MT/loop block, opposite pitch and zoom 8: rotate both jogs/Browse, check PFL D1/D2 and native headphone blend/volume endpoints, then hands off for listening/dual waveform observation. Restore mixer centers/faders before starting. Audit actual MIDI growth and all 22 playback strict deltas | NOT RUN on `ac9287af`; historical `04f0cfb6` smoke retained |
| Wake and lifecycle | First controller action only wakes screensaver; next action operates normally, with no ghost action/stuck release. Three stopped cable reconnects with matching profile/healthy 44100-Hz stream and LED resync; separate cold-power check. Controller reconnect, software restart and cold power are distinct evidence | Only one stopped software restart and GUI restoration confirmed |
| Final timing window | Run the agreed long gate only on the final unchanged image/configuration, with operator listening/visual confirmation and strict loss counters. Keep isolated output-late findings separate | NOT RUN for Hercules |

Capture one continuous record per short operator block and combine independent
observations inside it. Repeat only failed or ambiguous portions. Stop both
decks at each playback block's end or first strict error. An operator must be
present for listening, visible LEDs/waveforms, physical inputs and cable/power
actions; unattended USB liveness cannot close these gates. S2 stays closed.
