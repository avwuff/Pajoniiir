# DJ UI feature backlog

Oct 8, 2026 · @Francis

This backlog turns the 14 screens and the Day theme of the open DJ UI into features to build. Each feature has an ID, a screen, a priority, and a note on the ESP32-P4 work. The MVP is a two-deck player with real waveforms, a browser, and the DDJ-400 link.

## How to read this backlog

Every feature comes from an element on a screen board. Features that the boards do not show are not in the list, except the DDJ-400 link and the library database, which the screens need.

- **P0 (MVP):** the device cannot play a set without it.
- **P1 (v1):** the device is complete for a first public release.
- **P2 (v2):** the feature adds comfort or reach. Build it after v1.

The ESP32-P4 note says what limits the work: CPU, RAM, flash, or the LVGL display path.

## UI framework

One layout engine serves all six frames: three size classes, each in landscape and portrait.

| ID | Feature | Screen | Priority | ESP32-P4 note |
| --- | --- | --- | --- | --- |
| FW-1 | Three size classes by the long side: up to 800, up to 1024, above 1024 pixels | All | P0 | One set of LVGL style tokens per class |
| FW-2 | Landscape and portrait layouts from one screen definition | All | P0 | Rotate the layout, not the frame buffer, to save CPU |
| FW-3 | Navigation with five pages: Perform, Browse, FX, Record, Settings | All | P0 | Rail on the left in landscape, bar at the bottom in portrait |
| FW-4 | Deck dock with an overview waveform for decks A and B on every page | All | P0 | Draw each overview once into a cached image |
| FW-5 | Touch targets of 44 pixels or more, scaled by size class | All | P0 | Check on the 7 in 1024 x 600 panel first |
| FW-6 | Night and Day themes, with a matching waveform palette | Perform, Settings | P1 | Two LVGL themes, switched at run time |
| FW-7 | Gestures: half swipe on a row, long press on a row | Browse | P1 | LVGL gesture events, no extra library |
| FW-8 | Open fonts (Roboto 2.x, Cousine) and hand-drawn icons in flash | All | P0 | Convert fonts with the LVGL font tool. Keep the licence files |
| FW-9 | Language files for the UI text | Settings | P1 | One string table per language in flash |

## Playback

The Perform screen is the main page. The waveform is the core of it: three bands (low, mid, high) drawn from a stored envelope, never from gradient bars.

| ID | Feature | Screen | Priority | ESP32-P4 note |
| --- | --- | --- | --- | --- |
| PB-1 | Three-band waveform renderer (low violet, mid teal, high cream) | Perform | P0 | Store 3 envelope values per column. Draw with LVGL canvas or a custom draw callback |
| PB-2 | Zoomed waveform that scrolls with the playhead: horizontal in landscape, vertical in portrait | Perform | P0 | Redraw only the new columns. Target 30 frames per second |
| PB-3 | Overview strip for each deck, with playhead and cue marks | Perform | P0 | Draw once per track load |
| PB-4 | Deck card in the dock: title, BPM, key chip, deck state, time | Perform | P0 | Truncate long text with an ellipsis |
| PB-5 | Beat grid lines on the waveform, amber line on beat 1 of each bar | Perform | P0 | Use the grid from the analysis |
| PB-6 | MASTER, SYNC, and SYNC OFF badge on each deck card | Perform | P0 | State comes from the audio engine |
| PB-7 | Switch between 2 and 4 decks. Compact cards in landscape. The Perform board shows two decks only | Perform | P1 | Four decks need four audio streams. Test the CPU before you commit |
| PB-8 | Hot cue marks and cue trigger | Perform, Track info | P1 | Eight cues per track, stored with the analysis |
| PB-9 | Phase meter between two decks. It is one meter of the link bar (PB-17) | Perform | P1 | Cheap. Uses the two beat phases |
| PB-10 | Key match chip: filled when the key fits the other deck | Perform, Browse | P1 | Camelot table, no DSP |
| PB-11 | Time mode: elapsed or remaining | Perform | P0 | A display flag only |
| PB-12 | DDJ-400 USB MIDI input: play, cue, jog, tempo, crossfader, EQ, and LED output | Perform | P0 | USB host MIDI on the P4. The mapping is a table in flash |
| PB-13 | Set a cue under the finger: touch the waveform, then press cue | Perform | P2 | Round to the nearest beat when the grid is locked |
| PB-14 | Beat jump of 32 beats in the jump sizes | Perform | P2 | The jump sizes are a table. Add one entry |
| PB-15 | Pad lamp colour for each pad mode on the controller | Perform | P2 | Check what the DDJ-400 lamps support before you commit |
| PB-16 | Shortcut button to go back to Perform from any screen | All | P2 | One fixed button. No new state |
| PB-17 | Link bar between the two waveforms: tempo, beat, phase, phrase, bass clash, key fit, overlap | Perform | P1 | Compute the values from the two stored envelopes. Update at the waveform frame rate. Layers 5 to 7 are an extension |
| PB-18 | Weave lane: curves join the beats of deck A to the beats of deck B. Red dots mark a bass clash | Perform | P1 | Draw only the visible beats. Use one path for each frame |
| PB-19 | VU column at the left of each deck waveform: left and right bars with a peak line. In portrait, deck B has it at the right edge | Perform | P1 | Read the peak of each audio block from the engine. Update at 30 frames per second |

