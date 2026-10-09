# Documentation status

Status: **current documentation authority, reconciled 2026-10-06**.

The [Hercules preparation record](validation/HERCULES_INPULSE_500_PREPARATION_20261010.md)
documents PC VID/PID and jog-touch capture, exact fixture regeneration and
profile/registry host checks. Native output-control policy, filter gate and
descriptor-driven audio startup require resolution before M3 activation.
Hercules M3 audio/MIDI/LED/lifecycle acceptance remains NOT RUN. This offline
work does not change the concurrent S2 exact-image soak.

The 2026-10-08 [S2 performance candidate](../controllers/ni_traktor_s2_mk3_full/README.md)
documents source-backed mixer/pitch/jog/LED coverage, raw-10 inversion and
experimental HS UAC2. The [S2 evidence record](validation/S2_MK3_PROFILE_PREPARATION_20261007.md)
separates prior basic-profile acceptance from the new combined physical checks.
On clean installed `0729221a`, stopped-deck reconnect and Wi-Fi OFF/ON,
60-second dual MT/loop audio/visual playback and a 20-second active jog/Browse
supplement passed with operator confirmation and zero new strict audio/USB
counter deltas. These checks used the diagnostic 2-dBm Wi-Fi limit and retain
that image/configuration scope.

The prior clean candidate `ac864bdc` was installed VALID in `ota_0`; boot 401
records the normal 20-dBm Wi-Fi maximum. Its full host/M3 build and signed
package checks pass. Three normal-power loaded-track stopped reconnects,
60-second MT/loop audio/visual playback and a 30-second active jog/Browse
supplement passed with operator confirmation and zero new strict playback
counter deltas. Normal-power stopped-deck Wi-Fi OFF/ON also passed on boot 401
with preserved MIDI/audio readiness and repeated 20-dBm readback. Full
controller/reliability coverage and the final exact-image soak remain open.
The attempted 3600-second mixed-rate/MT/loop S2 soak is **FAIL**: one ISO
packet / six frames were lost at elapsed 306.865 seconds (23:10:59 local).
The operator noticed no audio, display or controller disturbance; the strict
loss criterion still fails. Both decks were safely stopped at the checkpoint;
the last recorded elapsed time was 1056.550 seconds. The observer had recorded
the failure without exiting its loop, so no native final result exists.
Preserved evidence and the checkpoint annotation are linked in the S2 record.
The host monitor now exits TimingSoak immediately on a hard failure and records
actual elapsed time. The heartbeat is paused. One isolated 11297-us output-late
event remains a separate monitoring finding. ISO root cause and exact-image
requalification remain open; no replacement soak was started at that checkpoint.

The subsequent ISO-rearm investigation confirms a same-microframe hardware
activation preceding the skipped packet. Mitigation candidate `00a1ca90`
reduces diagnostic task-context cache chunks to 1 KiB. Full host/M3 build,
board identity and signed package checks pass; it is installed VALID in
`ota_1`. The 60-second mixed-rate MT/loop retest passed with operator-confirmed
clean MAIN/S2 headphones and fluid waveforms, zero strict deltas and zero new
output-late events. The separate 600-second same-workload test failed after
514.981 seconds on two skipped ISO packets / 11 lost frames. Both decks were
automatically stopped; the heartbeat is paused. The 1-KiB bound did not
eliminate the fault. The final 60-minute soak remains NOT RUN.
The failed `ac864bdc` soak is retained; it does not qualify this candidate.

The subsequent `17f75286` candidate raises only M3 HS-diagnostic USB IRQ
priority and improves bounded packet/activation context. Full host/M3 build,
identity and signed packaging pass; it is installed VALID in `ota_0`.
Boot 403 reports PANIC after the old image's `ota_reboot` cache writeback path;
MIDI is present but the intended audio quarantine prevents preparation.
Operator reset restored boot 404 POWERON, healthy S2 44100-Hz streaming and
normal Wi-Fi. After operator MT/zoom/readiness confirmation, the 600-second
IRQ retest started around 00:14:36 Europe/Zagreb on 2026-10-10 and failed after
90.586 seconds on two skipped ISO packets / 11 lost frames. Cleanup stopped
both decks and the heartbeat is paused. Packets 1/2 of the same 56-packet block
were skipped without a captured late activation. The next candidate isolates
ISO descriptor lists in a fixed 2-KiB internal pool. Clean candidate `3f3605a7`
passes full host/M3 build, identity and signed packaging and is installed VALID
in `ota_1`. Boot 405 is a normal SW reset, with healthy MIDI/audio and Wi-Fi.
The descriptor pool is confirmed in internal BSS. Tracks are prepared/stopped;
operator reconnect/MT/zoom setup and physical retest are pending.
No playback/IRQ physical PASS is asserted;
the restart failure and sustained ISO-loss gate remain open.

