# Historical M3 OTA restart crash: offline evidence audit

Date: 2026-10-10. This continues the
[wireless crash capture](../validation/CRASH_DIAGNOSTICS_20261010.md)
after the operator closed further S2 testing. It examines the preserved dump
and outgoing image offline. The installed diagnostic image, USB connections,
controls and Wi-Fi were not changed during this analysis.

## Conclusion

The historical `ota_reboot` exception occurred on **core 0**, with the
interrupted task's stack in **internal memory**, during the IDF restart
cache-writeback path. Complete function bytes and stack return slots establish
the call chain below. Repinning the task, moving the writeback routine into
IRAM or increasing the stack without further evidence would repeat conditions
already present in the failed image.

The PC/address discrepancy has a concrete interpretation hazard: the compiled
IDF panic handler can replace the saved PC with a queued asynchronous bus-error
PC. The dump does not retain the bus-error queue CSRs or the pre-replacement PC.
It therefore cannot identify the precise rejected transaction, prove its
ordering or explain why the cache register rejected a store. **No causal
firmware repair is established by this audit.**

## Identity and reconstruction limits

The original 7456-byte dump remains unchanged, SHA-256
`75afa985b81eea00e253a8e2410ec55364960bcc65c873a4db547b65b8e0c839`.
Its embedded checksum passes the installed `esp_coredump` loader. Its stored
ELF identity is the nine-character prefix `5f70083d0`; this matches the
recorded outgoing image prefix, not a full cryptographic ELF match.

The preserved outgoing BIN has SHA-256
`7bf603045a3d39c398668dd3a6eea7e42b5b581a740e46cd675d6d87590a8e1a`
and embeds ELF SHA-256
`5f70083d0c64ec66ca38b049845e6eb6f9e3960a341e63c7b8cb54a55e052488`.
Its recorded clean source is `3f3605a7ba7fb2ce891b703aae1b69864bce6468`.

One isolated ESP-IDF 6.0.2 M3 build of that exact historical source completed
with exit code zero. It used the pinned dependencies, the current HS config
re-solved against historical Kconfig and the original descriptor compile epoch.
The historical checkout remained clean and its committed lock was unchanged.
This was an analysis build: it was not packaged, installed or released.

The reconstructed ELF hash is
`10ca3d47d1a07c6545c27ee53c9ab28f7728a4c1d9a3d38ed948b65f9c1ef0c7`.
It **does not match the original ELF**. Loaded image bytes differ beyond the
descriptor ELF hash; the original config was not retained. Serialized IRAM
segment boundaries also differ, but their combined address extent is the same
98756 bytes. Comparing by loaded address finds 492 differing IRAM/data bytes,
not a 520-byte deletion of restart code. Whole-image symbolization with this
ELF remains inappropriate.

Instead, each selected function's complete reconstructed bytes were compared
at its address against the original BIN, across serialized segment boundaries.
Only equal functions were used for the manual unwind:

| Function | Original address | Complete bytes | Byte equality |
|---|---|---:|---|
| `esp_restart` | `0x480014c6` | 50 | PASS |
| `esp_restart_noos` | `0x4ff0cc02` | 140 | PASS |
| `esp_restart_noos_inner` | `0x4ff00b60` | 78 | PASS |
| `Cache_WriteBack_All` | `0x4ff0c60c` | 72 | PASS |
| `delayed_restart_task` | `0x4803029c` | 24 | PASS |
| `vPortTaskWrapper` | `0x48188b00` | 142 | PASS |
| `panic_arch_fill_info` | `0x4800268a` | 122 | DIFFERENT: one byte |
| `service_log_sync` | `0x480ecf70` | 296 | DIFFERENT: six bytes |

The last two functions were not accepted as complete byte matches. The panic
mechanism described below was read directly from the original BIN; it does not
depend on treating the reconstructed panic function as identical.

## Exception frame and verified call chain

The first 128 bytes of the saved exception frame exactly match the first
PRSTATUS note's general registers. The remaining frame fields confirm:

| Field | Saved value | Interpretation |
|---|---|---|
| TCB/task | `0x4ff35eac`, `ota_reboot` | OTA restart helper |
| PC | `0x4ff0c624` | Inside byte-verified cache writeback |
| RA | `0x4ff00b70` | Return from writeback to restart inner |
| SP | `0x4ff33c80` | Internal task stack |
| `mhartid` | `0` | Actual exception core, independently of task pinning |
| `mcause` | `7` | Store access fault |
| `mtval` | `0x3ff1009c` | Cache sync map register address |
| `a3 / a4 / a5` | `0x3ff10000 / 0x3ff10098 / 4` | Cache register base/control state |

The manual unwind follows saved return slots using the exact matched RV32
prologues and call instructions:

```text
vPortTaskWrapper         call 0x48188b0c, return 0x48188b0e
  -> delayed_restart_task  call 0x480302b0, return 0x480302b4
    -> esp_restart           call 0x480014f4, return 0x480014f8
      -> esp_restart_noos      call 0x4ff0cc8a, return 0x4ff0cc8e
        -> esp_restart_noos_inner  call 0x4ff00b6c, return 0x4ff00b70
          -> Cache_WriteBack_All
```

Return addresses at function ends must be interpreted through their call
sites. A raw `addr2line` lookup of `0x4ff0cc8e` names the following function,
`esp_system_reset_modules_on_exit`; that is not evidence that this later
reset function caused the saved exception.