## Library

The library work has two parts: the pages the user sees, and the analysis that fills them. The analysis runs in the background and must never stop the audio.

| ID | Feature | Screen | Priority | ESP32-P4 note |
| --- | --- | --- | --- | --- |
| LB-1 | Track list with a category rail: Artist, Album, Genre, Tag, Key, Playlist, History, Match, Folder | Browse | P0 | Virtual list: build only the visible rows |
| LB-2 | Library index on the drive (title, artist, BPM, key, genre, source) | Browse | P0 | A small database file. Load it at boot |
| LB-3 | Load a track on deck A or B from the list | Browse, Track info | P0 | Decode start must finish in under one second |
| LB-4 | Source picker: USB 1, USB 2, SD card. One colour per source | Source | P0 | The colour shows again in Browse |
| LB-5 | Background analysis: BPM, beat grid, waveform envelopes | Source, Track info | P0 | Run on the second core at low priority. Store the result next to the file |
| LB-6 | Waveform preview on each list row | Browse | P1 | Use the stored envelope, scaled down |
| LB-7 | Match strip: filter by BPM, key, and genre | Browse | P1 | Filter in RAM on the index |
| LB-8 | Half swipe on a row: LOAD A, LOAD B, QUEUE, TAG | Browse | P1 | See FW-7 |
| LB-9 | Queue of next tracks | Browse, Track info | P1 | A list of track IDs in RAM |
| LB-10 | Live search with a touch keyboard. Amber match text | Search | P1 | Search the index on each key press. Use a debounce of 100 ms |
| LB-11 | Track info page: full waveform, phrase map, hot cues, file data | Track info | P1 | Phrase map comes from the analysis. See LB-13 |
| LB-12 | Source detail: storage, analysis status, safe eject | Source | P1 | Flush the writes before you unmount |
| LB-13 | Key and phrase analysis (intro, build, drop, break) | Track info | P1 | The heaviest job. Test on a 10-minute track |
| LB-14 | Grid edit: zoomed waveform, shift 1 and 10 ms, tempo /2, x2, 0.01 steps, TAP, SET BEAT 1, LOCK, RESET, SAVE, CANCEL | Grid edit | P1 | Write only the grid back to the analysis file |
| LB-15 | Network source (DJ link) and phone source | Source | P2 | The Source board shows the DJ link panel (LB-23). The phone source has no panel yet. The protocol is not chosen |
| LB-16 | Sortable column headings. Tap a heading to sort. A second tap reverses the order | Browse | P0 | Sort in RAM on the index. Keep the sort key in the settings |
| LB-17 | Free column: the user picks the fourth field (genre, year, or none) | Browse | P1 | Store the choice as one byte |
| LB-18 | Key colours: each key chip shows how well it fits the loaded deck | Browse | P1 | Camelot table, no DSP. Same rule as PB-10: fit means 1 step or less |
| LB-19 | Search ignores accents: ete finds Été | Search | P0 | Store a folded copy of each title in the index. Fold the query the same way. Add an option in Display settings |
| LB-20 | EDIT mode for a playlist: grips, position numbers, drag to reorder | Browse | P2 | Write the new order back to the playlist file when EDIT ends. |
| LB-21 | Grid spacing: change the beat distance in 1 ms steps. A drift chip shows the late beat | Grid edit | P1 | Beat 1 does not move. Store the new beat distance with the grid |
| LB-22 | Tags and category filter: tag chips filter the list. TAG and GENRE entries in the category rail | Browse | P1 | A tag has a name and a colour. A track can have many tags. The ANY or ALL button sets how tags combine. A long press on a row opens the tag picker. Store the tags in the track index |
| LB-23 | DJ link panel in the Source page: link state, linked players, interface, IP address, device name, library share, tempo master, player number 1 to 4, SCAN, LEAVE LINK | Source | P2 | Needs the Ethernet link (ST-13). The protocol is not chosen. See LB-15 |

## FX, Record, and Settings

The FX page has no stem faders. Real-time stem separation does not fit the ESP32-P4, and the studied DJ UIs do not place it on the FX page.

