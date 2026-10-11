# Documentation status

Status: **current documentation authority, reconciled 2026-10-11**.

[PR 46 diagnostics/media review](reviews/2026-10-11-pr46-diagnostics-media-review.md)
covers six further scopes (52 candidate files, 17 service issues), all repaired
locally, plus a reproduced stale service-log acknowledgement boundary and
adjacent SD format failure cleanup. FatFs has one service-unreviewed file and
is not a complete-directory review PASS. New injected regressions pass on
Windows and Ubuntu/WSL; full host qualification passes without SKIP. The final
ordinary M3 build/image verifier passes at
2,601,280 bytes. Installed firmware and frozen packages remain unchanged.

[PR 46 deck/USB/Wi-Fi lifecycle review](reviews/2026-10-11-pr46-deck-lifecycle-review.md)
covers nine further directories (48 files, 13 service findings): twelve findings
have local repairs; acquisition-token lease hardening remains explicitly deferred
after auditing current callers. A manually reproduced Beat Jump overflow is also
repaired. Changes cover LOAD contention, snapshot ordering, network seek bounds,
cue recovery/legacy deletion, settings migration, controller bootstrap/epochs and
USB/Wi-Fi startup races. Full host qualification passes without SKIP; the final
settings refinement has a separate 91-check focused PASS. The final ordinary
M3 build/image verifier passes at 2,600,528 bytes; new USB/Wi-Fi fault harnesses
also pass on Ubuntu/WSL.
Installed firmware and frozen packages remain unchanged.

[PR 46 startup/board/build review](reviews/2026-10-11-pr46-startup-board-build-review.md)
covers six further scopes (51 files, three confirmed service findings), all
repaired locally. Build-time identity refresh prevents stale SHA/dirty metadata;
SD DMA eligibility and the M3 I2S reclock diagnostic boundary are corrected.
Manual repairs support relocated build verification and selected-board startup
reporting. Full host qualification and the ordinary M3 ESP-IDF 6.0.2 build pass,
including actual image/isolation verification; dependency locks are unchanged.
Ubuntu/WSL identity execution remains NOT RUN because Ninja is absent; CI now
declares CMake/Ninja installation explicitly, without claiming a new CI run.
The installed firmware and frozen delivery artifacts remain unchanged.

[PR 46 web/OTA/library/storage review](reviews/2026-10-11-pr46-web-storage-review.md)
covers eight further directories (71 files, 11 findings), all addressed locally.
Repairs preserve HTTP during optional mDNS failure, fence an activated OTA
slot against a second upload, release failed receive resources, distinguish
HTTP server errors, keep cache reads free of mkdir, preserve cache entries on
close failure, support full beat counts, align both tested cache-write policies,
clear failed media IDs and publish fast-reconnect unmount edges. These changes
extend scoped software review; the installed image and frozen packages remain
unchanged. Whole-PR, current CI and physical board gates remain separate.
Full host qualification passes without SKIP, both library cache policies pass,
and the ordinary M3 ESP-IDF 6.0.2 build passes with unchanged locks. The final
storage mount regression passes separately; its runner registration is ready
for subsequent full runs.

[PR 46 profile/USB-host/LED review](reviews/2026-10-11-pr46-profile-host-led-review.md)
covers five further directories (26 files, six findings). Five confirmed
repairs address queue-reset starvation, malformed MIDI output admission,
rescan reactivation, activation logging and C++ LED linkage. The proposed
14-bit pair change remains an explicit compatibility-sensitive deferred item;
it is not reported as fixed. This extends scoped review evidence and does not
change the installed accepted firmware or frozen delivery artifacts.
Full host qualification (no SKIP), ordinary M3 ESP-IDF 6.0.2 build,
documentation checks and focused production-boundary regressions pass.
The existing six compiled controller profiles remain byte-identical.

