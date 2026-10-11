<p align="center">
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="docs/images/pajoniiir-logo-white.svg">
    <source media="(prefers-color-scheme: light)" srcset="docs/images/pajoniiir-logo-black.svg">
    <img alt="Pajoniiir" src="docs/images/pajoniiir-logo.svg" width="450">
  </picture>
</p>

# Pajoniiir

Shared-core development: portable P4 firmware lives in `firmware/p4-core`,
with hardware providers under `firmware/boards`. JC4880, JC1060 and M3 use linked
common startup; the M3 entrypoint is `firmware/main-deck-m3`.
See the [integration ledger](docs/SHARED_P4_CORE_INTEGRATION.md). This work does
not change the published M2.5 release or its accepted hardware scope.

The [Hercules Inpulse 500 profile](controllers/hercules_djcontrol_inpulse_500/README.md)
has an accepted exact-image M3 60-minute dual mixed-rate MT/loop playback scope,
with clean MAIN/headphones and fluid waveforms. Its supported functions and
deliberate pad/FX limits are documented separately from individual physical
coverage. The [current shared-core review](docs/reviews/2026-10-11-shared-core-hercules-closeout.md)
records final local software/build work, real-media and other-board gates;
this development branch is not a new production release.

[Local ordinary-target qualification](docs/validation/SHARED_P4_LOCAL_QUALIFICATION_20261011.md)
passes on clean `5ff76b6c`: complete host gate, nine simulator configurations,
both keylock policies and JC4880/JC1060/M3 ESP-IDF 6.0.2 builds with unchanged
locks and verified identities/budgets. Hardware and publication gates remain
separate; the installed Hercules bench image is unchanged.

The [delivery-readiness ledger](docs/reviews/2026-10-11-shared-core-delivery-readiness.md)
records all P01–P10 dispositions, remaining acceptance gates and migration
boundaries. Three frozen-source ordinary signed candidates are retained locally;
a [review description](docs/reviews/2026-10-11-shared-core-pr-description.md)
is prepared. No new image is installed or published by this handoff.

The [S2 MK3 performance candidate](controllers/ni_traktor_s2_mk3_full/README.md)
adds source-backed mixer/pitch/jog/LED mappings and experimental descriptor-driven
HS UAC2. Its M3 experiment uses S2 on USB3 and storage on USB2. S2 work is closed
by operator decision: exact-image full-hour playback at 48/44.1 kHz is accepted,
with remaining physical checks explicitly waived rather than passed. This does
not replace FLX4 production defaults or establish unrestricted S2 production
acceptance. Normal-power results, earlier 2-dBm diagnostics and remaining
general memory/OTA-restart work are separated in the
[bench evidence](docs/validation/S2_MK3_PROFILE_PREPARATION_20261007.md).

The M3 shared-core candidate uses `main-deck-m3`, a separate M3 version line and
`/m3` OTA channel. Its initial wired migration preserves the existing partition
layout and settings. Build verification does not qualify the new image on
hardware. See [board policies](docs/SHARED_P4_BOARDS.md),
[M3 migration](docs/M3_SHARED_CORE_MIGRATION.md) and the
[reliability campaign](docs/RELIABILITY_MONITORING_PLAN.md).

