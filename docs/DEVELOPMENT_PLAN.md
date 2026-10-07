# P4 post-release development plan

Shared-core/M3 implementation is tracked in
[the integration ledger](SHARED_P4_CORE_INTEGRATION.md). Portable code now lives
under `firmware/p4-core`; hardware providers live under `firmware/boards`.
This development branch does not extend the physical scope of M2.5.

The shared-core implementation retains donor library/transport/profile features
and accepted M3 BSP/DSP/UI policies in one runtime. Finish software variant gates,
then use [M3 wired migration](M3_SHARED_CORE_MIGRATION.md) on the frozen candidate.
New-image physical gates include GUI/touch, dual-deck MAIN/PFL, all five zooms,
signed OTA/startup/rollback, 30 Campaign A cycles and at least 60 minutes of
worst-case Campaign B timing. Legacy-image acceptance does not transfer.
Unavailable hardware is NOT RUN. Public board-channel publication requires
separate authorization after that board's acceptance.

The accepted M3 qualification baseline is `M3-dev-gfb5cbca13f33` from clean source
`fb5cbca13f33d21dfb4e16e0ba667c340399eb0c`, installed VALID in ota_0.
It removes an unused blocking audio-position read from per-refresh jog-mode
polling. Host qualification, unchanged M3 simulator hashes, M3 build and signed
identity/budget checks passed. Its short dual MT/zoom-8 retest passed with
operator-confirmed fluid waveforms and clean MAIN/headphones sound. A subsequent
short review confirmed all five zooms fluid and sound clean. The same installed
image then passed a continuous 3600.163-second Campaign B soak: telemetry and
operator-confirmed stable/fluid display and clean MAIN/headphones. Strict new
PCM/UAC/service-log losses were zero; 11 output-late events remain monitoring
findings, not a zero-late claim. Campaign A's 30 physical cycles and operator
checks were excluded by the operator. The [automatic supplement](validation/M3_AUTOMATIC_RECOVERY_20261007.md)
observed ten software-reboot recoveries, with nine complete measured cycles,
host visibility investigation flags and one incomplete timing window. This does
not establish physical Campaign A acceptance. [Autonomous negative OTA and injected
startup rollback tests](validation/M3_OTA_NEGATIVE_ROLLBACK_20261007.md) subsequently
passed and restored the exact accepted image. Current-source GitHub Actions is
SKIPPED BY OPERATOR (quota), not PASS; full hardware acceptance remains open.
Short per-zoom dwell coverage is
recorded separately from the completed zoom-8 soak.

[Catalog/cue software and web LOAD LOCK checks](validation/M3_LIBRARY_CUES_WEB_LOAD_LOCK_20261007.md)
subsequently passed on the unchanged image: 324 persistent identities remained
equal across reboot and six playing-deck replacements were refused. Production
NVS adapter error/corruption tests now join the host runner. Physical cue/NVS,
MIDI/touch LOAD LOCK and independent real-export/UI acceptance remain open.

The [real-export audit](validation/M3_REAL_EXPORT_AUDIT_20261007.md) now
reproduces lost artist names on 13 tracks, lost album names on 12 tracks and
missing source Hot Cue D from EXT on track 7. Correct row-specific name layouts
and transactional EXT cue enrichment, preserve local cue identities, and rerun
the actual 324-track comparisons before an M3 candidate build. No firmware was
changed by the audit. Playlists/source loops are absent from this medium;
their real-media gates and on-device rendering remain open.
Correction `082d2c0a` passes exact-source 324-track comparisons with zero import
differences, the default shared host gate, final focused regressions and the
M3 IDF 6.0.2 build/signed package. It is now installed VALID in ota_1, and the
live 324-track library recovers all 13 artist names. D1 A–D recall is now
operator-confirmed, including source D at 123347 ms in the stopped API snapshot.
The [on-device cue lifecycle](validation/M3_CUE_LIFECYCLE_20261007.md) now passes:
local override, deletion tombstone and restore-source persist through reboot.
Final source D recall is 123347 ms and both decks are stopped.
Next: remaining new-image physical gates; do not transfer fb5 acceptance.

The preceding bf2fdc47 skipped unchanged cover comparisons and hidden Library styling, and separated
chrome/UI-tail/LVGL timings. Host qualification, unchanged M3 simulator hashes,
M3 build and signed identity/budget checks passed. Its repeated physical test
still showed slight dual stutter with clean MAIN/headphones sound.

The preceding 1d3ee7b6 was installed VALID in ota_0.
Host qualification, unchanged M3 simulator baselines, the M3 build and signed
package verification passed. Its physical retest showed much improved visuals
with slight residual stutter and clean MAIN/headphones sound. Continuous dual
playback without loops also retains the small stutter. The full visual and
long-soak gates remain open.

The preceding b0c66744 deferred framebuffer-preservation repair passed host qualification,
unchanged M3 simulator baselines and the M3 IDF 6.0.2 build. The operator stopped
the test because of severe stuttering, beginning only when both decks played.
That image failed the physical visual gate; listening was not qualified.

