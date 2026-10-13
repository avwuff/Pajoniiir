# Numark DJ2GO2 Touch: software profile

Status: software profile. Hardware run: the operator ran it on a real DJ2GO2 Touch on 2026-10-13: buttons, faders, jog, levels, browse, crossfader, audio (Main and Cue) and USB disconnect OK; LEDs, Note Off velocity and the level double change were not reported (see "Bench checks"), so this is a partial acceptance. The profile comes from a public mapping.

Build: S3CP v4, 1999 bytes, 100 inputs, 30 outputs, VID/PID `15E4:003C`.

## Source

The MIDI addresses come from the Mixxx mapping for this controller. The author is tandy1000. The profile takes addresses and control roles only. It does not copy script code.

- Repository: [mixxxdj/mixxx](https://github.com/mixxxdj/mixxx), commit `0fffd3d0d07b755c34c1730f7b87afadd8f2be19`.
- `res/controllers/Numark_DJ2GO2_Touch.midi.xml`, SHA-256 `30807f8e574e857149c4e5b3f79e2d07ab8a841e159c040852a8c5d7b1641990`.
- `res/controllers/Numark_DJ2GO2_Touch_scripts.js`, SHA-256 `8c39e503fcd2c1c6be23c997d7c092b58ee91aec22d32fa1fa7392e1f1fd900f`.

The XML file has no outputs. The LED messages and the initial SysEx come from the script. Mixxx sends each LED message to the same note as the input.

An operator `lsusb -v` listing of a real unit confirms the USB ID `15E4:003C`. Public USB ID listings give the same ID.

## Controls

Deck 1 uses status `0x90` for transport and `0x94` for pads. Deck 2 uses `0x91` and `0x95`. Global controls use MIDI channel 16 (`0x9F` and `0xBF`). Notes are Note On. The profile also matches Note Off (`0x8n`) for every note.

| Control | MIDI | P4 event |
| --- | --- | --- |
| PLAY, CUE, SYNC | Notes `0x00`, `0x01`, `0x02` | `deckN.play`, `cue`, `sync` |
| Headphone cue | Note `0x1B` | `deckN.pfl` |
| Jog wheel touch | Note `0x06` | `deckN.jog_touch` |
| Jog wheel turn | CC `0x06`, two's complement | `deckN.jog_scratch` |
| Tempo fader | CC `0x09`, 7 bit | `deckN.tempo` |
| Level knob | CC `0x16`, 7 bit | `mixer.chN_volume` |
| Pads, CUES mode | Notes `0x01` to `0x04` | Hot cue pads 1 to 4 |
| Pads, CUES mode, PAD MODE held | Notes `0x09` to `0x0C` | Hot cue pads 1 to 4, shifted (clear) |
| Pads, AUTO mode | Notes `0x11` to `0x14` | Beat loop pads 1 to 4 |
| Pads, MANUAL mode | Notes `0x21`, `0x22`, `0x23`, `0x24` | Loop in, loop out, reloop exit, reloop stop |
| Pads, SAMPLER mode | Notes `0x31` to `0x34` | Not mapped |
| LOAD | Notes `0x02` (deck 1), `0x03` (deck 2) | `browser.load_deckN` |
| Browse push | Note `0x06` | `browser.press` |
| Browse turn | CC `0x00`, `0x01` and `0x7F` | `browser.delta` |
| Crossfader | CC `0x08`, 7 bit | `mixer.crossfader` |
| Master Level | CC `0x0A`, 7 bit | `system.master_volume` |
| Cue Level | CC `0x0C`, 7 bit | `system.headphone_level` |

The 7-bit controls use `cc7_to14`. The values 0, 64 and 127 become 0, 8192 and 16383. The tempo fader has no inversion. The Mixxx script uses the same direction as the Mixxx DDJ-400 script, and the DDJ-400 profile also has no inversion.

A release counts as a press when its velocity is above zero. The profile assumes a release velocity of 0. The Mixxx script uses the same rule.

## Sound card

The controller has a built-in USB audio interface. The Mixxx manual lists outputs 1 and 2 as Main and outputs 3 and 4 as headphones. The profile sets `usb_audio` and `headphone_channels` 3 and 4. These fields only describe the profile. They do not start audio.

The P4 audio runtime reads the USB descriptor and selects the format. It needs four channels, PCM, 16 or 24 bits, and 44100 or 48000 Hz. It tries 44100 Hz first. For a controller that is not an FLX4, the output endpoint must be adaptive or synchronous. The runtime accepts an asynchronous endpoint only with an explicit feedback endpoint.

The runtime starts USB audio only when the controller connects directly to a root port. A controller behind a hub gets MIDI only. If the descriptor does not meet the rules, MIDI stays active and audio stays off.

The `lsusb -v` listing shows a full-speed UAC1 device (control endpoint packet size 8). The audio stream has 4 channels, 16 bits and one rate, 44100 Hz. The isochronous OUT endpoint is synchronous and has no feedback endpoint. Its packet size is 384 bytes. The runtime accepts this descriptor. The stream path converts a 48000 Hz source to 44100 Hz.

The test `test_numark_touch_fs` in `tests/controller_usb_audio` runs the real format selection and stream start on this descriptor. It also checks the channel routing. MAIN goes to channels 1 and 2. The cue bus goes to channels 3 and 4. The test uses a descriptor rebuilt from the text listing, not raw bytes.

The profile maps Master Level and Cue Level to software levels. It does not set `native_headphone_controls`. It is not known if these knobs also change the analog output. If they do, the level changes twice. Test this on the bench. The compiler rejects `native_headphone_controls` together with the Cue Level mapping.

## Differences from the Mixxx mapping

- **Level knob.** Mixxx maps it to a quick-effect filter. The forum post for the mapping calls this assignment non-standard. The profile maps it to channel volume. To match Mixxx, use `mixer.chN_filter`.
- **Right deck RELOOP STOP.** The Mixxx XML lists status `0x95` twice for this pad. The left deck has `0x84` and `0x94`. The profile uses `0x85` and `0x95` for the right deck.
- **AUTO pads.** Mixxx makes loops of 1, 2, 4 and 8 beats. P4 beat loop pads 1 to 4 make loops of 1/32, 1/16, 1/8 and 1/4 beat. The `pad_bank` type cannot start at a later P4 pad.
- **SAMPLER pads.** P4 ignores the sampler pad mode. These inputs stay under `omitted_inputs` in `profile.json`.
- **MANUAL pads `0x23` and `0x24`.** The profile maps the inputs. P4 has no LED for these two pads.
- **Initial SysEx.** Mixxx sends `F0 00 01 3F 3C 48 F7` at start. The script names it `ControllerStatusSysex`. The profile sends it once at each activation. The reply is not documented. We expect the controller to send its knob and fader positions. A capture must confirm this.

## Jog wheel

Each wheel sends one CC for the turn and one note for the touch. The profile maps the CC to `jog_scratch`. The deck core uses the touch state to choose scratch, bend or scrub. Clockwise turns give positive values in the Mixxx script. No hardware test has confirmed this.

## Build and install

1. Run `python tools/controller_profile/compile_profile.py controllers/numark_dj2go2_touch/profile.json -o controllers/numark_dj2go2_touch/profile.s3bin`.
2. Copy `profile.s3bin` to `/controllers/numark_dj2go2_touch/` on the SD card.

To upload the file over Wi-Fi instead, follow [CONTROLLER_PROFILE_UPDATE.md](../../docs/CONTROLLER_PROFILE_UPDATE.md). Use firmware that accepts S3CP v4. The M2.4 image rejects this profile. The schema is in [CONTROLLER_PROFILE_SCHEMA.md](../../docs/CONTROLLER_PROFILE_SCHEMA.md).

## Tests

Run `tests/controller_profile/run_tests.ps1`. It builds `test_numark_dj2go2_touch.c` and runs it against `profile.s3bin`. The test checks these items: every mapped address, Note On and Note Off, the 7-to-14-bit values, the pads, the LEDs, the replay set, and CRC rejection.

Run `tests/controller_usb_audio/run_tests.ps1` for the descriptor and stream test.

## Bench checks

Operator run on a real DJ2GO2 Touch, 2026-10-13 (reported in one line, no values recorded):

| Check | Result |
| --- | --- |
| Buttons | OK |
| Faders (tempo, channel, crossfader) | OK |
| Jog wheels | OK |
| Levels and browse | OK |
| Crossfader | OK |
| USB cable disconnect and reconnect | OK |
| Note Off velocity of each button | OK, all releases are Note Off velocity 0 (see captures below); PFL is a latching button, see the finding |
| LEDs in the four pad modes | observed, see the notes below (partial) |
| Master Level and Cue Level double change | not reported |
| Audio to Main and to the headphones (Cue) | OK |

The one "not reported" row are still to be run or recorded. The USB ID and the audio descriptor already come from a real unit.

### Captures on a PC (2026-10-13, `aseqdump`, controller on the PC, not on the P4)

- PLAY, CUE, SYNC, LOAD (notes 2 and 3 on channel 16), browse push (note 6 on channel 16) and the pads: Note On velocity 127, release is **Note Off velocity 0**, on both decks. Matches the profile's release rule.
- **There is one PAD MODE button and no SHIFT button.** Pressing PAD MODE cycles CUES, AUTO, MANUAL, SAMPLER, CUES. Each press sends, on both pad channels at once (`0x94` and `0x95`), note 0: Note On velocity 5 (to AUTO), 11 (to MANUAL), 7 (to SAMPLER) and a Note Off velocity 0 (back to CUES). The pads then send the notes in the table above: CUES notes 1-4, AUTO 0x11-0x14, MANUAL 0x21-0x24, SAMPLER 0x31-0x34, all with Note Off velocity 0. The profile does not need note 0 on the pad channels: the mode is implied by the pad notes. The "PAD MODE held" row in the controls table (notes 0x09 to 0x0C) was not seen in these captures and is not confirmed.
- **Finding, PFL (headphone cue, note 0x1B on `0x90` and `0x91`):** the button latches. Press one sends Note On 127, press two sends **Note Off velocity 0**, press three Note On 127 again. The firmware toggles PFL only when the value is non-zero (`deck_core.c` `on_mixer_control`, `CTRL_ID_DECK1_PFL` / `DECK2_PFL`: `if (raw != 0)`), so the Note Off of the second press would be ignored and the P4 PFL would run opposite to the button from the second press on. Read from the code and the captures, **not yet checked on the P4**: press PFL three times with the controller on the P4 and watch `pfl1` / `pfl2` in `/api/status`. If it is confirmed, a fix is to map only the Note On of PFL (drop the `0x80` / `0x81` entries), so each press toggles once; the Note Off entries are in the profile today.

### Operator observations on the P4 (2026-10-13)

- **PFL desync confirmed.** Press 1: LED on (sound on the headphones). Press 2: LED still on, headphone sound **off**. Press 3: LED off, sound **on**. So from the second press the P4 state and the button state are opposite, as predicted from the Note Off being ignored. Not fixed yet.
- **Pad LEDs by mode** (operator wording): CUES: LEDs on for the pads that hold a cue, the others dimmed. AUTO: all four dimmed. MANUAL: pads 3 and 4 dimmed, pads 1 and 2 off. SAMPLER: all four dimmed. Reading: the controller dims an unassigned pad; pads 3 and 4 of MANUAL (reloop) have no P4 LED by design (`omitted_inputs`), pads 1 and 2 are loop in and loop out, off while no loop is set; SAMPLER is not mapped. Not checked: LED state with a loop set, an active beat loop in AUTO, and a cleared cue; not checked per deck.
