# P4 restart and pre-normalization bus-error capture

This implements the next diagnostic step from the
[historical restart audit](../reviews/2026-10-10-m3-restart-crash-analysis.md).
It adds evidence collection, **not a causal repair** of the historical
cache-register store fault. Further S2 testing remains closed by operator
decision. At the initial implementation checkpoint, installed clean `58f09941`
was VALID `ota_0` and implementation work performed no device action or live
query. Later natural-fault capture and the installed `ac9287af` candidate are
recorded chronologically below; their explicit scopes supersede that checkpoint.

## Production contract

The shared `restart_diagnostics` component uses linker interception of three
external IDF functions, without modifying the installed SDK or dependency
sources:

- `esp_restart`: record a restart request and the current core/cycle count,
  then continue the original shutdown-handler behavior;
- `esp_restart_noos`: record entry into the final restart stage, then continue
  the original core/interrupt/watchdog/cache/reset sequence;
- `panic_arch_fill_info`: retain the supplied exception frame and bus-error
  CSRs **before** delegating to IDF's possible saved-PC normalization.

The wrappers reside in IRAM. Two fixed 128-byte records reside in internal
RTC memory, in the IDF `COREDUMP_NOINIT_ATTR` user region. The normal ELF coredump
writer includes this region without enabling global DRAM/heap capture. No
steady-state audio hook, polling, allocation, logging, NVS write or SDK
upgrade was added. Capture has observer overhead and is not an atomic
hardware snapshot.

The actual M3 link exposed a duplicated `_coredump_dram_start/end` definition
in IDF 6.0.2's generated P4 linker script: the second DRAM bank overwrote the
bounds with an empty interval despite correct placement in the first bank.
The mandatory ELF gate rejected that image. The unique internal RTC noinit
coredump region avoids this without SDK changes or replacing global bounds.
A BSS readiness marker and bounded IRAM initialization clear stale noinit
bytes once per boot, before common services; restart and exception hooks also
initialize on pre-app entry. Repeated initialization preserves frozen evidence.
On P4, `SOC_RTC_MEM_SUPPORTED` selects `.rtc_noinit.coredump` and the writer's
`_coredump_rtc_noinit_start/end` bounds. The subsequent link check initially
expected generic DRAM noinit bounds and correctly refused qualification until
the checker was corrected to that actual P4 contract.

