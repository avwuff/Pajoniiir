# Hercules Inpulse 500 preparation, 2026-10-10

Status: offline preparation and Windows enumeration; M3 hardware NOT RUN.
This work does not alter the running `3f3605a7` S2 soak, controller profile,
firmware, USB connections or workload. No M3 API polling is added.

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
