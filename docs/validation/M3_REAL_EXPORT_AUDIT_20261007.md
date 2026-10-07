# M3 real Rekordbox export audit — 2026-10-07

Result: **AUDIT COMPLETE; THREE REPRODUCED PARSER DEFECTS**. This is a
read-only export audit, not a new firmware or physical UI acceptance.

## Scope and provenance

The operator inserted the previously tested medium into the PC as `F:` (exFAT).
Its `PIONEER/rekordbox/export.pdb` is 385024 bytes, SHA-256
`3b4d4271eae92d7aca032219c14ba7218de5a0e5034fb48f50847d8067e9461e`.
The hash matches the saved M3 catalog's export digest and was identical after
the audit. No files on the medium were edited. No firmware, device controls,
OTA or public channel were changed.

The production PDB and ANLZ sources at repository HEAD `0a11e876e5af` were
compiled with the existing standalone adapters and executed against all 324
tracks. Firmware modules remain those of accepted source
`fb5cbca13f33d21dfb4e16e0ba667c340399eb0c`. An independent Python binary reader
decoded PDB rows and ANLZ section fields for comparison. Layout references:
[Deep Symmetry PDB schema](https://github.com/Deep-Symmetry/crate-digger/blob/master/src/main/kaitai/rekordbox_pdb.ksy)
and [ANLZ schema](https://github.com/Deep-Symmetry/crate-digger/blob/master/src/main/kaitai/rekordbox_anlz.ksy).
These references describe reverse-engineered layouts, not Pioneer certification.

Evidence directory:
`.cache/m3-migration/20261007-fb5cbca1/real-export-audit/`:
`raw-pdb.json`, `production.json`, `audit-results.json`, `artwork-decode.json`.
The ignored helpers are `.cache/audit_m3_raw_pdb.py`,
`.cache/audit_m3_production_export.c` and `.cache/finish_export_audit.py`.
Production execution exited 0; the comparison helper exited 0 to indicate
completion, **not** acceptance of all compared fields. Its mismatches are
retained. Initial compiler execution without the UCRT DLL search path failed
with exit 1 and no diagnostics; appending `C:/msys64/ucrt64/bin` to PATH produced
a successful compile. This was a host setup issue, not a firmware build failure.

## Metadata findings

| Check | Actual result |
| --- | --- |
| Track count | 324 in source, production parser and saved M3 catalog |
| Title, audio path, analysis path, duration and artwork ID | Production PDB output matches all 324 raw rows |
| Artist | 13 nonempty source names lost by the production parser |
| Album | All 12 tracks referencing the five nonempty album rows lose their album name |
| PDB limits / invalid rows | No track/name/playlist/artwork truncation; invalid playlist/artwork row counters zero |
| Saved device title/path comparison | All paths match; two titles are truncated at the catalog's 95-byte title capacity |
| Artwork ID/path table | All 282 rows match raw source |

**Artist row offsets:** 109 source tracks have `artist_id=0`; those empty artists
are export data. There are 169 artist rows and no unresolved nonzero artist IDs.
The other 13 blanks arise because `rekordbox_pdb.c` reads the string at fixed
offset 10 instead of the row's name offset (12 in these records). This affects
12 distinct Unicode names, including `Jura Stublić`, `Pino D'Angiò`,
`Nouvelle Phénomène` and `Raffaella Carrà`. Track IDs are
11, 30, 47, 93, 131, 201, 225, 229, 246, 268, 274, 315 and 317.
None of the 109 source-empty artists has a nonempty artist tag in the file
metadata returned by ffprobe. This does not identify artists from filenames.

**Album row layout:** the common name-table parser also treats albums like
artist/key rows. Actual album IDs and name references use different offsets.
The 12 affected track IDs are 18, 48, 56, 79, 105, 132, 148, 258, 289, 305,
306 and 321. Full expected/actual values are retained in `audit-results.json`.

The catalog truncates titles for IDs 40 and 248. The production PDB decoder
retains both complete exported strings; this is a separate bounded catalog
representation, not title-index corruption or a mismatched media identity.

The decoder also classifies DeviceSQL `0x40` long ASCII as empty. No title/path
difference caused by that branch was found in this medium; add a regression
when correcting the row layouts rather than claiming a fourth observed loss.

## Analysis, cues and waveforms

All 324 DAT and 324 EXT files are present, structurally walkable and accepted
by the production parser. All **236924 beat entries** match source phase,
tempo and timestamp; phases cover 1–4 and times are strictly increasing for
each track. Phase 1 is retained as downbeat. This checks data fidelity, not
whether Rekordbox's musical analysis is acoustically correct.

All 324 tracks have PWAV, PWV3 and PWV4. Production low-waveform presence,
high-waveform lengths and PWV4 byte lengths match the source; no color preview
truncation was observed. Pixel appearance and on-device cover/cue rendering
were not inspected in this PC audit.

There are only two tracks with source cues, and no source loops:

- ID 4: Hot Cue B at 235 ms, retained.
- ID 7, `France Gall - Ella, elle l'a (Clip officiel)`: memory cue at 407 ms
  and Hot Cues A/B/C at 11801/17199/84366 ms are retained. **Hot Cue D at
  123347 ms is lost.** A/B/C reside in DAT PCOB; D resides in EXT PCOB and
  is also represented in PCO2. `anlz_parse_ext_stream()` currently enriches
  PWV3/PWV4 only, leaving the DAT cue list unchanged.

The comparison's single cue mismatch is therefore a real missing slot,
not merely array ordering. PCO2 sections are present throughout the export,
but only the two tracks above contain entries. Future merge code must reconcile
duplicate slots across DAT/EXT and retain separate memory/hot list semantics.
Source loops and memory-loop restore cannot be qualified from this export.

## Playlists, media and artwork

There are **zero playlists and zero playlist entries** in this PDB. Real folder
hierarchy/export order acceptance is **NOT EXERCISED**, even though synthetic
production-module tests passed earlier. This result concerns `export.pdb`, the
database consumed by the runtime; `exportLibrary.db` was not separately audited.

All 324 referenced audio files exist and ffprobe returned success. This is
metadata inspection, not a full audio decode, listening or supported-rate gate:

| Source encoding / sample rate | Files |
| --- | ---: |
| MP3 / 44100 Hz | 196 |
| MP3 / 48000 Hz | 115 |
| MP3 / 22050 Hz | 1 |
| PCM 24-bit / 44100 Hz | 8 |
| PCM 16-bit / 44100 Hz | 2 |
| FLAC / 44100 Hz | 1 |
| FLAC / 96000 Hz | 1 |

All 282 referenced artworks exist, identify as 80×80 JPEG and decode through
ffmpeg without error. This does not exercise the firmware JPEG worker/cache,
stale-generation fencing or a physical display. Tracks without an artwork ID
still require the normal fallback; no missing referenced artwork was found.

## Next implementation and acceptance

1. Correct artist and album row-specific ID/name offsets with bounded row
   references. Add real-layout small fixtures for short/Unicode/far-name and
   long-ASCII fields; do not copy private music or the full export into Git.
2. Merge source Hot Cues from EXT into the DAT model transactionally, including
   the observed D slot. Test duplicate/conflicting slots, absent/empty EXT,
   malformed data, memory/hot separation and local override/tombstone behavior.
3. Invalidate only the metadata cache when corrected import semantics change;
   preserve persistent media IDs and local cue records.
4. Rerun these same 324-track comparisons against corrected production modules,
   then relevant host regressions and an M3-only IDF 6.0.2 build. Build evidence
   must precede any separately authorized candidate installation. The accepted
   fb5 image and its exact-image soak remain immutable evidence.
5. On the corrected candidate, confirm recovered artist/album fields and source
   Hot Cue D. Physical MIDI/touch cue operations and NVS power-loss behavior
   remain separate. A playlist-bearing export is needed for real ordered/nested
   browsing; a source-loop-bearing export is needed for source-loop acceptance.

No GitHub Actions run was started (operator quota waiver remains in force).
No release, push or public OTA publication was performed.

## Correction candidate — software verified, not installed

Source `082d2c0abda01f3406fef3f35f433415c69521bd` corrects all three observed
defects in the shared production modules. Artist/album lookup now uses each
table's ID and near/far name-reference layout, bounds references to the heap
row and decodes long ASCII fields. EXT PCOB cues enrich DAT metadata
transactionally: identical slots are idempotent, conflicting source slots
reject enrichment, and memory lists remain distinct and bounded. The existing
optional truncated-PWV4 fallback to PWV3 is preserved. PCO2-only cue import,
comments and colors are not added by this targeted repair; the observed D
record is present in EXT PCOB.

Metadata cache header version is now 6. Version 5 misses and is rebuilt;
persistent identity and local cue storage are unchanged. Temporary EXT cue
parsing uses a bounded heap allocation, avoiding another full metadata object
on the firmware worker stack. Allocation failure rejects enrichment without
publishing partial data.

New regressions cover real row layouts, Unicode/long ASCII/far names,
out-of-row/slot references, source D and all eight Hot Cue slots, duplicate and
conflicting DAT/EXT slots, memory-loop separation/capacity, reparse, empty EXT,
truncated PCPT, missing PWV3 and old-cache rejection/rebuild. The first focused
run exposed an unintended change to the optional PWV4 fallback; that change
was corrected, and the original fallback regression remains passing.

Validation:

- Complete default shared host runner exited 0. Final focused PDB/ANLZ/cache
  suites also exited 0 after the last test/bounds changes. This is not a new
  physical soak or a `-Qualification` run.
- The exact committed production parser was rerun against all 324 source
  tracks and 648 DAT/EXT files. Independent comparisons report **zero artist,
  album, title, path, duration, artwork-ID or source-cue differences** and
  zero beat/low-presence/PWV3-length/PWV4-length differences. Hot Cue D is
  restored at 123347 ms. Export SHA remained identical.
- Audio/artwork probe results are retained from the preceding PC audit;
  those probes were not unnecessarily repeated in the correction comparison.
- M3 alone passed ESP-IDF **v6.0.2** `reconfigure build` on the clean source;
  this was an incremental build, not a newly full-clean build. Dependency
  lock stayed unchanged. Board identity, image budget, ECDSA bundle and signed
  manifest verification passed during packaging.
- JC4880/JC1060 rebuilds were deferred per the M3-first bench policy; current
  GitHub Actions remains SKIPPED BY OPERATOR. No UI/DSP module changed.

Exact candidate `M3-dev-g082d2c0abda0`, project `main-deck-m3`, board `m3`:

| Artifact | Size / SHA-256 |
| --- | --- |
| Image | 2573808 B; `70ee7fbb65f8d7cef0aa49e8b623c048ad39c409c9986a186cb9202553f7cd74` |
| Signed bundle | 2573996 B; `56cbd15e3b0d6416fea9bf2216078630460f39ddb4cbb2a890aa55c3ea2c9c4c` |
| ELF identity | `5b6148c686766314ef7aa983e87ca8572ff4107bd82c5265d75f07e30660c83a` |

Package directory:
`releases/m3-bench-082d2c0a/pajoniiir-m3-M3-dev-g082d2c0abda0/`.
Fresh comparison evidence:
`.cache/m3-migration/20261007-fb5cbca1/real-export-fix-082d2c0a/`, including
the asserted `result.json` PASS with source and evidence hashes.
Logs: `D:/AI/m3-real-export-fix-{host,focused-final,build,package}.log` and
`D:/AI/m3-real-export-fix-082d2c0a-production.log`.

The candidate has **NOT BEEN INSTALLED** at this checkpoint. On-device import,
cue/rendering and new-image physical acceptance remain open; fb5 exact-image
acceptance is not transferred. The operator was asked to return the stick to
M3 USB3 and stop both decks for the next OTA/device validation step. No public
publication or remote push was performed.

## Installed correction — device import verified

After the operator returned the medium to M3 USB3 and confirmed both decks
stopped, the verified signed candidate was uploaded locally. HTTP 200 was
followed by observed `M3-dev-g082d2c0abda0`, exact source/ELF, **VALID ota_1**
and OTA idle at `2026-10-07T15:58:19.386487+00:00`. The PC Wi-Fi interface did
not automatically reconnect after the AP restart; reconnecting the saved
`Pajoniiir-M3` profile restored access within the helper's observation deadline.
That host gap is not a measured firmware startup duration. The runtime's VALID
state confirms its startup health gate; no manual device reset was performed.

The live library contains 324 tracks and exactly 109 empty artists. Every
artist matches the independent source export, including all 13 corrected
names. FLX4 MIDI In/Out/UAC and mounted USB3 storage are available; Wi-Fi stays
enabled. Startup is `ready` with zero reported allocation/critical allocation
failures. These observations are import/availability evidence, not listening
or physical lifecycle acceptance.

D1 was loaded with track 7 (France Gall); D2 with track 317 (Jura Stublić).
Both reached READY without playback, and the live D2 artist is `Jura Stublić`.
The operator was asked to inspect the GUI and source Hot Cue D without
activating it. Album/source-cue contents are not exposed by the existing
status/library API, so the independent parser comparison does not substitute
for physical cue rendering. Both decks remain stopped; no new soak or audio
test was started. Evidence: `.cache/m3-migration/20261007-082d2c0a/`, including
`ota-install.json`, `after-library.json`, `device-import-check.json` and the
prepared stopped snapshot. Existing fb5 physical/soak records remain immutable.

All 324 persistent identities and export/path mappings were captured over 41
pages and match the previous fb5 catalog exactly. The initial ad-hoc capture
used the wrong pagination field (`has_more` instead of `next_offset`) and
stopped after eight rows; its assertion failed before writing a catalog. The
corrected read-only capture uses the actual endpoint contract and exited 0.

The operator confirmed an orderly GUI and the corrected artist label, but
reported **Hot Cue D not marked** in the first visual check. Do not treat this
as cue-rendering PASS. Follow-up asks for the explicit D1 target (France Gall)
and the observed A/B/C/D bank to distinguish target selection, local overrides
and source/rendering failure. No local cue record was deleted or restored to
force the test to pass. Parser/source comparison remains PASS; on-device cue
rendering is **NOT CONFIRMED** at this checkpoint.