Each core's record contains ABI magic/version/length, restart phase/cycles,
pre-normalization frame PC/RA/SP/cause/address/core, current MEPC/MCAUSE/MTVAL,
MHARTID, MEXSTATUS, MHINT and all three raw STPC/STTVAL pairs. Raw queue
validity bits are preserved. The P4
[TRM CSR table](https://documentation.espressif.com/esp32-p4_technical_reference_manual_en.pdf)
defines STTVAL0–2 at `0xBF8–0xBFA`; IDF 6.0.2's CSR header omits those names.

The record is marked captured only after all stores and an ordering fence.
Subsequent hooks on that core continue IDF behavior without overwriting the
captured evidence. No attempt is made to carry live DRAM across reboot:
the persisted evidence is the existing checksummed flash coredump. An old
dump remains old even after this new image is installed.

This hook covers the architecture-exception path, including the historical
MCAUSE-7 fault. Pseudo-exceptions handled by `panic_soc_fill_info` do not pass
through it; watchdog/abort capture must not be claimed from this hook alone.
Only the core running this hook is sampled; if the supplied frame's core
differs, the decoder reports the difference without conflating their CSRs.

## Software and build verification

The C host test executes the production wrapper source with mocked CSR reads
and IDF functions. It checks capture-before-normalization, original behavior
delegation, both-core separation, frozen evidence during panic restart, invalid
core/null-frame handling, stale noinit initialization, idempotence and the fixed
256-byte total reservation.

Four decoder regressions verify raw PC/address pairs and validity bits,
truncation, ABI/flags/phase/core rejection, and full BIN/ELF SHA matching.
`tests/run_p4_host_tests.ps1` includes the production and decoder tests.

Every P4 link now runs `tools/check_restart_capture_elf.py`: the actual ELF
must contain IRAM wrappers, real intercepted callers, delegation to the
original IDF functions, all required bus CSRs, and the complete 256-byte
record within the IDF user coredump noinit bounds, with IRAM initialization
called from startup and each hook. A missing wrapper or an
uncaptured region fails the build. The same build target exercises five
synthetic raw-coredump cases with the built app BIN/ELF: successful decoding,
bad checksum, wrong dump identity, truncated region and wrong ABI. These are
software fixtures, not an induced physical PANIC or observed hardware fault.

The focused production C test, four decoder regressions and full qualification
host suite pass (exit zero, no SKIP markers). The final isolated M3 ESP-IDF
6.0.2 build also passes, including actual ELF placement/interception, all five
synthetic raw-pipeline tests and `check_board_build.py --experimental-s2`.
An explicit reconfigure refreshed the source identity: an earlier incremental
build retained stale metadata and was rejected before packaging.
Clean M3 defaults now explicitly select the existing bounded
flash-coredump contract (eight tasks, boot check, diagnostic logs disabled),
matching the shared P4 defaults. The bench HS config already enables flash
coredumps. The final generated config differs from the retained `58f09941`
config only by disabling `CONFIG_ESP_COREDUMP_LOGS`; dump capture remains enabled.
JC4880/JC1060 builds are
deferred according to the operator's M3-first debugging preference.

## Prepared M3 development candidate

This is a local diagnostic artifact, **not installed or physically qualified**:

| Field | Verified value |
| --- | --- |
| Clean source | `0b08fb90af24e185631cc1001209cf79bb032096` |
| Version/project | `M3-dev-g0b08fb90af24` / `main-deck-m3` |
| Application | 2,601,696 bytes; below `0x380000` budget |
| BIN SHA-256 | `30b92c6eb9990e32821a71fd7866538e4ead5bf2515e9a615b28e61fc380d42d` |
| Matching ELF SHA-256 | `7db28ad1ef942c2cde98d115d34ecd48fb0a4dce6088372a8e0855119d00eb7c` |
| Signed bundle | 2,601,884 bytes; ECDSA P-256 / `rel-001` verified |
| Bundle SHA-256 | `b0e267a54cb42ff9315b1446b0c60793beab374d5e9cb998f94889c82dea6187` |
| Coredump record extent | `[0x50108080, 0x50108180)` in `.rtc_noinit` |

The package is retained under
`.cache/restart-bus-capture-candidate-0b08fb90/pajoniiir-m3-M3-dev-g0b08fb90af24/`.
Its `debug/` manifest retains the matching ELF/map/config/identity. Its
`verification/` manifest hashes ten software/build/package evidence files.
The package signature and manifest signature both verify; committed dependency
locks are unchanged. No public channel was changed.

Against the retained `58f09941` ELF, `.iram0.text` increases by 330 bytes,
`.rtc_noinit` by 256, `.dram0.data` by 8, and `.dram0.bss` decreases by 16;
ordinary `.noinit` is unchanged. These are whole-image static section deltas,
including alignment and the coredump-log config difference, not a runtime
heap/DMA budget or physical performance PASS.

The original 7,456-byte dump still hashes to
`75afa985b81eea00e253a8e2410ec55364960bcc65c873a4db547b65b8e0c839`.
The preserved installed-image ELF remains
`bc10d523fee5263ffb81a3033f01145d944e756645fddcc12d3bf038dac91798`.
Both were rechecked after building; neither was overwritten. This work makes
no new live device-state claim and does not reopen the operator-closed S2 gates.

## Reading future evidence

Retain the original raw download and the outgoing application's matching
BIN/ELF from its local `debug/` archive. Use the ESP-IDF Python environment:

```powershell
python tools/decode_restart_capture.py --raw crash-dump.bin `
    --app-bin outgoing/main-deck-m3.bin --app-elf outgoing/debug/main-deck-m3.elf `
    --output restart-capture.json
```

The decoder validates the raw dump checksum and exact extent, full BIN/ELF
SHA equality, stored dump ELF prefix, symbol/region extent and record ABI.
It refuses to overwrite input artifacts or an existing output. The dump's
short identity prefix remains an explicit association limit, even when the
retained BIN/ELF pair matches in full. Missing capture is an error; an
uncaptured record is reported as such, never as zero-valued fault evidence.

Capture does not change signed OTA identity, partition layout, startup health
gate, panic audio quarantine or cache writeback behavior. New installation,
physical crash export and restart reliability remain **NOT RUN** until they
actually occur. No additional S2 soak or acceptance is scheduled by this work.

## Wireless installation and GUI confirmation

One signed local POST `/api/ota/p4` returned HTTP 200 and installed the exact
clean `0b08fb90` source and ELF listed above in VALID `ota_1`; API returned idle.
The operator confirmed that the GUI returned normally. Both decks were stopped
and the controller/USB audio stream was absent at admission. This is installation
and stopped GUI evidence, not audio or historical restart-workload acceptance.

Evidence: `.cache/restart-capture-wireless/20261010T214139Z/`, including before/after
firmware, status, startup polls, install result and raw crash export. The new
7456-byte download has the same full SHA-256 as the validated historical dump
above; no new PANIC replaced it during this OTA. Outgoing `58f09941` and installed
`0b08fb90` matching ELF archives are retained. The user did not request a new
flash-image backup. Future natural PANIC capture and restart reliability remain
NOT RUN; no fault is intentionally induced and S2 tests stay closed.

## First natural captured OTA restart fault

The authorized local OTA from clean `90fba690` to `04f0cfb6` with Hercules
attached completed upload and booted the exact new image VALID, but replaced
the device's crash slot with a natural outgoing `ota_reboot` panic. Both decks
were stopped. The old 7456-byte original remains byte-identical in its separate
archive; no intentional fault or repeat upload was performed.

The new raw dump is 7776 bytes, SHA-256
`356d0a050d773a9eb287f7a1da51a1e9ba1c67bdb9d99e0af85d47d6902a4a14`.
It is retained as `natural-ota-panic.bin` under
`.cache/hercules-fifo-wireless/20261010T221928Z/`, together with download
headers/status, decoded JSON, matching-symbol results and disassembly.
The decoder validates checksum, exact extent, full outgoing BIN/ELF association
and the dump's `38c637996` prefix against outgoing ELF SHA-256
`38c63799653e616c0f707ddda7433b70423681aa5c8d1c86dd7463a950315387`.
Its nine-character stored prefix remains the explicit dump association limit.
Export occurred with stopped decks and the new idle USB stream active; this
was outside a playback qualification window. The new image was not reset.

Core 0's captured record is valid at restart phase 2 (`esp_restart_noos`).
The frame's PC is `0x4ff0c8e0`, cause `0x7`, fault address `0x3ff1009c`;
the contemporaneous CSR cause is `0x30000007`. Three valid queued-store PCs
resolve to `Cache_WriteBack_All` at `0x4ff0c8d0`, `0x4ff0c8d4` and
`0x4ff0c8de`, targeting `0x3ff100a0`, `0x3ff100a4`, `0x3ff10098`.
The return address resolves to `esp_restart_noos_inner`, line 165 of the pinned
ESP-IDF 6.0.2 source. Disassembly also identifies the preceding store at
`0x4ff0c8cc` to `CACHE_SYNC_MAP_REG` (`0x3ff1009c`). This explains the
normalized summary PC and establishes the cache-writeback register sequence
as the fault context. It does not establish why the bus rejected the access,
or prove that a specific peripheral caused it. Core 1 has no captured record.
Sampling is sequential on one core, not an atomic bus snapshot.

Natural raw capture/export/decoding now has actual-device evidence. Restart
reliability remains open even though installation reached VALID; no cache
writeback operation was bypassed or disabled as a speculative fix.

## Cache synchronization candidate after the natural fault

The pinned IDF normal restart calls shutdown handlers, disables preemption on
the restart task, then enters `esp_restart_noos`. That function resets/stalls
the other core before its unguarded `Cache_WriteBack_All` call. Normal `msync`
and mmap invalidation operations instead use the shared `esp_mm` cache sync
critical section. Resetting a peer in such an operation is an unjoined
transition; the dump does not contain the cache busy state needed to prove
that this transition caused the observed bus rejection.

The shared normal restart wrapper now joins that existing IDF critical section
after shutdown handlers and before the peer reset. The no-return reset retains
the lock, allowing an earlier sync to finish and excluding subsequent peer
sync operations from the final writeback. It retains IDF's writeback, watchdog,
peripheral reset and CPU reset sequence. There is no steady-state hook, delay,
polling, allocation or added log in playback.

Architecture exceptions and SoC pseudo-exceptions both set an internal SRAM
panic marker before delegating to the original IDF handler. A direct noos
restart, invalid core, or restart after either panic path bypasses the join:
the other core may already be stalled while owning the lock. The pseudo-panic
hook only marks this bypass; it does not claim new watchdog/abort bus capture.
The existing 256-byte raw capture ABI and old dump decoder remain unchanged.

Production host cases exercise normal restarts on both cores, frozen exception
restarts, direct noos entry and a pseudo-panic during shutdown. The actual ELF
gate additionally requires IRAM placement of the cache join, an internal SRAM
panic marker and interception/delegation of the SoC panic hook. This candidate
addresses the synchronization gap; physical restart reliability and causality
remain unqualified until a matching installed candidate is observed.

### Exact candidate and stopped restart observation

Clean source `ac9287af4a4c07d826474ed11f0688e81135e515`, version
`M3-dev-gac9287af4a4c`, passes the complete qualification host suite (exit 0,
no SKIP), the sole M3 ESP-IDF 6.0.2 build and the actual linked restart contract,
five raw-pipeline fixtures, board identity/isolation and documentation checks.
The production test's initial full-run C99 compile failure (missing explicit
`stdbool.h` in the test) was corrected; the final full suite passes. No JC build,
dependency update, public release or S2 physical retest was performed.

