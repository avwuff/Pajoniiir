# S2 closeout, memory reserves and OTA restart evidence

Further [offline frame/function audit](2026-10-10-m3-restart-crash-analysis.md)
confirms the actual exception core, internal stack and byte-verified restart
chain. It identifies IDF's possible asynchronous bus-error PC replacement,
while keeping the missing original ELF and fault-time CSR state explicit.
It adds no causal repair or new S2 test.

Later checkpoint: [diagnostic installation and raw capture](../validation/CRASH_DIAGNOSTICS_20261010.md#wireless-installation-and-historical-dump-capture)
completed after explicit operator authorization of the power-only USB1
fallback. Clean `58f09941` is now VALID `ota_0`; the historical dump checksum
passes and its stored nine-character ELF prefix matches `3f3605a7`'s recorded
prefix. The original application ELF and proven restart cause remain missing.
The review below describes the earlier pre-install evidence; no S2 tests were
reopened by the later diagnostic OTA.

Date: 2026-10-10. This review supersedes the remaining-test recommendations in
[the earlier post-soak review](2026-10-10-s2-post-soak-headroom-lifecycle.md).
It adds a stopped-deck resource snapshot and offline analysis of preserved
artifacts. No playback, USB changes, heap walks, reboot, OTA or firmware changes
were performed. Both decks remained stopped and all 22 strict counters were
unchanged across the live snapshot.

## S2 work closed by operator decision

The operator explicitly closed the S2 work and declined further testing.
Retain the exact-image full-hour physical/telemetry playback PASS at both
48000 and 44100 Hz, and the accepted 48000-Hz active jog/Browse block. The
44100-Hz hour had five output-late events without an investigation trigger or
operator-observed disturbance; it is not zero-late evidence.

Remaining active-control coverage at 44100 Hz, exhaustive LED/held-state
checks and any outstanding physical lifecycle/profile checks are
**WAIVED BY OPERATOR / NOT RUN**, not PASS. Operational reconnect results and
their cable/power-confounded ISO observation retain their original scope.
Do not schedule another S2 test or reinterpret earlier failures as successes.
S2 remains an experimental configuration, not an unrestricted production or
shared-core release qualification. The separate restart/resource investigation
continues without reopening controller acceptance.

## Verified installed state

The stopped-deck snapshot at about 22:00:49 Europe/Zagreb confirms clean
`a9f66bcd3175d86cacb3072b9e1a08bc95f77ba1`, version
`M3-dev-ga9f66bcd3175`, VALID `ota_1`, ELF
`90c9ef537ef6d3b8a60e41a2114084de82bda044a6110cb401c5fc7130d33689`.
S2 is streaming at 44100 Hz, HS 24-in-32, without a live audio fault; MAIN is
48000 Hz. S2 remains USB3/root0 and storage USB2/root1, with Wi-Fi enabled.
The persisted cache PANIC is historical, not a new crash during this snapshot.

## Memory and stack evidence

Every one of the 5260 samples from the accepted 3600.414-second 44100-Hz soak
was audited offline. These are cached counters, not allocation-fit probes.

| Free memory, bytes | First / last sample | Lowest sample | First / last quarter median |
|---|---:|---:|---:|
| Internal | 50743 / 50743 | 48215 | 50743 / 50743 |
| Internal DMA | 11163 / 11163 | 8635 | 11163 / 11163 |
| PSRAM | 21966540 / 21966540 | 21960376 | 21966540 / 21966540 |

There is no progressive retained-memory decline visible in this hour. This
does not exclude brief peaks, fragmentation or an unobserved lifecycle leak.
The existing DMA threshold is 8192 bytes: the lowest sampled value has only
443 bytes of margin. The older lifetime minimum is 7371 bytes, already present
at baseline and unchanged throughout the hour. It is below that threshold,
but neither dates the transient nor proves an allocation failure.

The live stopped snapshot reports `startup_phase=ready`, zero allocation
failures and zero critical allocation failures. The callback is registered
at startup. All nine task samples meet the existing checker thresholds:

| Task | Recorded minimum stack reserve, bytes | Threshold, bytes |
|---|---:|---:|
| Audio output | 2824 | 1024 |
| Decode D1 / D2 | 20548 / 20540 | 512 |
| Loader D1 / D2 | 2800 / 2808 | 512 |
| LVGL | 13368 | 512 |
| Controller | 6432 | 512 |
| Storage | 13616 | 512 |
| HTTP | 1884 | 1024 |

Samples were 145–2416 ms old relative to the after-snapshot uptime. These
are task-reported lifetime low-water marks, not new dual-playback stack
measurements or an `ota_reboot` stack sample. They do not justify shrinking
stacks based on one workload.

`/api/resources` reads atomic allocation counters, cached stack samples and
the constant-time custom LVGL allocation counter (68388 bytes). It samples
only its own HTTP task stack. It does not call the largest-block heap walker.
Largest internal/DMA/PSRAM block fields remain null in this image. HS audio
continues to send silence while stopped, so stopping decks does not make a
heap walk harmless. Do not enable such walks to fill missing qualification.

**Disposition:** allocation and recorded stack checks pass within their
scope; complete memory/fragmentation/lifecycle qualification remains
**INSUFFICIENT EVIDENCE**. Keep the accepted reservation scheme. No evidence
supports sacrificing a retained ISO buffer or reducing task stacks now.

## OTA PANIC: instruction-level evidence

The fc4 OTA install record identifies the outgoing application as clean
`3f3605a7ba7fb2ce891b703aae1b69864bce6468`. Its archived BIN is 2603824 bytes,
SHA-256 `7bf603045a3d39c398668dd3a6eea7e42b5b581a740e46cd675d6d87590a8e1a`.
The manifest and embedded application descriptor agree on version and ELF
SHA `5f70083d0c64ec66ca38b049845e6eb6f9e3960a341e63c7b8cb54a55e052488`.
Image-info also validates its checksum and image validation hash.

The later boot-406 crash summary names `ota_reboot`, PC `0x4FF0C624`,
RA `0x4FF00B70`, SP `0x4FF33C80`, store-access cause 7 and MTVAL
`0x3FF1009C`. Its 7456-byte dump and SP differ from the earlier 7840-byte
dump / SP `0x4FF341C0`; the later summary is not simply identical to that
earlier saved summary. The current device still reports the boot-406 summary.

Disassembly from the hash-verified archived BIN, rather than a newer ELF:

```text
4ff00b6a: li   a0,16
4ff00b6c: jal  0x4ff0c60c
4ff00b70: ...                  # recorded return address

4ff0c61c: lui  a3,0x3ff10
4ff0c620: sw   a5,156(a3)       # 0x3ff1009c, CACHE_SYNC_MAP_REG
4ff0c624: sw   zero,160(a3)     # recorded PC, CACHE_SYNC_ADDR_REG
4ff0c628: sw   zero,164(a3)     # CACHE_SYNC_SIZE_REG
```

The caller and callee instruction sequences agree with the locally pinned
ESP-IDF 6.0.2 `esp_restart_noos_inner` and `Cache_WriteBack_All` bodies.
The routine is in the internal IRAM image segment. The caller supplies the
L1 D-cache map, then proceeds to disable L2. This establishes a concrete
cache-writeback restart location in the candidate binary, rather than a
guess using the current ELF's shifted addresses.

Preserve the distinction: PC points to the following register store while
MTVAL names the preceding map register. Do not silently normalize PC to
`0x4FF0C620` or infer the bus/exception timing from this summary alone.
The HTTP summary omits the crashing application's ELF identity even though
IDF's `esp_core_dump_summary_t` contains it; the configured retrieved prefix
is only nine characters. The OTA record supports association with the
outgoing image, but the dump has not been cryptographically matched to that
ELF. The original ELF/raw coredump was not exported for full backtrace and
register analysis.

The restart helper was already pinned to core 0 with a 4096-byte stack in
the outgoing source. Pull install is also pinned to core 0. Reapplying the
older core-pinning mitigation would not repair a missing current setting.
The cache routine is already placed in IRAM; adding an IRAM annotation alone
does not address this incident. Current upstream retains the same
writeback-before-L2-disable restart ordering, as visible in
[Espressif's implementation](https://github.com/espressif/esp-idf/blob/master/components/esp_system/port/soc/esp32p4/system_internal.c).
This comparison is context, not proof of an upstream fix or a common cause.

**Disposition:** restart/cache access failure is supported by preserved
evidence. Its cause remains unresolved. Neither a playback memory shortage,
wrong restart core, externally allocated restart stack nor stale instructions
has been proven as the cause. Keep PANIC audio quarantine; do not bypass it,
skip cache writeback, force a register value or substitute a watchdog reset.

## Concrete next engineering work

1. Preserve the exact ELF and map alongside each future local candidate BIN,
   with a hash-verified sidecar. The existing BIN allowed this analysis, but
   cannot provide the missing full symbolic backtrace.
2. Expose the stored crash ELF prefix separately from the running image SHA,
   and provide a bounded stopped-only raw coredump export. Match the original
   dump to archived debug artifacts before assigning a source line/core.
   These are diagnostic improvements, not a restart fix; no new image was
   built or installed in this review.
3. Use the full matched exception state to decide whether shutdown ordering,
   register access protection or an IDF erratum actually applies. Any proposed
   firmware repair needs its own local M3 software checks and general OTA
   lifecycle acceptance. That work must not reopen the waived S2 test campaign
   or inherit the old image's accepted soaks.

The current evidence does not justify changing the accepted firmware merely
to try another timing/cache workaround. Hercules preparation can proceed
offline, but activation of a new image/configuration needs its own scope.

## Reproduction and retained evidence

Local records:
`.cache/s2-mk3/next-gates-20261009/post-closeout-resource-restart-20261010/`.
They contain the stopped before/after status, resources and exact firmware
snapshot, extracted original IRAM segments, disassembly, image provenance,
all-sample memory trend and SHA-256 audit of inputs/derived evidence.
The helper `.cache/audit_s2_closeout_resources_restart.py` reuses existing
snapshots on rerun instead of repeating device queries. Its assertions verify
stopped state, identity, unchanged strict counters, allocation/stack reserves,
BIN/manifest/descriptor hashes and all 5260 memory samples. It completed with
exit code zero. The stored artifacts are local evidence, not published assets.

Documentation checks passed: `tools/check_documentation.ps1` (110 Markdown/HTML
files, 134 retired paths), `git diff --check` and staged `git diff --cached --check`.
No firmware/build change required a new IDF build or full host suite. No push,
release, public OTA, additional S2 test or automatic monitor was started.
