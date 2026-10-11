<p align="center">
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="docs/images/pajoniiir-logo-white.svg">
    <source media="(prefers-color-scheme: light)" srcset="docs/images/pajoniiir-logo-black.svg">
    <img alt="Pajoniiir" src="docs/images/pajoniiir-logo.svg" width="450">
  </picture>
</p>

# Pajoniiir

**A standalone dual-deck DJ system with one ESP32-P4 firmware core for Guiton
JC4880P443C_I_W, JC1060P470C_I_W_Y and JC-ESP32P4-M3-DEV boards. Note that JC-ESP32P4-M3-DEV board has EYOYO 5 Inch Touchscreen  **

Pajoniiir hosts a Rekordbox-exported USB library and a USB DJ controller,
renders the touchscreen UI, and runs both playback decks, DSP mixer,
controller feedback, web control and signed OTA updates on the P4.

## Shared P4 beta

The shared-core development line is distributed as
[`shared-p4-beta.1`](https://github.com/dvucinozd/Pajoniiir/releases/tag/shared-p4-beta.1),
a **GitHub prerelease for contributor testing**. It includes separate signed
packages for each board, compiled controller profiles, source/configuration
provenance and SHA-256 inventories. Select the package by project identity;
the board images are not interchangeable.

The published **M2.5 JC4880/DDJ-FLX4 stable release and public OTA channels
remain unchanged**. Beta assets are downloaded from GitHub and installed
manually. The M3 beta uses its own `M3-dev-g<source SHA>` version and has no
automatic pull-update discovery. This beta does not claim full production
acceptance on every board or controller.

| Target | Entrypoint / signed project | Ordinary product policy | Physical evidence |
| --- | --- | --- | --- |
| JC4880P443C_I_W | `firmware/main-deck-p4` / `main-deck-p4` | 800 x 480, PCM5102A MAIN, controller USB1 / storage USB0 | Named Hercules audio/control/MT/loop/seek/CUE/PFL blocks accepted on `e22ff334`; long soak explicitly waived |
| JC1060P470C_I_W_Y | `firmware/main-deck-jc1060` / `main-deck-jc1060` | 1024 x 600, controller USB MAIN/cue, Ethernet provider, controller USB0 / storage USB1 | NOT RUN; contributor Kayrosen will test later |
| JC-ESP32P4-M3-DEV | `firmware/main-deck-m3` / `main-deck-m3` | DSI506/FT5426 800 x 480, fixed-48-kHz PCM5102A MAIN, Wi-Fi ON, controller USB2 FS / storage USB3 HS | Historical exact-image waveform/controller/soak acceptance; later images have their own recorded scope |

All three entrypoints link the same production audio, deck, Rekordbox,
controller, UI, web and OTA modules in `firmware/p4-core`. Peripheral providers
and product policies live under `firmware/boards`; independent protocols remain
in `firmware/common`. Dependency versions stay pinned to ESP-IDF **6.0.2**.

The shared core includes:

- PDB/ANLZ metadata corrections, playlist/folder catalogs, artwork workers,
  color waveforms, memory cues/loops and persistent media identities.
- Source/local Hot Cue merging with slot overrides, deletion tombstones and
  restore-source behavior. Legacy track-ID-only cues require explicit migration.
- Shared seek/preroll/loop handling, CUE/PLAY semantics and default-ON LOAD LOCK
  across MIDI, touch and web, including asynchronous load admission.
- M3 FIR/search-cache and consumer-owned PCM timeline fixes, plus board-specific
  waveform scheduling without a second playback implementation.
- S3CP v2/v3/v4 controller profiles, connection epochs, held-state reconciliation,
  LED feedback and descriptor-driven audio admission using the selected USB root.
- Board/project/source identity checks, signed OTA startup health/rollback,
  bounded journals and crash diagnostics.

FLX4 remains the ordinary reference controller. The
[Hercules Inpulse 500](controllers/hercules_djcontrol_inpulse_500/README.md) and
[Traktor Kontrol S2 MK3](controllers/ni_traktor_s2_mk3_full/README.md) profiles
document their supported controls and individual limits. M3 Hercules/S2 testing
uses an **explicit experimental port swap: controller USB3, storage USB2**;
these ordinary beta M3 assets retain the FLX4 port policy. S2 work is closed by
operator decision with remaining checks waived. A historical hour soak qualifies
only its exact image and configuration; it does not qualify the new beta assets.

Local three-target builds, board/component/lock/image-budget checks and the full
required-fixture host qualification passed on `722bb976`. Product UI simulator
results and unchanged-source equivalence are recorded in the
[final software matrix](docs/validation/SHARED_P4_FINAL_BUILD_MATRIX_20261011.md).
Hosted CI and wider real-media/controller/physical coverage retain their explicit
dispositions. Output-late observations remain monitoring findings, not zero-late
claims. Recorder, alternate DJ UI and M3 Ethernet/Pro DJ Link remain experimental
or disabled in ordinary builds.

Use the [final handoff](docs/reviews/2026-10-11-shared-core-final-handoff.md),
[integration ledger](docs/SHARED_P4_CORE_INTEGRATION.md),
[board policies](docs/SHARED_P4_BOARDS.md),
[JC1060 contributor handoff](docs/JC1060_BETA_HANDOFF_20261011.md) and
[beta delivery record](docs/validation/SHARED_P4_BETA_1_20261011.md) for exact
versions, installation boundaries and remaining gates.

[![Latest release](https://img.shields.io/github/v/release/dvucinozd/Pajoniiir)](https://github.com/dvucinozd/Pajoniiir/releases/latest)
[![Platform](https://img.shields.io/badge/platform-ESP32--P4-E7352C?logo=espressif&logoColor=white)](https://www.espressif.com/en/products/socs/esp32-p4)
[![License: MIT](https://img.shields.io/github/license/dvucinozd/Pajoniiir)](LICENSE)
![CodeRabbit Pull Request Reviews](https://img.shields.io/coderabbit/prs/github/dvucinozd/Pajoniiir?utm_source=oss&utm_medium=github&utm_campaign=dvucinozd%2FPajoniiir&labelColor=171717&color=FF570A&link=https%3A%2F%2Fcoderabbit.ai&label=CodeRabbit+Reviews)

![Pajoniiir running the dual-deck Overview screen](docs/images/Pajoniiir1.jpg)

Pajoniiir plays a Rekordbox-exported USB library without a laptop. The
ESP32-P4 hosts both the USB drive and selected controller, renders the touchscreen UI,
runs the two playback decks and mixer, and sends MAIN and headphone-cue audio.

Read the **[online user manual](https://docs.pajoniiir.eu/)** for setup,
controls and update instructions.

- No PC is required during a performance.
- MP3, WAV and FLAC playback is supported.
- Two independent decks provide waveforms, tempo control, Master Tempo,
  Hot Cues, loops, Beat Jump, Sync, Pad FX and Beat FX.
- MAIN audio is available on the PCM5102A RCA output.
- Headphone cue is returned through the selected supported USB controller.
- Signed local and remote OTA updates are supported.

## Release and support

The current release for **JC4880 + DDJ-FLX4** is
[`M2.5`](https://github.com/dvucinozd/Pajoniiir/releases/tag/M2.5),
also available through the [public OTA channel](https://ota.pajoniiir.eu/latest.json).
It retains the previous UI design and includes the integrated library and
transport improvements.

JC1060, DDJ-400 and Ethernet Pro DJ Link are implemented as development
configurations. Their physical qualification is still pending. Recording and
SD storage experiments remain disabled in regular builds.

The [release record](docs/validation/M2_5_RELEASE_20261006.md) documents
the tested configuration, explicit final-image soak waiver and remaining
qualification limits.

M2.5 adds a built-in Pajoniiir logo for tracks without artwork in Overview
and Library. The final image passed short OTA/startup/audio checks; the operator
waived a new 180-minute soak because audio code is unchanged. Historical v91
evidence remains tied to its original image.

## Improvements and collaboration

Thanks to collaborator **[kayrozen](https://github.com/kayrozen)** for his
work on the [Pajoniiir fork](https://github.com/kayrozen/Pajoniiir). Modules,
fixes and ideas from his work were adapted to this project's shared P4 core:

- **Track metadata and browsing:** Rekordbox title/downbeat corrections,
  hierarchical playlists, artwork and color waveform previews.
- **Deck transport:** cue preroll, more accurate seek/duration handling,
  loop resizing, memory cues and load-lock behavior.
- **Controller and board development:** the DDJ-400 profile and JC1060
  display/touch/Ethernet configuration.
- **Pro DJ Link development:** protocol codec, discovery, peer browsing,
  metadata access and network-sync logic.
- **Optional experiments:** the `dj_ui` presentation and SDMMC write workaround;
  the production UI retains the previous design.

These contributions were integrated with the existing persistent track
identity, audio timeline, worker ownership and signed OTA paths.
See the [integration notes and source attribution](docs/FORK_IMPROVEMENTS.md)
for module provenance and the detailed implementation history.

## Hardware

The parts, photographs, power precautions and wiring instructions below describe
the JC4880 reference build. For JC1060 and M3, use the board policy table above
and their BSP/wiring documentation; connector numbers differ between boards.

| Part | Purpose |
| --- | --- |
| Guition `JC4880P443C_I_W` | ESP32-P4 board, 4.3-inch touchscreen and main processor |
| Pioneer DDJ-FLX4 | MIDI control surface and four-channel USB audio device |
| Rekordbox USB drive | Music library connected to USB0 |
| PCM5102A DAC | Stereo MAIN output over RCA |
| Regulated 5 V / 3 A or better supply | Common system supply with separately protected USB outputs |
| Pajoniiir enclosure | Printable model and reference renders in [`misc/`](misc/) |

### Parts used

<table>
  <tr>
    <td width="50%" align="center"><img src="docs/images/parts/Part-JC4880P443C-I-W-Y.jpg" alt="Guition JC4880P443C_I_W ESP32-P4 touchscreen development board"></td>
    <td width="50%" align="center"><img src="docs/images/parts/Part-PCM5102A_side.jpg" alt="PCM5102A stereo RCA DAC module"></td>
  </tr>
  <tr>
    <td align="center">
      <sub>Guition JC4880P443C_I_W touchscreen board</sub><br>
      <a href="https://s.click.aliexpress.com/e/_c31cHQKD"><sub>[AliExpress]</sub></a>
    </td>
    <td align="center">
      <sub>PCM5102A stereo RCA DAC</sub><br>
      <a href="https://s.click.aliexpress.com/e/_c3rru3vP"><sub>[AliExpress]</sub></a>
    </td>
  </tr>
  <tr>
    <td align="center"><img src="docs/images/parts/Part-USB-A.png" alt="Panel-mount USB-A socket"></td>
    <td align="center"><img src="docs/images/parts/Part-Board2USB-A.jpg" alt="Board-to-panel USB cable"></td>
  </tr>
  <tr>
    <td align="center">
      <sub>Panel-mount USB-A socket for the Rekordbox drive</sub><br>
      <a href="https://s.click.aliexpress.com/e/_c3xWR5DP"><sub>[AliExpress]</sub></a>
    </td>
    <td align="center">
      <sub>Board-to-panel USB cable</sub><br>
      <a href="https://s.click.aliexpress.com/e/_c3jC7GFX"><sub>[AliExpress]</sub></a>
    </td>
  </tr>
  <tr>
    <td align="center"><img src="docs/images/parts/Part-usb-c.jpg" alt="Panel-mount USB-C OTG cable"></td>
    <td align="center"><img src="docs/images/parts/Part-power_usb_c.jpg" alt="Panel-mount USB-C power input"></td>
  </tr>
  <tr>
    <td align="center">
      <sub>Panel-mount USB-C OTG lead for the DDJ-FLX4</sub><br>
      <a href="https://s.click.aliexpress.com/e/_c3fyaSjJ"><sub>[AliExpress]</sub></a>
    </td>
    <td align="center">
      <sub>Panel-mount USB-C power input</sub><br>
      <a href="https://s.click.aliexpress.com/e/_c4a9xDsN"><sub>[AliExpress]</sub></a>
    </td>
  </tr>
</table>

These are reference photos of the parts used in the build. Seller revisions
and cable pinouts can vary; verify every part electrically and follow the
wiring documentation rather than relying on product photos.

> [!CAUTION]
> USB0 and USB1 must receive safe, current-limited 5 V power. Isolate the
> native P4-side VBUS conductors before injecting protected downstream VBUS.
> Never combine independent supplies with a passive Y-cable. Read the complete
> [hardware wiring and electrical acceptance guide](docs/HARDWARE_WIRING.md)
> before building or changing the power wiring.

## Connect the system

### JC4880 reference build

![Pajoniiir M2 wiring diagram](docs/images/wiring_M2.jpg)

1. With power off, connect the regulated system supply and common ground as
   shown above.
2. Insert the Rekordbox USB drive into **USB0**.
3. Connect the DDJ-FLX4 to **USB1** using the USB-C OTG connection.
4. Connect the PCM5102A RCA output to the MAIN amplifier or powered speakers.
5. Connect headphones to the DDJ-FLX4 if cue monitoring is required.
6. Power on the system and wait for the Library to appear.
7. Browse a track, load it to D1 or D2, and press Play on the controller.

Both USB insertion orders and reconnect recovery have been physically tested.
For connector details and electrical limits, use
[`docs/HARDWARE_WIRING.md`](docs/HARDWARE_WIRING.md) as the source of truth.

### Other boards

For ordinary M3, USB1 supplies power/debug, FLX4 connects to USB2 FS and the
Rekordbox drive to USB3 HS. PCM5102A BCK/LCK/DIN use GPIO1/2/3, with SCK grounded
and shared ground. Preserve the accepted DSI506/FT5426 settings. The first
transition from historical M3 `main-deck-p4` requires the
[preserving wired migration](docs/M3_SHARED_CORE_MIGRATION.md), not cross-board OTA.

JC1060 uses USB0 for the controller and USB1 for storage, controller USB audio
for MAIN/cue, and its own display/touch/Ethernet provider. Do not enable the
JC4880/M3 PCM5102A pins on JC1060; they conflict with Ethernet. Follow the
[contributor handoff](docs/JC1060_BETA_HANDOFF_20261011.md) before installation.

## Touchscreen and controller workflow

- **Overview** shows both deck waveforms, transport state, time, BPM, pitch,
  Master Tempo and the active Beat FX.
- **Library** browses the Rekordbox USB collection and loads tracks to either
  deck.
- **Hot Cues** displays and manages performance cue points.
- **Settings** controls device options, diagnostics and Wi-Fi Remote.
- The DDJ-FLX4 controls transport, jog/vinyl, tempo, mixer/EQ, cue, loops,
  Beat Jump, Sync, pads and effects, with LED state restored after reconnect.

The screensaver consumes the first controller action to wake the display;
the second PLAY starts playback. Touch wake does not activate the control below
the screensaver. LOAD LOCK refuses replacing a playing deck, including shifted
LOAD; stop the deck before loading another track.

## Wi-Fi Remote and updates

These Wi-Fi instructions apply to JC4880 and M3. JC1060 uses its Ethernet
provider and has no Wi-Fi capability. Ordinary M3 starts with Wi-Fi enabled;
its AP name is `Pajoniiir-M3` (use the name shown in Settings).

1. Enable **Wi-Fi Remote** on the Settings screen.
2. Connect a phone or computer to the `Pajoniiir` Wi-Fi network. The default
   WPA password is `Pajoniiir`.
3. Open [`http://192.168.4.1`](http://192.168.4.1) or
   [`http://pajoniiir.local`](http://pajoniiir.local).

For a JC4880 local update, upload only the signed `main-deck-p4.ddjota` package. Keep
power stable, stop playback and wait for the device to reboot completely.
Remote stable releases are published through `https://ota.pajoniiir.eu`.
JC1060 and M3 require `main-deck-jc1060.ddjota` and `main-deck-m3.ddjota`
respectively. The beta does not update any public pull channel. Never upload a
raw BIN through the signed OTA endpoint or select a different board's package.

See the [OTA update procedure](docs/OTA-UPDATE.md) for installation,
verification, rollback and wired-recovery instructions.

## Gallery

<table>
  <tr>
    <td width="50%"><img src="docs/images/Pajoniiir2.jpg" alt="Pajoniiir side view with touchscreen and USB cable"></td>
    <td width="50%"><img src="docs/images/Pajoniiir3.jpg" alt="Pajoniiir dual-deck Overview screen"></td>
  </tr>
  <tr>
    <td align="center"><sub>Compact standalone player and touchscreen</sub></td>
    <td align="center"><sub>Dual-deck Overview during playback</sub></td>
  </tr>
  <tr>
    <td colspan="2"><img src="docs/images/Pajoniiir4.jpg" alt="Rear of the Pajoniiir enclosure with USB drive, RCA outputs and service connector"></td>
  </tr>
  <tr>
    <td colspan="2" align="center"><sub>Enclosure connections and ventilation</sub></td>
  </tr>
</table>

## DEMO Video

[![Watch the video](https://img.youtube.com/vi/3RuC3cYCGyE/hqdefault.jpg)](https://youtu.be/3RuC3cYCGyE)


## 3D-print enclosure

The printable enclosure and assembly references are stored in
[`misc/`](misc/):

- [`PajoniiirCASE.stl`](misc/PajoniiirCASE.stl) — binary STL ready for slicer
  preparation;
- [`case_anim.gif`](misc/case_anim.gif) — exploded assembly animation;
- `case_assembe.png`, `case_assembe2.png` and `case1.jpg` — assembly and case
  reference views.

![Pajoniiir enclosure exploded view](misc/case_anim.gif)

Confirm dimensions, orientation, supports and material settings in your slicer
before printing.

## Build from source

ESP-IDF **v6.0.2** is required.

```powershell
. C:\Espressif\tools\Microsoft.v6.0.2.PowerShell_profile.ps1
idf.py --version  # must report ESP-IDF v6.0.2

$repoRoot = git rev-parse --show-toplevel
Set-Location "$repoRoot\firmware\main-deck-p4"
idf.py build
```

Select `firmware/main-deck-jc1060` or `firmware/main-deck-m3` instead for the
other products. Use a fresh build directory and SDKCONFIG per target/experiment;
do not reuse a role-swap configuration for an ordinary M3 release.

Run the same P4 host regression suite used by CI:

```powershell
Set-Location $repoRoot
$env:Path = "$env:Path;C:\msys64\ucrt64\bin"
.\tests\run_p4_host_tests.ps1 -Qualification
```

Run the headless UI screenshot and navigation gate:

```powershell
.\tests\ui_simulator\run_ui_simulator_e2e.ps1
```

Generated build directories, local `sdkconfig` files, signing keys and release
packages are intentionally excluded from Git. The reproducible dependency locks
for all three product entrypoints are committed; builds must not change them.

## Project status and documentation

The stable JC4880/FLX4 M2.5 release remains available. The shared P4 beta is a
separate contributor prerelease, with JC1060 physical checks deferred and JC4880
long soak explicitly skipped by the operator. DDJ-400/Pro DJ Link and broader
controller/real-media qualification retain their development limits. Current
status and acceptance limits are maintained in the
[final handoff](docs/reviews/2026-10-11-shared-core-final-handoff.md),
[documentation status](docs/DOCUMENTATION_STATUS.md) and
[development plan](docs/DEVELOPMENT_PLAN.md).

- [Online user manual](https://docs.pajoniiir.eu/)
- [Project overview](docs/PROJECT_OVERVIEW.md)
- [Architecture](docs/ARCHITECTURE.md)
- [Hardware wiring](docs/HARDWARE_WIRING.md)
- [Startup and release checklist](docs/STARTUP_CHECKLIST.md)
- [FLX4 MIDI map](docs/DDJ_FLX4_MIDI_MAP.md)
- [OTA update procedure](docs/OTA-UPDATE.md)
- [Security and provisioning policy](docs/SECURITY_PROVISIONING_POLICY.md)
- [Development plan](docs/DEVELOPMENT_PLAN.md)
- [Risk register](docs/RISK_REGISTER.md)

The JC4880, JC1060 and M3 entrypoints are `firmware/main-deck-p4`,
`firmware/main-deck-jc1060` and `firmware/main-deck-m3`. All use one shared
P4 core. Historical S3 and dual-processor material remains in Git history and
is not part of the active product.

## License

Pajoniiir is available under the [MIT License](LICENSE).
