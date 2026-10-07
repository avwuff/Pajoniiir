# Traktor Kontrol S2 MK3: basic transport profile

Status: experimental; basic inputs/navigation observed on a PC, remaining
controls and LED outputs source-derived. M3 physical acceptance is NOT RUN.
This is a limited S3CP v2 profile, compatible with v91 and the earlier M2.4
runtime. Do not advertise full S2 MK3 support based on SD installation.

See the [current preparation and capture checklist](../../docs/validation/S2_MK3_PROFILE_PREPARATION_20261007.md)
for the borrowed-device MIDI capture workflow, shared-runtime gaps and ordered
physical gates. The historical upload below is not current M3 acceptance.

## Source and reproduction

Factory mapping `NI Traktor Kontrol S2 MK3.djayMidiMapping` extracted from
[Algoriddim djay Pro 2.2.9](https://download.algoriddim.com/djay/202010171657/djay_Pro_2.2.9.zip),
member `djay Pro 2.app/Contents/Resources/MIDI Mappings/NI Traktor Kontrol S2 MK3.djayMidiMapping`.
Downloaded 2026-10-01 without installing or executing the application.

- Archive SHA-256: `a3c00db3030859bdb71c74368ac5adcc85d62a06790e31a635fa6c8f663775ee`
- Mapping SHA-256: `abd3519e35ab24235d950ab5940dfdf72cbb6d5342f7d62fba089945cbebfe0e`
- USB ID in mapping: `0x17CC1710` (VID `17CC`, PID `1710`).
- djay channel fields are zero-based; source channels 1/3 become statuses
  `0x91`/`0x93`, not `0x90`/`0x92`. The archive's DDJ-400 map independently
  uses channels 0/1 for its known `0x90`/`0x91` PLAY messages.
- Source mapping has 235 input records, including repeated modifier layers.
  Internal `customClassName` logic is not contained in the plist.

The original user-supplied Traktor TSI uses native control identifiers and is
not the MIDI address source. Original third-party artifacts remain in ignored
`tmp/traktor-s2-mk3/`; only this limited conversion is included here.

From the repository root:

```powershell
python controllers/ni_traktor_s2_mk3_basic/convert_djay.py "tmp/traktor-s2-mk3/NI Traktor Kontrol S2 MK3.djayMidiMapping"
python tools/controller_profile/compile_profile.py controllers/ni_traktor_s2_mk3_basic/profile.json -o controllers/ni_traktor_s2_mk3_basic/profile.s3bin
```

## Included behavior

Both decks: PLAY, CUE, SYNC, headphone PFL, separate LOAD buttons,
Browse encoder press for the shared browser action and rotation for navigation,
loop encoder turn to halve/double an already active loop, and loop encoder
press to exit/re-enter the last loop. This does **not** create an automatic
loop when no previous loop exists; use the screen for initial loop creation.

Pads are deliberately fixed to eight hot cues; shifted pad addresses clear
hot cues. HOTCUES/SAMPLES mode buttons and djay modifier layers are not mapped.
Only PLAY/CUE/SYNC/PFL LED outputs are included, using source on=127/off=20.
Pad LEDs, RGB and meters remain unmapped.

## Remaining work

- Mixer and tempo use 7-bit CC values; current semantic mixer/tempo consumers
  expect 14-bit values. Mapping these directly with `cc7_abs` would give the
  wrong range. v91 supports `cc7_to14` scaling (v3/v4), but this fixture still
  omits them; map and qualify the controller-specific inversion before enabling.
- Browser rotation uses the observed offset-binary values `0x41` (+1) and
  `0x3F` (-1). Actual on-device list direction remains unqualified. Tempo is
  marked `flipped` in the source and remains disabled pending qualification.
- Jog relative encoding is documented by the source, but its sensitivity and
  acceleration differ from the FLX4 path. Jog controls remain disabled pending
  proper scaling and touch/bend behavior validation.
- SHIFT routing, pad modes and any initialization performed by djay's custom
  class require further work. No custom-class behavior is guessed here.
- USB audio is disabled in this profile. Descriptor-driven UAC support in the
  integrated runtime does not physically qualify the S2 MK3 audio path.
- Confirm USB MIDI enumeration/VID/PID, all button edges, shifted pads, LEDs,
  reconnect/reboot behavior, and eventually MAIN/cue audio on physical S2 MK3.

Enter MIDI mode by holding the left FLX button while connecting USB, with
current controller firmware, per [Native Instruments instructions](https://support.native-instruments.com/support/solutions/articles/69000879412-traktor-switching-your-kontrol-s2-mk3-to-midi-mode).

## Validation and SD installation (2026-10-01)

This section describes the historical 864-byte source-derived profile, not
the updated PC-capture revision below.

- Compiler succeeded: S3CP v2, 864 bytes, 46 expanded input entries and 8 LED outputs.
- Checked header version, exact size, CRC, VID/PID, unique input addresses,
  deck channel offsets and absence of unsupported mixer/tempo entries.
- Existing `tests/controller_runtime/run_tests.ps1` passed, including compiled
  profile runtime and FLX4 regression checks. This suite does not physically
  qualify S2 MK3 controls.
- POST `/api/controller-profile` with overwrite disabled returned `ok:true`.
  GET `/api/controller-profiles` confirmed `ni_traktor_s2_mk3_basic`,
  `valid:true`, VID `0x17CC`, PID `0x1710`, size 864.
- The connected FLX4 retained active profile `pioneer_ddj_flx4`; the S2 profile
  is stored on SD, not activated or hardware-tested.
- No firmware changes, firmware build, OTA or device reboot performed.

## PC-capture navigation revision (2026-10-08)

The KLIKNI PC enumerated `17CC:1710` with Microsoft MIDI and USB Audio 2.0
drivers, one S2 MIDI input and one S2 MIDI output. The initial complete capture
was `S2-MK3-capture-20261008-010551-ef9851`; D1 LOAD contained zero events
because the operator missed the window. The isolated navigation repeat
`S2-MK3-capture-20261008-011403-ccc0e0` completed all six steps with no invalid
messages, driver errors, pressure or dropped messages.

| Input | Left deck | Right deck | Interpretation |
| --- | --- | --- | --- |
| Separate LOAD | `91 13` | `93 13` | Per-deck load; `7F` press / `00` release |
| Browse push | `91 18` | `93 18` | Shared browser press; `7F` / `00` |
| Browse rotation | `B1 19` | `B3 19` | `41` clockwise / `3F` counterclockwise, per operator sequence |
| Additional contact-associated note | `91 29` | `93 29` | Unmapped; no wake or action generated by this profile |

`convert_djay.py` applies these explicit capture overrides after the pinned
source conversion. It does not assign semantics to the additional note.
PC captures also confirm PLAY/CUE/SYNC/PFL press/release addresses on both decks.
Raw evidence and SHA-256 inventory are retained locally under
`.cache/s2-mk3/20261008-pc-capture/`; they are not release artifacts.

The regenerated S3CP v2 binary is 928 bytes, 50 expanded inputs, eight outputs,
SHA-256 `20efffe7778991b79fb2ba50d7dd425eac89bd50fe0dcbafcc7abfde26aaebbe`.
USB audio remains disabled. Production runtime tests execute this actual binary
and check per-deck LOAD, browser push edges, rotation direction, ignored `29`
notes and wrong PID rejection. Compiler/parser and controller runtime suites
pass, including existing FLX4 parity. Profile upload/activation, S2 LED output,
M3 browser direction and audio/reconnect acceptance are NOT RUN. No firmware
source, installed image, OTA or device wiring was changed by this revision.
