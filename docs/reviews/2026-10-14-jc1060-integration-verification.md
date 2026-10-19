# JC1060 selective integration: software verification

Implementation source: clean `c0ee81527b4dbd5d83599140aaa106ebcd27772a`.
This record is a documentation-only follow-up; its commit is not the compiled
image identity. The [implementation report](2026-10-14-jc1060-selective-integration.md)
records fixes, provenance and remaining physical gates.

## Ordinary ESP-IDF 6.0.2 matrix

Fresh ordinary build directories were configured from board defaults, followed
by reconfigure/build from the clean implementation commit. All three builds,
board/isolation/budget checks and actual linked restart-capture checks pass.
The compiled source metadata has `source_dirty=0` on all three images.
Dependency locks are unchanged. No image has been installed or packaged here.

| Target | Version | Image bytes | Result |
| --- | --- | ---: | --- |
| JC1060 | `M2.5-226-gc0ee8152` | 2,668,576 | Software PASS; physical NOT RUN |
| M3 | `M3-dev-gc0ee81527b4d` | 2,603,200 | Software PASS; physical NOT RUN |
| JC4880 | `M2.5-226-gc0ee8152` | 2,609,856 | Software PASS; physical NOT RUN |

All are within the `0x380000` application budget. Project identities remain
`main-deck-jc1060`, `main-deck-m3` and `main-deck-p4`, respectively. The M3
development version has no automatic pull-update eligibility. This is not a
release-version or signed-package qualification.

The [machine-readable evidence](evidence/2026-10-14-jc1060-integration/verification.json)
contains image/ELF/config/lock hashes, exact source identities, local build paths,
actual verifier output and linked restart hook/coredump bounds. Original build
logs remain under `.cache` at the recorded paths and hashes.

## Host and UI gates

- `tests/run_p4_host_tests.ps1 -Qualification -KeepArtifacts`: exit 0 on the
  clean implementation source. Mandatory MP3/WAV/FLAC/PDB/ANLZ fixtures execute;
  no optional SKIP. The [full log](evidence/2026-10-14-jc1060-integration/host-qualification.txt)
  includes existing production regressions and the new hub, cadence, CUE,
  USB MAIN, loop EOF, network startup, canceled-worker and ELF-contract checks.
- Production stream harness passes under JC4880 and M3-HS policies, including
  DDJ-400 1-ms HS UAC1 and existing FLX4/Hercules/S2 behavior. Profile compile,
  parser/storage CRC and both-deck four-note press/release mappings pass.
- UI simulator `product-compact`, `product-wide`, `product-m3` and `link-wide`:
  navigation and every existing screenshot hash pass. Baselines are unchanged.
  Filtered results are retained in the evidence directory; complete local logs
  are hashed by the manifest. Idle layout is unchanged; production function
  regression separately executes busy progress and cancellation retirement.
- CMake ancestry transform and scoped Git checkout regression: PASS on Windows
  and Ubuntu/WSL. The [Linux log](evidence/2026-10-14-jc1060-integration/linux-dependency-policy.txt)
  exercises actual CMake source selection, drift rejection and Git CRLF clone
  bytes while preserving inherited settings. Ubuntu initially lacked Ninja;
  a task-local `/tmp` virtual environment supplied pinned Ninja 1.11.1.4.
  The full Linux firmware build and Docker tools remain NOT RUN.
- Documentation integrity and tracked/staged whitespace checks: PASS. README
  remains English; all existing README image embeds are unchanged.

To reproduce the software gates, initialize the mandatory IDF 6.0.2 profile,
append MSYS2 UCRT64 GCC to PATH for host tests, and run the host command above.
For each ordinary target use a fresh `idf.py -C firmware/<project> -B <build>`
directory and its own `SDKCONFIG`, then run `tools/check_board_build.py` and
`tools/check_restart_capture_elf.py`. The latter validates linked code and is
not a physical PANIC-export test. UI commands use `-Presentation <name>
-KeepArtifacts`; do not update baselines automatically.

## Open acceptance

JC1060 SD warm reset, real hub routing/recovery, USB MAIN reconnect without
LOAD, DDJ-400 MIDI/LED/audio, requested Ethernet/Wi-Fi readiness and signed
OTA/rollback still require the exact candidate on hardware. Live Link peer
loss/download/artwork cases remain experimental. Hosted CI was not started.
No push, installation, new beta or public OTA update occurs in this step.
Historical contributor listening, M3 controller soaks, S2 operator closeout and
the JC4880 soak waiver retain their original scope.
