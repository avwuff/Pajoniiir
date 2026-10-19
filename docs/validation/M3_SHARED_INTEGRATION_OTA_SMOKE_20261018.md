# Ordinary M3 integration OTA and FLX4 smoke

Recorded 2026-10-18. This local installation reuses the clean ordinary image
from the [three-target software verification](../reviews/2026-10-14-jc1060-integration-verification.md).
No firmware was rebuilt. Packaging used the unmodified tool from a clean
detached checkout of the exact source and the existing verified build metadata.
This is not a new release or long soak.

## Installed identity and admission

| Field | Installed candidate |
| --- | --- |
| Source | `c0ee81527b4dbd5d83599140aaa106ebcd27772a`, clean |
| Board / project | `m3` / `main-deck-m3` |
| Version | `M3-dev-gc0ee81527b4d` |
| Slot / image state | `ota_0` / `valid` |
| ESP-IDF | 6.0.2 |
| Application / signed bundle | 2,603,200 B / 2,603,388 B |
| Signing | `rel-001`, existing trusted release key, ECDSA P-256 / SHA-256 |

Application SHA-256:
`7b63771563a5da57ad4547f345edd54bac48d43128e9d2285dfe7a4f2e80d13c`

Signed `.ddjota` SHA-256:
`723ac82f1f7d003882927b65e33971fcd55e917386effa0c7c898ac6e580615d`

Matching ELF SHA-256:
`282250f0665266e2fb4aa8a348dbf5c0e619401d75df5c686f229fb652267340`

The former clean image `6166d93627304895857b626e719032de3b4f884d`,
`M3-dev-g6166d9362730`, VALID `ota_1`, used experimental reversed USB roles.
The operator restored FLX4 to USB2 and the music stick to USB3 before upload.
The former firmware still assigned USB3 to the controller; the stick did not
mount there until the ordinary image restored the intended storage role.

Exactly one marked signed `POST /api/ota/p4` completed HTTP 200. Windows
reconnected using the existing `Pajoniiir-M3` Wi-Fi profile after reboot; DHCP
was available. The firmware API confirmed the expected source, ELF, project,
version and VALID slot, with OTA state `idle` and no last error. Both decks
were stopped before admission and after installation.

## Restored topology

| Service | Observed state |
| --- | --- |
| Controller | FLX4 USB2, FS / root1; MIDI In/Out, `pioneer_ddj_flx4` active |
| Storage | Stick USB3, HS / root0; mounted, 260 catalog tracks |
| MAIN | PCM5102A, fixed 48,000 Hz |
| Headphones | FLX4 UAC1 FS, streaming, no fault; 44,100 Hz, 16-bit / 2-byte subslot |
| Display / touch | 800x480; operator confirms normal GUI/touch |
| Network | Wi-Fi ON; web status/control available |

The historical 7,776-byte `ota_reboot` dump was downloaded before updating.
Its ELF prefix `38c637996` and complete crash summary are identical before
installation, after installation and after the smoke. This does not indicate
a new panic during this OTA. The operator's earlier cable/power context stays
attached to the historical incident; its cause is not reclassified here.
Saved dump SHA-256:
`356d0a050d773a9eb287f7a1da51a1e9ba1c67bdb9d99e0af85d47d6902a4a14`.

## One mixed-rate smoke

D1: `Casanova Action {320}` (Latin Lover), source 44.1 kHz. D2:
`Dino Dvornik - Ritam moj`, source 48 kHz. Both started around 15 s, pitch zero,
Master Tempo OFF. This is not the earlier MT/opposite-pitch worst-case workload.
The operator confirmed readiness with hands on the controller and was asked
to turn both jogs/Browse during the first 20 s, then listen to MAIN/FLX4
headphones and watch the waveforms. PFL D1 was enabled. The actor used
transient CDJ cues; persistent Hot Cue data was not edited.

| Measurement | Result |
| --- | --- |
| Duration / samples | 60.532 s / 101 |
| New MIDI packets | 2,100 |
| All 22 present strict counter deltas | 0 |
| Output-late delta / final since-boot maximum | 0 / 0 us |
| Every sample: identity, UAC health, storage mount | Unchanged / healthy |
| Final state | Both stopped, both loops OFF |

Nonzero stopped/startup baselines were retained: headphone underflow 7,839,392
frames; host recovery requests 1; controller MIDI disconnects 1 and MIDI IN
pipe errors 1. These did not change through playback and final stop. Missing
counters were not substituted with zero. No `/api/resources`, explicit heap
walk or large diagnostic-log request was made during playback.

| Action | Elapsed | Observed result |
| --- | ---: | --- |
| Both `loop_4` | 25.375 s | Both loops active |
| Both `loop_clear` | 35.766 s | Both loops inactive |
| D1 CUE tap | 43.969 s | D1 parked, D2 continued |
| D1 PLAY | 44.141 s | D1 playing again |
| D2 CUE tap | 51.329 s | D2 parked, D1 continued |
| D2 PLAY | 51.485 s | D2 playing again |

The offline independent audit rechecked all samples, all counter paths,
monotonic uptime/elapsed time, source, storage mount, UAC format/health, track
identity, pitch/MT, final stopped state and package hashes. It passed.
The operator's final confirmation on 2026-10-18 closes this short physical
smoke: both jogs/Browse responded correctly, MAIN and FLX4 headphones remained
clean, both waveforms were fluid, and loop/CUE transitions had no disturbance.
The [operator confirmation](evidence/2026-10-18-m3-integration-ota/operator-confirmation.json)
is retained separately from the original telemetry result, whose pending field
reflects the time at which the runner finished. GUI/touch were confirmed before
the smoke. This closes only the exact-image short smoke described here.

## Evidence and boundaries

Tracked [evidence](evidence/2026-10-18-m3-integration-ota/audit.json) includes the
runner, offline audit script, all samples, baseline/final snapshots,
installation result, signed manifest and byte-preserving SHA-256 inventory.
Original working artifacts and signed binaries remain in
`.cache/m3-ota-c0ee8152-20261018/`; no private signing key is archived.

This short smoke does not qualify a long soak, USB reconnect campaign,
negative OTA/rollback test or every controller action. Historical physical
results remain bound to their original images. S2 remains operator-closed;
the JC4880 soak waiver stands. JC1060 hardware/SD warm-reset and hosted CI
gates remain open; its contributor kit stays local and unsent. No push,
release, public OTA channel or GitHub Pages publication occurred here.
