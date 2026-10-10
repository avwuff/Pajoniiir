# Crash diagnostics and local debug artifact retention

Scope: prepare the general OTA-restart investigation after the operator closed
S2 testing. This implementation is diagnostic infrastructure, not a repair of
the cache-register store fault. The accepted installed `a9f66bcd` image, USB
configuration and historical S2 PASS/waiver records remain unchanged.

## Implemented

- `/api/status` exposes the stored dump's bounded hexadecimal ELF SHA prefix
  and its length. It never substitutes the running firmware SHA when crash
  identity is missing, invalid or summary extraction fails.
- Marked local `GET /api/crash-dump` exports raw bytes without erasing the
  dump or changing playback/USB. Playback/loading/recording/OTA admission is
  checked initially and between bounded reads. The copy aborts on busy state,
  read/send error or timeout; incomplete HTTP chunks must not be accepted as
  a complete capture. Operator input is not locked out during download.
- Export size is bounded by the actual coredump partition and 256 KiB, with
  512-byte chunks and a five-second copy deadline between synchronous operations.
  Socket sends also have the server timeout. Raw corrupt dumps remain evidence;
  CRC/SHA validation occurs offline. No active-stream heap walk was introduced.
- Packaging preserves matching ELF/map/config/build identity in a local
  `debug/` archive with a size/hash manifest. Missing or mismatched ELF and
  conflicting archive files are rejected. Signed OTA payload/manifest contracts
  are unchanged; this does not publish debug files or regenerate old releases.
- The package reader now requires the complete app-descriptor SHA extent
  (208 bytes), rather than reading it after only checking a 112-byte minimum.

Before rebuilding, the accepted `a9f66bcd` ELF was hash-verified against its
image identity and archived with its map/config/metadata in
`.cache/debug-archives/a9f66bcd3175/debug/`. Its full SHA remains
`90c9ef537ef6d3b8a60e41a2114084de82bda044a6110cb401c5fc7130d33689`.
The original `3f3605a7` ELF remains unavailable; retaining a later ELF does
not fill that historical gap.

## Verification checkpoint

Focused production export-core regressions pass: prefix validation and bounds,
7456-byte exact copy, busy admission, playback beginning during a read, flash
read error, socket send error, deadline, partition overrun and hard size cap.
Archive tests pass: matching-byte retention, idempotent rerun, wrong ELF,
missing map, invalid identity and refusing conflicting archived bytes.

Full P4 host suite and one local M3 ESP-IDF 6.0.2 build are being run for this
implementation. Their final results will be appended after reading exit codes.
No JC build, physical export/OTA lifecycle test, additional S2 test, push or
publication is part of this checkpoint. Build/software results must not be
represented as installed-device acceptance or an OTA PANIC fix.

See [capture instructions](../OTA-UPDATE.md#local-crash-diagnostics-and-debug-artifacts)
and [the original investigation](../reviews/2026-10-10-s2-closeout-resources-restart.md).
