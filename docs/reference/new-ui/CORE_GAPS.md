# New UI: what the core must provide

Status: checked against the core, 2026-10-13. Branch `kay/new-ui`.

All 15 screens are drawn in LVGL on demo data (`components/ui_next`, rendered by
`tests/ui_next_sim`). Every `CORE-GAP` comment in that component points to a line
below. Nothing is wired to the core and nothing reacts to touch yet.

The new UI adds **no feature to the core**. Each screen draws demo data. When a
screen needs something the core does not give, the need is listed here and the
screen shows a placeholder.

`dj_ui` was a preview of this UI. Its callback and setter API is the model for the
data the screens read.

## How this was checked

A static read of the public headers and sources under `firmware/p4-core/components`
and `firmware/common`, plus searches for each feature. Nothing was built or run, so a
status says what the code declares, not what works on a board. The old `ui/`
component was left out of the searches, except where a line says so, because the new
UI replaces it. Evidence names files and symbols, not line numbers, so it stays
valid after edits.

## Legend

| Mark | Meaning |
| --- | --- |
| exists | the core gives it today |
| partial | the core gives part of it, or only the inputs |
| missing | the core has nothing |

## Result

| Status | Lines |
| --- | --- |
| exists | 7 |
| partial | 25 |
| missing | 24 |

## Gaps by screen

### Perform

| Need | Backlog | Status | Evidence in the core |
| --- | --- | --- | --- |
| 3-band envelope per track, 3 values per column | PB-1 | partial | `anlz_metadata_t` holds `waveform_low` (PWAV, 400 single values), `waveform_high` (PWV3) and `color_preview` (PWV4, six bytes per column, up to 1200 columns). `anlz_color_preview_column` returns a height and an RGB, not three separate band values. Whether the colour carries the bass, mid and treble split cleanly is not verified on a real file. |
| Beat grid with beat 1 of each bar | PB-5 | exists | `anlz_beat_t` has `beat_phase` 1 to 4, `time_ms` and `bpm_x100`; `anlz_beat_is_downbeat`. |
| Per-deck level, left and right, with peak, 30 fps | PB-19 | partial | `audio_engine` publishes `deck_peak_display[2]` (instant attack, gentle decay, made for a screen) and `main_meter_peak`. One value per deck, not left and right. |
| Link bar: tempo, beat, phase, phrase, bass clash, overlap, key fit | PB-17 | partial | Inputs exist: pitch (`deck_core_pitch_percent`), both beat grids, deck positions, and `beat_phase_align_target_ms` in `beat_jump`. Nobody computes the beat error, phase, phrase offset, bass clash or overlap. The UI can compute them from the two grids and envelopes. |
| Weave lane: beat pairs of A and B, clash flags | PB-18 | partial | Same inputs as the link bar. The pairing and the clash flags are UI-side work. |
| Camelot key fit between the two decks | PB-10, LB-18 | exists | `library_track_t.key` carries the Camelot text from the PDB. The parser in `library.c` is private; `ui_next_key_fit` already does the rule in the UI. |
| Time mode, elapsed or remaining | PB-11 | exists | `app_settings_t.time_remain`. |
| Section map: intro, build, drop, break, outro | LB-13 | missing | The ANLZ parser reads PPTH, PVBR, PQTZ, PWAV, PWV2, PWV3, PWV4 and PCOB. No phrase tag (PSSI). |
| 4-deck mode | PB-7 | missing | `DECK_CORE_DECK_COUNT` and `AUDIO_ENGINE_DECK_COUNT` are 2. Open question on CPU stays. |
| DDJ-400 USB MIDI link | PB-12 | partial | A profile exists in `controllers/pioneer_ddj_400`. Not checked on hardware. `controller_usb_host_is_connected` reports the link. |

### Browse, Search, Track info

