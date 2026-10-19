# JC8012 source and IDF compatibility audit, 2026-10-19

PR54 is merged into canonical `master` as
`37d5bddb663b97a2d893bade2175afe6392788a1`. Its reviewed head is
`b8b49eae5fa3a634b84d687f4de0a94e4b03063a`; all thirteen migration jobs pass
on both revisions, including the enabled JC1060 SD-power configuration.
Master's documentation, Pages and separate dual-USB workflows also pass.
The duplicate PR migration run was deliberately cancelled; the successful push
run qualifies the same head SHA. Manual review found no blocking issue; the
previously authorized manual workflow was retained rather than requesting a
CodeRabbit CLI review. No donor branches were deleted.

The research below changes no production firmware, dependencies or board
selection. It identifies a published panel-table candidate, an explicit
firmware exception inside a registry package labelled MIT, and a narrow
IDF6.0.2 compatibility result. JC8012 remains inactive.

## Sources and decisions

| Candidate | Pinned source/evidence | Decision |
| --- | --- | --- |
| `guition-jc/esp_lcd_touch_gsl3680` 0.1.0 | Registry archive and repository `Guition-JC/Guition` at `2afaa070e4e35a5b3ff9c54cfba7cb41c24a4f81`; component LICENSE, NOTICE, manifest and code inspected | New/reworked wrapper contributions are MIT, but NOTICE explicitly excludes the retained controller firmware. Do not import the complete package. |
| HomeTiles JC8012 V2 panel header | `GalusPeres/HomeTiles` at `51440114b059b554cbd74da7000b7b761e724bd2`, per-file Apache-2.0 notice | Published candidate for the command table; all 204 ordered commands/data/delays equal the donor factory table. Not imported or activated here. |
| Manufacturer's public JD9365 source | `guitionofficial/P4-series` at `9f1510954d281e30b19cd8baf115e2ed2a65a90a`, file blob `80492fc86104e5349d5fec659bfbd1ea9562b700`, Apache-2.0 file header | Observed table has 197 commands and differs from donor. Do not substitute it just because the board model matches. |
| `mangoo1/esp_lcd_touch_gsl3680` 1.0.0 | Registry README declares GPL-2.0-or-later tracking code and vendor files without their own original licence headers | Not a permissive replacement for this repository. A dependency declaration alone does not establish permission for retained payloads. |
| `onitake/gsl-firmware` | Repository README explicitly distinguishes GPL tools from firmware that may carry proprietary terms | No verified distributable JC8012 payload found; not a licence solution by itself. |

Primary references:

- [Published touch package](https://components.espressif.com/components/guition-jc/esp_lcd_touch_gsl3680/versions/0.1.0/readme),
  [dependencies](https://components.espressif.com/components/guition-jc/esp_lcd_touch_gsl3680/versions/0.1.0/dependencies).
- [Pinned touch NOTICE](https://github.com/Guition-JC/Guition/blob/2afaa070e4e35a5b3ff9c54cfba7cb41c24a4f81/components/touch/esp_lcd_touch_gsl3680/NOTICE),
  [manifest](https://github.com/Guition-JC/Guition/blob/2afaa070e4e35a5b3ff9c54cfba7cb41c24a4f81/components/touch/esp_lcd_touch_gsl3680/idf_component.yml).
- [Published Apache-2.0 panel header](https://github.com/GalusPeres/HomeTiles/blob/51440114b059b554cbd74da7000b7b761e724bd2/src/devices/guition_jc8012p4a1_v2/vendor/jd9365_init_cmds.h),
  [first-hand panel investigation](https://github.com/GalusPeres/HomeTiles/issues/18).
- [Alternative registry package](https://components.espressif.com/components/mangoo1/esp_lcd_touch_gsl3680/versions/1.0.0/readme),
  [Silead firmware repository](https://github.com/onitake/gsl-firmware).

## Touch: the package label is insufficient

The actual 33,818-byte registry archive was downloaded and examined, not only
its web description. Its SHA-256 is
`6d10bb0f75cf37ca03346b8262bf3558e183e3e8b15d1588e36f5252bc21a9ea`.
The archive's driver, LICENSE, NOTICE and firmware header match the pinned
repository bytes. Its manifest differs in serialization, repository URL
scheme and registry-injected commit metadata; dependency and licence fields
are semantically equal.

NOTICE says the MIT licence does not grant rights to the bundled firmware and
calls for its owner's authorization. Thus the registry's MIT label does not
resolve the canonical payload distribution gate. The firmware remains in the
ignored research cache only; it is not copied into tracked source, evidence or
a generated component. The published wrapper uses its own lightweight tracking
rather than the old Silead algorithm, but that does not remove its firmware
dependency.

The examined published and donor payloads both have 4,356 register/value pairs,
with the same address sequence but **two differing values**. They are not
payload-equivalent; the meaning of those differences is not established here.
Do not infer functional parity from their size or matching controller name.

## IDF6.0.2: syntax compatibility, not a usable component

The published manifest requires `idf >=6.1.0,<6.2.0` and
`espressif/esp_lcd_touch ==1.2.1`. Canonical firmware already pins touch1.2.1,
but its IDF6.0.2 does not satisfy that published manifest. Do not loosen the
manifest silently or upgrade all boards to solve this research task.

A targeted RISC-V `-fsyntax-only` probe executes the unmodified MIT wrapper
against the ordinary JC1060 compile command, IDF6.0.2 headers/sdkconfig and the
existing touch1.2.1 dependency. The controller firmware include is replaced
with a one-entry synthetic stub. Header tracing confirms the stub was used and
the real firmware header was not included. The probe passes without source/API
edits or warnings. A replay helper reproduces the comparisons and probe.

This establishes only syntax/type compatibility of the wrapper against those
headers. It does not resolve the declared version constraint, link or run the
driver, exercise firmware upload/tracking, measure resources, or qualify P4v3,
touch mapping, corners, wake/drag or multi-contact behavior. No linked image is
produced. The synthetic stub is never a runtime firmware candidate.

## Panel: a concrete published candidate

The pinned HomeTiles header has an explicit Espressif copyright and
Apache-2.0 SPDX notice. Its 204 command IDs, transmitted data lengths/bytes,
order and delays match the donor's decoded factory table. The normalized
semantic SHA-256 is
`3bbccf59f487f54eb71bbe027ffd30f4e6899757b536c3cc1672f882e104c414`.
File hashes differ because declarations, include paths and comments differ;
this is command-sequence equivalence, not byte-identical source files.

This supplies a published, explicitly licensed source candidate rather than
relicensing the old factory dump. A later import should use the published file,
pin its commit, retain its notice and Apache licence, and adapt only the include
path/array name while testing sequence equivalence. The V2 label on that source
does not establish board/silicon/timing equivalence with the donor's P4v3 unit.
No BSP timing, display or hardware gate is closed by this comparison.

The manufacturer's currently observed older file contains 197 commands, not the
198 described for an older bundle in the issue. Direct pinned-source results
take precedence over a historical count. It is not equivalent to the 204-entry
candidate and is not selected.

## Next bounded work

1. A published-source panel import can now be prepared independently of touch,
   preserving notices and the verified command sequence. Keep it board-scoped
   and inactive until the complete BSP is built and tested.
2. Resolve the exact touch firmware's redistribution terms with the publisher
   or rights holder. Alternatively, evaluate a separately provisioned firmware
   provider without bundling that payload; this is a product/provisioning
   decision, not an automatic permission or an implemented workaround.
3. Once a distribution path exists, adapt the MIT wrapper in isolation for
   IDF6.0.2 and run actual initialization/error/ownership regressions, then
   physical mapping/wake tests on the exact JC8012 panel revision.

No maintainer/vendor message was sent. Existing M3/JC4880 images, releases and
public OTA are unchanged; JC1060 SD physical acceptance remains with Kayrosen,
the experiment remains OFF in production, and S2 stays operator-closed.

The [evidence inventory](../validation/evidence/2026-10-19-jc8012-source-audit/README.md)
retains immutable source metadata, notices, digest/count comparisons, compiler
probe/replay results and the PR54 merge/CI closeout. This is research and
documentation; no full firmware rebuild or hardware run is claimed.
