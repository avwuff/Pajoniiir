# PR 46 recorder and DJ Link review

Date: 2026-10-11. CodeRabbit service source is clean commit
`9d05464a382c538e6b0bd02f059a3098a6cbdbd2`, compared with
`9af99cd234e775521f3ec83c0104f5c6a0c72920`.

## Service coverage

CodeRabbit CLI 0.9.0 completed four sequential scoped requests. Original NDJSON
is retained at `.cache/coderabbit-pr46-<scope>-9d05464a.ndjson`.
Returned coverage/outcomes and original NDJSON SHA-256 values are saved in
`.cache/pr46-recorder-link-service-audit.json`.

| Scope | Returned reviewed files | Issues | Outcome |
| --- | ---: | ---: | --- |
| audio_recorder | 16 | 3 | completed_with_warnings |
| dj_link_service | 6 | 1 | completed_with_warnings |
| common/dj_link_core | 1 (README only) | 0 | completed |
| common/djlink | 1 (UPSTREAM only) | 1 | completed |

The service raised five issues (one major and four minor); all were verified
and addressed locally. The wrapper completed with exit code 0. The two common
directories have no C-source differences against this review base. Consequently
the service inspected only their changed documentation, not the inherited
protocol/cache/parser implementation. Zero reported issues in that README scope
is not a full protocol review PASS. Repairs were tested locally; this record
does not claim a second service review of the repaired diff or whole-PR approval.

## Repairs and evidence

1. Recorder STOP and producer admission now use sequentially consistent atomic
   ordering at the store/load ownership boundary. Release/acquire alone did not
   establish the total order needed to prevent both parties from missing the
   other's publication. Existing ownership tests and a two-thread, 20,000-round
   close/admission race pass. Host stress is not a formal RVWMO proof or physical
   recorder qualification; the memory-order correction follows the atomic
   contract directly.
2. An oversized PCM block is rejected and counted as a full dropped block,
   rather than silently truncated and accepted. Production runtime regression
   verifies that this closes admission, enters ERROR and aborts the segment
   without publishing a normal WAV. The current audio block remains 256 frames.
3. Sink header comments now accurately describe checkpoint as flush/fsync of
   appended PCM. WAV sizes are patched at finalize or rebuilt during recovery;
   checkpoint does not produce a current playable header on another device.
4. DJ Link acquires the snapshot mutex before removing a queued LOAD. A busy
   reader no longer silently discards that request. A harness extracts and runs
   the actual production function: the old version fails the queued-request
   retention assertion; the repair passes contention, empty queue, stale-source
   rejection and unavailable-lock cases. It is registered in the full host suite.
5. Donor provenance names `firmware/main-deck-jc1060/components/djlink`, the
   path verified in the GitHub tree of donor
   `428b97dd4a175f03d3a172c8db9c4d5ed94195fb`. The historical upstream path is not
   a current shared-core component location.

An additional manual regression exposed ring corruption at uint32 sequence
wrap with a non-power-of-two capacity. The old modulo-based indexing fails a
three-block payload-order assertion across UINT32_MAX. Separate producer and
consumer physical cursors now wrap at capacity while sequence subtraction
continues to track occupancy. This is a long-runtime boundary repair, not a
diagnosis of any observed bench waveform/audio fault.

## Validation and delivery boundary

- Focused baseline and repaired recorder/DJ Link host suites pass.
- New pipeline, STOP race and production LOAD queue regressions pass on Windows
  and Ubuntu/WSL. The deterministic ring and LOAD tests were observed failing
  against the old implementations before repair.
- Full host qualification passes with exit code 0 and no SKIP in
  `.cache/pr46-recorder-link-qualification.log`; required fixture integrity and
  the production LOAD queue regression both ran. Documentation checker passes
  (125 Markdown/HTML files, 134 retired paths) and `git diff --check` passes.
- Ordinary M3 ESP-IDF 6.0.2 build and board verifier pass: 2,601,280 bytes,
  isolated M3 BSP, LVGL 9.5.0 and correct app identity. Dependency locks are
  unchanged. Recorder/service sources compile in this build; the recorder tap
  and optional network service remain disabled.

No experimental service was enabled on the device. Installed firmware, frozen
packages and public OTA remain unchanged. Recorder SD fault-injection/listening,
real Pro DJ Link interoperability, current GitHub CI and JC4880/JC1060 physical
gates remain open. An inherited protocol implementation review needs a separate
scope/base that actually includes those sources. This batch does not close full
shared-core acceptance.
