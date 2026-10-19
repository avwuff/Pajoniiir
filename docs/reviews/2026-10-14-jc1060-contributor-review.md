# JC1060 / DDJ-400 contributor review

Date: 2026-10-14. Disposition: **review complete; selective integration requires
repairs. Do not merge the contributor branch wholesale.** This completes the
requested review step, not the subsequent integration, installation or beta step.
The existing JC4880/M3 benches, S2 operator closure and published assets retain
their recorded dispositions.

## Frozen inputs and method

| Input | Exact identity |
| --- | --- |
| Canonical repository | `dvucinozd/Pajoniiir` |
| Local review baseline | `78dc3b7d07966579b189f73700f9bff89c888d6b`, `codex/shared-p4-core-m3` |
| Contributor branch | `origin/kay/jc1060-hw-bench` |
| Reviewed contributor tip | `00041a0dd1c704781af4d991b663c05cdb6bd4c3` |
| Merge base / master at fetch | `8d56f303d13c06daa3d194ae1211785a8cdc33e3` |

The remote was fetched/pruned before review. All 71 changed files were extracted
byte-for-byte from the frozen Git object into an ignored review directory.
The net diff has 18,726 insertions and 52 deletions: three controller files,
ten firmware files, five tool files and 53 documentation/evidence files.
No contributor component was copied into the production tree. The four local
commits above master, including the later controller/LED epoch repair, remain
intact. No hardware command, OTA, release mutation or contributor message was sent.

