# JC8012P4A1 integration preparation

This directory is a preparation workspace, **not a fourth production target**.
No BSP/entrypoint, touch driver, panel table or OTA identity is activated.
The supported production targets remain JC4880, JC1060 and M3.

The [2026-10-19 source audit](../../../docs/reviews/2026-10-19-jc8012-source-audit.md)
identifies a published Apache-2.0 panel-table candidate equal to all 204 donor
commands, without importing it here. A registry MIT touch wrapper has a passing
IDF6.0.2 synthetic-payload syntax probe, but its manifest requires6.1 and its
NOTICE explicitly excludes the controller firmware. Touch distribution remains
unresolved; the original vendor/factory payloads below remain excluded.

Frozen donor: `694ecd6a3f59caa99fd0811e11e0c29d77219411` on
`jc8012p4a1-v3`. Relevant adaptations must preserve the canonical USB ancestry,
audio, controller, startup-health and shared-core fixes; do not merge that
branch wholesale.

## BSP contract to implement after provenance resolution

| Area | Donor contract | Canonical status |
| --- | --- | --- |
| Geometry | 1280x800 UI, 800x1280 panel, rotation 270 degrees | Prepared requirement, no active capability entry |
| Panel | JD9365, two DSI lanes; reset GPIO27; LDO3/2.5 V | Factory init table excluded: donor explicitly records no attached licence |
| Backlight | GPIO23 MCPWM; donor reports LEDC faults on P4 v3.2 | Requires resource cleanup/fallback audit and exact-board verification |
| Touch | GSL3680, I2C address 0x40 | Vendor code and firmware table excluded: no attached licence |
| SD | Slot 0, GPIO43/44/39/40/41/42, LDO4 | Reuse reviewed board-owned sequence only after board ownership verification |
| Network/audio/USB | Board adapters over shared production modules | No imported Wi-Fi policy or FIFO changes |

Both the GSL3680 driver **and** `jd9365_guition_init.inc` need documented
distribution terms or compatible replacements. The donor panel header says its
204-entry table was decoded from factory firmware without an attached licence.
Local donor bench permission does not resolve canonical distribution. No copy
of either payload is stored here. Do not substitute an unverified panel table
and present a successful compile as display acceptance.

## Private PPA preparation

The donor's seven-file PPA override matches stock IDF6.0.2 in six files and
changes only the SRM DIG-734 leftover calculation in `src/ppa_srm.c`.
`ppa/pins.json` records normalized SHA-256 values and the frozen donor result.
The original Espressif Apache-2.0 notices remain in generated files; the IDF
licence is copied alongside them. The before/after fragments are adapted from
Espressif's IDF6.0.2 code and the MIT-licensed donor changes. The donor cites
upstream `469aa16`; no independent upstream equivalence is asserted here.

`tools/prepare_jc8012_ppa.py` accepts only the pinned seven input files, applies
one exact replacement, verifies the complete resulting SRM hash and writes a
new output directory. It never edits IDF, managed dependencies or shared core.
Existing output and a wrong board fail before generation. A future entrypoint
may include `ppa/select.cmake`, call
`pajoniiir_prepare_jc8012_ppa(jc8012p4a1 private_ppa)` **before** IDF component
discovery, then append that returned path to `EXTRA_COMPONENT_DIRS`. It must
define the matching board Kconfig choice. The generated component rejects all
other boards again during configuration.

No existing entrypoint includes this helper. The ordinary three-target build
verifier requires the stock IDF PPA component **and its compiled SRM source**.
The private overlay has not been compiled as a complete JC8012 firmware. Its
algorithm fragments and configure/generator contracts are host-tested; panel
freeze recovery, rendering, touch, backlight, USB, audio and resource budgets
all remain NOT RUN on a canonical JC8012 image.