The [current M3 candidate record](docs/validation/M3_SHARED_WAVEFORM_REGRESSION_20261006.md)
identifies the installed development candidate and its signed OTA health result.
Audio/USB lifecycle campaigns and full hardware qualification remain open.
The initial dual-deck timing attempt exposed a
[waveform regression](docs/validation/M3_SHARED_WAVEFORM_REGRESSION_20261006.md);
the signed development image has no production hardware PASS.
Both paired-buffer bench revisions failed the physical test. The historical
`b0c66744` image had operator-confirmed severe stutter when both decks played.
The `1d3ee7b6` revision distinguishes
retained position observations from fresh ones, with RAM age counters to check
the mechanism on the device. The operator reports much improved visuals with
slight residual stutter and clean sound; full fluidity remains open. The next
M3 revision `bf2fdc47` removed repeated UI work on unchanged covers/hidden controls;
its repeated physical test still showed slight stutter with clean sound.
The `fb5cbca1` qualification image was installed VALID in ota_0 and removes a blocking
audio-position read from jog-mode polling. Its short dual MT/zoom-8 retest passed
with operator-confirmed fluid waveforms and clean MAIN/headphones sound.
A subsequent short operator review confirmed all five zooms fluid with clean
sound. Local JC4880/JC1060 builds then passed on the same clean source without
rebuilding M3. Its exact-image 60-minute Campaign B soak passed telemetry and
operator visual/listening acceptance on 2026-10-07, with zero new PCM/UAC/service-log
losses and 11 output-late events retained for monitoring. Physical Campaign A
was excluded by the operator; an [automatic software-reboot supplement](docs/validation/M3_AUTOMATIC_RECOVERY_20261007.md)
observed ten recoveries with nine complete timing windows and retained host
visibility flags. [M3 negative OTA and injected startup rollback tests](docs/validation/M3_OTA_NEGATIVE_ROLLBACK_20261007.md)
then passed, restoring the same accepted image. Current-source GitHub Actions
was skipped by the operator because of quota; it has no CI PASS.
This is not a production release or a physical Campaign A PASS.

[Catalog/cue software and web LOAD LOCK checks](docs/validation/M3_LIBRARY_CUES_WEB_LOAD_LOCK_20261007.md)
subsequently passed on the unchanged M3 image: 324 stable identities across a
reboot and six refused playing-deck replacements. At that checkpoint, on-device Hot Cue mutation,
physical MIDI/touch LOAD LOCK and independent export/UI review remain open.

The subsequent [read-only real-export audit](docs/validation/M3_REAL_EXPORT_AUDIT_20261007.md)
found three reproducible import defects: 13 artist names, 12 album names and
one EXT Hot Cue D are lost. The installed candidate is unchanged. All 324
analysis pairs parse and 282 referenced JPEGs decode; this export contains no
playlists or source loops, so their real-media gates remain open.
Correction source `082d2c0a` now passes the same real-export comparisons, shared
host tests, M3 build and signed packaging. Its candidate is now installed VALID
in ota_1; the live 324-track library recovers all 13 artist names. The operator
confirmed D1 A–D recall, with the API observing source D at 123347 ms. Cue
[set/delete/restore persistence](docs/validation/M3_CUE_LIFECYCLE_20261007.md)
now passes on this image: local D survived reboot, its deletion stayed effective,
and HOLD RESTORE returned source D at 123347 ms after another reboot.
Broader new-image physical acceptance remains open.
Subsequent [physical LOAD LOCK checks](docs/validation/M3_PHYSICAL_LOAD_LOCK_20261007.md)
passed ordinary FLX4 and touch LOAD refusal on each playing deck, with unchanged
tracks, operator-confirmed uninterrupted sound and zero strict counter deltas.
Selected [stopped-deck routing checks](docs/validation/M3_PHYSICAL_LOAD_ROUTING_20261007.md)
also passed: FLX4 LOAD D1, touch LOAD D2 and FLX4 Shift+Load D1/D2.
The remaining ordinary stopped LOAD combinations then passed, completing
FLX4/touch on both decks. Repeated [playing Shift+Load checks](docs/validation/M3_PHYSICAL_SHIFT_LOAD_LOCK_20261007.md)
also passed on both decks, with Comanchero selected and Combat Ready retained.
The controller wake candidate `4620b83c` was installed VALID in ota_0
to repair [controller screensaver wake](docs/validation/M3_CONTROLLER_SCREENSAVER_WAKE_20261007.md).
Local USB semantics now enter the common wake policy; full default host tests
and the M3 build/signed package passed. Physical wake retest failed because the
first PLAY started playback. The current M3 bench is now clean `af4ca590`, VALID
in ota_1, with an atomic UI activity handoff and read-only wake counters.
Default host tests and M3 build/signed packaging pass. The operator confirmed
first PLAY wakes only and subsequent PLAY works; the API confirms one consumed
wake. Browse and touch wake regressions also pass on this exact image, with
both decks finally stopped. The final soak remains deferred.
Earlier cue/LOAD tests retain their 082d2c0a scope. The final soak is deferred.

