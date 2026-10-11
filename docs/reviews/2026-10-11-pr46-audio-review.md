# PR 46 audio-core review, 2026-10-11

## Scope and service result

CodeRabbit CLI 0.6.5 reviewed the audio-engine directory at clean PR head
`ecfe85704ad935cff6bd463564184c73b147ede2` against base
`9af99cd234e775521f3ec83c0104f5c6a0c72920`.
The complete PR request failed because the service counted 707 files against
a 150-file limit. That attempt produced no findings and no whole-PR PASS.
An isolated Linux checkout avoids Windows worktree-pointer incompatibility.

The scoped audio review completed with warnings/unverified findings and raised
2 issues, both minor. Each was checked against production code before editing.
Raw service output is retained locally at
`.cache/coderabbit-pr46-audio-ecfe8570.ndjson`; the failed full attempt is
`.cache/coderabbit-pr46-ecfe8570.ndjson`. Remaining PR directories have not
completed this review; no unrestricted review/merge readiness is claimed.

## Confirmed issues and repairs

1. `audio_output_service_open_codec` published codec readiness before assigning
   the output rate and configuring shared FX state. The higher-priority output
   task could observe readiness while coefficients were being rewritten.
   Rate/FX initialization now precedes release publication; every readiness
   reader uses acquire and every writer uses the common atomic bool helpers.
2. Decoder media faults consumed the bounded eight-attempt retry budget without
   waiting between immediate cache failures. Both initial MP3 rate discovery
   and steady-state zero-sample fault retries now wait 25 ms outside `AE_LOCK`.
   The fault state is snapshotted under the lock; successful reads retain the
   existing reset path. The steady-state path also covers pending FLAC
   recovery/seek faults. Permanent failure still records `MEDIA READ ERR` and
   EOF after eight attempts. Seven retry intervals provide at least 175 ms at
   the current tick configuration; this is not a promise to survive arbitrary
   unmounts or removal, and it does not add sleep to the output task.

## Verification and artifact boundary

The new `tests/audio_fw_runtime/test_review_boundaries.py` extracts and executes
production functions/branches with RTOS stubs. It checks observation during FX
configuration, fixed/source-rate opens, repeated open, invalid rate, bounded
backoff outside the lock, reset and deck isolation. It is registered in the
host runner. This is software boundary evidence, not physical outage timing.

The focused regression, full host `-Qualification` run and ordinary M3
ESP-IDF 6.0.2 build pass (exit 0). Host qualification includes the new production
boundary regression; no optional SKIP was emitted. The repair build ran with
uncommitted fixes and is compilation evidence, not a clean signed candidate.
No JC build, new
signed package, OTA or physical test is implied by this repair.

The installed accepted Hercules image remains unchanged. Frozen `5ff76b6c`
signed packages and their archived BIN/ELF/MAP/configuration evidence remain
unchanged; they do not contain these fixes. The mutable M3
`build_qualification_final_20261011` working directory was reused for the
repair build and no longer represents the frozen M3 build. Use the immutable
local delivery archive for that original image; do not rerun its build-directory
audit against this rebuilt directory or relabel historical qualification.
