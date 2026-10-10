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

The full P4 host suite completed with exit code zero; no SKIP markers were
found in its log. One local M3 ESP-IDF 6.0.2 build completed with exit code zero
on clean source `58f09941ee55a88cc8baf8a73287d447a0dfb0ca`. No dependency lock
changed. The board/build budget checks and actual packaging/signature checks
pass, including the new debug-archive path. Documentation integrity passes
(111 Markdown/HTML files, 134 retired paths) and whitespace checks pass.

Exact prepared candidate: `M3-dev-g58f09941ee55`.

| Artifact | Size, bytes | SHA-256 |
|---|---:|---|
| BIN | 2609408 | `e40f43073016effa97224e556f80f5a06f69cc7aba3927b3e5365e27047a4afa` |
| Signed bundle | 2609596 | `4c0443fc92da2ebc0c49b83bd168c464c8ae0d246a702ae2efb89b033c706386` |
| ELF | 29252464 | `bc10d523fee5263ffb81a3033f01145d944e756645fddcc12d3bf038dac91798` |

Local candidate and verified debug archive:
`.cache/crash-diagnostics-candidate-58f09941/pajoniiir-m3-M3-dev-g58f09941ee55/`.
Logs and exit records: `.cache/crash-diagnostics-full-host*`,
`.cache/crash-diagnostics-m3-build*`, `.cache/crash-diagnostics-package.log`,
`.cache/crash-diagnostics-documentation*`.

**Not installed.** The running accepted image remains `a9f66bcd`; the new
endpoint and fields are not available on that image. No JC build, physical
export/OTA lifecycle test, additional S2 test, push or publication was performed.
Build/software results are not installed-device acceptance or an OTA PANIC fix.

## Next evidence acquisition

Before installing a diagnostic image, retain the original raw dump through
USB1/CH340 serial access: a new OTA PANIC could overwrite it. This is a small
coredump evidence capture, not a new firmware-image backup. The M3 partition
is `0xc20000`, size `0x10000`; validate the current partition table before
reading, keep the full partition bytes and extract only the header-declared
dump extent as a separate derived file. Validate its IDF checksum offline,
extract the ELF notes/registers and match the stored SHA prefix against the
archived outgoing candidates. Never use the newly built ELF to symbolize an
old dump solely because its addresses look similar.

The serial-port inventory found no ports at this checkpoint. The operator
has been asked to connect USB1 and provide the COM port. Neither the existing
dump nor the firmware has been erased or overwritten while awaiting access.

See [capture instructions](../OTA-UPDATE.md#local-crash-diagnostics-and-debug-artifacts)
and [the original investigation](../reviews/2026-10-10-s2-closeout-resources-restart.md).
