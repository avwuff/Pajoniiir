# Crash diagnostics and local debug artifact retention

Scope: prepare the general OTA-restart investigation after the operator closed
S2 testing. This implementation is diagnostic infrastructure, not a repair of
the cache-register store fault. The historical `a9f66bcd` S2 PASS/waiver records
retain their exact-image scope. The diagnostic candidate is now installed;
its OTA/export checkpoint is recorded below and does not inherit those soaks.

## Wireless installation and historical dump capture

The operator clarified that USB1 is available only for power and explicitly
authorized OTA despite the risk that a restart PANIC could replace the old
dump. Serial capture is therefore not a prerequisite for this authorized path.
The PC reconnected using its saved `Pajoniiir-M3` Wi-Fi profile. Preflight
confirmed clean `a9f66bcd`, VALID `ota_1`, stopped IDLE decks and an idle OTA
service. The candidate bundle signature, image and bundle size/hash passed
verification before the single upload.

`POST /api/ota/p4` returned HTTP 200 with `ok:true,rebooting:true`. Firmware
returned as clean `58f09941ee55a88cc8baf8a73287d447a0dfb0ca`,
`M3-dev-g58f09941ee55`, VALID `ota_0`, with ELF SHA
`bc10d523fee5263ffb81a3033f01145d944e756645fddcc12d3bf038dac91798`.
The retained service log records boot 412, reset `SW`, Wi-Fi startup at normal
20-dBm power and a loaded library of 260 tracks. Both decks remained stopped.
This single OTA completed without a newly observed PANIC; it is not a general
restart reliability qualification or evidence that the historical cause is fixed.

The new marked `GET /api/crash-dump` returned a complete 7456-byte raw dump,
matching its `X-DDJ-Crash-Bytes` header and status-reported size. Raw SHA-256:
`75afa985b81eea00e253a8e2410ec55364960bcc65c873a4db547b65b8e0c839`.
The installed `esp_coredump` loader validated its embedded checksum offline
and extracted a RISC-V core ELF without substituting a later application's
ELF. The raw header declares exactly 7456 bytes, version `0x120104`, chip
revision 103. Nine register notes and eighteen load segments were preserved.

The dump's `ESP_CORE_DUMP_INFO` note contains **only nine hexadecimal ELF SHA
characters**, `5f70083d0`, matching the recorded outgoing `3f3605a7` image's
ELF SHA prefix. Its size, task, PC, RA, SP, MCAUSE and MTVAL match the pre-OTA
summary. No raw pre-OTA bytes were available, so this is not a before/after
byte-equality proof or a full SHA-256 match. The original `3f3605a7` application
ELF remains unavailable; the newly built ELF must not symbolize this old dump.

The first register note identifies TCB `0x4ff35eac` and confirms
PC `0x4ff0c624`, RA `0x4ff00b70`, SP `0x4ff33c80`. Register `a3` contains
`0x3ff10000`, the cache-register base used by the preserved original BIN's
disassembly. The same note has `a4=0x3ff10098` and `a5=4`, values assigned
later in that instruction sequence than the recorded PC. Preserve these
values and the PC/MTVAL discrepancy; do not infer precise bus fault timing,
rewrite PC to the previous instruction or claim a protection/ordering cause.

Local original and derived evidence, including pre/post status and firmware,
startup polls, raw dump, extracted core/segments, offline analysis and a
34-file SHA-256 inventory:
`.cache/crash-diagnostics-wireless/20261010T203454Z/`.
The capture and offline analysis helpers are retained under `.cache/`.
The original raw dump was not erased or edited. No additional S2 test, new
build, playback, USB/Wi-Fi control cycle, push, release or public OTA occurred.
Physical GUI/audio acceptance of the diagnostic image remains NOT RUN.

## Implemented

Later [offline restart analysis](../reviews/2026-10-10-m3-restart-crash-analysis.md)
confirms core 0, an internal stack and the restart call chain using complete
function-byte comparisons against the original BIN. One isolated historical
M3 analysis build passes, but its ELF is not the missing original ELF. The
original panic handler's asynchronous bus-error PC replacement is confirmed;
the queue CSRs and unmodified trap PC were not retained. No cause or repair is
established. The successful diagnostic OTA had no enumerated controller or
USB audio stream and does not qualify that earlier restart workload. This
later offline work made no device changes and did not reopen S2 testing.

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

**Historical pre-install checkpoint.** The running accepted image was `a9f66bcd`; the new
endpoint and fields are not available on that image. No JC build, physical
export/OTA lifecycle test, additional S2 test, push or publication was performed.
Build/software results are not installed-device acceptance or an OTA PANIC fix.

## Historical serial capture proposal

Before installing a diagnostic image, retain the original raw dump through
USB1/CH340 serial access: a new OTA PANIC could overwrite it. This is a small
coredump evidence capture, not a new firmware-image backup. The M3 partition
is `0xc20000`, size `0x10000`; validate the current partition table before
reading, keep the full partition bytes and extract only the header-declared
dump extent as a separate derived file. Validate its IDF checksum offline,
extract the ELF notes/registers and match the stored SHA prefix against the
archived outgoing candidates. Never use the newly built ELF to symbolize an
old dump solely because its addresses look similar.

The serial-port inventory found no ports at this checkpoint. The later
operator clarification and risk-authorized wireless capture supersede the
serial dependency; see the completed installation/capture above. Next work
is offline analysis of the preserved core and original candidate code, with
the missing original application ELF and short identity prefix kept explicit.

See [capture instructions](../OTA-UPDATE.md#local-crash-diagnostics-and-debug-artifacts)
and [the original investigation](../reviews/2026-10-10-s2-closeout-resources-restart.md).