The saved 100-byte TCB fits the reconstruction's DWARF layout: name
`ota_reboot`, priority 5, core field 0, stack base `0x4ff32d54`
and end `0x4ff33d50`, a 4096-byte extent. This layout interpretation is
corroboration, not recovered original DWARF. The frame and SP directly prove
internal placement. The dump does not include the bottom stack canary and
does not prove that no earlier overflow or corruption occurred.

## Why PC and MTVAL cannot identify one synchronous store

Original instructions distinguish the adjacent stores:

```text
0x4ff0c620  sw a5,156(a3)    address 0x3ff1009c: sync map
0x4ff0c624  sw zero,160(a3)  address 0x3ff100a0: sync address
0x4ff0c628  sw zero,164(a3)  address 0x3ff100a4: sync size
0x4ff0c62c  addi a4,a3,152   address 0x3ff10098: sync control
0x4ff0c630  li a5,4
```

ESP32-P4 supports asynchronous load/store bus-error reporting: later
instructions may execute before the exception is delivered. Its additional
bus-error CSRs retain queued instruction/address information. See the
[Espressif TRM, Bus Error Response](https://documentation.espressif.com/esp32-p4_technical_reference_manual_en.pdf).

The local pinned IDF helper
[`rv_utils_asynchronous_bus_get_error_pc`](https://raw.githubusercontent.com/espressif/esp-idf/v6.0.2/components/riscv/include/riscv/rv_utils.h)
checks the bus-error flag and chooses the oldest valid store PC from STPC2,
then STPC1, then STPC0. The
[panic handler](https://raw.githubusercontent.com/espressif/esp-idf/v6.0.2/components/esp_system/port/arch/riscv/panic_arch.c)
can overwrite `regs->mepc` with this PC.

The same mechanism is confirmed in the **original BIN**, at
`0x480026ba–0x48002702`: MEXSTATUS read, bit-8 check, MCAUSE-7 branch,
STPC2/1/0 reads, valid-bit removal and the frame-PC store at `0x480026ec`.
This is a concrete explanation for why saved general registers may describe
a later execution point than the selected error PC. It is not proof of which
queue entry was selected in this particular crash. MEXSTATUS, STPC/STTVAL
pairs and the original MEPC are absent from the preserved frame and notes.

## Repair implications and remaining evidence

- **Wrong core:** excluded for this exception by `mhartid=0`. Core-0 pinning
  was already present. It is not a new repair.
- **External task stack:** excluded at the saved exception point. Increasing
  the 4096-byte allocation has no demonstrated connection to this store fault.
- **Missing IRAM writeback patch:** excluded for the faulting routine. The
  preserved implementation is in IRAM and already contains the double-write
  sequence; the [upstream IRAM patch](https://github.com/espressif/esp-idf/commit/751a75a982)
  is not an absent fix to apply again.
- **General heap exhaustion or waveform failure:** not established by this
  dump. Earlier cached resource evidence retains its limited workload scope.
- **Bus/cache protection, ordering or shutdown interaction:** unresolved.
  No saved register state identifies the rejecting condition. Activity of
  another task's saved context is not proof it was executing concurrently at
  the failing store.

The necessary next diagnostic change is to retain the **unmodified trap MEPC,
MEXSTATUS, STPC0–2 and their STTVAL0–2 pairs before IDF normalizes the PC**,
plus bounded restart-phase markers. Capture must be allocation-free, avoid
logging/NVS in the exception path, use internal storage and survive the panic
through a verified coredump or persistent-record contract. Reading these CSRs
after reboot cannot reconstruct the old fault. This must be designed and
verified against the actual IDF panic/coredump lifecycle before installation;
it is not implemented by this report.

Do not bypass cache writeback, replace `esp_restart` with a watchdog reset,
disable panic audio quarantine or upgrade IDF speculatively. Those changes
would alter safety/restart behavior without evidence of a causal fix.

The latest successful diagnostic OTA had **no enumerated controller and no
USB audio stream before or after restart**. It confirms one installation,
health gate and wireless export, but does not reproduce the historical
controller/audio workload or qualify restart reliability. The installed
`58f09941` image remains unchanged. S2 testing stays closed; no acceptance
waiver was converted to PASS.

## Reproducibility and verification

Original evidence is under `.cache/crash-diagnostics-wireless/20261010T203454Z/`;
the original 34-file hash inventory was rechecked without replacing it.
Derived reconstruction evidence is separate under `.cache/r3/`:

- `build.ps1`, `build-exit.json`, `build.log`, historical clean checkout,
  reconstructed ELF/BIN/map/config;
- `verify_reconstruction.py`, `reconstruction-audit.json`;
- `verify_function_bytes.py`, `function-byte-audit.json` with per-body hashes;
- `audit_exception_frame.py`, `exception-frame-audit.json`, `tcb-layout.txt`;
- verified disassembly and a separate derived-evidence hash inventory.

Offline function/frame audits pass their explicit assertions. One analysis
M3 build passes; no JC build or new firmware host suite was needed for this
documentation/evidence-only change. Documentation integrity passes for 112
Markdown/HTML files and 134 retired paths; whitespace checks pass. No device
query, playback, controller
test, OTA, push, release or public publication was performed in this audit.
