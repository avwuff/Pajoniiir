# M3 controller screensaver wake regression — 2026-10-07

## Reproduction and cause

The operator reported on installed clean `082d2c0a` that the two-minute idle
screensaver dismisses on touch but does not dismiss on FLX4 input. The shared
`control_link_local.c` submitted semantics directly with xQueueSend, bypassing
`deck_core_queue_event` and its UI activity callback. The frozen historical M3
source e95417c4 includes a local activity callback introduced in 435bcfe; that
explicit direct-controller wake handling was lost during shared-core extraction.

## Repair

The transport-neutral control link now requires an event sink at initialization,
instead of accepting a raw queue. Shared startup binds it to the existing
`deck_core_queue_event` after registering `ui_activity_notice`, before USB input
starts. Buttons, jog, Browse and pitch therefore reset the same idle timer and
consume the physical wake event before playback/controller mutation.
No UI timeout, display policy, audio DSP or hardware topology is changed.

Physical link STATE notifications bypass activity consumption in the deck queue:
connect/disconnect must reach held-state cleanup while the display is covered,
and are not operator activity. Authenticated remote actions retain the existing
wake-without-consumption policy.

## Software validation

Focused `deck_core_dual`, `deck_core_dual_scratch` and `ui_idle` passed, exit 0.
The new regression compiles the real local semantic adapter and production deck
queue policy, with a capturing host RTOS boundary. It checks button/jog/Browse/
pitch wake and consumption, next PLAY reaching production transport, covered
disconnect forwarding without activity, and unchanged authenticated remote
behavior. The UI callback and audio output are fakes; this is not physical wake
acceptance. The initial runner used an unknown suite name and the first test
compile used an incorrect pitch identifier; corrected focused runs pass.

The full default shared host suite also completed with exit 0. This is the
default gate, not a release qualification invocation. M3-only ESP-IDF 6.0.2
incremental reconfigure/build, board isolation/identity/budget, unchanged
dependency lock, signed bundle/manifest verification and documentation integrity
passed. No JC4880/JC1060 build or new soak was run.
JC4880/JC1060 builds remain deferred until the M3 operator retest, as requested.
GitHub Actions remains skipped by operator; no production PASS is inferred.

## Installed candidate

Local signed OTA returned HTTP 200 and confirmed clean
`4620b83cb652974c0cca31aea99632e5530ef749`, `M3-dev-g4620b83cb652`,
VALID ota_0 / OTA idle. Image: 2573824 bytes,
SHA-256 `5c1ce6a2f741ec273e561a017c91d2ffb1083c1ff11ee98003db658211a44e60`.
Bundle: 2574012 bytes,
SHA-256 `e605fe2244a425df1a251a27db56dcb220529c6e99b0ae4f4474603e6fb3e557`.
ELF: `5a417397815478d4b01f4f0cd5c9fe3a0dd62d0fe61340726c569157cc3b4461`.
Package: `releases/m3-bench-4620b83c/pajoniiir-m3-M3-dev-g4620b83cb652/`.
Artifacts: `.cache/m3-migration/20261007-4620b83c/ota-install.json` and
`screensaver-wake/` firmware/preparation/resource captures. D1 France Gall
and D2 Jura Stublić are READY/stopped, library 324; Wi-Fi remains enabled.
The physical retest FAILED: the operator confirmed the screensaver was visible
and PLAY was the first input, yet the track started immediately. The assistant
stopped both decks and saved `first-play-started-status.json` and
`first-play-stopped-status.json`. Do not qualify 4620b83c from host/OTA success.

## Follow-up atomic wake handoff

Code inspection found a further race in `ui_activity_notice`: it wrote the
activity flag before reading published visibility. The UI task could process
that notification and clear visibility before the producer returned, allowing
the wake press through. The two separate volatile flags are replaced by a
shared atomic bitfield: fetch-or both posts activity and returns the preceding
visibility. UI fetch-and consumes only activity; show/hide changes only the
visibility bit, preserving concurrent activity. This removes the observed
code ordering race; its responsibility for the physical failure remains an
inference until the next exact-image retest.

