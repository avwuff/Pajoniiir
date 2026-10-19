# M3 late review and JC4880 cache-bound OTA

## Scope

This follow-up reviews the saved, operator-accepted M3 hour offline and prepares
and installs the corresponding ordinary JC4880 candidate. No production code,
dependency, strict loss threshold or firmware setting is changed. The M3 image
remains `1556e8d7`, VALID `ota_1`; its accepted hour is not rerun. S2 stays
operator-closed, JC4880 soak waived and JC1060 hardware deferred to Kayrosen.

## Output-late review completed

[The frozen review](evidence/2026-10-19-m3-late-jc4880-preparation/late-review.json)
filters the saved TEXT journal to boot431 and uptime8,175,552–11,776,991 ms.
This prevents attribution of historical failures from older boots or earlier
windows to the accepted hour. Its22 aggregate AUDIO_OUTPUT_LATE records sum to
24 events, agreeing with the independently audited status counter delta.

The unchanged warning threshold is10,668 us; maximum12,988 us, approximately
24 events/hour, peak sample-observed cluster3/60 s. The investigation trigger
remains, separate from all22 strict counters staying unchanged and the operator's
full-hour clean audio/fluid display acceptance. No >=50-ms AUDIO_BLOCK_OUTLIER,
new WDT trace or controller-disconnect entry occurs in the selected window.
This does not prove that the historical masking call has been identified or
that no future deadline miss is possible.

There are23 primary late-phase records, all labelled `mix`, with maximum block
12,835 us, dominant phase11,823 us and individual mix group2,649 us. Their23 DMA
contexts show maximum observed gap5,391 us and zero queue overflow. Sampled task
contexts include `sdio_read`13, `ae_output`5, `sdio_write`3, `esp_timer`1 and
`tcpip`1. These are interrupted-task context samples, not instruction-level
attribution or a CPU-time breakdown; they cannot establish a Wi-Fi/DSP cause.
The production phase journal is rate-limited to2 s and is not a one-record-per-
late-event census. The diagnostic observer itself also has nonzero overhead.

The original139-ms stall does not reproduce in this hour. Retain the accepted
firmware and the existing investigation finding; no speculative scheduling,
gain, cache, DSP or USB change is justified by these records. The original
[hour evidence and operator confirmation](M3_MT_FALLBACK_HANDOFF_20261018.md)
remain intact. The review is complete; residual cause attribution and sustained
input pressure specifically through loop exit remain separate open scopes.

## JC4880 candidate

Clean source `0a33ec1fb16c0136dccaa85ca63a6fbc00a4d527` has production, test and
tool sources identical to accepted M3 `1556e8d7`. Only documentation/evidence
commits intervene. ESP-IDF6.0.2 ordinary JC4880 incremental reconfigure/build
passes, along with identity/BSP/LVGL/budget verification. This is not a fresh
clean build. All three dependency locks remain unchanged. Existing full host
Qualification on identical production/test sources is retained; no firmware or
test change requires rerunning it in this documentation-only follow-up.

| Artifact | Value |
| --- | --- |
| Project/version | `main-deck-p4` / `M2.5-249-g0a33ec1f` |
| BIN bytes | 2,609,952 |
| BIN SHA-256 | `a3ab2b6a8aedf36b265a0b451c4ea63a1c260d209a5a734a702a890a12770b79` |
| Signed bundle bytes | 2,610,140 |
| Bundle SHA-256 | `6dc08acd5398fd1b4348adbd3659d453fedaa745d8ccd5e16940b324c6bceefa` |
| ELF SHA-256 | `078cb8c09cbf4b77622facc5d0a09fedef3382f0970a3cd18bbd5ec0357975bc` |

The canonical packager verifies both the bundle and manifest ECDSA signatures.
The local package and debug ELF remain under
`releases/jc4880-cache-bound-0a33ec1f-20261019/pajoniiir-M2.5-249-g0a33ec1f/`.
[Preparation evidence](evidence/2026-10-19-m3-late-jc4880-preparation/package-evidence.json)
retains the build/package logs, signed manifest and hashes. Its NOT RUN
installation fields describe the preparation instant, before the later OTA.
This is a local signed bench candidate, without push/public release/channel
publication; no remote-source or hosted-CI PASS is claimed.

## Installation and short-test progression

The operator prepares JC4880 with FLX4 and storage, both decks stopped. A single
marked signed upload from clean `605bcd20` /VALID `ota_1` returns HTTP200 in
25.107 s. Windows reconnect disables DHCP; the operator restores it in an
administrator shell and confirms the GUI. No second upload or hardware reset
is used. The runner confirms exact new source/ELF, VALID `ota_0`, idle OTA and
both decks stopped. FLX4 MIDI/UAC1 FS44.1-kHz/16-bit/subslot2 streaming is healthy.
The connected library contains189 tracks; this is not the260-track M3 medium.

The first smoke stops after0.076 s /one sample because its runner incorrectly
requires M3's fixed48-kHz MAIN rate. The saved JC4880 capability explicitly
reports `fixed_output_sample_rate:0`; its selected MAIN and FLX4 rate are44.1 kHz.
All22 strict deltas and output-late remain zero, and final stop is confirmed.
This is an incomplete monitor attempt, not a reproduced firmware timing failure.
The original runner/result are retained. Only this board-rate assertion is
corrected; strict counters, limits and firmware are unchanged. A separately
confirmed60-s combined smoke follows, with its own telemetry/operator record.

## Corrected smoke accepted

The separate corrected run completes **60.234 s /93 samples**, with1,902 MIDI
packets, all22 strict deltas zero and output-late0. An independent offline audit
also checks baseline/final and six saved loop/CUE/resume snapshots:101 status
snapshots in total retain source identity, source rates44.1/48 kHz, MAIN/FLX4
44.1 kHz, MT and opposite pitches. Initial/final UI snapshots confirm zoom8.
Both loops activate at25.886 s and clear at36.404 s. D1 CUE parks only D1 at
43.811 s, then resumes; D2 does the same at51.074 s. Both decks end stopped,
loops OFF. The operator confirms every requested control/transition, clean
MAIN/FLX4 headphones and fluid waveforms throughout.

Sampled PFL states are both ON; MIX spans0–3307 and LEVEL0–1350. Individual PFL
selection is physically confirmed, not reconstructed from missing intermediate
polls. All observed MIDI arrives7.371–19.472 s, before automatic loop activation
and exit; controls under playback are accepted, not sustained input pressure
through loop exit. Startup recovery/MIDI disconnect/pipe counters1 and stopped
headphone underflow3,187,384 already exist in baseline and remain unchanged.
This is a no-new-loss result, not zero loss since boot.

[The hashed OTA/smoke archive](evidence/2026-10-19-jc4880-cache-ota/audit.json)
preserves the single upload, startup/readbacks, original incomplete attempt,
corrected runner/raw samples/action snapshots and separate operator confirmation.
The earlier preparation record's installation NOT RUN is superseded only by
this exact-image installed/short-smoke evidence. There is no new long soak,
cross-board physical acceptance, public release or public OTA update.

Hosted CI remains NOT RUN under the user's quota restriction; JC4880's soak
waiver and JC1060's deferred physical gates remain unchanged. Neither this
package nor M3 acceptance transfers physical qualification to JC1060 or closes
the entire shared-core implementation.
