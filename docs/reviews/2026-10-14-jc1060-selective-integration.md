# JC1060 contributor changes: selective integration

This implements the software follow-up to the
[frozen contributor review](2026-10-14-jc1060-contributor-review.md).
The donor is `00041a0dd1c704781af4d991b663c05cdb6bd4c3`; integration starts
from review commit `1a0f8528c75d2057928dafb2de0d06d6561eed9b` in the canonical
shared-core checkout. It is a selective adaptation, not a branch merge.
Original donor evidence, licenses and changed-file hashes remain in the review.

## Implemented changes

| Finding | Implementation and regression boundary |
| --- | --- |
| R01 root admission | Preserve the live parent device handle in the pinned USB hub owner. Resolve bounded ancestry in the common manager; reject unknown, ambiguous, cyclic or wrong-root metadata. Retire descendants when a parent disappears or its address is reused. JC1060 may mount through a hub on its storage root; M3/JC4880 retain direct-storage policy. The MSC adapter always honors the resolved match. |
| R02 HS UAC1 | Keep raw `bInterval=4` for the DDJ-400 synchronous 1-ms family. Reject native 125-us UAC1 endpoints, adaptive/feedback families and qualified FLX4 on HS. Existing UAC2 and FS policies remain. Production stream tests execute start/control/packetization/write/stop, including fractional 44.1-kHz cadence. |
| R03 remote CUE | Admit one internal CUE tap event rather than separately queued press/release. Both transitions execute in one deck actor turn. Full/one-slot queues cannot orphan a press; physical held CUE ownership is preserved. No controller-profile ABI changes. |
| R04 USB MAIN recovery | Pause loaded playing decks at the last audible head when USB MAIN disappears. Preserve the output task, decoder ownership and EOF/drain state. Reconnect does not autoplay; explicit PLAY on a healthy sink re-seeks and resumes without LOAD. Handle the write/disconnect race and failed resume seek. Independent PCM MAIN is unaffected; genuinely fatal sink errors retain terminal handling. |
| R05 loop EOF | Validate ordered loop ranges against the known decoded duration at the engine boundary and the actor's same-session metadata boundary. Rejected resize retains the old range/LED selection. Test both decks, exact EOF, decoded-vs-analysis length, manual loop and unknown-length fallback. |
| R06 startup readiness | JC1060 provides HTTP startup/readiness to the common startup sequence before pending-image acceptance. Required Ethernet must have an up netif, nonzero DHCP address and running HTTP. Requested Wi-Fi remains an additional condition. Missing controller/disk does not gate acceptance. Test timeout, allocation/start failure, absent DHCP, optional provider and already accepted/factory images. |
| R07 dependency bytes | Preserve all three committed locks and pinned versions. CI already requests CRLF Git checkouts. Scope that same policy to CMake/resolver child processes while retaining inherited Git settings; do not mutate the host's global Git configuration. A real Git clone regression verifies the bytes. |
| R08 library progress/retirement | Render resolving/download progress after ordinary and alternative Library status rendering. Retire canceled worker results without leaving LOAD buttons disabled. A stale result cannot release the newer job's busy state. No new table gesture or speculative artwork fix. |
| R09 HIL PID reuse | Excluded. Unsafe Linux HIL/Docker tooling is not imported; no claim that the donor helper is repaired or qualified. |

The root metadata transform composes with the existing indexed idle-recovery
transform. It fails configure when pinned anchors/source selection drift; it
does not edit managed components. Board verification checks that the generated
patched hub is the selected compiled source. The linked restart-capture checker
now verifies the provider entry, ordinary delegation when linked, and the real
board `app_main` path. Panic/restart hooks still require direct initialization,
IRAM placement, actual interception and the original cache-lock/CSR contracts.

Web Host validation keeps active AP priority and also accepts the live board
provider address when both transports are requested. Both paths use the same
strict host/port parser; inactive Ethernet, suffix spoofing and malformed ports
are rejected. No wildcard host acceptance is introduced.

DDJ-400 notes `90/51`, `90/53`, `91/51`, `91/53` implement loop halve/double
on both decks. The regenerated S3CP v4 binary has 284 inputs, 127 outputs and
6,112 bytes, SHA-256
`2fc922ec2986249f4af7c7a077367e4bb5b9e9f22ced5b3d137e853d21f74408`.
Production parser/storage CRC and the four press/release mappings are tested.
Four memory-store/delete inputs remain explicitly omitted.
The storage experiment build helper can use an already initialized Linux IDF
environment; its existing ESP-IDF 6.0.2 requirement remains.

## Verification ledger

The ordinary software matrix, clean `c0ee8152` source identity and evidence
inventory are recorded in the [completed verification record](2026-10-14-jc1060-integration-verification.md).
Focused production regressions cover the table above, existing FLX4/Hercules/S2
stream behavior, controller epochs, USB teardown and firmware lifecycle.
Screenshot gates use the existing baselines; no baseline update is authorized
or needed for these changes.

The DDJ-400 HS fixture is a focused 95-byte audio-stream descriptor fixture
derived from the frozen descriptor evidence; it is not a complete physical USB
capture. CUE tests execute the real deck actor. The USB MAIN, network startup
and UI worker tests execute production functions with explicit RTOS/decoder,
netif/HTTP or LVGL boundaries stubbed. They verify software ownership/admission
and do not establish acoustic, electrical, timing or hardware acceptance.

## Remaining gates and delivery limits

- JC1060 SD warm reset is still open. IDF already uses slot-specific SDMMC
  deinitialization; a global host reset would conflict with shared Hosted
  ownership. No speculative reset fix is imported without causal evidence.
- JC1060/DDJ-400 MIDI/LED/audio/hub/reconnect, Ethernet DHCP and signed OTA
  readiness/rollback need the exact integrated image on Kayrosen's hardware.
  Historical `4e3aa2b4` Observe evidence does not qualify this source.
- Live Link peer loss during a download, the reported individual failed-track
  load/artwork/source-return cases and wider table gestures remain experimental.
  The two confirmed UI defects above are the bounded software repair.
- Full Linux/Docker clean builds, hosted CI, physical tests, signed packaging
  and installation are NOT RUN in this follow-up. Existing locks are retained;
  cross-host byte policy is tested locally, not certified by an unrun matrix.
- S2 remains operator-closed. The JC4880 long-soak waiver is retained, not PASS.
  New ordinary M3/JC4880 builds do not inherit historical physical acceptance.

No firmware is flashed, no release assets or public OTA channels are changed,
and no branch is pushed by this implementation step. README images and existing
operating instructions are retained. A future beta needs its own clean source,
version, ELF/package hashes and board-specific acceptance ledger.