| Need | Backlog | Status | Evidence in the core |
| --- | --- | --- | --- |
| Library index: title, artist, BPM, key, genre, source, folded title | LB-2, LB-19 | partial | `library_track_t` has title, artist, album, key, BPM, duration and artwork. No genre (the PDB parser does not read it), no folded title. Source is `media_source_t`: USB or DJ link. |
| Category rail: artist, album, genre, tag, key, playlist, history, match, folder | LB-1 | partial | Playlists and folders exist (`library_playlist_count`, `library_get_playlist`, `is_folder`). No genre, tag or history data. |
| Sortable heads, sort key kept in settings, free fourth column | LB-16, LB-17 | partial | `library_sort` and `media_catalog_sort` take artist, title, BPM and key. The key is not saved. No free column. |
| Row waveform preview | LB-6 | partial | Metadata is loaded per track on demand (`library_load_anlz`, `track_meta_cache_load`). The index holds no envelope per row. |
| Match strip: filter by BPM, key and genre | LB-7 | partial | BPM and key are in the index. Genre is not. The filter itself is UI-side. |
| Queue of next tracks | LB-9 | missing | No queue in the core. |
| Tags, ANY or ALL filter | LB-22 | missing | No tag data anywhere. |
| Playlist write-back after EDIT | LB-20 | missing | The library is read-only, built from `export.pdb`. No write path. |
| Live search, accents ignored, 100 ms debounce | LB-10, LB-19 | missing | No search function. |
| Phrase map and key analysis | LB-11, LB-13 | missing | No phrase data. The key comes from Rekordbox; the device does not analyse it. |
| 8 hot cues per track | PB-8 | exists | `ANLZ_MAX_CUES` 8 and 16 memory cues in ANLZ; `hot_cue_store` keeps 8 slots (`HOT_CUE_STORE_SLOT_COUNT`). |
| File data: bitrate, plays, label, grid OK | LB-11 | missing | Album and duration exist. No bitrate, play count, label or grid check. |

### Source

| Need | Backlog | Status | Evidence in the core |
| --- | --- | --- | --- |
| Sources USB 1, USB 2, SD card, network, phone, one colour each | LB-4 | partial | One USB mount (`usb_storage_is_mounted`). The SD card is used by the recorder (`/sd`). Network is the DJ link (`MEDIA_SOURCE_DJ_LINK`). No second USB, no phone. The colour is UI-side. |
| Storage use, analysis status, safe eject | LB-5, LB-12 | missing | `usb_storage` reports mount events only. Free space is read once at record start (`audio_recorder_sink_prepare`). No eject call. Analysis is done by Rekordbox on a computer. |
| DJ link panel: players, interface, IP, tempo master, player number | LB-23 | partial | `dj_link_discovery_t` lists peers with number, name, IP, BPM, on-air and tempo master flags (`djlink_status_t`). `dj_link_service_format_status` and `app_settings_t.dj_link` give state and on or off. Player numbers are claimed automatically (`numbers[2]`); the user cannot set one. No library share, no SCAN command. The protocol is chosen: Pioneer DJ link. |
| Network source and phone source | LB-15 | partial | DJ link source exists with browse and download (`dj_link_service_browse`). No phone source. |

### Grid edit

| Need | Backlog | Status | Evidence in the core |
| --- | --- | --- | --- |
| Shift, spacing, tempo /2 and x2, TAP, SET BEAT 1, LOCK, RESET, SAVE | LB-14, LB-21 | missing | No call for any of them. Grids are read from Rekordbox ANLZ. `track_meta_cache_save` caches parsed metadata but is not a grid editor. Writing a grid back to the Rekordbox export is a risk to decide. |

### FX

| Need | Backlog | Status | Evidence in the core |
| --- | --- | --- | --- |
| Three slots with their own on and off | FX-1 | partial | `deck_core_beat_fx_state_t` holds one effect at a time: filter, echo, flanger or delay. No reverb. The board shows echo, reverb and filter together. |
| XY pad setting two values | FX-2 | missing | One `depth` per effect. The delay has `wet_q15` and `feedback_q15` and the filter has a raw value, but none is a live control. |
| Echo: time in beats, wet and dry, feedback | FX-3 | partial | Beats 1/4, 1/2, 1, 2 and 4 (`deck_core_beat_fx_beat_t`; the board shows four). `audio_delay_fx_config_t` has wet and feedback. |
| HOLD and RESET | FX-4 | missing | Nothing in the effect state. |
| Deck A and deck B selector | FX-5 | exists | `ctrl_beat_fx_target_t`: channel 1, channel 2 or both. |

### Record

| Need | Backlog | Status | Evidence in the core |
| --- | --- | --- | --- |
| Master to a WAV file on the card | RC-1 | partial | `audio_recorder` records the post-limiter master to `/sd/recordings`, as `.wav.part` renamed to `.wav` (`audio_recorder_sink`). The format is stereo **16-bit** PCM (`AUDIO_RECORDER_WAV_BITS 16u`). The board and the backlog say 24-bit. |
| Timer, running waveform, level meters | RC-2 | partial | `audio_recorder_status_t` gives frames written and the rate, so the timer is derivable. Meters: master peak, mono. The running waveform is UI-side. |
| Free space chip and recent files | RC-3 | partial | Free bytes come from `audio_recorder_sink_prepare` at start. No call lists the recent files. |
| MARKER written as a cue chunk | RC-4 | missing | The WAV helpers write a 44-byte header only. |

### Settings