**A standalone dual-deck DJ system for the Pioneer DDJ-FLX4, powered by a
single ESP32-P4 board.**

[![Latest release](https://img.shields.io/github/v/release/dvucinozd/Pajoniiir)](https://github.com/dvucinozd/Pajoniiir/releases/latest)
[![Platform](https://img.shields.io/badge/platform-ESP32--P4-E7352C?logo=espressif&logoColor=white)](https://www.espressif.com/en/products/socs/esp32-p4)
[![License: MIT](https://img.shields.io/github/license/dvucinozd/Pajoniiir)](LICENSE)
![CodeRabbit Pull Request Reviews](https://img.shields.io/coderabbit/prs/github/dvucinozd/Pajoniiir?utm_source=oss&utm_medium=github&utm_campaign=dvucinozd%2FPajoniiir&labelColor=171717&color=FF570A&link=https%3A%2F%2Fcoderabbit.ai&label=CodeRabbit+Reviews)

![Pajoniiir running the dual-deck Overview screen](docs/images/Pajoniiir1.jpg)

Pajoniiir plays a Rekordbox-exported USB library without a laptop. The
ESP32-P4 hosts both the USB drive and DDJ-FLX4, renders the touchscreen UI,
runs the two playback decks and mixer, and sends MAIN and headphone-cue audio.

Read the **[online user manual](https://docs.pajoniiir.eu/)** for setup,
controls and update instructions.

- No PC is required during a performance.
- MP3, WAV and FLAC playback is supported.
- Two independent decks provide waveforms, tempo control, Master Tempo,
  Hot Cues, loops, Beat Jump, Sync, Pad FX and Beat FX.
- MAIN audio is available on the PCM5102A RCA output.
- Headphone cue is returned through the DDJ-FLX4.
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

## Touchscreen and controller workflow

- **Overview** shows both deck waveforms, transport state, time, BPM, pitch,
  Master Tempo and the active Beat FX.
- **Library** browses the Rekordbox USB collection and loads tracks to either
  deck.
- **Hot Cues** displays and manages performance cue points.
- **Settings** controls device options, diagnostics and Wi-Fi Remote.
- The DDJ-FLX4 controls transport, jog/vinyl, tempo, mixer/EQ, cue, loops,
  Beat Jump, Sync, pads and effects, with LED state restored after reconnect.

## Wi-Fi Remote and updates

1. Enable **Wi-Fi Remote** on the Settings screen.
2. Connect a phone or computer to the `Pajoniiir` Wi-Fi network. The default
   WPA password is `Pajoniiir`.
3. Open [`http://192.168.4.1`](http://192.168.4.1) or
   [`http://pajoniiir.local`](http://pajoniiir.local).

For a local update, upload only the signed `main-deck-p4.ddjota` package. Keep
power stable, stop playback and wait for the device to reboot completely.
Remote releases are published through `https://ota.pajoniiir.eu`.

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

Run the same P4 host regression suite used by CI:

```powershell
Set-Location $repoRoot
$env:Path = "$env:Path;C:\msys64\ucrt64\bin"
.\tests\run_p4_host_tests.ps1
```

Run the headless UI screenshot and navigation gate:

```powershell
.\tests\ui_simulator\run_ui_simulator_e2e.ps1
```

Generated build directories, local `sdkconfig` files, signing keys and release
packages are intentionally excluded from Git. The reproducible dependency lock
at `firmware/main-deck-p4/dependencies.lock` is committed.

## Project status and documentation

The JC4880/FLX4 release is published and merged into `master`.
JC1060/DDJ-400/Pro DJ Link hardware acceptance and broader controller
qualification remain development work. Current status and acceptance limits
are maintained in the [release record](docs/validation/JC4880_V91_RELEASE_20261005.md),
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

The production JC4880 entrypoint is `firmware/main-deck-p4`; the
development JC1060 entrypoint is `firmware/main-deck-jc1060`. Both use a shared
P4 core. Historical S3 and dual-processor material remains in Git history and
is not part of the active product.

## License

Pajoniiir is available under the [MIT License](LICENSE).
