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

Full host suite, repaired JC4880 build, signed packaging, installed identity and
physical audio/control smoke are pending in this record. No new M3 or JC1060
build or physical qualification is implied. No push, public OTA or release is
authorized by this local repair.