| Need | Backlog | Status | Evidence in the core |
| --- | --- | --- | --- |
| Master and headphone output: DAC or USB sound card | ST-1 | partial | `audio_main_sink_t` (PCM5102A or USB) with `audio_engine_set_main_sink`. `app_settings_t.audio_out` is a legacy speaker or RCA flag. USB headphone streaming exists in the engine. No per-output choice stored as a setting. |
| Levels, cue and master mix, MONO MASTER, SPLIT CUE | ST-2 | partial | `app_settings_t.cue_mode` is stereo master or split mono, which is SPLIT CUE. `master_trim_preset` has three steps (0, -3, -6 dB), not a level. The output mixer has headphone and cue gains. No MONO MASTER switch. |
| Buffer size, sample rate choice | ST-3 | missing | No setting for either. |
| Master limiter | ST-4 | partial | A limiter runs in `audio_mixer` (`audio_mixer_limiter_stats_t`). Always on, no switch. |
| Theme Dark, Day, Auto | ST-5, FW-6, SY-7 | missing | No theme setting. |
| Brightness | ST-5 | exists | `app_settings_t.backlight_pct`, 10 to 100. The UI maps it to 5 steps. |
| Playhead position, beat grid lines, phase meter flags | ST-6 | missing | Only `time_remain` exists. |
| Waveform colour editor, presets | ST-7 | missing | No colour setting. |
| Controller status | ST-8 | exists | `controller_usb_host_is_connected`. |
| Jog wheel weight | ST-8 | missing | The old UI has a jog mode (vinyl or CDJ). No weight. |
| Language | ST-10, FW-9 | missing | No language setting or string tables. |
| AUTO ANALYZE | ST-10, LB-5 | missing | No analysis on the device. Metadata is parsed on load from Rekordbox files. |
| Firmware version, update, licence page | ST-11 | partial | Version and update live in `p4_ota` and `firmware_health`. No licence page; it is a release gate. |
| Factory reset | ST-12 | missing | No reset call. |
| Wi-Fi status and setup | ST-9 | partial | `wifi_link` runs over ESP-Hosted, with an access point and a station mode (`wifi_link_switch_to_sta`). SSID and URL are in `app_settings_ota_*`. |
| Ethernet status, IP, DHCP or static | ST-13 | partial | The DJ link uses Ethernet on the JC1060 (`dj_link_service_init` takes a netif provider). No IP mode setting. |

### Alerts and Boot

| Need | Backlog | Status | Evidence in the core |
| --- | --- | --- | --- |
| Event stream: controller lost, disk space low, recording saved | SY-3 | missing | No event or toast source. The inputs exist: the USB mount callback, `audio_recorder_state_t` and the free bytes. |
| Library load progress | SY-4 | partial | `media_catalog_load_in_progress` and `media_catalog_count`. No fraction. |
| Device name from the settings file | SY-5 | missing | No device name setting. |
| Licence notice on the boot footer | SY-6 | missing | Same as the licence page. |

## What the boards show that the core cannot do

These change what the UI can honestly display:

1. **Record shows "WAV 24".** The core writes 16-bit. Either the core moves to 24-bit (RC-1), or the UI shows "WAV 16".
2. **FX shows echo, reverb and filter at once.** The core has one effect at a time, and flanger and delay instead of reverb. The slot names need a decision.
3. **Meters are mono.** Perform and Record draw left and right bars; the core gives one value.
4. **No genre, tags, history or queue.** Browse filters and columns on those have no data. Only playlists and folders do.
5. **Analysis runs on a computer.** Rekordbox does it; the device parses and caches. This answers the backlog question about where analysis runs, for now. "Analysis status" on Source has nothing to show.
6. **One USB drive.** The Source board shows two.
7. **Echo beats.** The core offers 1/4, 1/2, 1, 2 and 4 beats; the board shows the first four.

## Rules for the UI side

- No call into the core that does not exist today. Use a stub with a demo value.
- Each stub carries a `// CORE-GAP: <id>` comment that matches a line above.
- The UI never blocks the audio task. Alerts run on the UI task only.

## UI gaps left

What the boards do not settle. These need a design decision, not core work.

| Where | Gap | State |
| --- | --- | --- |
| Alerts | No Day board. The dim layer is a dark scrim at 45 % (rgba 20,24,31) on Day, 74 % of the frame colour on Night. | Assumption, to confirm |
| Source | The panel of a drive (storage, analysis status, safe eject, LB-12) has no board. Only NETWORK has one. | No design |
| Perform | Layers 5 to 7 (structure, DJ aid, link) are only named in the chips; the boards draw nothing more for them. | Nothing to draw |
| Device | Cost on the P4 is not measured: 23 font files in flash, the shadow and scale of the lifted row (Browse edit), the 135 degree cover gradients, 30 fps on the three-band waveforms. | To measure |
| Fonts | The licence text of each font is not confirmed (open question of the backlog). | Open |
