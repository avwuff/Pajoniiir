# JC4880 Hercules FS FIFO repair

## Scope and observed failure

The operator authorized a local JC4880 candidate and Hercules Inpulse 500
acceptance over Wi-Fi. Shared-core source `5aa9b5503e7f0c9dd2424bc938b23bb6a5305d26`
was installed as `M2.5-208-g5aa9b550`, VALID `ota_0`. GUI, touch and Browse
were confirmed by the operator. The controller profile was uploaded without
removing existing profiles, and the library retained 260 tracks.

Hercules `06F8:B12B` enumerated on controller FS/root1, with storage on HS/root0.
Its captured UAC1 four-channel packed-24 endpoint advertises 540-byte MPS.
The ordinary JC4880 HCD periodic TX FIFO and stream admission were limited to
400 bytes. MIDI worked, but audio format admission failed before stream startup.
The previous PC stream test silently widened admission with `PC_TEST`, masking
this production configuration failure.

## Repair and regression

JC4880 now shares JC1060's FS allocation: 160 periodic TX lines (640 bytes),
20 non-periodic TX lines (80 bytes), and 76 RX lines (296 usable bytes after
the HAL's two-line status reserve). The 256-line total is preserved, both MIDI
bulk directions retain capacity for 64-byte packets, and Hercules fits.
HS allocation and M3's FS allocation remain unchanged.

Stream FS admission uses the same board policy without a PC-only capacity
override. The production stream suite explicitly selects JC4880. The FIFO
regression executes the generated HCD policy and actual stream admission
defines for all three boards, with and without the HS diagnostic option.
It checks matching admission/FIFO bounds, Hercules/FLX4 packet capacity,
MIDI capacity, HS storage capacity and total allocation.

The new JC4880 capacity assertion failed on the original policy. After repair,
all six FIFO configurations and the captured Hercules production stream suite
pass, including feedback pacing, teardown and qualified FLX4 isolation.

## Evidence and open gates

Original installation and descriptor evidence is retained under
`.cache/jc4880-bench-5aa9b550/`. The initial OTA observer timed out after Windows
disabled DHCP; the preserved later firmware snapshot independently confirmed
the exact VALID image. This host connectivity incident is not a firmware boot
failure.

The full host suite initially exposed a second standalone stream-test runner
that did not select a board. Both runners now select the actual JC4880 policy.
The repeated full host suite exits 0, as do the documentation integrity check,
ESP-IDF 6.0.2 JC4880 build and board-isolation/identity/image-budget verifier.

## Installed repair candidate

- Clean source: `e22ff334e01f7e9da6515970ea6ab8e9cef86073`.
- Version/project/board: `M2.5-209-ge22ff334` / `main-deck-p4` / `jc4880`.
- Application: 2,607,824 bytes; SHA-256
  `60e04a9bb8e6a2e4fdd7dc7ace50b3d51d749fedae73e981a5e5b05abe63e1e8`.
- Signed bundle: 2,608,012 bytes; SHA-256
  `3f596eb35bbb0e8fdacb03f8d0fcce03538f7e0a72e3f8862529eaf7e1619a4d`.
- ELF identity:
  `5d63acf5665091a82be61110769aed4b0a464b8c2a19dcdd40497df80f9e8558`.

Local OTA returned HTTP 200. Startup health gate confirmed VALID `ota_1`,
the exact clean source/ELF and idle OTA service. Hercules automatically opened
UAC1 FS at 44,100 Hz, packed 24-bit / three-byte subslots, streaming without
configuration fault. MIDI In/Out and the active Hercules profile are present;
all six stored profiles and the 260-track library remain available.

Mixed-rate test tracks (44.1/48 kHz) are loaded at 15 s, pitch zero, no MT or
loops, both READY/stopped. Evidence and signed package are retained under
`.cache/jc4880-hercules-fifo-20261011/`; full host log is
`.cache/jc4880-fifo-host-final.log`.

The operator confirmed normal GUI and readiness. The combined smoke ran
30.516 s / 54 samples with 2424 MIDI packets, all 22 strict counter deltas zero
and no new output-late event. Both decks ended stopped. The stopped baseline
underflow count (1,158,854) and one earlier host recovery request were unchanged
through playback; they are not new test losses. Exact image identity and healthy
Hercules streaming were preserved. Artifacts are in
`smoke-30s-20261011T205855Z/` under the repair evidence directory. The operator
confirmed normal jogs/Browse, MAIN and waveforms, but initially reported silent
headphones. Both PFL states were false in the recorded status; the profile uses
native headphone controls and a cue-only software feed. The operator then
clarified that PFL had not been pressed and confirmed headphone sound works
after enabling it. This is a subsequent listening confirmation, not a claim
that headphones were monitored throughout the original 30-s window. No second
firmware change or retest was needed for that operator setup issue.
This short smoke is not a soak or full JC4880/controller qualification.
Only JC4880 was built for this repair. No new M3 or JC1060 physical qualification,
push, public OTA or release is implied.

## Combined JC4880 functional block

On the same installed `e22ff334` image, a subsequent authorized block completed
60.140 s / 94 samples with 546 MIDI packets, all 22 strict deltas zero and no
new output-late event. Workload: 44.1/48-kHz sources, D1 -5% / D2 +5%, both MT,
four-beat loops and zoom 8. PCM5102A MAIN and Hercules UAC1 ran at 44.1 kHz.
Both loops were cleared at approximately 22 s, then web seek requested D1 30 s
and D2 45 s; advancing positions were verified at 28.5 s. Samples recorded
physical CUE stopping D1 at 39.484 s and D2 at 42.562 s, followed by PLAY
resuming at 40.640 / 44.047 s. MT and opposite pitch remained set in all samples.
Both decks finally stopped, and exact clean image/ELF identity was preserved.

The operator explicitly confirmed every requested step: MT/loops, seek,
CUE-to-PLAY on both decks, PFL and native headphone Mix/Volume, clean MAIN and
headphones, and fluid waveforms. This closes the named short functional block,
not a long soak, every transport edge case or unrestricted board acceptance.
Evidence, runner, independent sample audit, operator confirmation and hashes:
`.cache/jc4880-hercules-fifo-20261011/functional-60s-20261011T210510Z/`.

## Physical-root admission follow-up

The standard fresh three-board build/UI matrix completed on `95c1befb`.
Inspection during that qualification reproduced an admission regression in the
M3 controller role-swap configuration: Hercules is a Full Speed device on the
HS PHY/root. The initial `e22ff334` cleanup had tied its admission to the FS
root's 400 bytes instead of the selected HS root's 640 bytes. The captured
production stream test failed in the actual M3-HS configuration. This was found
in software; no new M3 image was installed and no S2 physical tests were reopened.

Admission now follows the controller root: JC4880/JC1060 640 bytes; ordinary
M3 FS 400; M3 HS 512 without the diagnostic FIFO expansion and 640 with it.
Device speed still selects UAC protocol/pacing. The capacity regression executes
eight board/root/diagnostic combinations and matches admission to the selected
root's generated HCD FIFO. The standalone production stream runner tests both
JC4880 and M3-HS, including the captured Hercules descriptor; both now pass.
Final incremental builds and full host qualification for this follow-up remain
pending until recorded in the final matrix.
