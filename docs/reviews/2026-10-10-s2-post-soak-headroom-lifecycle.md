# S2 post-soak headroom and lifecycle review

Historical review. Its proposed remaining S2 tests are superseded by the
operator's explicit closeout and waivers in
[the later resource/restart review](2026-10-10-s2-closeout-resources-restart.md).
The later review also adds 44100-Hz acceptance, stack/allocation snapshots and
archived-BIN disassembly. Do not schedule the physical block below.

Date: 2026-10-10. Scope: offline evidence and production-code review while the
operator is away. No playback, USB recovery, reboot, OTA or firmware mutation
is part of this review. Installed-image acceptance remains tied to source
`a9f66bcd3175d86cacb3072b9e1a08bc95f77ba1`, S2 48000 Hz, VALID `ota_1`, ELF
`90c9ef537ef6d3b8a60e41a2114084de82bda044a6110cb401c5fc7130d33689`.

## Findings

The completed 3600.296-second / 5705-sample window has telemetry and operator
playback PASS. No ISO descriptor fault was captured. It does not establish
the intermittent loss's cause, prove that the observer has no scheduling
effect, or qualify 44100-Hz S2 output or active jog/Browse use.

### Cached memory trend

Every recorded sample was inspected. Values below are cached `/api/status`
counters; no live heap walk was performed.

| Counter, bytes | First | Last | Lowest sampled value | First / last quarter median |
|---|---:|---:|---:|---:|
| Internal free | 50739 | 50739 | 48995 | 50739 / 50739 |
| DMA free | 11159 | 11159 | 9415 | 11159 / 11159 |
| PSRAM free | 21968124 | 21968120 | 21961960 | 21968124 / 21968124 |
| Internal lifetime minimum | 48599 | 48339 | 48339 | 48547 / 48339 |
| DMA lifetime minimum | 9019 | 8759 | 8759 | 8967 / 8759 |

There is no observed progressive internal/DMA free-memory decline during
steady playback. A lower lifetime minimum records a transient allocation
peak, not a retained allocation by itself. The four-byte PSRAM endpoint
difference does not demonstrate an increasing leak. Sampling and cache
refresh cannot exclude short-lived allocations or other scenarios.

The remaining DMA reserve is limited. The existing UI resource checker uses
8192 bytes as a total-free threshold, leaving only 567 bytes above it at the
8759-byte lifetime minimum. Its largest DMA/internal block and stack-reserve
requirements are not measured by this stream: largest-block fields are null.
This is **insufficient resource qualification**, not a new playback failure.
No claim that a fresh large allocation fits follows from total-free bytes.

Linked map evidence places the internal ISO descriptor pool at 2048 bytes,
the frozen capture at 656 bytes, and its state/sequence plus pool-claim words
at 12 bytes combined. These are static sizes, not the total observer cost:
activation metadata also enlarges HCD buffers/pipes/ports. Three OUT payload
reservations request `3 * 112 * 56 = 18816` bytes before allocator overhead;
feedback, URB metadata and HCD pipes add further allocations.

Evidence: run directory
`.cache/s2-mk3/next-gates-20261009/first-fault-durable-20261010T154001Z/`,
`memory-trend-audit.json`, `evidence-audit.json`, and
`firmware/main-deck-m3/build_s2_hs/main-deck-m3.map`.

### Reconnect ownership

`controller_usb_audio_stream.c` admits a single output writer through its
atomic gate and waits for active control/ISO transfers and that writer to
retire before detaching retained HS URBs. Cleanup retains only three idle OUT
URBs; device handles and endpoint/packet state are detached. Control and
feedback transfers are released. A larger incompatible endpoint reservation
or a switch to FS releases the old pool. The four-list static HCD descriptor
pool releases claims after pipe retirement and fails allocation explicitly
instead of falling back to PSRAM.

The production stream host regression now tracks live transfer count and
requested payload bytes over 30 alternating device-gone/soft-stop reconnect
cycles with new 56-packet OUT allocations deliberately rejected. Every cycle
reuses the same OUT pointers and performs no new OUT allocation; control and
feedback are the only transient reservations. Cleanup returns exactly to
three retained transfers, and the final FS transition returns to zero live
transfers and payload bytes. Existing incompatible-capacity, timeout, canceled
callback, 44100-Hz fallback and startup allocation/submit cases still execute.
The focused regression passes. These are **host cycles**, not 30 physical
USB enumerations, HCD retirements or bus/power tests.

### OTA restart PANIC remains separate

Saved boot-406 `internal-payload-startup-status.json` contains an `ota_reboot`
crash summary: store-access fault `mcause=7`, `mtval=0x3FF1009C`, PC
`0x4FF0C624`, RA `0x4FF00B70`. In the local required ESP-IDF 6.0.2 hardware-v1
headers, `DR_REG_CACHE_BASE = 0x3FF10000` and
`CACHE_SYNC_MAP_REG = base + 0x9c`. Thus the fault address matches that cache
register. `Cache_WriteBack_All` writes this register; the restart path calls
it before disabling L2 cache. The old image's exact ELF was not retained, so
instruction-level symbolization remains provisional. Register address and
task identity support investigating restart/cache access; they do not prove
why the store faulted or implicate playback allocations.

The project already pins `delayed_restart_task` to core zero. Its panic
quarantine deliberately prevents HS audio startup after a PANIC boot. The
successful fc4-to-a9f local OTA is one successful transition, not a closure of
earlier intermittent reboot failures. No quarantine bypass or speculative IDF
cache/register patch was made. Reproduction is deferred until physical RESET
recovery is available; preserve the exact old ELF and crash before reflashing.

## Next combined physical block

Keep the accepted image and confirmed 48000-Hz S2 configuration. When the
operator returns, combine a short dual MT/loop playback with jog/Browse input,
then stop both decks before a small reconnect block. Verify live streaming,
new MIDI packets, epochs and allocation/ISO counters after each attachment.
Record audio/display observations separately from counters. Read expensive
resource details only with decks stopped, and do not transfer that idle
measurement into a dual-playback largest-block/stack PASS.

Keep the 44100-Hz reproduction and restart investigation separate. A new
image is not justified solely by this hour's non-reproduction. Preserve the
first-fault observer for the next actual fault; do not erase it by rebooting
before collecting its snapshot. Full S2 acceptance and shared-core release
remain open; no public release/channel changes are authorized by this review.

## Verification

The focused production-stream host regression and full P4 host suite both
completed with exit code zero. The full-suite log contains no SKIP/skip
markers. Evidence: `.cache/s2-mk3/next-gates-20261009/unattended-host-20261010.log`
and `unattended-host-20261010-exit.json`. Existing image, signed bundle and
release manifest were reverified with the public key; the bundle SHA-256 is
`42aa6a1785ac83e2389d2df29493da24de98ab16f68ce2e193313115ef0f0252`.
`git diff --check` is required before the selective local commit.

Only test stubs/assertions and documentation changed. No firmware rebuild
was performed; the installed accepted image is unaffected. Physical reconnect,
active-control playback, numeric largest-block/stack qualification, 44100-Hz
device output and reboot reproduction were NOT RUN during this unattended
work. Hosted CI, push and release/public OTA were not requested or performed.
