# Numark DJ2GO2 Touch: software profile

Status: software profile. Hardware acceptance: **NOT RUN**. No test with a real DJ2GO2 Touch has run. The profile comes from a public mapping and passes the host tests only. Do not call it supported until a bench check records the real MIDI, LED and audio behavior.

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

## Bench checks not run

The USB ID and the audio descriptor already come from a real unit. These checks need the P4 and the controller together.

1. Connect the controller directly to a P4 root port.
2. Press each button. Record the Note Off velocity.
3. Move each tempo fader. Check the direction and the end points.
4. Turn each jog wheel. Check the direction and the touch state.
5. Check the LEDs in all four pad modes.
6. Set Master Level and Cue Level. Listen for a double level change.
7. Play audio to Main and to the headphones. Disconnect and connect the USB cable.
