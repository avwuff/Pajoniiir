# Shared P4 delivery readiness and outstanding gates, 2026-10-11

## Decision and authority

The shared production architecture, ordinary three-board local software
qualification and frozen-source signed packaging are ready for review. This is
a development integration, not a new production release or a whole-project
acceptance declaration. This ledger supersedes earlier next-action tables;
dated validation records keep their original scope.

Operator clarification, 2026-10-11: the historical OTA restart discussed in
this handoff was associated with the operator moving the cable. It is closed
as an operator-reported cable-intervention event, not an outstanding spontaneous
firmware restart defect. Original logs/dumps remain preserved. This is a context
correction, not a firmware repair or a new exact-image lifecycle PASS.

Canonical repository: `dvucinozd/Pajoniiir`, branch
`codex/shared-p4-core-m3`. Frozen qualification/package source:
`5ff76b6cb1474779c6e58ecd5b273dcee378a3ed`. Subsequent commits are documentation
only. Donor/M3 bases, migration decisions and module provenance are recorded in
[the integration ledger](../SHARED_P4_CORE_INTEGRATION.md).

Installed M3 remains `e16ecfb5e7b0daef81e4640ba46e9f9409184963`, VALID `ota_0`,
ELF `96279a267ef9a93f89148ba01de95519c93a87d6581f609e576739002323252f`.
It uses Hercules USB3/root0, storage USB2/root1, MAIN 48000 Hz, Hercules
UAC1 FS 44100-Hz packed-24 and Wi-Fi ON. Its accepted 3600.016-second playback
has 22 strict counter deltas zero and operator full-hour audio/visual PASS;
15 output-late events, maximum 12534 us, remain monitoring findings.
The new ordinary M3 package uses FLX4 USB2/controller and USB3/storage.
Neither source nor configuration acceptance transfers between those images.

## P01–P10 disposition

Finding numbers follow the [original comparison](2026-10-06-ddj-ffl4-port-comparison.md).
P05 is beat phase/downbeat; P06 is PDB title index. Earlier integration tables
had these two labels exchanged; their numbering is corrected without changing
the tests or historical evidence.

| Finding | Software disposition | Retained physical evidence | Outstanding gate / classification |
| --- | --- | --- | --- |
| P01 board-isolated OTA | Implemented; three frozen signed packages and project/descriptor/signature checks PASS | Named M3 rejection, interruption and rollback records | New exact-image installation/lifecycle and other-board acceptance NOT RUN; release gate |
| P02 persistent cue identity | Implemented; collision, merge, tombstone, restore, interrupted-write and migration regressions PASS | Selected M3 set/delete/restore/reboot passed | Legacy migration apply conditional on actual legacy data; cross-media physical variants NOT RUN |
| P03 LOAD LOCK | Implemented; default ON, shared semantic path and atomic worker reservation; host race cases PASS | Named FLX4/touch/Hercules normal and Shift load refusal passed | Physical simultaneous LOAD/PLAY and lock OFF NOT RUN; coverage gap |
| P04 tagged PCPT/list type | Implemented; production parser fixtures PASS; actual EXT cue defect corrected | Selected source A–D recall passed | Source memory-loop and broader export cases NOT RUN; medium/coverage gap |
| P05 PQTZ phases/downbeat | Implemented; phases 1–4 and downbeat-1 binary regressions PASS | No independent complete beat-phase physical review | NOT RUN; software success is not an acoustic/downbeat confirmation |
| P06 PDB title index 17 | Implemented; required parser fixtures and selected real-export comparison PASS | Named metadata/UI review passed | Arbitrary/export-version coverage not implied; wider media variants NOT RUN |
| P07 decoder/transport/DSP | Implemented; mandatory MP3/WAV/FLAC, EOF/onset/loop and production keylock regressions PASS | Named mixed-rate/MT/manual-loop and controller transport gates passed | Broader physical seek/EOF/race/active-removal matrix NOT RUN |
| P08 pending startup health | Implemented; requested actual AP/HTTP readiness, timeout and rollback tests PASS | Named M3 injected timeout/rollback gates passed | Historical restart closed with operator-confirmed cable-intervention context; new-image lifecycle separate |
| P09 CI and qualification | Three-board/variant workflows and required no-SKIP fixtures implemented; full ordinary local matrix PASS | Physical Campaign A excluded by operator | Hosted CI SKIPPED BY OPERATOR (quota); fresh experimental firmware builds NOT RUN |
| P10 canonical documentation | Implemented; boards, provenance, migration, package/configuration manifests and review draft prepared | Exact-image operator records retained | Publication and remaining board acceptance records open |