| Artifact | Verified value |
| --- | --- |
| BIN | 2,602,400 bytes; SHA-256 `4e0e7145bb57092a52f455808617a286cc422a2557bae767fc8381891ea1e2bf` |
| ELF | SHA-256 `53808ea215b37addd063c783afbd995b219b4a3c000b9d8b332cc86230da9113` |
| Signed bundle | 2,602,588 bytes; SHA-256 `faae2bb5293f1911c0252be77aea1945ba03123cc877bb2f79d5dc91f16ab1a6` |

Package/debug archive and hashed software evidence are under
`.cache/restart-cache-join-candidate-ac9287af/pajoniiir-m3-M3-dev-gac9287af4a4c/`.
The manifest and bundle verify with the existing trusted `rel-001` key.
One local OTA returned HTTP 200 and the exact image reached VALID `ota_1`.
The original natural dump remained byte-identical. Host Wi-Fi reconnection was
needed during observation; that delay is not a demonstrated device boot fault.
OTA evidence: `.cache/restart-cache-join-wireless/20261011T080615Z/`.

One subsequent marked POST `/api/validation/reboot` returned HTTP 202 with
Hercules MIDI and its idle 44100-Hz UAC1 stream active, storage present and both
decks stopped. The persisted journal shows boot 418 -> 419, exact candidate
version/slot unchanged and `reset=SW`. The API returns the same exact VALID
image; Hercules MIDI and 44100-Hz streaming recover without a fault. Before and
after raw crash exports both hash to
`356d0a050d773a9eb287f7a1da51a1e9ba1c67bdb9d99e0af85d47d6902a4a14`.
The single restart completed without new panic evidence and without a second
request. Records: `.cache/restart-cache-join-check/20261011T080821Z/`.

This is **one stopped normal restart observation**, not a long restart campaign,
playback/audio qualification, new-image eyes-on GUI confirmation, or proof that
the historical cache fault's cause is resolved. No cache registers are sampled
from the fault handler and no writeback is skipped. The S2 closeout stays closed.
