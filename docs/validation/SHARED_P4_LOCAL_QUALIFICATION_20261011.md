# Shared P4 local ordinary-target qualification, 2026-10-11

## Frozen source and scope

Qualified clean source: `5ff76b6cb1474779c6e58ecd5b273dcee378a3ed`.
All commands below ran locally. This record qualifies ordinary JC4880, JC1060
and M3 software builds, the complete host gate and UI simulator matrix.
It is not a physical acceptance or publication record. Later documentation
commits do not change the qualified artifact SHA.

The installed M3 remains accepted Hercules bench source `e16ecfb5`, VALID
`ota_0`, with controller USB3/storage USB2. No flash, OTA, playback, USB change,
Wi-Fi change or reset was performed. The newly built ordinary M3 instead has
production FLX4 USB2/storage USB3 defaults; no bench acceptance is transferred
to that image or topology. S2 stays operator-closed.

## Simulator defect found and repaired

The first legacy simulator link failed because `ui.c` references the shared
atomic `ui_idle_signal_notice` and `ui_idle_signal_is_shown` functions but the
simulator omitted `ui_idle.c`. Commit `5ff76b6c` adds that production source to
the common simulator CMake source list. Runtime/product variants inherit the
list. No mock substitute, production firmware change or screenshot-baseline
update was needed.

The initial JC4880 build started on previous `d11753d2` compiled successfully,
but its exact-source verifier correctly rejected it after the test fix changed
HEAD. It is excluded from final qualification, with its log/result retained
under `pre-simulator-fix`. All three final builds use fresh
`build_qualification_final_20261011` directories and separate SDKCONFIG files.
The complete host run was also repeated on unchanged clean `5ff76b6c`.

## Executed gates

| Gate | Result |
| --- | --- |
| `tests/run_p4_host_tests.ps1 -Qualification -KeepArtifacts` | Exit 0; source unchanged throughout final run; no optional SKIP; committed MP3/WAV/FLAC/PDB/ANLZ fixtures exercised |
| UI simulator presentations | All nine pass: legacy, product-compact/wide/m3, native-compact/wide, runtime-compact/wide, link-wide; existing screenshot hashes retained |
| Production keylock PC soak, JC policy | 300 virtual seconds dual-deck; zero source-clock drift, no detected clicks/clipping |
| Production keylock PC soak, M3 policy | Same 300-second workload and checks pass |
| ESP-IDF version | Exactly v6.0.2 |
| Ordinary clean-directory builds | All three exit 0 |
| `tools/check_board_build.py` | All three pass single matching BSP, canonical component paths, embedded project/source identity and ordinary feature policies |
| Dependency locks | Before/after hashes identical for all three targets |
| Image budget | All three below `0x380000` (3670016 bytes) |
| Independent artifact audit | Host/result/source agreement, simulator/keylock records, build identities, locks and BIN/ELF/MAP hashes pass |

The host suite includes tests of experiment policies and fixtures. That is not
a new ESP-IDF build matrix for recorder/Link/alternative-UI/HS role experiments.
Those current-source firmware variants were not built in this run. Hosted
GitHub Actions remains SKIPPED BY OPERATOR (quota), not PASS. JC physical gates
and real-export/transport variants retain their NOT RUN status.

## Ordinary firmware artifacts

| Target/project | Binary bytes | SHA-256 of complete BIN |
| --- | --- | --- |
| JC4880 / `main-deck-p4` | 2605344 | `b0efce8bd64da237c9451f0111b0a37c46191f3a2af67335bc736add089d3ec2` |
| JC1060 / `main-deck-jc1060` | 2663424 | `42ce466cfb3cb9a676e7fe88310362261330434e008b368ef143c7a11970e703` |
| M3 / `main-deck-m3` | 2598608 | `4a87ff980c8a19a5a5b53fb65138a530c5eeaac0e2607fbf9d0478346e1dd5e2` |

Every compiled identity is the frozen full SHA above with `source_dirty=0`.
Each board uses its committed dependency lock. The three regular build
directories retain BIN/ELF/MAP, board identity and budget JSON files.
The independent audit records hashes for all 15 files; these are unsigned
local build artifacts, not signed OTA packages or release assets.

Logs, SDKCONFIG files, before/after lock hashes, build results, exact-source
host result, simulator/keylock logs and independent audit/hash manifest are
retained at
`.cache/shared-local-qualification-20261011/final-5ff76b6c/`.
The audit ran with clean unchanged HEAD before documentation edits.

## Remaining delivery boundaries

Ordinary local software qualification is closed for this exact SHA. A future
delivery must select each reviewed configuration and preserve its exact
signed package/debug provenance. Fresh experiment firmware build gates,
remaining real-media/JC physical gates, historical OTA-restart causation and
publication authorization remain separate. No new physical campaign or repeat
of accepted Hercules/S2 tests was started. The shared-core whole-project
completion criterion is not established solely by this local matrix.
