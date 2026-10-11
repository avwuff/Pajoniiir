# Hercules DJControl Inpulse 500 preparation

The source-backed profile is software-qualified and installed on the M3 bench.
A combined physical smoke passed on `04f0cfb6`; full hardware acceptance and
playback qualification on the current `ac9287af` remain open. See the [MIDI map](../../docs/HERCULES_INPULSE_500_MIDI_MAP.md)
and [M3 preparation record](../../docs/validation/HERCULES_INPULSE_500_PREPARATION_20261010.md).

The current fixture is S3CP v4, 5592 bytes, with 268 expanded inputs and
106 outputs. SHA-256:
`570b59ecca6baa7319ef85ca92d7d3a11fbe11ea1f9c9c580a8779031a40f549`.
The historical 5700-byte v2 fixture remains documented in the preparation record.
It matches exact USB identity `06F8:B12B`; `06F8:B105` is rejected.

```powershell
python tools/controller_profile/compile_profile.py controllers/hercules_djcontrol_inpulse_500/profile.json -o .cache/hercules-profile.s3bin
```

Install matching native-headphone firmware before uploading this profile.
M3 `0b08fb90` and older v4 firmware ignore the new capability bit 5. The profile
uses `channel_filter_requires_smart_cfx: false` for independent channel filters
and `native_headphone_controls: true` for hardware cue/master blend and level.
The firmware supplies the selected mono pre-fader cue bus at unity on USB 3/4
and retains MAIN on 1/2. Software headphone mirrors and the native master-cue
LED output are omitted. Software headphone settings remain stored for other
profiles. MASTER intentionally controls PCM5102A volume; controller RCA MAIN
is not the reference output and remains unqualified.

The first matching clean firmware was `eafdb7a5`. The current installed image
is clean `ac9287af`, VALID `ota_1`; its M3 build, full host qualification and
signed local OTA pass. The profile remains active and unchanged. One stopped
software restart and GUI restoration are confirmed on this image; the earlier
audio/control smoke belongs to `04f0cfb6`. See the preparation record for exact
hashes, evidence and the combined remaining operator blocks.

The manufacturer documents 24-bit, 44100-Hz audio, MAIN channels 1/2 and
headphones 3/4. The JSON `audio` section is descriptive: the compiler does
not serialize its channel/rate fields. The corrected 44100 preference
does not configure the USB driver or establish format support. Actual USB descriptors and clock replies are required.

Headphone level/mix and master-headphone selection remain native hardware
controls. Verify both blend endpoints and PFL physically before accepting this
route. No arbitrary initialization SysEx is sent.

S2 work is closed by the operator. With both decks stopped:

1. Save the exact-image result and physical confirmation before changing USB.
2. Read Hercules descriptors first. Initially retain the bench controller port
   (USB3) and stick on USB2; this is a test arrangement, not a final default.
3. Install the audited binary atomically with
   `POST /api/controller-profile`, `X-DDJ-Control: 1`,
   `X-DDJ-Profile-ID: hercules_djcontrol_inpulse_500`. Confirm profile matching;
   do not overwrite the S2 profile or use an arbitrary initialization SysEx.
4. Combine navigation, transport, pitch/mixer, pads/LEDs and jog checks in one
   stopped-deck block; restore mixer centers/faders before playback.
5. Combine MAIN/headphone routing, PFL, jog/Browse and dual MT/loops in one
   short playback block with strict ISO/PCM/UAC counters.
6. Check reconnect and screensaver wake, then qualify the final image/config
   with a separate soak. Repeat only failed or ambiguous checks.

Measured M3 topology is Hercules on USB3/root0 as a full-speed UAC1 device,
44100 Hz, packed 24-bit; storage is on USB2/root1. Historical short audio/control
acceptance does not establish full qualification or transfer to a new image.
S2 testing stays closed. No public release or production USB-default change
is implied by this experimental arrangement.
