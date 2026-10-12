# Controller lifecycle software follow-up

Date: 2026-10-12. Baseline: `8d56f303d13c06daa3d194ae1211785a8cdc33e3`.
Scope: the operator requested software review of held-state recovery, LED output
queues and LOAD/PLAY ordering. The combined physical Hercules Pad FX/global
FX/LED/VU block is deferred until the operator participates. No device controls,
playback, USB connections, firmware installation or publication were changed.

## Reproduced defects and repairs

### P1: Unaccepted commands survived a controller disconnect

`controller_runtime_set_connected(false)` released durable held states but kept
the bounded action buffer, retry event and partial analog replay snapshot. With
downstream backpressure, old PLAY/LOAD/jog work could execute after reconnect.
The application disconnect drain stops when no event can be accepted, so it
does not resolve this case. Connection notifications alone do not cancel the
remaining discrete commands.

Disconnect now retires unaccepted buffered/retry/snapshot work under the runtime
mutex. Queue drop/coalescing history is preserved. Durable held release levels
and the ordered disconnect notification remain pending until accepted. New
input after reconnect still dispatches normally.

Callbacks run outside the mutex. A captured connection generation now fences
their completion bookkeeping: an old failed callback cannot recreate a retry
after unplug/replug, an accepted old snapshot cannot advance the cleared replay
cursor, and an old held-state acknowledgement cannot retire a newer connection's
reconciliation. An event already accepted by the downstream sink is not undone;
this repair cancels work not yet accepted.

New production-runtime regressions cover queued LOAD/jog plus a failed PLAY
retry, durable platter release, fresh PLAY after reconnect, disconnect/reconnect
inside a failing callback, and disconnect inside an accepted partial snapshot.
The new stale-action regression fails on the baseline implementation and passes
with the repair. An artifact-only baseline run also fails the partial snapshot
assertion; removing only the in-flight retry fence fails its specific assertion.
These are host reproductions, not observed device failures.

### P2: LED mapping and its shifted bank could cross an output generation

`controller_led_runtime_send` previously called the generic packet sender
separately for the primary and FLX4 shifted mirror. Each call captured the
current output generation. A reconnect between mapping/sends could therefore
admit an old controller address through the replacement connection.

The LED runtime now captures the output generation before mapping and sends
both packets through `controller_usb_host_send_packet_for_generation` using
that same generation. A departed generation returns an error; it is not silently
acknowledged. The existing LED publisher retries failed state changes on its
next publication. A regression reconnects between primary and mirror: the old
implementation accepts both packets, while the repaired implementation rejects
the old mirror and permits a fresh complete send.

## Reviewed paths that retain their behavior

- The routed USB host closes producer admission before resetting the output
  queue, waits with `vTaskDelay(1)` for active producers, and tags queued packets
  with the host generation. Enqueue remains nonblocking; capacity failures
  remain counted. Its product MIDI OUT depth stays **256**.
- The real FLX4 LED publisher and mapper produce **162 packets** for a forced
  two-deck snapshot including shifted pad banks. Two VU and two LOAD packets
  leave **90 slots** in a modeled 256-slot output sink. Unchanged values are
  suppressed, and a changed PLAY LED rejected by a full sink is retried after
  space returns. This exercises production mapping/publication with an injected
  bounded sink; it does not measure USB wire throughput or arbitrary bursts.
- LOAD admission uses actual target-deck audio transport. The asynchronous UI
  worker reserves replacement after metadata preparation and before resetting
  the deck or replacing the audio session/loaded-track snapshot. PLAY starting
  while metadata is read therefore prevents replacement when LOAD LOCK is on.
- The production deck test now runs both target-deck interleavings: PLAY wins
  and refuses LOAD; LOAD wins and blocks target PLAY/CUE while the other deck
  remains usable. Invalid/stale reservation completion cannot release a newer
  replacement. Explicit LOAD LOCK OFF retains its documented behavior.
  These execute production deck logic against the audio stub; they are not a
  physical timing or asynchronous storage qualification.

## Verification and evidence

Evidence directory: `.cache/controller-software-gates-20261012/`. It retains
baseline/fixed runtime and LED logs, negative sensitivity source/logs, focused
deck/USB output checks, full required-fixture host qualification, M3 build and
board verification, and source/artifact hashes.

| Check | Result |
| --- | --- |
| Controller runtime, builtin/profile/parity, held reconciliation and concurrent callback tests | PASS, exit 0; extended builtin harness 247 checks |
| LED runtime + production snapshot publisher + modeled bounded sink | PASS, exit 0; 55 checks / 176 send attempts |
| Routed output gate/reset and USB audio host suites | PASS, exit 0 |
| LOAD LOCK and production dual-deck tests, scratch disabled/enabled | PASS, exit 0; both target-deck orderings covered |
| Full `run_p4_host_tests.ps1 -Qualification` | PASS, exit 0; required fixtures present, no SKIP |
| Fresh ordinary M3 ESP-IDF v6.0.2 configure/build | PASS, exit 0 |
| M3 board/component/project/budget verifier | PASS, exit 0 |
| Documentation integrity and `git diff --check` | PASS; 135 Markdown/HTML files checked |

The M3 build uses an empty `build_review_controller_20261012` directory and
dedicated SDKCONFIG derived from the existing ordinary defaults. It is a
development verification build on baseline `8d56f303` with the working-tree
repair (`source_dirty: 1`), not a clean release or installed image. Version
`M3-dev-g8d56f303d13c-dirty` occupies **2,601,200 bytes**, leaving **1,068,816
bytes** below `0x380000`. BIN SHA-256:
`5D546569C359DF6B8DC15CD892109F10D0E4FB259C576E5DAFF9CAB21BF87422`.
M3 dependency lock SHA-256 remains
`D034FE6B34C26D451D75878546947B95DBA2A0FEDF9F53D10216182784F14F3C`.
Source hashes and the patch distinguish the verified working-tree repair from
the baseline SHA; committing this repair does not relabel this build as clean.

## Delivery boundaries

Only M3 is built for this repair, as requested for debugging changes. JC4880 and
JC1060 are not newly built or physically tested in this follow-up. Existing
board/controller acceptance and explicit waivers retain their exact historical
image/configuration scope. S2 remains operator-closed; JC4880 long soak and
hosted CI remain operator-skipped, and JC1060 physical work remains deferred.

No OTA, packaging, release asset replacement, stable channel change or new soak
is performed. Published `shared-p4-beta.1` remains unchanged. The next joint
step is the short Hercules Pad FX press/release/global FX/LED/VU block; if it
uses these firmware repairs, it first requires an explicitly identified fresh
candidate and installed-image verification.