Production `ui_idle` host tests cover captured pre-wake visibility surviving
UI dismissal, next input forwarding, pending activity preservation and read-only
visibility. Small RAM-only deck input counters and published visibility are
added to the existing read-only `/api/ui-timing` response for a before/after
physical test. They do not log per event, write NVS or run a new monitor.
Follow-up focused and full default host gates passed, exit 0. M3-only IDF 6.0.2
incremental reconfigure/build, identity/isolation/budget, unchanged lock,
signed image/bundle/manifest and documentation checks also passed.
Local signed OTA returned HTTP 200 and installed clean
`af4ca59007e3cbd55453bf2d767eff412a7bba93`, `M3-dev-gaf4ca59007e3`,
VALID ota_1 / OTA idle. Image: 2574192 bytes,
SHA-256 `7a6d96069f1a2c0a66d40f37e2a579765bd7aba178befbb24c3c897ec82c76e7`.
Bundle: 2574380 bytes,
SHA-256 `ef4cc645a1824f2f3b9efe02dc401453a2da7a6d9e9e5bdeab9787c9c99d803b`.
ELF: `79a83af8e6b85f9ad8d75ea33bb94025b31ade0138337d829e8b330b7709d8d2`.
Artifacts: `.cache/m3-migration/20261007-af4ca590/`; before-idle timing confirms
shown=false, physical_events=0 and wake_consumed=0 with both decks stopped.
A read-only helper confirmed published shown=true, both decks stopped and all
physical/wake counters zero before requesting the first PLAY; no web mutation
was issued during that wait. The operator confirmed GUI wake with both decks
stopped, then confirmed subsequent PLAY operates normally. The later API
snapshot shows shown=false, wake_consumed=1 and last_consumed_id=16 (D1 PLAY),
with both decks stopped. It already contains six physical events and position
8602 ms, so it is not an isolated one-press trace and does not prove zero
position movement throughout the interaction. Subsequent PLAY acceptance is
operator-confirmed; its later snapshot also finds the deck already stopped.
Both snapshots are preserved; no artificial playback repetition substitutes
for these observations.

## Physical outcome on af4ca590: PASS for exercised wake paths

The operator also confirmed first Browse press dismisses the screensaver.
The after snapshot reports wake_consumed=2, last_consumed_id=99 (Browse press),
shown=false and both decks stopped. The read-only pre-Browse observer did not
capture shown=true before the operator acted; it was canceled to avoid labeling
a later idle interval as pre-Browse evidence. Its separate completion records
that limitation; the operator observation and counter increment remain valid.

After the next idle screensaver, the operator confirmed touch restores GUI
without unintended action. Final API snapshots show shown=false, both decks
stopped, unchanged physical_events=8 and wake_consumed=2, and the same clean
af4ca590 VALID ota_1 / OTA idle identity. Touch does not fabricate a controller
event. Individual operator verification JSON files preserve each observation.

The reported controller-wake bug is closed for PLAY and Browse plus touch
regression on this exact image. Physical jog/pitch/fader wake were not separately
exercised; the shared-path host regression covers jog and pitch. No other-board
hardware qualification or final soak is inferred. The earlier 4620b83c failure
and all snapshot/observer limits remain preserved.

## Operator procedure

On the exact repaired installed image, allow stopped decks to idle for two
minutes without touch/controller/web mutations. Confirm first FLX4 PLAY wakes
only and leaves both decks stopped; subsequent PLAY starts the intended deck.
After another idle timeout, confirm jog/Browse wake and touch wake still work.
Confirm restored prior screen and no accidental load, cue or stuck held state.
Physical observations and image hashes must be recorded before closing this gate.
The final soak remains deferred until the end of the remaining work.