## Shared-core migration authority

Canonical source is `dvucinozd/Pajoniiir`, branch `codex/shared-p4-core-m3`.
[Integration/provenance](SHARED_P4_CORE_INTEGRATION.md),
[board/capability policies](SHARED_P4_BOARDS.md),
[preserving wired migration](M3_SHARED_CORE_MIGRATION.md),
[explicit cue migration](M3_CUE_MIGRATION.md) and
[exact-image monitoring](RELIABILITY_MONITORING_PLAN.md) govern this candidate.
Earlier JC and M3 release/acceptance records retain their original image scope.
Build, package, installed-device, operator acceptance and publication evidence
must be recorded separately. Current shared-M3 fb5cbca1 has exact-image
waveform/60-minute Campaign B acceptance and completed
[negative OTA/injected startup rollback gates](validation/M3_OTA_NEGATIVE_ROLLBACK_20261007.md).
Physical Campaign A and current-source GitHub Actions are SKIPPED BY OPERATOR
(Actions quota); remaining hardware/integration gates remain open.
The M3 repository remains historical and has not been remotely archived.

## Published product

The current public release is **M2.5**, frozen at
`20f1c3f04a615209ae25e9bbdae649d0f5b44e8d`. It is accepted for
**JC4880 / DDJ-FLX4 / PCM5102A MAIN / FLX4 headphone cue**, with the previous
product design and regular configuration. Preview, recorder and SD experiments
are disabled. The [release record](validation/M2_5_RELEASE_20261006.md)
is authoritative for artifacts, physical acceptance and publication.