| ID | Feature | Screen | Priority | ESP32-P4 note |
| --- | --- | --- | --- | --- |
| FX-1 | Three effect slots (Echo, Reverb, Filter) with on and off switch and amount | FX | P1 | Float DSP. Keep each effect under 5 % CPU |
| FX-2 | XY pad that sets two effect values with one finger | FX | P1 | Smooth the values to avoid clicks |
| FX-3 | Echo settings: time in beats (1/4, 1/2, 1, 2), wet/dry, feedback | FX | P1 | Delay line in PSRAM |
| FX-4 | HOLD keeps the effect on after the finger lifts. RESET clears it | FX | P1 | A state flag |
| FX-5 | Deck A and deck B selector for the effect | FX | P1 | Route the effect to one deck bus |
| RC-1 | Record the master output to a 24-bit WAV file at 48 kHz | Record | P1 | Write in large blocks. Use a ring buffer |
| RC-2 | Large timer, running waveform, and left and right level meters | Record | P1 | Reuse the waveform renderer (PB-1) |
| RC-3 | Free space chip and recent files list | Record | P1 | Read the file system once per second |
| RC-4 | MARKER adds a time mark to the file | Record | P2 | Write a cue chunk in the WAV header |
| ST-1 | Master and headphone output: internal DAC or USB sound card | Settings audio | P0 | Two independent output paths |
| ST-2 | Master level, headphone level, cue and master mix, MONO MASTER, SPLIT CUE | Settings audio | P0 | Mixer gains in the audio engine |
| ST-3 | Buffer size: short for low delay, long for slow hardware | Settings audio | P1 | Changing it restarts the audio engine |
| ST-4 | Master limiter | Settings audio | P1 | Look-ahead of 1 ms |
| ST-5 | Theme (Dark, Day, Auto) and brightness | Settings display | P1 | Brightness is a backlight PWM duty |
| ST-6 | Playhead position, beat grid lines, phase meter, and time mode | Settings display | P1 | Display flags read by the Perform screen |
| ST-7 | Waveform colour editor for three bands, four presets, live preview | Settings display | P2 | Save the colours in the settings file |
| ST-8 | Controller status (DDJ-400) and jog wheel weight | Settings system | P1 | Jog weight scales the MIDI jog value |
| ST-9 | Wi-Fi link status and setup | Settings system | P2 | ESP-Hosted on the P4 board with a C6 radio |
| ST-10 | Language and AUTO ANALYZE switch | Settings system | P1 | Linked to FW-9 and LB-5 |
| ST-11 | Firmware version, update, and licence page | Settings system | P0 | The licence page is a release gate |
| ST-12 | Factory reset with a second tap to confirm | Settings system | P1 | Erase the settings partition only |
| ST-13 | Ethernet link: status, IP address, and DHCP or STATIC mode | Settings system | P2 | The P4 has an Ethernet MAC. The board needs a PHY chip. The DJ link uses this link when the cable is connected |

## System

One rule governs all alerts: the audio never stops because of an alert.

| ID | Feature | Screen | Priority | ESP32-P4 note |
| --- | --- | --- | --- | --- |
| SY-1 | Modal dialog only for events that need a choice. It dims the screen behind it | Alerts | P0 | Run it on the UI task. Keep the audio task apart |
| SY-2 | Toasts at the top that fade after four seconds. Teal is done, amber is care, red is a fault | Alerts | P0 | One toast queue |
| SY-3 | Alert events: controller lost, disk space low (less than 1 GB), recording saved | Alerts | P0 | Events come from the USB, file system, and record tasks |
| SY-4 | Boot screen with a ring mark and a library load bar | Boot | P0 | Show the screen in under one second. Load the library after it |
| SY-5 | Device name set in the settings file. The boards show a placeholder | Boot | P2 | The maker picks a free name |
| SY-6 | Licence notice on the boot footer | Boot | P0 | Same source as ST-11 |
| SY-7 | Colour themes: Night and Day | Settings display | P2 | A table of colour tokens. Switch at run time |
| SY-8 | Extension marker: the user turns each optional feature on or off | Settings system | P2 | A feature that is off stays out of the menus |

## Release plan

The backlog holds 81 features: 29 for the MVP, 38 for v1, and 14 for v2. Each release ends with a test gate.

| Release | Features | Gate to pass |
| --- | --- | --- |
| MVP (P0) | Layout engine, navigation, deck dock, 3-band waveforms, DDJ-400 link, USB and SD library, background analysis, audio outputs, alerts, boot, licence page | Play two tracks for 2 hours with no audio drop. Rotate between landscape and portrait with no restart |
| v1 (P1) | Search, match strip, tags, swipe actions, track info, grid edit, 4 decks, link bar, VU meters, FX page, recording, display and system settings, Day theme | Record a 1-hour set to WAV while two decks play and the FX are on. CPU stays under 80 % |
| v2 (P2) | Network source with the DJ link panel, phone source, Ethernet settings, colour editor, Wi-Fi setup, record markers, device name | No gate. Build by demand |

Build order inside the MVP: (1) layout engine and waveform renderer on a fixed test track, (2) audio engine and DDJ-400 link, (3) library index and analysis, (4) alerts, boot, and licences.

## Open questions

- [ ] Can the P4 decode and play four decks at once? Measure it before you commit to PB-7.
- [ ] Which audio format set is required: MP3, FLAC, WAV, AAC?
- [ ] Where does the analysis run: on the device, or on a computer before the set?
- [ ] Does the 1024 x 600 panel give a stable 30 frames per second with PB-2 and four overview strips?
- [ ] Which protocol serves the DJ link panel and the phone source (LB-15, LB-23)?
- [ ] Confirm the radio chip on the P4 board before you plan Wi-Fi (ST-9). Confirm the Ethernet PHY chip before you plan ST-13.
- [ ] Confirm the licence file of each font before the first release.
