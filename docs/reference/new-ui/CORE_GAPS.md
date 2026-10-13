# New UI: what the core must provide

Status: draft, 2026-10-13. Branch `kay/new-ui`.

The new UI (`firmware/p4-core/components/ui_next`) adds **no feature to the core**.
Each screen draws demo data. When a screen needs something the core does not give
yet, the need is listed here and the screen shows a placeholder.

`dj_ui` was a preview of this UI. Its callback and setter API is the model for the
data the screens read.

**Not yet checked against the core code.** Each line comes from the backlog
(`new-ui.md`). Before a screen goes live, verify the line against the core and
mark it `exists`, `partial` or `missing`.

## Legend

| Mark | Meaning |
| --- | --- |
| ? | not checked |
| exists | the core gives it today |
| partial | the core gives part of it |
| missing | the core has nothing |

## Gaps by screen

| Screen | Need | Backlog | Status |
| --- | --- | --- | --- |
| Perform | 3-band envelope per track (low, mid, high), 3 values per column | PB-1 | ? |
| Perform | Beat grid with beat 1 of each bar | PB-5 | ? |
| Perform | Per-deck level, left and right, with peak, at 30 fps | PB-19 | ? |
| Perform | Link bar values: tempo, beat, phase, phrase offset, bass clash %, overlap %, key fit | PB-17 | missing |
| Perform | Weave lane: beat pairs of deck A and deck B, clash flags | PB-18 | missing |
| Perform | Camelot key fit between the two loaded decks | PB-10, LB-18 | ? |
| Perform | Time mode flag (elapsed or remaining) | PB-11 | ? |
| Perform | Section map: intro, build, drop, break, outro | LB-13 | missing |
| Perform | 4-deck mode | PB-7 | missing, open question on CPU |
| Browse | Library index with BPM, key, genre, source, folded title | LB-2, LB-19 | ? |
| Browse | Sort key kept in settings; free fourth column | LB-16, LB-17 | ? |
| Browse | Row waveform preview from the stored envelope | LB-6 | ? |
| Browse | Queue of next tracks | LB-9 | missing |
| Browse | Tags (name, colour, many per track), ANY or ALL filter | LB-22 | missing |
| Browse | Playlist write-back after EDIT | LB-20 | ? |
| Track info | Phrase map, key analysis | LB-11, LB-13 | missing |
| Track info | 8 hot cues per track | PB-8 | partial (see `hot_cue_store`) |
| Source | Per-source colour, analysis status, safe eject | LB-4, LB-12 | ? |
| Source | DJ link panel: players, interface, IP, tempo master, player number | LB-23 | missing, protocol not chosen |
| Search | Accent-folded search on the index, 100 ms debounce | LB-10, LB-19 | missing |
| Grid edit | Shift 1 and 10 ms, spacing in 1 ms steps, tempo /2 and x2, TAP, SET BEAT 1, LOCK, RESET, SAVE | LB-14, LB-21 | ? |
| FX | 3 effect slots, XY pad, HOLD, RESET, deck selector | FX-1 to FX-5 | ? |
| Record | WAV 24-bit 48 kHz, level meters, free space, recent files | RC-1 to RC-3 | partial (see `audio_recorder`) |
| Record | MARKER written as a cue chunk | RC-4 | missing |
| Settings audio | Output choice, mix, mono master, split cue, buffer size, limiter | ST-1 to ST-4 | ? |
| Settings display | Theme Dark, Day, Auto; waveform colour editor | ST-5, ST-7, FW-6 | missing |
| Settings system | Controller status, jog weight, language, auto analyze, factory reset | ST-8, ST-10, ST-12 | ? |
| Settings system | Wi-Fi, Ethernet | ST-9, ST-13 | missing, hardware not confirmed |
| Alerts | Event stream: controller lost, disk space low (< 1 GB), recording saved | SY-3 | ? |
| Boot | Library load progress, device name from the settings file | SY-4, SY-5 | ? |
| All | Language string tables | FW-9 | missing |

## Rules for the UI side

- No call into the core that does not exist today. Use a stub with a demo value.
- Each stub carries a `// CORE-GAP: <id>` comment that matches a line above.
- The UI never blocks the audio task. Alerts run on the UI task only.
