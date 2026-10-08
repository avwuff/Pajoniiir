# Traktor Kontrol S2 MK3 performance candidate

Experimental S3CP v4 profile for `17CC:1710`: 80 expanded inputs,
26 outputs, 1624 bytes. It uses the shared controller runtime. The basic
928-byte profile is preserved as a recovery/reference fixture.
Binary SHA-256: `ab9259c51c2b18cf5b67e2e279e3a0fbf9ee167a489f345b813a9ef37f40725f`.

The generator imports the pinned djay factory mapping through the basic
converter, retains PC-captured navigation corrections, and adds source-backed
mixer, pitch, jog and LED mappings. Source SHA-256:
`abd3519e35ab24235d950ab5940dfdf72cbb6d5342f7d62fba089945cbebfe0e`.
The original factory file is not redistributed.

```powershell
python controllers/ni_traktor_s2_mk3_full/convert_djay.py "<factory.djayMidiMapping>"
python tools/controller_profile/compile_profile.py controllers/ni_traktor_s2_mk3_full/profile.json -o controllers/ni_traktor_s2_mk3_full/profile.s3bin
```

| Group | Mapping / evidence |
| --- | --- |
| Navigation | Both dedicated Browse/LOAD paths retain PC-captured addresses; shifted LOAD is a product mapping and requires physical confirmation |
| Transport, pads, loops | PLAY/CUE/SYNC/PFL, shifted pads, loop size/reloop inherited from the basic source fixture; historical acceptance remains image-specific |
| Mixer | Channel volume, trim, three EQ bands, filter; crossfader and headphone mix; CC7 scaled to 14-bit with replay |
| Pitch | Inverted CC7 to 14-bit, exact 8192 center, no replay |
| Jog | Touch, relative scratch/bend, shifted search; 1:1 relative steps are a candidate requiring physical sensitivity validation |
| Feedback | Transport LEDs, eight Hot Cue LEDs per deck, source-specific Note On VU velocity |
| Audio | Descriptor-validated experimental HS UAC2 at 48 kHz, MAIN channels 1/2 and PFL channels 3/4 |

Filters work without the FLX4 Smart CFX enable gate. Unlabelled custom djay
controls, a standalone SHIFT address, pad-mode buttons and hardware MAIN/
headphone volume controls are not guessed. djay rotary sensitivity/acceleration
units are not copied into the DSP. No arbitrary initialization SysEx is sent.
This is the complete source-backed performance candidate, not a claim that
every physical control or proprietary mode is supported.

Use the experimental M3 role swap: **S2 on USB3 HS, stick on USB2 FS**.
Enter MIDI mode by holding left FLX while connecting. FLX4 production roles
remain unchanged. The audio flag declares capability; actual descriptors and
clock replies determine whether streaming can start.

Install matching firmware before this profile: older v4 parsers reject raw 10.
The registry chooses the first matching VID/PID; do not leave two matching S2
variants on the SD card. On the current bench, atomically overwrite the existing
SD id `ni_traktor_s2_mk3_basic` with this candidate binary. The SD id is a storage
location, not the profile variant. Keep the repository basic binary for recovery.

## Combined physical checks

Keep both decks stopped for one brief control batch: both channel faders,
trim/EQ/filter centers and endpoints, crossfader, headphone mix, both pitch
directions/center, jog touch/rotation/shifted search, and dedicated LOAD routing.
Restore mixer controls before playback. Avoid modifying source cues during this batch.

Then use one short dual-playback batch for clean PCM5102A MAIN and S2 headphones,
PFL/mix separation, jog bend/touch behavior, CUE/PLAY handoff, loop resize/exit,
SYNC and visible LED/waveform behavior. Report failures by deck/control; repeat
only failed or ambiguous steps. Run the final timing soak last, on the exact
accepted image. Reconnect and FS storage qualification remain separate open gates.

Software tests are not a physical or headphone PASS. The authoritative evidence
is [the S2 validation record](../../docs/validation/S2_MK3_PROFILE_PREPARATION_20261007.md).
