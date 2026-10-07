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
default gate, not a release qualification invocation. M3-only build/packaging
are pending.
JC4880/JC1060 builds remain deferred until the M3 operator retest, as requested.
GitHub Actions remains skipped by operator; no production PASS is inferred.

## Required physical retest

On the exact repaired installed image, allow stopped decks to idle for two
minutes without touch/controller/web mutations. Confirm first FLX4 PLAY wakes
only and leaves both decks stopped; subsequent PLAY starts the intended deck.
After another idle timeout, confirm jog/Browse wake and touch wake still work.
Confirm restored prior screen and no accidental load, cue or stuck held state.
Physical observations and image hashes must be recorded before closing this gate.
The final soak remains deferred until the end of the remaining work.
