# Shared P4 core and Hercules scope review, 2026-10-11

## Outcome

The common production architecture is implemented. JC4880, JC1060 and M3
entrypoints select `firmware/p4-core/components` and their board directories.
This review closes the Hercules source/fixture/support-scope reconciliation
and updates the shared-core acceptance ledger. It does not declare the original
whole-project completion criterion met.

Subsequent [ordinary local qualification](../validation/SHARED_P4_LOCAL_QUALIFICATION_20261011.md)
closes the full host/simulator/keylock and three ordinary board-build rows below
for clean `5ff76b6c`. The following tables retain the review checkpoint's scope;
fresh experiment firmware builds, physical and publication gates stay separate.

Installed M3 remains clean source
`e16ecfb5e7b0daef81e4640ba46e9f9409184963`, VALID `ota_0`, ELF
`96279a267ef9a93f89148ba01de95519c93a87d6581f609e576739002323252f`.
No production source, profile binary, device settings, USB topology or firmware
was changed during this review. Documentation commits do not transfer acceptance
to a newly compiled image. S2 remains operator-closed.

## Hercules support boundary

The [profile scope table](../../controllers/hercules_djcontrol_inpulse_500/README.md#supported-scope-and-deliberate-limits)
is the compact supported-function reference. Mapping, DSP behavior and physical
evidence are separate columns. The source-backed profile is S3CP v4, 5592 bytes,
SHA-256 `570b59ecca6baa7319ef85ca92d7d3a11fbe11ea1f9c9c580a8779031a40f549`.
Regeneration is byte-identical to the committed fixture; it is not NVS readback.

`audio_pad_fx.c` implements filters on pad indices 0/1 and echoes on 2/3,
with NONE on 4..7 in both banks. FX1 echoes are 250/500 ms; FX2 echoes are
125/1000 ms. The shared deck dispatcher ignores Key Shift and actual Sampler
semantics; the Hercules physical SAMPLER selector instead maps to Beat Jump.
These are deliberate engine limits, not missing Hercules MIDI addresses.
Vinyl/Slip, Shift+Play stutter, guide functions and modes 7/8 remain deferred.
Adding arbitrary effects or new controls would change the accepted product
and require a separately scoped implementation/acceptance.

The final unchanged-image Hercules playback gate passes 3600.016 s / 5171
samples with all 22 strict deltas zero and full-hour operator audio/visual
confirmation. Fifteen output-late events, maximum 12534 us and rolling-60-s
cluster 2 remain monitoring findings. Earlier active-input, wake/LOAD LOCK,
three stopped reconnects and one cold-power check keep their documented scopes.
The combined pad/FX record does not prove every release, audible effect, RGB
colour or global FX transition. Those rows remain PARTIAL/NOT RUN rather than
being converted to PASS by the final soak. See the
[validation ledger](../validation/HERCULES_INPULSE_500_PREPARATION_20261010.md).

## Architecture and implementation audit

Static inspection covers `p4_app`, `audio_engine`, `deck_core`, `library`,
`controller_profile`, `controller_profile_runtime`, `controller_usb_host`,
`controller_usb_audio`, `usb_storage`, `ui`, `web_server`, `p4_ota` and
`restart_diagnostics`. All have one canonical component directory; none has
a `main-deck-*/components` copy. All three entrypoints include that directory
and their own board provider, with committed dependency locks. The inspected
audio/library/controller-runtime C sources have no M3 identity conditional;
policy defaults remain in Kconfig and board contracts. This source audit is
not a fresh linked build-graph check; `tools/check_board_build.py` must verify
each final build's component paths, BSP, identity and budgets.

| Original phase | Implemented state and retained evidence | Remaining work |
| --- | --- | --- |
| 0 provenance | Frozen donor/M3 SHAs and adoption matrix retained in the integration ledger | Preserve licences and recovery artifacts in final delivery |
| 1 common core | Shared modules, startup and board providers in place | Current final-source host qualification and JC linked builds |
| 2 M3 board/DSP | Accepted M3 BSP, scanout policy, fixed MAIN rate and shared DSP | JC physical regressions; do not transfer M3 acceptance to JC |
| 3 OTA/startup | Separate projects/channels, signed identity and requested AP/HTTP health gate | Historical general OTA-restart PANIC cause remains unresolved; final-candidate lifecycle qualification separate |
| 4 library/cues | Shared PDB/ANLZ, persistent identities, override/tombstone/restore and offline legacy migration | Selected legacy migration apply only if needed; real export variants below |
| 5 transport | Shared LOAD reservation, decoder seek/loop/CDJ behavior, M3 MP3 manual-loop correction | Broader EOF/seek/race/active-removal physical variants remain unrun |
| 6 UI/catalog | Shared hierarchy/artwork/memory/PWV4 and board layout | Real ordered nested playlists, artwork stale-media and source-loop cases absent from accepted medium |
| 7 USB/profiles | One mount/runtime/profile/recovery path; v2/v3/v4; explicit roots | Throughput/FAT variants and controller-specific partial coverage; role-swap bench is experimental |
| 8 diagnostics/experiments | Shared bounded journals/resources and gated Link/recorder/alternative UI | Experiment physical acceptance excluded from M3 defaults; restart analysis remains separate |

| Finding | Current conclusion | Remaining acceptance boundary |
| --- | --- | --- |
| P01 | Board/project isolation and signed packaging implemented; M3 rejection/interruption/rollback evidence retained | Final exact-source artifacts and other-board qualification |
| P02 | Cue identity/merge/tombstone/restore implemented; selected M3 physical lifecycle passed | Legacy-record apply is not implied; cross-media physical variants remain unrun |
| P03 | Shared default-on LOAD LOCK; FLX4/touch and Hercules normal/Shift refusal accepted in named records | Physical simultaneous LOAD/PLAY race and lock OFF not run |
| P04 | Tagged PCPT and list type implemented; actual EXT Hot Cue D defect corrected and recall accepted | Real source memory-loop/playlist export coverage still limited |
| P05 | Title/name layout correction and real-export metadata comparison passed | Evidence belongs to its export/image; not arbitrary exports |
| P06 | PQTZ 1–4/downbeat binary regressions implemented | Independent physical beat-phase/downbeat review remains unrun |
| P07 | Production decoder/DSP tests plus named mixed-rate/MT/manual-loop acceptance | Full broader physical transport matrix not implied |
| P08 | Real startup readiness and injected rollback implemented/tested | Unresolved historical restart cause is separate from startup readiness |
| P09 | Three-target/variant workflows and required fixtures implemented | Hosted Actions SKIPPED BY OPERATOR (quota); final local matrix still to run |
| P10 | Canonical architecture, migration, board policy and exact-image evidence documented | Final delivery manifests and remaining board acceptance records |

## Checks executed in this review

- Controller profile runner: format/runtime, FLX4 golden input/LED parity,
  S2 software fixture and Hercules parser/storage/native-control/activation tests.
  Running the S2 software regression does not reopen its physical campaign.
- Version resolver: 3 tests; S2 DMA isolation policy: 4 tests.
- Required qualification fixture manifest: 1 test.
- Preserving factory migration: 19 tests; cue migration: 10 tests. These execute
  mocked device/NVS and offline parser paths, not a new physical migration.
- Hercules fixture regeneration/hash equality and static shared-source audit.

All listed commands exit 0. Local source-audit details are retained in
`.cache/shared-core-closeout-20261011/source-audit.json`. No new ESP-IDF build,
full host qualification, simulator matrix or physical test was run in this
review. Historical successful runs retain their original SHA/configuration.

## Next bounded work

1. Run the complete local `-Qualification` host gate, required simulator matrix
   and final-source ordinary builds for JC4880/JC1060/M3; retain unchanged locks,
   board/component identity checks and `0x380000` image budgets. Run configured
   experiment software gates separately. Hosted CI remains explicitly skipped.
2. Produce exact-source signed local packages and debug archives for reviewed
   board configurations. Keep M3 production FLX4 USB2/storage USB3 separate from
   accepted Hercules bench USB3/storage USB2; a bench soak does not qualify the
   ordinary topology or new compiled source. Do not install merely for paperwork.
3. Preserve unresolved/NOT RUN/waived physical rows. Real-media or JC hardware
   acceptance needs suitable exports/hardware and the operator. The excluded
   30-cycle physical Campaign A remains excluded, not PASS. Do not automatically
   reopen S2 or repeat accepted Hercules blocks.
4. Continue historical OTA-restart analysis only from matched crash evidence;
   successful later restarts do not establish a causal fix. No speculative
   firmware change is justified by this documentation/source review.

No push, release, public OTA-channel update or new remote publication is part
of this closeout. The original whole-core completion condition remains open
until its software and physical gates are satisfied or explicitly waived.
