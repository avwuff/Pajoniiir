# M3 catalog, cue software and web LOAD LOCK, 2026-10-07

## Image and authorized scope

The operator authorized the automatic library/cue and web LOAD LOCK checks on
the existing firmware. No firmware build, OTA, profile upload, NVS/cue mutation,
cable action or operator listening/visual test was performed.

The device remained on clean `fb5cbca13f33d21dfb4e16e0ba667c340399eb0c`,
`M3-dev-gfb5cbca13f33`, board `m3`, project `main-deck-m3`, ota_0 VALID,
ELF `e6382dff0c82d9db463e03b956f435ad2d6bad2f4fdee9f9d476cd152d1505a7`.
Production firmware sources were checked unchanged from that accepted source.
Only host tests and evidence documentation were added.

## On-device catalog: PASS

Two complete read-only `/api/cue-migration/catalog` captures, each **41 pages**,
returned all **324** tracks with contiguous indices, unique legacy keys, unique
nonzero full persistent IDs, present audio files, nonzero sizes and FAT mtimes.
Every row used the same export digest:
`3b4d4271eae92d7aca032219c14ba7218de5a0e5034fb48f50847d8067e9461e`.

All indices, keys and titles matched `/api/library`. The complete identity/path/
stat/title rows and library track arrays were identical after the controlled
software reboot. Session generation was 1 in both boots; the reboot did not
increment it. This proves identity stability for the same unchanged export,
not physical media-swap fencing or isolation of two real USB sticks.

A stale generation query returned HTTP 409, `Catalog unavailable or changed`.
A stale-generation LOAD returned HTTP 409, `Library generation changed`, and
invalid deck 3 returned HTTP 400 without loading. All titles were nonempty;
122 artist fields were empty. The USB was attached to M3, not mounted on the PC,
so no independent original-export/ID3 comparison was available. Empty artist
fields are an observation, not a demonstrated parser defect.

## On-device web LOAD LOCK: PASS

Stopped-deck LOAD succeeded for D1 key 5 and D2 key 139, with the requested
titles visible before playback:

- D1: `Alphaville - Big In Japan (Extended 12'' Version Edit)`, 44,100-Hz source.
- D2: `Dino Dvornik - Ritam moj`, 48,000-Hz source.

Both decks then played at neutral pitch, without MT or loops, through fixed
48,000-Hz PCM5102A MAIN with FLX4 UAC available. Three replacement attempts per
deck, using the other track, all returned HTTP 409 `Deck is playing (load lock)`.
Before/after snapshots retained both playing states, title, artist, duration,
BPM and pitch, and monotonic positions. Both playheads advanced more than two
seconds during the rejection window. No new PCM underruns, UAC dropped/overflow/
packet losses or service-log drops occurred; output-late delta was also 0 in
this short window. This is not a new soak or acoustic playback qualification.

The finally path stopped both decks before issuing the existing guarded
software reboot to restore their original empty state. Journal boot 349 reports
ota_0 / SW. Final live state confirmed both IDLE/stopped, the exact accepted
source/ELF/VALID slot, OTA idle, Wi-Fi API, FLX4 MIDI In/Out/UAC, mounted storage,
324 tracks and startup `ready` with zero allocation/critical allocation failures.

The first helper exited 1 after incorrectly expecting HTTP 200 for reboot.
The endpoint had correctly returned **202 Accepted** and already scheduled the
restart. Raw `result.json` retains that FAIL and all successful test cases.
A separate read-only completion verified the reboot, captured the second catalog
and final state, and exited 0 with **PASS_DEVICE_LIBRARY_AND_WEB_LOAD_LOCK**.
No playback or reboot was repeated to replace the original record.

## Production-module host checks: PASS

Fifteen selected suites passed, without SKIP, using production modules:
`media_identity`, `hot_cue_store`, `hot_cue_store_nvs`,
`deck_loaded_track_store`, `ui_deck_anlz_store`, `ui_library`, `anlz`,
`library_anlz`, `rekordbox_pdb`, `ui_artwork_jpeg`, `deck_load_lock`,
`ui_load_gate`, `library_validation_gate`, `deck_core_dual` and
`deck_core_dual_scratch`. Ten offline cue-migration tests also passed.
All three focused host runner invocations and the migration command exited 0; no full qualification rerun or
GitHub Actions job was requested.

The existing store suite uses a RAM-backed standalone branch. The new
`tests/hot_cue_store_nvs` suite instead compiles the production NVS branch and
executes it against a bounded fake NVS API. It covers separate export identities
with identical path/stat, per-slot source/local overrides, persisted tombstones
and restore-source after handle reopening, cross-export preservation on clear,
open/read/set/commit/erase error propagation and handle closure, truncated/CRC-
damaged record rejection, and full-identity collision refusal. Failed commit
does not imply rollback: the fake deliberately permits the preceding write to
be visible, and the tests assert the error rather than guaranteeing old contents.
The suite is part of the ordinary shared host runner for future CI runs.

Parser tests cover title index 17, tagged PCPT, memory versus Hot Cue lists,
PQTZ/downbeat phase, ordered playlist hierarchy/missing references and artwork
error handling. The PDB file passed to the host runner is the committed minimal
qualification fixture (one track), not this 324-track export. Deck-core variants
exercise LOAD/PLAY reservation races and semantic cue/tombstone/restore actions;
their audio/store/platform fakes are not physical controller acceptance.

## Remaining gates

| Gate | Status / reason |
| --- | --- |
| On-device Hot Cue set/delete/restore/recall and persistence across reboot | Subsequently PASS on correction image 082d2c0a via operator touch actions and guarded reboot/reload; see [exact-image lifecycle record](M3_CUE_LIFECYCLE_20261007.md). Exact 30000 ms set-time round trip was not verified. |
| Physical interrupted NVS write / power-loss recovery | NOT RUN; host contract tests do not establish flash atomicity |
| Two real exports with the same track ID / media swap | NOT RUN; one unchanged medium captured; software identity tests passed |
| Legacy cue migration apply | NOT RUN; requires the explicitly selected old export and guarded wired NVS apply |
| Independent real metadata, beatgrid, playlists and artwork/UI review | Subsequent [PC export audit](M3_REAL_EXPORT_AUDIT_20261007.md) completed; artist/album and EXT cue defects reproduced. Playlists/source loops absent; physical UI review remains NOT RUN |
| MIDI/touch playing-deck LOAD LOCK | Subsequently PASS for ordinary FLX4 and touch refusal on both decks on 082d2c0a; see [physical record and unrun variants](M3_PHYSICAL_LOAD_LOCK_20261007.md) |
| New audio listening / waveform visual acceptance | NOT RUN; earlier exact-image acceptance remains separate |
| Current-source GitHub Actions / physical Campaign A | SKIPPED BY OPERATOR, unchanged from earlier records |

Artifacts: `.cache/m3-migration/20261007-fb5cbca1/library-web-validation/`,
including both full captures, per-rejection before/after snapshots, stopped/final
states, journal, raw result and `completion.json`. Helpers are
`.cache/test_m3_library_web.py` and `.cache/finish_m3_library_web.py`.
Console logs are `D:/AI/m3-fb5-library-{web-test,web-completion,cue-host-tests}.log`,
`D:/AI/m3-fb5-cue-{semantic-host-tests,migration-tests,nvs-host-tests}.log`.
No remote push, release tag or public OTA update was performed.
