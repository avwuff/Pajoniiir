# Hercules DJControl Inpulse 500 preparation

The existing source-backed profile is an offline candidate, not a hardware
support claim. See the [MIDI map](../../docs/HERCULES_INPULSE_500_MIDI_MAP.md)
and [M3 preparation record](../../docs/validation/HERCULES_INPULSE_500_PREPARATION_20261010.md).

The baseline fixture is S3CP v2, 5700 bytes, with 274 expanded inputs and
107 outputs. SHA-256:
`4eb737e7dd6c96432878e273b1d0466c19ff07b295fc372940c42e3c0db5aa70`.
It matches exact USB identity `06F8:B12B`; `06F8:B105` is rejected.

```powershell
python tools/controller_profile/compile_profile.py controllers/hercules_djcontrol_inpulse_500/profile.json -o .cache/hercules-profile.s3bin
```

Before installing, resolve the native hardware output-control policy and
the filter gate. The current profile inherits the FLX4 Smart CFX filter
requirement; independent Hercules filters need the existing v4
`channel_filter_requires_smart_cfx: false` capability. No new firmware ABI is
required for that flag on the current shared-core candidate.

The manufacturer documents 24-bit, 44100-Hz audio, MAIN channels 1/2 and
headphones 3/4. The JSON `audio` section is descriptive: the compiler does
not serialize its channel/rate fields. Its current 48000 preference therefore
does not configure the USB driver and must not be used as evidence of a
48-kHz Hercules mode. Actual USB descriptors and clock replies are required.

Master volume, headphone level/mix and master-headphone selection also act in
the controller hardware. Decide which events should affect PCM5102A MAIN and
which should be left to native hardware before enabling their software mirrors;
avoid applying the same attenuation or cue blend twice.

After the running S2 soak finishes and both decks are stopped:

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

Hardware tests, USB root/speed, audio format and headphone routing remain
**NOT RUN** until measured on M3. No profile is uploaded during the S2 soak.