The [local qualification and signed-delivery record](../validation/SHARED_P4_LOCAL_QUALIFICATION_20261011.md)
contains exact SHA, checks, sizes and hashes. It covers the complete no-SKIP
host suite, nine unchanged simulator baselines, two 300-virtual-second keylock
policies, three IDF 6.0.2 ordinary builds, unchanged locks, single BSP/core
isolation and image budgets. All three bundle/manifest signatures and debug
archives verify. Local evidence is not successful hosted CI.

## Prioritized residual work

| Priority | Item | Required next evidence | Treatment now |
| --- | --- | --- | --- |
| Closed operator context | Historical OTA restart | Operator confirmed moving the cable during the event; original evidence retained | Removed from open firmware defects; no repair claim or further test scheduled |
| High for a board release | Exact candidate configuration and installation | Selected topology, exact BIN/bundle/ELF and required board lifecycle/operator checks | New packages uninstalled and physically NOT RUN |
| Medium | Nested ordered playlists, stale artwork after media change, source memory loops and wider formats | Suitable actual export/media and bounded tests | Physical coverage gaps; no evidence here of an unimplemented shared module |
| Medium | USB throughput, FAT variants, physical races and active removal | Board-specific measurements and operator/hardware access | NOT RUN; do not substitute host fixtures |
| Medium | JC4880/JC1060 physical display/audio/topology | Exact board image and real hardware | NOT RUN; M3 evidence does not qualify JC boards |
| Deferred experiment | Recorder, Link/network and alternative UI | Fresh separate-config firmware builds, then appropriate physical acceptance | Default policies preserved; ordinary packaging rejects experiment images |
| Explicit partial scope | Hercules individual pad/FX/release/RGB/global-FX coverage | Separate scoped acceptance only if requested | Existing playback/controls gates retained; unsupported engine semantics documented |
| Operator decision | S2 remaining physical tests / Campaign A / hosted CI | Reauthorization only if the operator changes the decision | S2 closed with remaining checks waived; Campaign A excluded; CI skipped, none converted to PASS |

No new confirmed production firmware defect was established by this evidence
reconciliation. Incomplete coverage and deliberately deferred features are not
automatically implementation bugs. The historical restart is closed with the
operator's cable-intervention clarification; isolated output-late is monitored under the
existing rules, not described as zero-late.

## Migration and delivery handoff

1. Review the [prepared PR description](2026-10-11-shared-core-pr-description.md)
   and board configuration scope. Reviewable local packages are under
   `.cache/shared-delivery-5ff76b6c/`; retain `delivery-audit.json`,
   `delivery-sha256.json`, signed manifests and debug archives together.
   The `.cache` directory is ignored and is not included in a future PR.
2. Existing shared-identity M3 devices already use signed `main-deck-m3` OTA.
   This bench must not repeat the historical factory transition. Select a
   compatible ordinary or experimental configuration before any later install.
3. Historical `main-deck-p4` M3 devices need the one-time wired identity
   transition described in [preserving migration](../M3_SHARED_CORE_MIGRATION.md).
   Preserve the 16-MiB layout/settings and verified recovery artifacts; there
   is no full-chip erase. USB1 is power-only on the current bench, so no new
   wired operation is assumed available or requested by this handoff.
4. Legacy cue import is optional, explicitly tied to the old export and NVS
   backup; ambiguous records stay archived. See [cue migration](../M3_CUE_MIGRATION.md).
   Cache rebuild is independent and must not delete persistent cue data.
5. Keep the MIT repository licence and component notices, particularly
   `firmware/p4-core/components/sd_idle_wait/LICENSE`/`NOTICE` (Apache-2.0)
   and `firmware/p4-core/components/ui/DJ_UI_NOTICE.md`, with source delivery.
   Historical M3 recovery remains separate; no remote archive is performed.
6. Local review preparation does not push, create a remote PR, publish a tag,
   change public OTA discovery or overwrite existing assets. Those actions and
   a production claim require their own authorized scope and remaining gates.

The strict hosted-CI candidate creator remains strict. These local packages
must not be relabelled as its CI-qualified candidate output. The original
whole-core criterion remains open because its physical and experimental gates
are not all satisfied; this handoff is ready for review, not an unrestricted
production acceptance.