| Item | Frozen release value |
| --- | --- |
| Release / annotated tag | `M2.5` |
| Source | `20f1c3f04a615209ae25e9bbdae649d0f5b44e8d` |
| Project / toolchain | `main-deck-p4` / ESP-IDF v6.0.2 |
| Application | 2,576,432 bytes; SHA-256 `7aaab31d4a7c00990939cb9ccecb04aba1cba463b1db919d168477c7df3210ba` |
| Signed bundle | 2,576,620 bytes; SHA-256 `44db191c1785168781e9b2893cac6dda1b92a20a5342c328d7fbfdbbd8707e27` |
| Public channel | [latest.json](https://ota.pajoniiir.eu/latest.json) |
| GitHub release | [M2.5](https://github.com/dvucinozd/Pajoniiir/releases/tag/M2.5) |
| Last publication-session device observation | M2.5 / `ota_1`, boot 618; decks stopped, AP/API and USB/FLX4 available, startup ready, zero allocation failures |

A recorded slot/boot is session evidence, not a fresh observation of the device.
Documentation and later diagnostic commits on `master` do not alter the frozen
image. The integration branch was merged into `master` on 2026-10-05; it is
no longer a separate source of unreleased A-L work.

## Acceptance and limits

Final M2.5 passed signed OTA, startup readiness, USB/FLX4 availability and
62.622211 seconds of dual playback with zero new strict audio/USB faults and
operator-confirmed Overview/MAIN/D1/D2 cue. The operator explicitly waived
a new 180-minute M2.5 soak because audio/deck/DSP code is unchanged. This is
an image-specific exception; the following v91 evidence remains historical.

Focused exact-v91 scenarios passed with operator-confirmed sound:
MP3/WAV/FLAC (including 96 kHz/24-bit FLAC), MAIN/D1/D2 cue, scratch,
CUE/PLAY/Master Tempo, manual loop OUT/half/double/exit, combined scratch/MT,
local cue save/reload/reboot/delete, normal D1 LOAD lock, stopped USB storage
and FLX4 reconnect, and post-reconnect playback.

The saved soak intervals total **10,801.3664176 seconds (180m01.37s)**:
1,015.8899243 seconds followed, after interruption, by 9,785.4764933 seconds.
The completed continuation has zero strict audio/USB fault deltas, valid
memory/stack reserves and operator-confirmed clean sound. This is an explicit
operator-accepted **segmented-soak release exception**.
**An uninterrupted 180-minute v91 soak remains NOT RUN.** Neither historical
monitor failure is erased or converted into PASS.

Diagnostic-only images separately passed unconfirmed-image rollback and
startup-readiness rejection at **60.253 seconds**, automatically restoring v91.
The latter forces the readiness input false while real AP/API remain available;
it does not prove an induced radio failure. Diagnostic images are excluded
from ordinary release packaging.

These scenarios remain NOT RUN on the frozen release:

- other deck/format combinations and full advanced transport variants;
- shifted/touch LOAD-lock variants;
- source-cue tombstone reimport and physical cross-media cue isolation;
- exhaustive playlist/artwork/PWV4, touch and metadata comparison;
- active removal/cancellation and unexercised held-control reconnect variants;
- JC1060, DDJ-400, other controllers, real CDJ/rekordbox Link peers, output
  phase/latency calibration and experimental SD/recorder qualification.

Unrun scenarios remain outside the accepted scope; software coverage does not
turn them into physical passes. Full details and failed intermediate candidates
are retained in the [hardware ledger](validation/JC4880_L_CANDIDATE_20261005.md)
and [release review](validation/JC4880_V91_RELEASE_REVIEW_20261005.md).
Automatic recovery passed in the accepted configuration. The earlier bare
M2.4 power-cycle observation is historical, not a universal v91 requirement.

A [focused library smoke](validation/JC4880_LIBRARY_SMOKE_20261006.md) on
installed v91 additionally confirmed artwork and memory-cue visibility,
navigation of `untitled playlist`, catalog consistency, local load and rejection
of stale catalog requests. Exported order, cue positions, downbeat/PWV4 source
comparison and artwork isolation remain open; the development logo was not
installed during these checks.

## Maintained architecture and software scope

Released M2.5 additionally includes the user-supplied default Pajoniiir
artwork logo. Overview and track Library rows use pre-rendered read-only
thumbnails when real artwork is unavailable; empty product decks and folder
rows remain distinct. It is absent from the immutable v91 image. Before final
M2.5, a clean `M2.4-111-ge09e9ca4` candidate from `e09e9ca4` was
installed in `ota_0` through signed OTA. Startup, API, FLX4 and storage
are available, with zero allocation failures, and the operator confirms logo
visibility in Library and on loaded D1 (`Red For Love {320}`). A matched
stopped-track memory comparison has unchanged largest internal/DMA blocks
and only eight bytes less free memory, matching BSS growth. A subsequent
62.419-second dual-deck audio check passed with operator-confirmed MAIN/D1/D2
cue sound, zero strict fault deltas and zero allocation failures. Largest
internal/DMA blocks remain 31,744 bytes during playback. The empty-deck
startup largest-block variation is retained in the record; this focused check
does not constitute a new exact-image 180-minute soak. The operator also
confirms correct real-cover-to-logo replacement and two-page Library browsing
without stale artwork. Media replacement and delayed/corrupt cover variants
remain open. See the
[candidate record](validation/M2_5_DEFAULT_ARTWORK_20261006.md).

One authoritative P4 core serves two board entrypoints:

| Configuration | Identity | Status |
| --- | --- | --- |
| JC4880 / FLX4 / PCM5102A MAIN + USB cue | `firmware/main-deck-p4` / `main-deck-p4` | Published frozen M2.5; focused physical scope and explicit soak waiver above |
| JC1060 / DDJ-400 / USB MAIN + cue / Ethernet Link | `firmware/main-deck-jc1060` / `main-deck-jc1060` | Software verified; all new-board/peer physical gates NOT RUN |

Packages A-L are software verified. The [integration ledger](FORK_IMPROVEMENTS.md)
maps modules and dated evidence, including adaptations from collaborator
[kayrozen](https://github.com/kayrozen), donor
`428b97dd4a175f03d3a172c8db9c4d5ed94195fb`. Persistent media identity,
audio-session fencing, worker ownership and signed OTA remain common-core
contracts. S3CP v2/v3/v4 parsing, compilation, storage and upload validation are
integrated; public Web Profile Builder deployment is a separate project gate.

JC4880 keeps USB0 storage, USB1 FLX4 and Wi-Fi service; it has no Link provider.
JC1060 Link is default OFF and Ethernet-only, with bounded discovery/claim,
browse, verified SD downloads and network sync. Unidentified NFS volumes use
session-local identity and fresh downloads, so local edits cannot migrate
between those sessions. LOCKED is a model state; sink/rate latency is
UNMEASURED until physically calibrated and engineering calibration is RAM-only.
No local browsable Link server is advertised.

The P4 owns playback, deck position, DSP, controller binding, LEDs and UI.
There is no active S3 firmware, UART control link or inter-board PCM bridge.
Compatibility names `control_link`, `S3CP` and `profile.s3bin` do not imply S3.
APTA remains on its separate branch and is outside this release.

## Security and inherited evidence

Shared service credentials, unchanged enclosed wiring and uncaptured numeric
thermal/RF/strain margins are accepted limitations. Secure Boot, Flash
Encryption and security eFuses remain disabled. Offline encrypted primary
and backup signing-key copies were operator-confirmed; backup recovery signing
and successor-key overlap remain future maintenance. See
[security policy](SECURITY_PROVISIONING_POLICY.md) and [risk register](RISK_REGISTER.md).

Earlier releases and validation records preserve only their own image results.
Start from the [documentation index](README.md) for the M2.1-M2.4 history,
duplicate-track-ID candidate acceptance, power/VBUS and lifecycle records.
Do not transfer historical passes to new code or claim waived tests were run.

## Source-of-truth order

1. Exact release artifacts/configuration and current code/tests for their own SHA.
2. [Published M2.5 record](validation/M2_5_RELEASE_20261006.md) for accepted
   hardware and publication; this status for current scope.
3. Operational architecture, wiring, OTA, security, checklist and risk documents.
4. Dated validation and integration history for their named images.
5. Git history for retired implementations and superseded plans.

The [documentation audit](validation/DOCUMENTATION_RECONCILIATION_20261006.md)
records this reconciliation. A documentation-only update does not reflash,
requalify or republish firmware.

The 3f3605a7 internal ISO pool ten-minute test has telemetry and operator-confirmed playback PASS: 600.408 s, 983 samples, all 23 strict deltas zero, zero new output-late events, both decks stopped. MAIN/S2 headphones and dual waveforms were physically confirmed clean/fluid throughout. S2 ran at 48000 Hz; prior failed candidates ran at 44100 Hz, so this does not establish a sole root cause or qualify the 44100-Hz path. Controls were untouched (MIDI delta zero). Final 60-minute soak, active-control and remaining lifecycle gates remain open; checkpoint automation is paused.

Final exact-image 3f3605a7 S2 48-kHz soak is INCOMPLETE: host observer absent without result.json at the 01:13 checkpoint; records stopped at 1032.793 s (00:59 local). Last recorded and live audio loss/output-late deltas were zero. Both decks were safely stopped, evidence retained, and heartbeat paused. Cause of observer disappearance is unproven; this is not a completed 60-minute PASS. Earlier ten-minute PASS retains its scope.

The independent final 3f3605a7 S2 48-kHz soak is strict ISO FAIL at 1336.174 s / 2185 samples: two packets and 12 frames lost, other 20 strict deltas and output-late delta zero. Operator noticed no physical disturbance. Both decks stopped, final evidence saved, scheduled task cleaned up after exit and heartbeat paused. Internal descriptor placement did not eliminate the inner-block skip pattern; final soak gate remains open. Prior INCOMPLETE record and ten-minute PASS retain their scope.

Next S2 candidate fc4ed608 moves experimental M3 ISO payloads into internal DMA memory; full host suite, M3 IDF 6.0.2 build, board check and signed-package checks pass. Local OTA installed exact clean image VALID ota_0, but old-image OTA restart produced boot406 PANIC and HS audio quarantine. Operator RESET restored normal boot407 and healthy S2 44100-Hz streaming. The 60.477-s dual MT/loop retest has telemetry and operator playback PASS: all22 strict deltas and output-late delta zero, both decks stopped. After track load cached DMA free is13995 B (minimum13087 B), so lifecycle/headroom remain qualification considerations. Final exact-image soak remains NOT RUN; prior strict FAIL and separate OTA-restart risk remain open.