[PR 46 UI/controller review](reviews/2026-10-11-pr46-ui-controller-review.md)
records nine local UI repairs and one unresolved font-permission/provenance
item from ten service findings. All nine simulator screenshot gates, the
full host qualification and an ordinary M3 build pass; controller-runtime
review reports no findings in seven files. This is scoped review evidence,
not whole-PR approval or acceptance of a new installed image.
The separate 13-file USB-audio review found a confirmed major FS 48-kHz
feedback packet-capacity defect. Its local repair and synthetic 16/24-bit
regressions are recorded in the same review; physical controller formats and
the installed accepted firmware remain unchanged.

[PR 46 scoped audio review](reviews/2026-10-11-pr46-audio-review.md) reports two
confirmed minor CodeRabbit issues repaired locally: FX-ready publication and
media-fault retry spacing. The full service review was rejected at 707 files
against its 150-file limit; audio-core review is partial, not whole-PR approval.
New repair evidence does not qualify the frozen packages or change the installed
Hercules image. Remaining review directories and board gates stay open.

Current review handoff: [P01–P10 dispositions and residual gates](reviews/2026-10-11-shared-core-delivery-readiness.md)
and [prepared PR description](reviews/2026-10-11-shared-core-pr-description.md).
Finding labels now follow the original comparison (P05 beat phase; P06 PDB
title). Software/package rows are closed for frozen `5ff76b6c`; historical
deferred experiments, coverage gaps and operator exclusions
are separately classified. No device action or remote publication is implied.

Operator correction, 2026-10-11: the historical OTA restart was associated with
moving the cable. The current ledger closes it as an operator cable-intervention
event and removes it from open firmware defects. Earlier investigation paragraphs
below remain historical evidence, superseded by this clarification. No firmware
fix, new test or new-image lifecycle acceptance is claimed.

