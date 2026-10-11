# Shared library export audit and regression follow-up

Date: 2026-10-12. Disposition: **software gates PASS; real-export comparison
complete with explicit source-data defects; wider physical checks NOT RUN**.
The operator subsequently requested retaining only software/audit results for
now. No fresh test export, physical library campaign or firmware OTA was run.

## Source and read-only scope

Repository checkpoint `c26989fe19eb4ca26fff7d9abdc0fef041f74b91` contains the
same production library/UI/cue source as beta `40e72fcb` and qualified `722bb976`.
Only host regressions and documentation are added in this follow-up; the
published beta binaries/tags/assets remain immutable.

Before moving the stick, read-only API snapshots identified JC4880 running
`e22ff334e01f7e9da6515970ea6ab8e9cef86073`, VALID `ota_1`, both decks stopped.
This is context, not new hardware acceptance. The operator then connected the
stick to the PC as `F:` (FAT32). No writes were made to the medium.

`F:/PIONEER/rekordbox/export.pdb`: 372,736 bytes, SHA-256
`034b6d3d410bd9e4b934a7a751a7cf6331bd087a576ee367fc96a88f0bae11c7`.
Its before/after digest matches, and every read analysis/artwork input was
rehashed unchanged at audit completion. The production PDB/ANLZ modules were
compiled with standalone adapters and executed against all referenced tracks.
The independent DeviceSQL/tag reader uses the
[Deep Symmetry PDB schema](https://github.com/Deep-Symmetry/crate-digger/blob/master/src/main/kaitai/rekordbox_pdb.ksy)
and [ANLZ schema](https://github.com/Deep-Symmetry/crate-digger/blob/master/src/main/kaitai/rekordbox_anlz.ksy)
as reverse-engineered layout references, not Pioneer certification. Golden
offset/tag expectations are independent of production parsing.

## Real export results

| Check | Result and scope |
| --- | --- |
| Track metadata | All 260 IDs, titles, artists, albums, audio/analysis paths, duration and artwork IDs match independent rows; no import truncation or invalid playlist/artwork rows |
| Playlist records/order | Three flat lists and all 69 entries match independent ID/index/order data: HOUSE (1), TECHNO (1), Untitled Playlist (67) |
| Folder hierarchy | No folders exist in this export; real nested-folder check NOT RUN |
| Valid DAT/EXT pairs | 257 pairs match beat timestamps and original phases, source Hot Cues, PWAV/PWV3 presence/length and PWV4 length/truncation contract |
| PWV4 | Present in all 257 valid pairs; production color-preview metadata matches raw sections |
| Memory cues/loops | None exist in the valid export analyses, including extended memory records; real positive-source memory cue/loop check NOT RUN |
| Artwork | 230 referenced rows; 210 existing JPEGs decode with independent Pillow 12.3.0; 20 files are missing; no progressive JPEGs among decoded images |
| Cross-export IDs | Compared to the saved independent 324-track export: all 260 current raw IDs are reused, with 260 different paths and titles, and a different export digest |
| Physical downbeat/artwork/cues | NOT RUN; binary phase equality is not independent musical-downbeat or on-device rendering acceptance |

Pillow's JPEG decode proves readable source pixels, not the firmware thumbnail
decoder or a physical screenshot. The existing production JPEG probe regression
was also run. Cross-export row comparison demonstrates why raw track IDs cannot
identify persistent cues; it is not a physical two-stick cue lifecycle test.

### Incomplete or inconsistent source data

- Track 190, **Fancy - Lovers (Extended)**: both DAT and EXT are absent under
  `/PIONEER/USBANLZ/P02D/000061D1/`.
- Track 191, **Fancy - Slice Me Nice**: both analyses are absent under
  `/PIONEER/USBANLZ/P052/00019F3C/`, and
  `/Contents/Fancy/UnknownAlbum/Fancy - Slice Me Nice.mp3` is absent.
- Track 2, **France Gall - Ella, elle l'a (Clip officiel)**: DAT declares
  158,914 bytes but contains 6,540; EXT declares 218,106,895 but contains 139,892.
  These are inconsistent source headers, not an observed new parser regression.
  Production DAT parsing rejects the malformed structure with standalone
  `ESP_ERR_INVALID_SIZE`; it does not publish partial analysis. No EXT enrichment
  is attempted after a failed DAT by the audit driver.
- Twenty referenced covers `a160.jpg` through `a179.jpg` are absent under
  `/PIONEER/Artwork/00009/`.

The export is therefore not given an overall media PASS. Its otherwise matching
metadata remains useful, and missing optional artwork requires fallback rather
than blocking audio. The cause of these source-data defects is not established;
no files were deleted, rewritten or repaired during this audit. A fresh export
from intact originals is the recommended preparation for future positive tests.

## Executed software regressions

Ten selected suites exit 0 using the standard host runner:
`rekordbox_pdb`, `anlz`, `library_anlz`, `library_anlz_cache_write`,
`media_identity`, `hot_cue_store`, `hot_cue_store_nvs`, `ui_library`,
`ui_deck_anlz_store`, `ui_artwork_jpeg`. This is a focused library gate,
not a new complete `-Qualification` run. Existing fixtures exercise folder
relationships/order and invalid references, tagged memory loops and Hot Cue
separation, PWV4 columns/fallback, original PQTZ phases 1–4/downbeat 1,
cache rebuilding and persistent cue failure/restore behavior.

The persistent cue suite now derives two identities from different export
digests while holding audio path/stat constant. A local cue and deletion
tombstone on export A do not affect B. Saving B's override, restoring A's source
and clearing A leave B's cue intact. Production identity derivation, persistence
and source/local merge execute directly; the extended suite exits 0.

The production artwork cache harness now executes `ui_artwork_poll` and
`ui_artwork_begin_page`, in addition to the existing cache ownership functions.
It checks delayed completion before the new catalog's first UI request,
same-key replacement across generations, page-epoch invalidation, NONE fallback,
evicted requests, unchanged pixels/content revision for stale results, semaphore
release and bounded queue retry. Missing/corrupt/decode-failed input is injected
as the worker's NONE result; this does not claim real asynchronous worker I/O
or JPEG decoding coverage. Two artifact-only negative mutations removing the
generation or page-epoch publication fence each fail the intended assertion;
the unchanged production fences pass. Production firmware was not modified.

## Deferred checks and evidence

The operator chose **software/audit results only for now**. A real nested folder
and positive memory-loop export, independent acoustic/UI downbeat review,
artwork after physical media replacement, and two-medium physical cue lifecycle
remain NOT RUN. They require suitable intact exports and a separately scheduled
short hardware block. No new soak or reopening of S2 qualification is implied.

Evidence: `.cache/library-gates-20261012/`, including `raw-pdb.json`,
`production.json`, `audit-results.json`, `input-hashes.json`, production parser
and focused host logs, artwork/cross-export regression logs, sensitivity
assertions, independent readers and exporter source. Before-test firmware/status
snapshots are `.cache/library-next-{firmware,status}.json`. The evidence inventory
retains SHA-256 hashes of audit reports, helpers and regression artifacts.