The [71-file provenance inventory](evidence/2026-10-14-jc1060/provenance.json)
records every exported file's size and SHA-256. The contributor's
[original chronological record](https://github.com/dvucinozd/Pajoniiir/blob/00041a0dd1c704781af4d991b663c05cdb6bd4c3/docs/validation/jc1060/evidence/00-baseline/RESULT_shared_M2.5-221-ge8cea37e-dirty.md)
is retained unchanged in its Git history. Its old filename and opening image
header are not the identity of every subsequent test in that document.

Review used complete net production/tool diffs, per-image evidence reconciliation,
the actual stream/deck/parser modules compiled with host boundary stubs,
extracted production helpers, and offline JSON/line-ending audits. Helper
extraction is not a full RTOS/hardware reproduction; those limits are stated
below. Existing host tests cannot alone certify the new board configuration.

## Findings that block an unchanged integration

### R01 — P1: hub acceptance discards a known wrong root

Contributor `usb_storage_shared.c:21-39` calls
`usb_host_manager_device_matches_root(address, storage_root, !hub, &matches)`
and returns true whenever `storage_via_hub && rc == ESP_OK`. The manager's
contract returns `ESP_OK` for an observed device even when `matches` is false.
Therefore this accepts known storage on the controller root, not just a child
of the permitted storage hub. Recovery still targets the configured storage
root, so accepted media and the recovery target can disagree.

The exact helper was compiled and executed against that boundary contract:
`ESP_OK/matches=false` is accepted with the hub flag, rejected without it;
unknown devices remain rejected. This confirms the admission bug, not the
hardware cause of the contributor's missing parent metadata. Positive hub
enumeration on one bench does not validate the negative root cases.

Required repair: preserve trustworthy root ancestry through the common manager,
or fail closed while it is unresolved. Add direct/child right-root, wrong-root,
unknown/delayed publication and address-reuse regressions. Recovery must remain
root-specific. Do not import the current bypass or enable it for M3/JC4880.

### R02 — P1: HS UAC1 fallback also accepts a native 125-us endpoint

Contributor `controller_usb_audio_stream.c:212-269` falls back from UAC2 to
UAC1 and rewrites an HS `bInterval=4` to `1` for the existing 1-ms validator.
It leaves an original HS `bInterval=1` unchanged, so that also passes. The
stream's UAC1 packetization remains based on 1,000 service periods per second;
the original HS interval 1 is 125 us, requiring 8,000. Endpoint cadence and
packet size/rate then describe different clocks.

The production selector was tested with the captured DDJ-400 AS fields:
four channels, 44.1 kHz, 16-bit, synchronous OUT, 576-byte maximum, interval 4.
The contributor accepts that intended case **and** a mutation to interval 1;
the local baseline rejects both HS UAC1 cases. Interval 2 and qualified FLX4
HS remain rejected. Existing Hercules/S2 stream tests still pass, demonstrating
why those tests alone miss this admission error. These AS fields are a focused
fixture derived from `ddj400_lsusb_v.txt`, not a newly captured full descriptor.
The HS interval units are specified by the
[USB-IF USB 2.0 specification](https://www.usb.org/document-library/usb-20-specification).

Required repair: separate raw bus interval from service cadence. Initially
admit only the proven HS UAC1 synchronous 1-ms family (`bInterval=4`), with
packet/FIFO/sample-width limits, and reject other unqualified cadence/feedback
families. Do not rewrite the raw descriptor contract into an ambiguous FS value.
Exercise the public start/control/stream/stop path with the DDJ-400 descriptor,
in addition to selector negatives and existing FLX4/Hercules/S2 policies.

### R03 — P1: remote CUE tap is not admitted atomically

Contributor `web_server.c:2565-2581` queues CUE press and then release as two
separate zero-wait calls. With one queue slot available, the press succeeds and
the release fails. The HTTP operation reports failure after changing state.
There is no guaranteed remote release pending record in this path.

An extracted, unchanged CUE branch was run with one free queue slot and its
accepted press delivered to the real deck actor. It left `cue_held=true`,
`cue_preview=true` and playback active. Delivering a later release cleared them.
This is a deterministic queue-boundary reproduction, not a timed firmware
saturation test. The positive 12-cycle contributor test does not cover this case.
The baseline's missing release is a real bug; rejecting this non-atomic repair
does not dismiss the original report.

Required repair: one semantic tap command, or an atomic actor-owned press/release
batch with an explicit failure contract. Preserve physical CUE hold/release and
PLAY handoff. Test full queue, one slot, concurrent producers, both decks and
repeated remote CUE. Do not silently synthesize a global physical release.

### R04 — P1: USB MAIN loss leaves the output service terminal

This is an existing shared-core issue exposed by the contributor, not introduced
by its new mapping. `C-ddj400/c7_replug_uart.log` records the sink fault at
885.146 s, disconnect at 885.245 s and resolved local profile at 892.722 s.
The operator reports sound returning only after LOAD.

Current `audio_engine.c:3856-3878` marks every loaded deck error/EOF, stops
playing, clears paused, leaves `playback_finished=false`, and sets
`s_output_run=false`. The actual helper was executed with two loaded decks and
confirmed that terminal state. Code review of `audio_engine_play_for_deck()`
shows that PLAY does not restart the output service; existing restart calls
are in open/sink-switch paths. This explains the observed LOAD dependency.
The helper test does not simulate a real reconnect or decoder threads.

Required repair: define a recoverable sink-unavailable state and preserve track
ownership/position. Prefer stopped/paused recovery followed by explicit PLAY,
without unsolicited automatic playback. Restart once after the new controller
epoch has a healthy sink; retain fault behavior for actual fatal output errors.
Regress unplug/failed-start/replug/PLAY, repeated loss and independent PCM MAIN
versus USB MAIN policies. M3 headphones loss must not inherit JC1060 MAIN rules.

### R05 — P1: loop resizing can exceed the decoded track length

Existing `deck_core.c` LOOP_DOUBLE and loop-size encoder logic guards arithmetic
overflow but not track EOF; `audio_engine_deck_set_loop()` also stores the
requested range without length validation. The new DDJ-400 notes expose this
existing semantic action more directly.

The production deck suite was extended only in an ignored review harness:
a loaded 300,000-ms track with `[100000,250000)` doubles to an accepted end of
400,000 ms. The contributor separately reports `[45437,237949)` on a roughly
149-s track and playback stopping during H2. No timing or DSP claim is inferred
from the host stub.

Required repair: share one range validator for buttons, encoder, web and touch;
use decoded length when available, with an explicit unknown-length policy.
Refuse or musically resize an out-of-bounds request while preserving the prior
valid loop. Cover EOF, preroll/analysis length disagreement, overflow, both
decks, resize and loop exit. Validate the engine boundary as well as the actor.

### R06 — P1: Ethernet HTTP is started after pending-image acceptance

The Ethernet startup/Host-address changes fix an actual unavailable web path:
JC1060 has no Wi-Fi AP, while previously `wifi_link` alone started HTTP.
However, contributor `main/app_main.c` calls common startup before
`web_server_start()`. Common startup already calls
`firmware_health_mark_ready()`. Its saved `boot_network_required` condition
is `capabilities.wifi && settings.wifi_remote`, always false on JC1060.
HTTP startup failure only logs an error after the pending image is accepted.

The real startup-gate host suite confirms the intentional contract that
`network_required=false` becomes READY with `network_ready=false`; the JC1060
binding/order is the missing part. This is a confirmed startup ordering gap,
not evidence of an observed rollback failure. Contributor OTA group G is NOT RUN.

Required repair: provide board-aware requested network/HTTP readiness before
marking a pending image valid. Separate optional Link from required remote
management; preserve Wi-Fi-off behavior and the 60-s deadline. Test Ethernet
start failure, HTTP failure, DHCP readiness/timeout and a successful signed OTA.

### R07 — P2: lock hash replacement is a line-ending policy change

The two affected components remain pinned to the same Git commit
`cc65dc268f9fb6e89b8b3c6c9e94f5aa1dbb2ccb`. They are Git sources, despite the
`espressif/usb_host_msc` name. Using the installed component manager's real
`hash_dir()` with its manifest filters, normalization of current managed source
bytes produced **exactly** the two contributor hashes:

| Component | Current CRLF hash | Contributor / normalized LF hash |
| --- | --- | --- |
| `usb` | `46e3c0998d347f0ed9413ed9884b8be7393e3bb364bae3c87cdc21419aca9eab` | `6250cd00a0fc5cf760980d7d6ef09718044aee4b03bc4594d97ca5c019b7a56a` |
| `usb_host_msc` | `f5f178d5941a0ff4baa97ea8cfe0a7831731acc548773a2560ded1947a29a52e` | `77da0bd49685353d8de7e0515acf0f4aa2b827d526f8ffae27b3644c3d0da4d0` |

No file or managed component was modified during this audit. There is no
dependency upgrade, but replacing locks without controlling dependency Git
checkout line endings makes platform behavior depend on `core.autocrlf`.
A repository `.gitattributes` alone need not govern a separate dependency clone.
Choose deterministic dependency checkout bytes, then validate clean Windows
and Linux resolves/builds with all three committed locks unchanged. Do not
alternate locks per machine or disable checksum verification.

### R08 — P2: ordinary Library discards network-download progress text

Current `ui_library_update()` computes `DOWNLOAD ...`/`RESOLVING SOURCE` and
passes it to `ui_library_set_load_busy()`, whose body ends with `(void)hint`.
It only disables/enables LOAD buttons. Ordinary status text displays decoder
percentage only in `AE_LOADING`, later than the network transfer. This confirms
a code-level progress-presentation gap consistent with the reported symptom.
LOAD being disabled while a single-flight worker runs is intentional; a
permanent freeze and the failed "Le Matos" load are not diagnosed by this fact.

Cancellation/current-request checks and remote artwork decode paths already
exist. Source cycling invalidates a remote load, and the downloader checks
that identity. Do not describe those features as wholly absent. Required
follow-up: visible download state/cancel action with bounded worker completion,
tests for cancellation during each phase, late results, failed downloads,
peer loss/reappearance and missing artwork. Preserve audio/UI responsiveness.

### R09 — P2: HIL stop trusts a reusable PID without process identity

New `tools/hil/hil.sh` considers any live PID in `capture.pid` its capture,
then sends `kill` to it. A stale PID file and PID reuse can target a different
process; startup can also report an existing unrelated process as the capture.
No capture or kill command was executed in this review.

Record and verify process command/start identity before status/stop, handle
stale files and verify daemon startup. Select a stable explicit serial device
identity rather than the first `/dev/ttyUSB*` for multiple-board work. Serial
open DTR/RTS behavior should be explicit and wiring-specific. The HTTP helper
serves raw build files on all interfaces; it is not signed OTA qualification.

## Disposition of all nine contributor bug groups

| Contributor report | Reviewed disposition |
| --- | --- |
| C7 output recovery | Log/operator finding plus current production terminal-state reproduction; R04, shared repair required. |
| E9 SD after warm RST | Reported alternating failed/successful mounts on boots 24-34; earlier retained boot logs support timeouts. The full warm-reset journal is not retained here. Hardware defect remains open; root cause is not proven. |
| Playlist touch | Table callback selects rows only; opening a node is in the LOAD/open path. A row tap on already-selected first playlist does nothing. This supports a UI contract/discoverability gap, not parser corruption. Brief open/back behavior and generation reset cause remain unreproduced. Test tap/select/open/Back and sort/source generation explicitly. |
| Link progress/cancel/freeze/artwork | R08 confirms ignored progress text. Cancellation and artwork code exist; particular failure causes remain unproven. Playback from an already-downloaded track survives peer loss in audited F8 rows; in-progress loss was NOT RUN. |
| Loop ×2 past EOF | Reproduced against current production actor; R05. Applies to all semantic inputs, not only DDJ-400. |
| Memory cue write | Four memory store/delete profile inputs intentionally omitted because actions are absent. Memory-source display/import is distinct from local memory editing. Requirements gap, not an invalid profile or newly introduced loss. |
| Missing LOAD LOCK | Current default-ON guard and ordinary Library locked captions exist. A dedicated ordinary Settings toggle is absent; alternate DJ UI has a field. Manual wording promises a setting more broadly than implemented. Contributor D10 remains BLOCKED, not proof that load admission is absent. |
| Observability | Legacy `PHASE1 STATUS` validator is not a fit for this hub/runtime format. ELF is in `/api/firmware`; it need not also appear in `/api/status`. WARN-only/empty UART captures do not prove event absence. No-stick recovery spam needs bounded state logging; R01 remains the actual root-boundary defect. |
| Jog queue / scratch range | UART shows drops reaching 47 and `scratch begin ... cue pre-roll pending`. This confirms backpressure/rejected scratch in that window, not one proven cause for all jog behavior. Queue 32 and scratch history are bounded designs; prioritize/coalesce high-rate work without losing edges, and document the approximately 4.35-s history limit. Both speculative contributor fixes were reverted. |

The shared JC peripheral SD mount path retries three times and can install a
Hosted-dependent no-op host initializer. Existing slot teardown/power semantics
and warm-card state deserve inspection on the actual JC1060. Do not copy an M3
C6/SD reset sequence or globally deinitialize the shared controller based only
on the alternating symptom. Required evidence: exact warm/cold reset type,
card/slot/LDO/host lifetime, cleanup return codes and reproduction on Kayrosen's
board. Cold-boot playback success does not close E9.

The reported jog drops occurred on an earlier image. Local `c7d2e9cd` adds
later controller/deferred-work epoch protection but does not establish a fix
for high-rate actor saturation or USB MAIN recovery. Neither old contributor
PASS nor our newer host PASS transfers physical acceptance across those images.

## DDJ-400 profile audit

The four added note bindings are `90/51`, `90/53`, `91/51`, `91/53`, mapping
to each deck's loop halve/double. The source compiles byte-identically to the
committed v4 binary: **6,112 bytes**, SHA-256
`2fc922ec2986249f4af7c7a077367e4bb5b9e9f22ced5b3d137e853d21f74408`.
Its 126 source input nodes expand to **284 binary inputs**; 29 source output
nodes expand to **127 outputs**. It stays within limits 320/160, with 18 pair
slots and 12 bytes of initialization SysEx. Production parser and storage
metadata validator both accept it; both reject a corrupted CRC. All four new
bindings produce the expected press/release semantic IDs in the real mapper.

The 64-byte growth is four input records. Remaining four omitted inputs are
memory store/delete. Runtime-preferred sample rate still follows descriptors
and selection policy; descriptive JSON audio metadata is not a clock command.
The profile has useful contributor hardware evidence, but its README still
says all hardware/halve-double acceptance NOT RUN. Reconcile that wording to
the exact tested branch image while retaining canonical/new-image gates.
Fix loop EOF validation before promoting the new loop controls to users.

## Physical and telemetry evidence reconciliation

One contributor unit: Guition **JC1060P470C_I_W_Y**, DDJ-400 directly on
controller root0, stick through a self-powered hub on storage root1, Ethernet
and Vynull as a simulated Link peer. No real CDJ tempo master was tested.
The final library contains 22 tracks, all MP3 44.1 kHz; mixed-rate/WAV/FLAC
qualification is NOT RUN in this campaign. Do not substitute our 260-track
Hercules/JC4880 bench or M3 hardware topology for this evidence.

Final contributor image 10 has clean source
`4e3aa2b41ebcebdedb7ab9e25715553a6bff9416`, application size **2,663,152 B**,
BIN SHA-256 `198541f03e1331b7a68ce8e67192b36f2b3ad436cef5c9ddece21b6f97616a45`,
ELF SHA-256 `08e56f320fd473fc1046950ba54922ac8a0724e17ede15255372c884511c736e`.
Reported wired flash offset is `0x20000`; this is provenance, not a flashing
instruction for another layout. The 0x380000 budget has 1,006,864 B headroom.
It reports factory/OTA idle, not pending-OTA VALID qualification.
Version string remains `M2.5-221-ge8cea37e-dirty` despite the clean later SHA.
That inconsistency requires version/configure provenance reconciliation; the
suffix alone does not override the recorded `source_dirty=false` identity.

| Record | Duration / samples | Audited scope |
| --- | --- | --- |
| O5, image 6 / `4ca426d8` | 120.111 s / 480 | Observe returns pass but stopped UAC underflow grows **5,289,178**; not zero-loss campaign evidence. |
| D7, image 7 / `9f362cf3` | 76.343 s / 182 | All 12 operations retained; raw samples have both decks playing and no changes in eight audited audio/late counters, flags zero. Queue-pressure CUE negatives absent. |
| D8, image 10 / `4e3aa2b4` | 600.007 s / 2,397 | Seven declared deltas independently recalculated zero; operator listening separately reported. |
| E9 cold-boot playback, image 10 | 1800.207 s / 7,183 | Seven deltas zero, listening reported; warm-reset mount gate still FAIL. |
| H2, image 10 | 3600.163 s / 14,378 | Seven deltas zero; operator listening reported. Only 97.8% dual-playing is reported, after oversized loops stopped playback near the start. Not uninterrupted dual workload acceptance. |
| H3, image 10 | 14400.093 s / 57,493 | Seven deltas zero, 100% dual-playing and listening reported. No reset reported; status latency maximum 1,501.468 ms. Scoped long Observe evidence. |
| F8 peer loss / return | 114.02 s / 115 retained rows | Both decks playing in all rows; PCM/late/allocation/SD deltas zero; max position-vs-uptime-step deviation 22/23 ms. File has 115 rows, versus 106 stated in the prose. In-progress download loss and automatic browse recovery not qualified. |

All five Observe result objects have `timing_workload.samples=0` and
`physical_operator_acceptance="NOT RUN"`. Later prose supplies listening
confirmations; preserve that separate origin. D8/E9/H2/H3 contain the same
clean source/ELF and seven baseline/final counters, with all declared deltas
matching arithmetic. These seven omit long-window ISO packet/frame counters
and the full current controller/USB strict set. They cannot become an all-22
TimingSoak PASS. D7 does retain ISO counters for its short window.

Large raw long-window files are not in the branch. O5 and D8 have raw-file
hashes/locations reported; the review did not retrieve those external files.
H2/H3 occupancy/uptime assertions are contributor reports, not an independent
audit of 57,493 raw rows. Empty WARN UART files merely show no captured WARN
line. The overview audit preserves both successful and failed earlier results
rather than relabeling the entire campaign zero-loss.

OTA group G, wrong-board/signature/upload-interruption/rollback testing,
hotplug/insertion-order/brownout matrix, GPT, real Link tempo master and broader
controller qualification remain open. Contributor source `4e3aa2b4` is neither
published beta source `40e72fcb` nor current local `78dc3b7d`.

## Net-change integration matrix

| Change / contributor commits | Disposition and acceptance before promotion |
| --- | --- |
| DDJ-400 loop notes, `97ca2d8d` | Keep mapping intent and regenerated v4 artifact after R05; retain memory omissions and exact-image scope. |
| HS UAC1, `0ad149bc` | Rework R02; positive DDJ-400 and cadence/feedback negatives plus all existing stream policies. |
| Hub storage, `9aa6d36a` | Do not import bypass; redesign R01 root provenance and recovery. |
| Ethernet HTTP, `bec51a40` | Keep service/Host-address intent; finish R06 readiness contract before signed board OTA acceptance. AP priority/address fallback still needs host-policy regression. |
| Web CUE, `9f362cf3` | Preserve tap semantics with R03 atomic admission and physical hold/handoff regressions. |
| Three lock files, `ec655c61` / `83ecc998` | Resolve R07 dependency-clone line endings first; no version upgrade. Clean Windows/Linux unchanged-lock matrix required. |
| Linux storage build helper | Accept portability intent; ESP-IDF 6.0.2 check remains, no flashing behavior. |
| Docker tools | PowerShell 7.6.6 tarball checksum matches the official release; syntax review only. Docker build/runtime NOT RUN. Base tag/apt packages are not fully immutable. |
| HIL tools | Useful contributor capture helper after R09; no serial/daemon/serve/stop run in this review. |
| Validation plan/logs | Preserve originals; add per-image provenance/scope reconciliation. Do not change failed gates into PASS. |
| MIDI 3-ms pacing and jog 40-ms grouping | Both reverted (`d74262c4`, `3e33a70f`); net production code absent. Do not cherry-pick these experiments. |

Official PowerShell checksum reference:
[v7.6.6 release](https://github.com/PowerShell/PowerShell/releases/tag/v7.6.6).
The pinned `powershell-7.6.6-linux-x64.tar.gz` SHA-256 is
`ddbc4a2d113bbd46d283cfedcbcd117a70caefd7673f41f2b4e0000badf103bc`.

## Verification completed for this review

- Production stream suite on current JC4880 policy: PASS.
- Production M3-HS stream suite with frozen baseline and contributor selector:
  PASS, with interval-admission difference reproduced; existing captured
  Hercules/S2, feedback, cleanup and 30-host-reconnect checks pass.
- Production deck actor suite with loop EOF and partial CUE admission
  reproductions: PASS as an expected-defect reproduction, not a repaired gate.
- Extracted production storage route and output sink-fault helpers: expected
  defects reproduced. Stub boundaries and scope documented above.
- DDJ-400 compile identity, production parse/map and storage-validator CRC:
  PASS; four new bindings exercised on both decks.
- Real startup gate suite: PASS; Ethernet call ordering audited separately.
- Library helper host suite: PASS, **48 checks**. It does not execute the live
  LVGL table callback, so it cannot certify the reported playlist gesture.
- Five monitor arithmetic/identity audits, 182 D7 raw samples, 115 F8 raw rows,
  and real component-manager LF/CRLF hash reproduction: PASS for stated scope.
- Both new Bash scripts: `bash -n` PASS. Docker/serial operation NOT RUN.

Sources/commands/results are retained in the
[review reproduction log](evidence/2026-10-14-jc1060/reproduction-run.txt),
[JC4880 stream log](evidence/2026-10-14-jc1060/jc4880-stream.txt),
[reproduction manifest](evidence/2026-10-14-jc1060/reproductions.json) and
[offline evidence audit](evidence/2026-10-14-jc1060/evidence-audit.json).
Reproduction helpers are preserved alongside these records. On the unchanged
Windows baseline, run `extract_and_audit.py`, `run_reproductions.py`, then
`audit_evidence.py` with the IDF Python environment; the scripts use the pinned
Git objects and write only `.cache/jc1060-review-20261014`. The production
reproduction script refuses a changed firmware/test/compiler baseline.
Dependency hash audit additionally needs the existing managed components.
`SHA256.json` inventories the eight retained review evidence/helper files.

No production file changed in this review. New clean ESP-IDF builds, the full
host matrix, live hardware tests, signed OTA and hosted CI were **NOT RUN** for
an integrated contributor candidate, because that candidate does not yet exist.
Documentation integrity check: PASS (137 Markdown/HTML files, 134 retired
documentation paths checked). The tracked diff check also passes; the complete
staged diff is checked before committing the new report and evidence.

## Bounded next step

Selective integration should first implement regression-backed R01/R02/R03,
R04/R05 and R06, then reconcile dependency-clone bytes and profile artifacts.
Retain current controller epoch fixes. Follow with ordinary three-target
build/lock/isolation/budget checks and affected software tests. Release-blocking
JC1060 warm-reset/storage and reconnect behavior need Kayrosen's exact-candidate
hardware evidence; wider optional Link/UI gaps can retain an explicit
experimental disposition rather than being advertised as accepted.

A new beta needs new source/version/ELF, immutable packages and a board-specific
acceptance ledger. Historical Observe listening evidence is useful but does not
qualify that future image. This review neither starts integration automatically
nor reopens the user's waived JC4880 soak or operator-closed S2 tests.