Latest local delivery: three ordinary `5ff76b6c` candidates are signed and
independently verified, with BIN/ELF/MAP, configuration manifests, frozen
qualification evidence and delivery hashes retained locally. The existing
packaging guards ran against clean exact-source HEAD; the working branch was
restored afterwards. No device installation or publication occurred. Exact new
image physical gates remain NOT RUN; hosted CI is SKIPPED BY OPERATOR (quota).
See [frozen-source signed delivery](validation/SHARED_P4_LOCAL_QUALIFICATION_20261011.md#frozen-source-local-signed-delivery-2026-10-11).

Latest Hercules shortened gates on unchanged clean `e16ecfb5`, VALID `ota_0`:
three stopped USB reconnect/recovery observations with operator Browse/LED
confirmation, one separate cold-power readiness/GUI check, 60.219-s mixed-rate
dual MT/loop audio/visual PASS and 20.656-s active jog/Browse PASS (2044 MIDI
packets). All 22 playback strict deltas/output-late zero; decks stopped.
The combined lifecycle recorder remains INCOMPLETE; independent sample audit
establishes three USB epochs, with middle-epoch MIDI growth unsampled.
Final soak and wider control coverage remain open. See
[shortened gates](validation/HERCULES_INPULSE_500_PREPARATION_20261010.md#shortened-lifecycle-and-mixed-rate-mt-gates-2026-10-11).

Latest Hercules wake/protection gates on unchanged clean `e16ecfb5`, VALID
`ota_0`: first PLAY D1 wakes screensaver without playback; subsequent PLAY
starts normally. Physical normal/Shift LOAD on each playing deck and web LOAD
are refused, with unchanged tracks and operator-confirmed clean audio.
The dual-playing window completed 90.485 s, all 22 strict deltas and output-late
delta zero, final decks stopped. Full-sample audits pass. Cable reconnect/cold
power, wider controls, final mixed-rate MT smoke and soak remain open.
Evidence: [Hercules wake and LOAD LOCK](validation/HERCULES_INPULSE_500_PREPARATION_20261010.md#hercules-screensaver-wake-and-load-lock-2026-10-11).

Latest pad/FX/LED observation on `e16ecfb5`: 180.281 s / 313 samples /
136 MIDI packets, all 22 strict deltas and output-late delta zero, both decks
finally stopped. Sampled loop changes are incomplete relative to the requested
sequence and no global FX change was observed. Operator reports
"sve je bilo uredno" for the listed controls/audio/visual question; no
individual completion list was supplied. This is not full pad/FX/LED
qualification. See the
[combined observation](validation/HERCULES_INPULSE_500_PREPARATION_20261010.md#combined-padfxled-observation-2026-10-11).

Latest installed manual-loop candidate: clean `e16ecfb5`,
`M3-dev-ge16ecfb5e7b0`, VALID `ota_0`, exact ELF
`96279a267ef9a93f89148ba01de95519c93a87d6581f609e576739002323252f`.
Complete host qualification without SKIP, M3 ESP-IDF 6.0.2 build, identity/budget,
restart link placement and signed packaging pass. One local OTA returned HTTP
200; the exact image reached VALID and both decks remained stopped. Hercules
MIDI/UAC1 44100-Hz stream is healthy. Operator confirmed GUI restoration and
readiness for the targeted manual-loop retest. That 90.406-s / 164-sample test
passes all 22 strict deltas and output-late delta zero. Both physical IN/OUT/Exit
routes and decoder prefix publications are observed, with both decks stopped.
Operator confirmed normal loop/Exit response, clean MAIN/Hercules headphones
and fluid waveforms. The observed manual-OUT PCM regression is closed for this
exact tested configuration; other gates stay open.
No JC build, push, release or public OTA was performed.

Previous installed restart/Hercules-support candidate: clean `ac9287af`,
`M3-dev-gac9287af4a4c`, VALID `ota_1`, API idle, both decks stopped. Hercules
`06F8:B12B` MIDI/profile and UAC1 full-speed packed 24-bit/44100-Hz stream
are active; decks are stopped. Explicit feedback updates grow without invalid values. The missing
HCD integration was its 512-byte HS periodic FIFO, below Hercules MPS 540;
the experiment now uses 640 bytes while standard layouts remain unchanged.
Full host qualification of the claim diagnostic passed without SKIP; the final
FIFO patch additionally passes four compiled policy configurations. The exact
final M3 ESP-IDF 6.0.2 build, board identity/budget, restart link/raw gates,
signed bundle/manifest and local OTA pass. Matching BIN/ELF and hashed evidence
are retained. JC builds and public release were not run.

The cache synchronization restart candidate passes the complete qualification
host suite without SKIP and its exact M3 ESP-IDF 6.0.2 build, linked IRAM/panic
bypass contracts, five raw fixtures, board/budget checks and signed packaging.
One authorized local OTA installed it. A subsequent stopped normal restart
passes boot 418 -> 419, reset `SW`, the same exact VALID image and byte-identical
crash dump, with Hercules MIDI/UAC1 44100 Hz restored. The peer-reset/cache-sync
serialization gap is addressed; causality and broader restart reliability remain
open. The operator confirmed that the new-image GUI is normal on 2026-10-11.
This confirms GUI restoration, not new-image playback/audio qualification. Evidence and scope:
[restart candidate](validation/RESTART_BUS_CAPTURE_20261010.md#cache-synchronization-candidate-after-the-natural-fault).

An unattended read-only unloaded idle observation on exact `ac9287af` completed
180.172 s / 171 audited samples: all 21 connection/loss counter deltas zero,
45,121 new valid feedback updates, no USB disconnect or reboot, output-late
delta zero. Hercules remained UAC1 FS/44100 Hz and both decks remained unloaded
and stopped. Empty-ring zero-fill underflow grew by 7,960,226 frames with zero
submitted producer frames; it is explicitly separate in this idle-only scope,
not a zero-underflow claim or relaxation of the 22-counter playback gate.
Deterministic local profile regeneration matches the existing SHA. Current
controller README/MIDI acceptance state is reconciled, with combined remaining
operator blocks prepared. No new build/OTA/playback or S2 retest was performed.
Evidence and limits: [unattended Hercules observation](validation/HERCULES_INPULSE_500_PREPARATION_20261010.md#unattended-unloaded-idle-observation-2026-10-11).

On the same `ac9287af`, the returned operator completed the stopped navigation
and mixer window (180.234 s / 328 samples / 11,574 MIDI packets) and its focused
Gain/Shift+LOAD supplement (60.469 s / 110 samples / 3304 MIDI packets).
Normal and shifted load routes preserve the other deck; both Gain, pitch,
faders, EQ/filter and crossfader movements are observed, with operator-normal
response. No reboot or guarded connection/ISO errors occurred; both decks stayed
stopped. This is partial controller coverage, not playback, analog calibration,
pad/LED or full transport acceptance. Details:
[stopped controls](validation/HERCULES_INPULSE_500_PREPARATION_20261010.md#stopped-navigation-and-mixer-blocks-2026-10-11).

The subsequent manual transport window is strict PCM2 FAIL at 110.719 s / 199
samples: PCM2 +795 immediately after the first observed D2 IN/OUT loop,
other 21 strict deltas and output-late delta zero, no new USB loss or reboot.
Both decks stopped; operator noticed no symptom and identified D2 loop controls.
The decoder's FLAC-only retained-IN prefix left indexed MP3 with only a 2048-frame
reseek reserve. A bounded correction extends prefix eligibility to verified-index
MP3; real decoder/prefix continuation and slow-seek host regressions pass.
The complete host qualification suite and the exact M3 build/package pass;
the new `e16ecfb5` is installed VALID. Targeted manual-loop telemetry passes;
operator audio/visual confirmation also passes. Previously checked CUE/PLAY
handoff and Sync are retained per operator clarification; no repeat scheduled.
Evidence and limits: [manual loop failure](validation/HERCULES_INPULSE_500_PREPARATION_20261010.md#manual-loop-transport-failure-and-bounded-mp3-correction-2026-10-11).

On earlier exact `04f0cfb6`, the operator-confirmed combined 60.078-s test passes MAIN/headphones, both
jogs/Browse, PFL D1/D2, native headphone Mix/Volume and fluid waveforms. All 85
samples were audited: all 22 strict deltas zero, 649 MIDI packets, output-late
delta zero. This is a short smoke, not full Hercules qualification. Profile is
5592 bytes, v4; other stored profiles remain unchanged. Details and remaining
gates: [Hercules record](validation/HERCULES_INPULSE_500_PREPARATION_20261010.md).

The outgoing `90fba690` OTA restart produced a natural panic even though the
new image reached VALID. The 7776-byte raw dump and captured core-0 register
sequence decode against its exact retained ELF into `Cache_WriteBack_All`.
Capture/export is proven; the reason for the bus fault and restart reliability
remain open. The original historical dump is preserved separately. See
[natural restart capture](validation/RESTART_BUS_CAPTURE_20261010.md#first-natural-captured-ota-restart-fault).
S2 testing stays operator-closed; no push, release or public OTA occurred.

[Restart bus capture implementation](validation/RESTART_BUS_CAPTURE_20261010.md)
adds bounded shared IRAM hooks and 256 bytes of internal RTC user-coredump memory
to retain frame/queue state before IDF PC normalization. Focused production
wrapper and decoder tests pass. Actual link/coredump-region and synthetic raw
pipeline checks are now mandatory build gates. Full qualification host suite
passes with no SKIP. Clean M3 `0b08fb90` passes ESP-IDF 6.0.2, linked RTC-region
and interception checks, five raw-pipeline fixtures, board identity/isolation
and signed development packaging. Its matching BIN/ELF and verification hashes
are retained locally. A signed local OTA installed clean `0b08fb90`, VALID
`ota_1`, API idle; the operator confirmed GUI restoration. Both decks were
stopped and no controller/audio stream was active. The historical 7456-byte
crash export is byte-identical and preserved. At that earlier checkpoint,
physical PANIC export and restart reliability were **NOT RUN**; that restart does not
qualify the earlier crash workload or reopen S2 testing.

[Offline historical restart audit](reviews/2026-10-10-m3-restart-crash-analysis.md)
confirms core 0, an internal interrupted stack and the cache-writeback call
chain by complete original-BIN function-byte checks. One isolated historical
M3 analysis build passes; its ELF differs from the original and is used only
for individually verified functions. The original panic handler can replace
PC from an asynchronous bus-error queue. Missing queue CSRs/pre-normalization
MEPC prevent identification of the rejected transaction and causal repair.
The latest successful OTA had no controller or USB audio stream, so it does
not qualify the earlier workload. Device state and S2 closeout remain unchanged.

[Crash diagnostic implementation](validation/CRASH_DIAGNOSTICS_20261010.md)
adds distinct stored-dump ELF identity, a bounded marked stopped-deck raw export
and verified local ELF/map archives. Focused behavioral regressions pass;
full host suite, the single M3 IDF 6.0.2 build, board/budget checks and actual
signed packaging/debug archive verification pass on clean `58f09941`.
Documentation checks pass. After the operator clarified USB1 is power-only
and authorized the dump-overwrite risk, one signed local OTA installed clean
`58f09941`, VALID `ota_0`, boot 412 with reset `SW`. The new endpoint exported
the complete 7456-byte historical dump; its embedded checksum passes offline.
The stored nine-character ELF prefix `5f70083d0` matches the recorded outgoing
`3f3605a7` identity prefix, not a full SHA-256 match. Registers/segments and
original bytes are retained with hashes. The original application ELF remains
missing, and the historical restart PANIC cause remains open. S2 testing stays
closed with waivers; its accepted `a9f66bcd` ELF/map and exact-image soaks remain
historical evidence, not acceptance of the installed diagnostic image.

Current S2 disposition, 2026-10-10: **work closed with explicit operator
waivers; no further S2 testing**. Exact clean `a9f66bcd`, VALID `ota_1`, retains
full-hour 48000/44100-Hz telemetry and physical playback PASS and 48000-Hz
active jog/Browse acceptance. Remaining 44100-Hz active controls, exhaustive
LED/held-state and outstanding physical lifecycle/profile qualification are
WAIVED BY OPERATOR / NOT RUN, never converted to PASS. Earlier pending entries
below remain historical. No unrestricted S2/shared-core production release.

The [closeout/resource/restart review](reviews/2026-10-10-s2-closeout-resources-restart.md)
records the continuing general investigation: all 5260 cached soak samples show
no progressive retained-memory decline, a stopped snapshot has zero allocation
failures and adequate recorded task stack reserves, and largest-block DMA
qualification remains unavailable. Original outgoing-BIN disassembly supports
the cache-writeback restart location; its cause and raw-dump/ELF association
were unresolved at that review checkpoint. The diagnostic capture above adds
raw bytes and registers, but no causal repair or new S2 qualification. No new
build, S2 test, push, release or public OTA was performed during this capture.

The [Hercules preparation record](validation/HERCULES_INPULSE_500_PREPARATION_20261010.md)
documents PC VID/PID and jog-touch capture, exact fixture regeneration and
profile/registry host checks. A new shared v4 native-headphone capability and
independent filter policy resolve the software output-control audit; focused
production PCM/parser/storage/runtime tests pass. Full qualification and the
new M3 firmware build/install now pass on `eafdb7a5`. Actual descriptors, native blend,
controls/LEDs and audio require physical Hercules acceptance on M3.
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

For the current S2 strict ISO investigation, see the
[2026-10-10 scheduling and evidence analysis](reviews/2026-10-10-s2-hs-isoc-loss-analysis.md).
It identifies observability gaps and the next causal capture; it adds no
firmware or physical acceptance claim.

The subsequent bounded first-fault observer is implemented and installed as
clean `a9f66bcd`, VALID `ota_1`. Full host suite, M3 ESP-IDF 6.0.2 build,
board/image and signed-package checks pass. The API confirms capture enabled,
not yet captured; S2 MIDI and live 48000-Hz audio are active, decks prepared
and stopped. This diagnostic image has no new physical playback/soak PASS.
See the [S2 validation record](validation/S2_MK3_PROFILE_PREPARATION_20261007.md)
for exact SHA, ELF, artifact hash and outstanding reproduction gate.

The observer's short playback completed 60.514 s / 95 samples at S2 48000 Hz,
all 22 strict deltas and output-late delta zero, both decks stopped. Capture
is enabled but empty. Final diagnostic-log text was recovered after a helper
JSON-decoding error; original evidence is preserved. Physical confirmation
and long fault reproduction remain pending.

The operator subsequently confirmed that short observer playback had clean
MAIN/S2 audio and fluid waveforms. A bounded independent first-fault capture
run started on the same image at approximately 17:40:04 local, 2026-10-10,
up to 3600 seconds or the first strict error. Initial fresh status confirms
dual playback, S2 48000 Hz and zero packet failures/output-late. Fifteen-minute
checkpoints are active; final diagnostic outcome and long acceptance are open.

That first-fault diagnostic window completed 3600.296 s / 5705 samples on
exact clean `a9f66bcd`, S2 48000 Hz: all 22 strict deltas and output-late delta
zero, both decks stopped, same ELF/VALID `ota_1`. All samples were audited;
capture is enabled but empty. ISO loss was not reproduced, so no causal fix
is established and the 44100-Hz path remains unqualified. The operator
confirmed clean MAIN/S2 audio, fluid waveforms and stable display for the full
hour: exact-image 48000-Hz playback telemetry and physical PASS. Active-control
qualification remains separate. Minimum observed DMA free is 8759 bytes; lifecycle
and headroom remain open. Final evidence and hashes retained, one-time task
removed after runner exit, heartbeat paused. No push or publication.

The [unattended post-soak review](reviews/2026-10-10-s2-post-soak-headroom-lifecycle.md)
checks all recorded memory samples and reconnect/restart code. Cached free
internal/DMA memory has no observed progressive decline, but largest blocks
and lifecycle reserves remain unqualified. The extended production-stream
host regression passes 30 reconnect cycles with bounded live reservations,
no new OUT allocations and full release after FS transition. These are host
tests, not physical Campaign A. The prior OTA crash address matches the cache
sync map register; its underlying failure is still unresolved. No new firmware
build or install is needed for these test/documentation changes.

Full P4 host suite for the unattended test changes passes with exit code zero,
without SKIP markers. The existing a9f66bcd signed package/manifest verifies.
Physical active-control/reconnect and reboot checks were NOT RUN while the
operator was away; the installed exact-image playback PASS is preserved.

After operator return, a 30.042-second active-control block on the accepted
a9f66bcd/S2 48000-Hz setup passed: 1122 MIDI packets, all 22 strict deltas and
output-late delta zero, decks stopped. Operator confirmed both jogs/Browse,
MAIN/S2 audio and waveforms were correct. Short active-control telemetry and
physical PASS; reconnect/lifecycle and the intermittent ISO cause remain open.

The first physical reconnect capture stopped during its first removal and the
operator subsequently identified an incorrectly seated cable. Original fault
evidence is retained; this is operator-invalid, not a clean three-cycle PASS.
Post-operator counters show three reconnections and healthy final S2 44100-Hz
audio, decks/tracks and storage preserved. Per-cycle telemetry remains open;
the 48000-Hz playback PASS is not transferred to 44100 Hz.

The repeated reconnect observer captured three operational MIDI/audio returns
at 44100 Hz; operator confirmed both Browse controls. Tracks, stopped decks,
storage and image remained unchanged, allocation/root-recovery failure deltas
zero. One ISO packet/five frames was lost before the first removal; operator
explicitly confirmed manipulating cable or power in that period. Evidence is
preserved with context and hashes; neither spontaneous firmware causation nor
manipulation as the cause is established. This is not clean ISO qualification.
The completed task was removed after exit. Shared monitor ISO checks now also
cover stopped Observe/UsbRecovery; the regression reproduced false PASS before
the fix and all 11 monitor tests pass after it. No firmware build or OTA change.

A separate read-only 600-s idle observation of S2 44100-Hz streaming started
2026-10-10 19:47:11 local with both decks stopped and no intentional USB/power
changes. Records `stopped-idle-20261010T174710Z` under the next-gates directory;
independent task and live process verified. Baseline ISO 1 packet/5 frames and
frozen fault remain retained. Result is pending; this does not qualify playback
or full S2 acceptance. Checkpoint automation will report completion and pause.

That stopped S2 44100-Hz idle observation completed 600.019 s / 1084 samples,
wrapper exit zero. All samples preserve exact clean `a9f66bcd`, stopped decks,
tracks, storage and healthy live streaming; all 22 strict deltas and output-late
delta zero. Final firmware remains VALID `ota_1` with the same ELF. Existing
ISO 1/5 counters and frozen capture are unchanged. Ten-minute idle telemetry
PASS only; 44100-Hz playback, full S2 acceptance and ISO causation remain open.
Full-sample audit/hashes saved, task removed after exit, automation paused.
No firmware, playback, connection, reboot or publication changes.

A separate 600-s S2 44100-Hz dual MT/loop playback window started at about
2026-10-10 20:07:52 local after operator restored zoom 8 and confirmed readiness.
Exact `a9f66bcd` remains installed; one independent monitor checks all22strict
deltas and safely stops both decks on error/end. Initial playing samples show
unchanged ISO counters (historical1packet/5frames). Prior frozen fault is not
new playback evidence. Records `playback-441-20261010T180751Z` under next-gates;
task/runner verified, checkpoint active. Outcome and physical confirmation
remain pending; no new firmware/build/USB/Wi-Fi changes or publication.

That S2 44100-Hz ten-minute playback window completed 600.622 s / 877 samples,
all 22 strict deltas zero, safely stopped, same clean VALID `a9f66bcd`/ELF.
Full-sample audit confirms dual MT/loops, mixed-rate source tracks, opposite
pitch and healthy streaming. Three output-late events (max11334us, largest
sample-observed 60-s cluster2) remain monitoring findings. Operator confirmed
clean MAIN/S2 audio, fluid dual waveforms and stable display for the whole run:
exact-image ten-minute 44100-Hz playback telemetry and physical PASS. MIDI0
does not qualify active controls; existing ISO1/5 and frozen capture unchanged.
Long 44100-Hz qualification and intermittent ISO cause remain open. Evidence,
hashes/operator confirmation preserved; task removed after exit, automation
paused. No firmware, reboot, USB, push or publication changes.

Final 3600-s S2 44100-Hz timing soak started about 2026-10-10 20:28:00 local
on the same clean VALID `a9f66bcd`, dual MT/loops, mixed-rate tracks, opposite
pitch and zoom8. One independent task/runner is verified; initial playing
samples retain historical ISO1/5, feedback-invalid3 and output-late3. Records
`final-soak-441-20261010T182759Z` under next-gates; checkpoints every15minutes.
Strict losses stop the run; output-late rolling cluster/max findings are
recorded separately rather than treating total10 over an hour as a cluster.
Four local offline policy regressions pass. Outcome/operator acceptance pending;
old frozen capture remains historical. No firmware/build, USB or Wi-Fi change,
push or publication; full S2/shared-core and ISO causation remain open.

The final S2 44100-Hz soak completed 3600.414 s / 5260 samples, wrapper exit0.
Full-sample audit confirms exact clean `a9f66bcd`, unchanged dual MT/loop/pitch
workload and healthy streaming; all22strict deltas0, both decks safely stopped,
same ELF/VALIDota1 and zoom8. Five output-late events, max11348us/60-s cluster2,
remain monitoring findings; no investigation trigger fired. Cached DMA free
starts/ends11163B, sampled minimum8635B; prior lifetime minimum7371B unchanged.
Historical ISO1/5 and frozen capture unchanged, MIDI0. Exact-image full-hour
44100-Hz telemetry and physical playback PASS: operator confirmed clean
MAIN/S2 audio, fluid dual waveforms and stable display throughout the hour.
Root cause and remaining S2/shared-core gates stay open. Original hashes
rechecked, operator confirmation and updated audit preserved,
task removed after exit, automation paused; no firmware, restart or publication.

The [2026-10-11 shared-core/Hercules scope review](reviews/2026-10-11-shared-core-hercules-closeout.md)
reconciles original phases and P01–P10 with current source and acceptance.
The unchanged Hercules fixture regenerates identically; focused profile,
version/DMA, fixture and both migration regressions pass. All three entrypoints
select one canonical set of reviewed modules and board providers. The profile
support table now separates implemented pads 1–4 from unsupported FX pads 5–8
and deferred functions. Final current local full qualification/build matrix,
real-media/JC physical gates and historical restart causation remain open;
hosted CI remains SKIPPED BY OPERATOR. This review does not flash or publish.

[Ordinary local software qualification](validation/SHARED_P4_LOCAL_QUALIFICATION_20261011.md)
subsequently passes on clean `5ff76b6c`: complete no-SKIP host suite, all nine
unchanged simulator baselines, JC/M3 300-s keylock policies and fresh IDF 6.0.2
builds JC4880 2605344 B / JC1060 2663424 B / M3 2598608 B. Board/component/source
identity, unchanged locks, budgets and 15 BIN/ELF/MAP/identity/budget artifact
hashes pass. The simulator CMake omission of shared `ui_idle.c` was corrected;
production firmware and installed e16ecfb5 remain unchanged. Fresh experiment
firmware matrix, physical gates, signed delivery and publication stay separate.

Final Hercules exact-image `e16ecfb5` 44100-Hz playback soak completed
3600.016 s / 5171 samples, wrapper exit 0. Every sample and final stopped
snapshot have all 22 strict counter deltas zero; clean source/ELF/VALID ota_0,
mixed-rate MT/loop/pitch workload and zoom 8 verified. Operator confirmed clean
MAIN/Hercules headphones, fluid dual waveforms and stable display throughout
the hour. Fifteen output-late events, max 12534 us and rolling-60-s cluster 2,
remain monitoring findings; no investigation trigger fired. This is telemetry
plus physical PASS for the named final playback scope, not zero-late or full
profile/shared-core completion. MIDI delta 0; the earlier 2044-packet active-input
supplement remains separate. Audit, original artifacts and hashes retained;
task removed after exit and heartbeat paused. S2 remains operator-closed.
See [Hercules validation](validation/HERCULES_INPULSE_500_PREPARATION_20261010.md).

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

S2 candidate fc4ed608 moves experimental M3 ISO payloads into internal DMA memory; full host suite, M3 IDF 6.0.2 build, board check and signed-package checks pass. Local OTA installed exact clean image VALID ota_0, but old-image OTA restart produced boot406 PANIC and HS audio quarantine. Operator RESET restored normal boot407 and healthy S2 44100-Hz streaming. The 60.477-s dual MT/loop retest has telemetry and operator playback PASS: all22 strict deltas and output-late delta zero, both decks stopped. After track load cached DMA free is13995 B (minimum13087 B), so lifecycle/headroom remain qualification considerations. The subsequent independent 3600-s soak is strict ISO FAIL after529.276s/783samples: two packets and11frames lost, other20strict deltas zero, output-late delta1. Both decks stopped; operator noticed no disturbance. Evidence and post-failure diagnostics retained, checkpoint automation paused. Internal payload placement did not eliminate loss. Final soak acceptance and separate OTA-restart risk remain open; short PASS retains its scope.
