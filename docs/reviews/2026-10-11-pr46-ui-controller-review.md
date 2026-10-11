# PR 46 UI and controller review, 2026-10-11

## Scope

CodeRabbit CLI 0.6.5 reviewed clean local source
`ada14dea779ebd887955ba05fdb6667a3361eb84` against base
`9af99cd234e775521f3ec83c0104f5c6a0c72920`. The UI directory review covered
89 files and reported 10 issues (3 major, 7 minor). The controller-runtime
directory covered 7 files and reported no issues. Both service results were
`completed_with_warnings`, with the service's unverified-findings disclaimer.
They do not constitute whole-PR approval. The full request previously exceeded
the service's 150-file limit.

The separate USB-audio review covered 13 files and reported one major issue,
with service outcome `completed`. It used the same source and base above.

Raw outputs are retained at `.cache/coderabbit-pr46-ui-ada14dea.ndjson` and
`.cache/coderabbit-pr46-controller-ada14dea.ndjson`.
The USB-audio output is `.cache/coderabbit-pr46-usb-audio-ada14dea.ndjson`.

## UI dispositions

| Finding | Disposition |
| --- | --- |
| Major: native Settings widgets absent in runtime DJ Overview | Guard record, wireless, Link and peer widgets. This affects the experimental Overview configuration; the service's claim of an ordinary production boot crash is not established. |
| Major: retained deck positions treated as fresh samples | The DJ bridge uses the observed-sample interpolation API and preserves the retained flag, matching the ordinary Overview path. |
| Major: RGB565 rectangle fill omits paired-buffer dirty tracking | Mark the physical rectangle when two framebuffers are configured and propagate cache-sync errors. |
| Minor: loading progress remains associated with D1 | Assign the requested deck in asynchronous submission and simulator/local load entry points, including remote loads. |
| Minor: MAIN-sink rejection permanently overwrites active-sink label | Report the rejection through the bounded status message; keep the active sink label accurate. |
| Minor: restored-screen declaration outside C linkage | Move the declaration inside the header's `extern "C"` block. |
| Minor: callback-free splash deletes the active screen | Keep it active without a timeout; with a callback, delete only after another screen becomes active. |
| Minor: fallback beat clock accumulates integer truncation | Use rational 64-bit position/BPM arithmetic. Regression covers exact 128-BPM beat/bar boundaries at one minute and one hour. |
| Minor: generated font headers contain a personal absolute path | Retain conversion options with portable input/output names; font pixels are unchanged. |
| Minor: font provenance/permission and branding record absent | Record inherited assets separately from MIT DJ UI code in `DJ_UI_NOTICE.md`. Original font permission and branding review remain unresolved; catalogue statements are not permission evidence. No invented approval or visual replacement. |

Nine issues are repaired; the font issue has a documented partial disposition
and remains open. No hardware fault is inferred from a service finding alone.

## USB-audio disposition

The FS asynchronous packet preparation path rejected more than 48 frames,
although the existing feedback validator accepts a slightly faster 48-kHz
device clock when the endpoint has capacity. A 48.01-frame/ms feedback value
periodically requires 49 frames and previously reached `ESP_ERR_INVALID_SIZE`,
causing the completion path to fault the stream.

Packet staging and its frame bound now share a 51-frame capacity:
`ceil(48 * 1.05)`, the upper bound of the existing supported-rate feedback
validator. Endpoint packet bytes and transfer capacity remain authoritative
additional checks. Staging grows by only 24 bytes (four int16 channels, three
extra frames). The supported rates and feedback-validation tolerance are
unchanged. This is a generic FS 48-kHz repair; the captured Hercules format
still advertises 44.1 kHz and this does not identify a cause of earlier S2 losses.

The actual stream-owner regression uses synthetic asynchronous FS endpoints
for both 16-bit and packed 24-bit formats. It generates 1,000 packets at
approximately 48.01 frames/ms, verifies ten 49-frame packets without a stream
fault, and checks that an endpoint limited to 48 frames rejects the feedback
and cannot submit an oversized packet. The captured controller fixtures are
unchanged. The focused controller USB-audio suite passes (exit 0).
An isolated copy with the old 48-frame guard restored fails the new regression
at packet preparation, confirming that it detects the repaired boundary.
After this repair, the full host qualification passes again (exit 0, no SKIP),
including the newly registered production RGB565 fill regression, and the
ordinary M3 ESP-IDF 6.0.2 rebuild passes (exit 0). Documentation integrity and
`git diff --check` pass. These are local software gates; no OTA was performed.

## Validation and artifact boundary

The runtime simulator exercises public setters when native Settings widgets
are absent. Simulator scenarios also keep a callback-free splash alive beyond
three seconds. Screenshot baselines are not updated. The existing retained
position and framebuffer-pair tests remain part of the shared host suite.

Full host `-Qualification -KeepArtifacts` passes (exit 0), with no optional
SKIP. All nine simulator presentations pass their navigation and unchanged
screenshot gates. The ordinary M3 ESP-IDF 6.0.2 build passes (exit 0) in
`build_review_ui_20261011`; application size is 2,599,056 bytes, within
`0x380000`. Dependency locks are unchanged. This build includes uncommitted
repairs and is compilation evidence, not a clean signed candidate.

An additional extracted production RGB565 fill regression passes separately:
bounded pixels, paired dirty region and cache-sync error propagation. It is
registered in the host runner after the full run above; that added entry was
executed separately. The stub harness reports the existing signed/unsigned
framebuffer-index comparison warning, matching the production expression;
the test does not suppress other warnings as errors. It does not qualify
physical paired-panel scanout.

No new signed package, OTA, physical acceptance, JC build or publication is
implied by these local repairs. The installed accepted Hercules image and
frozen `5ff76b6c` delivery assets remain unchanged. Remaining controller-profile,
USB-host, LED and other PR directories still require their scoped reviews.