During unresolved M3 bench diagnosis, build, sign and test M3 only. Resume
JC4880/JC1060 builds and full cross-board CI after the operator accepts the M3
repair. Full integration/release requirements remain mandatory afterwards.
The redundant matrix runs were cancelled; the earlier JC4880 build had already
finished and the JC1060 configure was interrupted. Keep separate M3 bench
evidence rather than marking full software qualification complete.

The paired-buffer image 6d55ce2a failed its physical test and the bench was
returned to the signed ee54f0d1 image, VALID in ota_0. Its captured solo window
spent 6.04 ms copying old framebuffer content before redraw. The subsequent
dual interval lost HTTP access and persistent logs recorded audio/UAC losses.
The installed b0c66744 revision keeps complete-frame ownership but defers preservation until
after rendering, copying only previous damage not covered by new writes.
Exact bitmap coverage preserves paused content and holes between rectangles;
fully redrawn strips require no old-pixel copy. Pixel reference tests and M3
simulator baselines, exact M3 build and signed packaging passed. The physical
retest failed. The next revision changes only UI position freshness handling:
retained observations cannot rebase the display prediction backwards. RAM
observation-age counters will distinguish this reproduced host defect from
unproven device causes. Audio DSP and panel configuration remain unchanged.
See the candidate record for the pre-v3 contract and image-specific evidence.

The preceding `M3-dev-ge4536de2ec6f` was installed in M3 ota_1 with VALID health state,
verified against its exact clean source and ELF on 2026-10-07 within the
installation helper's polling deadline. The preceding 180b008c helper's expired
deadline remains a separate historical finding.
[The current candidate record](validation/M3_SHARED_WAVEFORM_REGRESSION_20261006.md) separates
the signed OTA, 324-track catalog and mixed-rate load evidence from remaining
operator audio/UI and USB campaigns. Initial wired migration and earlier GUI/
touch/settings confirmations remain in the image-specific
[95e installation record](validation/M3_SHARED_CORE_INSTALL_20261006.md).
Both 11574f4e and the installed c698fa67 ordering repair failed on
operator-confirmed dual waveform deformation. The latter also showed visual
stutter while audio remained clean. The measurement candidate reproduced the
failure: 57 coalesced refreshes in about 12 seconds of dual playback. The next
repair, now installed after all three builds and exact-source CI passed,
removes duplicate PPA cache maintenance and unchanged frame resampling;
see [the regression record](validation/M3_SHARED_WAVEFORM_REGRESSION_20261006.md).
The installed repair also failed the physical retest: solo sharp, dual deformed,
audio clean, 55 coalesced refreshes in the short dual window. Investigate the
remaining blocking UI status reads before another candidate; the full timing
gate must restart after a successful exact-image physical retest.
The installed e4536de2 candidate shares one zero-wait, UI-owned audio-status observation per
deck between position, session-checked duration and status/progress rendering.
Busy decode retains the last observation; transport decisions keep their
authoritative APIs. Production mutex-contention and simulator recovery tests
and all three clean builds passed, but its physical retest still failed: solo
sharp, dual deformed, audio clean, with 65 coalesced refreshes. The next ee54f0d1
candidate removes the remaining waveform-phase loop-query waits, but its operator
report still rejects the visual result. The
[diagnostic-overhead audit](validation/M3_DIAGNOSTIC_OVERHEAD_REVIEW_20261007.md)
separates quiet service-file logging from recurring heap inspection and UART
producer reports; the full soak remains NOT RUN.

Status: **M2.5 published; A-L merged into master; hardware extensions open**
(2026-10-06).

## Current baseline

The accepted release is `M2.5` from
`20f1c3f04a615209ae25e9bbdae649d0f5b44e8d`, for JC4880/FLX4 with PCM5102A
MAIN and FLX4 cue. The previous product design is retained. The
[release record](validation/M2_5_RELEASE_20261006.md) preserves the explicit
final-image soak waiver, exact artifacts and focused physical scope.

Packages A-L are software verified and merged into `master`. The
[integration ledger](FORK_IMPROVEMENTS.md) records modules adapted from
[kayrozen](https://github.com/kayrozen) and their test/provenance history.
JC1060/DDJ-400/Ethernet Link is a development configuration, not released
hardware support. The recorder, SD experiments and optional UI preview remain
off in regular builds. APTA is outside this integration.

Physical rollback and the 60-second startup rejection passed using isolated
diagnostic images. They restored the original v91; those later diagnostic
commits are not the hardware-qualified release image. Ordinary packaging
rejects both fault-injection flags. Original tags and versioned assets remain
immutable.

## Next work

M2.5 JC4880/FLX4 is published after exact-source CI, final tagged OTA/startup
and operator-confirmed short audio acceptance. Subsequent maintenance uses
change-driven regressions; the remaining local-library cases below stay open.
JC1060/DDJ-400/Link, experimental recording and APTA need not block a release
that explicitly excludes their production support. Web Profile Builder v4
deployment is required only if its public exporter is part of the release scope.

The user-supplied Pajoniiir logo is now the released M2.5 artwork fallback for
loaded tracks and Library rows. It is stored as native 34x34/40x40 RGB565
constants; the existing real-artwork worker/cache remains authoritative.
Candidate `M2.4-111-ge09e9ca4` has passed signed OTA, operator-confirmed logo
display and a 62.419-second dual-deck listening/resource check; see the
[candidate record](validation/M2_5_DEFAULT_ARTWORK_20261006.md).
Real cover, missing cover and normal page/media return passed on the logo
candidate. Final M2.5 passed 62.622211 seconds of clean dual playback and
startup/resource checks. Its new long soak was explicitly waived; v91 remains
immutable and its earlier evidence is not relabeled as M2.5.

| Work | Required evidence / boundary |
| --- | --- |
| JC4880 maintenance | Change-driven regressions on a new exact image; do not rebuild current master and call it the accepted v91 |
| Remaining local-library/transport variants | Real title/downbeat, ordered nested playlists, artwork/PWV4, source-cue restore/reimport and cross-media identity; advanced touch/shift and active-removal cases remain NOT RUN |
| JC1060 bring-up | Board/panel revision, flash/PSRAM, touch/render, USB root roles, SD, RMII PHY/DHCP, memory/stack/deadline measurements |
| DDJ-400 and other controllers | Real descriptors, MIDI/LED/SysEx/reconnect and MAIN/cue channel isolation; profile fixtures alone do not qualify hardware |
| Link I/J | Real CDJ and separately named rekordbox version; discovery claims/conflicts, ordered browse, NFS downloads, cancel/media-swap/cache/space/power-loss cases |
| Link K | Measured output phase, drift, master handoff and listening per sink/sample-rate; LOCKED alone is insufficient |
| SD/recorder experiments G | Exact-card A/B latency, ring overflow, finalize/removal/full-card/power-loss, simultaneous audio deadlines and listening; separate experimental result |
| Web Profile Builder | Coordinated v4 exporter already has software evidence; deployment/acceptance belongs to that repository and is not implied by firmware publication |
| Security maintenance | Per-device credentials if exposure expands, disposable backup signature check, successor-key overlap and sacrificial hardware-rooted protection pilot |
| LIBAPTA | Separate branch/program; revalidate upstream and entry gates in [its plan](LIBAPTA_P4_INTEGRATION_PLAN.md) |

JC1060 must keep PCM5102A disabled because its pins overlap Ethernet;
ES8311 is disabled in the USB-only configuration. No Wi-Fi Link or local
NFS/DBServer library server is part of this program.

## Maintenance rule

Repeat gates because behavior or assumptions changed, not because records aged.

| Change area | Minimum checks |
| --- | --- |
| Documentation/media | Documentation integrity, whitespace, links; visual Pages review for manual layout changes |
| Tool/profile format | Relevant suites; parser/compiler/storage/upload compatibility and generated fixtures together |
| Shared firmware | P4 host runner, affected IDF v6.0.2 target builds, lock stability and image budget |
| UI | Actual presentation simulator layouts, visual review, exact-image display/touch and active playback checks |
| USB/storage/audio/OTA | Relevant focused hardware regression, counters, operator sound and lifecycle checks |
| DSP/scheduling/buffering | Reproduce/fix the failure; exact-image active timing and appropriately long audible test |
| Partition/bootloader/trust key | Wired recovery plan, isolated signed build and supervised installation |

Shared-core changes require all three entrypoint builds. Use committed independent
dependency locks and the fixed `0x380000` image budget. Preserve actual source,
configuration, version, slot/boot, strict deltas and operator results.
A host soak or simulator cannot replace physical deadlines or sound.

## Release discipline

- Freeze an exact clean source and configuration; run applicable automated gates.
- Install and accept that exact image for a named board/controller/sink/peer set.
- Default duration gate remains an uninterrupted 180-minute dual-deck run.
  The recorded v91 segmented exception is image-specific, not an automatic
  waiver for later releases.
- Publish immutable signed artifacts before atomically switching the correct
  board channel; independently verify public size/hash/signature and assets.
- Record NOT RUN and explicit exceptions separately from PASS.
- Never move published tags or overwrite frozen assets.
- Do not use fault-injection, preview, recorder or storage-experiment images as
  ordinary production releases.
- Keep private keys and hosting credentials outside Git/logs/artifacts.
- No eFuse operation is implicit in release tooling.

## Historical plans

The [M2.4 record](validation/M2_4_PRODUCTION_RELEASE_20260929.md),
[M2.5 handoff](M2_5_HANDOFF_20261002.md),
[H migration history](validation/FORK_IMPROVEMENTS_PACKAGE_H_PROGRESS_20261004.md)
and [L hardware ledger](validation/JC4880_L_CANDIDATE_20261005.md) retain
older installation, supply, resource and audio-failure sequences. Their former
"next step", installed-version and unpublished-channel statements describe
those sessions; current status is [DOCUMENTATION_STATUS.md](DOCUMENTATION_STATUS.md).
Do not reopen completed v91 gates without a new failure or changed assumption.
