# FLX4 club-use headphone level investigation

## Operator report and unchanged bench

On 2026-10-18 the operator reported the JC4880's first club performance on the
previous day: operation was good, but FLX4 headphone output was insufficient
with HEADPHONES LEVEL fully up and MIX fully towards CUE. Headphones were
Audio-Technica ATH-M20x (47 ohms, manufacturer sensitivity 96 dB/mW). FLX4
received power only from JC4880. No external FLX4 power adapter is available
for an electrical comparison; neither a power limitation nor an exact acoustic
gain difference has been measured.

After restoring Windows DHCP, the API identifies clean JC4880
`c7d2e9cda2b7bcb99637907694e0ebbe00894783`, `M2.5-221-gc7d2e9cd`, VALID
`ota_0`, ELF `a4562f257dbc1db092a529ebce5f41cde77a17a40add6ef447514e29c5990e7d`.
MAIN is PCM5102A. The stick mounts. Initially only the stick had enumerated;
one operator FLX4 cable reconnect establishes MIDI In/Out, active
`pioneer_ddj_flx4` and healthy FS UAC1 44.1 kHz / 16-bit / 2-byte streaming.
Both decks remain stopped and unloaded. No audio is started during diagnosis.
Initial mixer values are boot defaults, not evidence of yesterday's knobs.
The old status API omits headphone LEVEL despite retaining it in the mixer
snapshot; the candidate adds this field without removing existing fields.

## Confirmed digital loss

The final production `controller_usb_audio_stream_write()` transport divides
all four channels by four for qualified FLX4 with PCM/I2S-paced MAIN. This
condition is true on this installed bench. A dedicated headphone sample of
16000 reaches the ring as 4000: **-12.041 dB**, after the mixer has already
applied PFL, MIX and LEVEL. Maximum software LEVEL itself is unity. MIX fully
towards CUE excludes the independent half-CUE contribution at midpoint.

The shift entered with direct P4 FLX4 audio commit `6e41d7f9` on 2026-08-27.
Commit `51ac8dab` on 2026-10-04 restricted it to PCM-paced FLX4, preserving
generic/USB-MAIN unity; Package F explicitly preserved the old attenuation.
No rationale for cutting the dedicated headphone bus was found in those
records. The earlier mixer-only investigation missed this later transport
stage; it cannot be used to rule out software attenuation.

The captured FLX4 configuration has no UAC AudioControl Feature Unit descriptor
for a host-programmable USB volume. Its working 44.1-kHz SET_CUR is a sampling
frequency request, not volume configuration. This does not describe proprietary
or internal analog gain behavior. Weak supply may independently limit physical
output; the manufacturer's manual explicitly names this possibility.

## Bounded candidate

Dedicated headphone PCM now passes to the ring at unity. This restores the
existing mixer output without multiplication above full scale. The legacy
USB MAIN mirror attenuation, missing-headphone fallback, PCM5102A MAIN,
generic/native controllers, pacing, packet layout and USB startup are unchanged.
The fixed path also applies to ordinary M3 FLX4 because it shares this module;
it does not establish a new M3 physical acceptance.

The production-stream regression fails before repair at the dedicated
headphone channel amplitude assertion. After repair it checks positive/
negative full-scale and one-LSB samples, distinct MAIN/cue routing, absent
MAIN, and amplitude through the 48-to-44.1-kHz resampler. Existing MAIN-only
fallback and generic/native format/lifecycle tests remain in the suite.
Both JC4880 and M3-HS host policy variants pass, including 48-to-44.1-kHz
amplitude preservation. Full `-Qualification` host suite passes with no SKIP.
Documentation integrity and a clean ordinary JC4880 ESP-IDF 6.0.2 build pass,
including board/component isolation, linked source identity and image budget.
Only JC4880 firmware was built; no other target's physical gate is reopened.

Clean candidate source: `605bcd20d6dadf250ce1816d82353ee2ace21867`, version
`M2.5-231-g605bcd20`. Application 2,609,808 B, signed bundle 2,609,996 B.

Application SHA-256:
`b5ce34b69fd99b5e983dc024ecf60d41ec515c648aad23b1cd0671f8e3ccb2b8`

Signed bundle SHA-256:
`90f19d01348852a01aa8950bdf3f9ce6776ae2c81650f4cba0a3112587fcc0db`

Matching ELF SHA-256:
`f1c788368a7feff414909e248043df085e8e5e5e484d7394a5cf99c80fedfeb7`

The production packager verifies the image, signed bundle, manifest, trusted
key and exact source. The [verification record](evidence/2026-10-18-flx4-headphones/candidate-verification.json)
retains identities and raw regression/qualification evidence. The operator
confirms LEVEL is at minimum and readiness for OTA/listening. One signed
upload returns HTTP 200. During startup the operator confirms switching power
OFF/ON. The next read shows the former `c7d2e9cd` VALID `ota_0`; the old crash
summary is byte-for-byte equivalent and its ELF prefix `e2719f73a` does not
identify this candidate. The service journal records upload verification on
boot 627, then old firmware boot 629 / POWERON. This is a power-interrupted
pending-image attempt, not a clean autonomous startup-failure reproduction.
A controlled retry uses the identical signed bundle, with no requested
power/RESET action. It completes HTTP 200 and confirms clean `605bcd20`,
matching ELF, `M2.5-231-g605bcd20`, VALID `ota_1`, idle OTA state and both decks
stopped. FLX4 MIDI/profile and 44.1-kHz/16-bit UAC are healthy; the old crash
summary remains identical. Windows DHCP must again be restored after reconnect;
this host-network issue is not an image startup failure. There were two uploads
in total: the power-interrupted first attempt and the controlled retry.

## Short listening comparison and corrected operator result

The operator confirms the post-OTA knob exercise. Live API readback establishes
MIX=0 (full CUE) and LEVEL=0 before playback, rather than treating unchanged
physical knobs as authoritative over boot defaults. D1 `Casanova Action {320}`
is loaded at 15 s, PFL D1 enabled, pitch zero/MT OFF, D2 stopped/unloaded.
The stick has 189 tracks. ATH-M20x and the JC4880-only FLX4 power connection
remain the original bench setup; there is no separate power adapter test.

One **30.016-s / 55-sample** solo-D1 window records **136 MIDI packets** and
LEVEL raw **0..5776** (maximum approximately 35.3% of 16383). All 22 present
strict counter deltas and output-late delta are zero. Both decks finish stopped.
No headphone-level maximum or club sound pressure measurement is inferred from
these samples. A separate offline all-sample audit binds the source, healthy
FLX4 stream, fixed CUE/PFL routing, counters and final stopped state.

The operator first selected "louder but still insufficient", then explicitly
corrected that answer: **"krivo sam ti rekao. taman je glasno. kad okrenem do
100% mislim da bi to zadovoljlo i klupske uvjete"**. The corrected answer is
the final authority for the short listening result: headphone loudness is
sufficient in this bench test. No distortion was reported; the correction
specifically confirms loudness. The expectation about 100% in a club remains
an operator prediction, not a new club acceptance or recorded 100% sweep.

This closes the reported low-headphone-level issue for this short D1 bench
comparison on the exact installed image. The following combined smoke adds
the separate two-deck controls/MT/audio and PFL/MIX coverage; it is not inferred
from the solo test. No new soak,
electrical/VBUS measurement, reconnect campaign or public publication occurred.
For any later audition, start LEVEL low and raise gradually. Historical
hardware/soak results retain their original image scope.

## Combined dual-deck MT / transport / headphone smoke

Later on 2026-10-18, the same clean `605bcd20`, VALID `ota_1` passes one
operator-ready **60.125-s / 86-sample** combined smoke. No rebuild, OTA,
controller reconnect or profile change precedes this supplement. D1 is
`Casanova Action {320}` at 44.1 kHz / +5%; D2 is `Dino Dvornik - Ritam moj`
at 48 kHz / -5%. Both use MT, zoom 8 and the ordinary PCM5102A 48-kHz MAIN /
FLX4 FS UAC1 44.1-kHz, 16-bit headphone path. The catalog has 189 tracks.

The operator is asked to use PFL D1/D2 and headphone MIX/LEVEL in the first
20 s, both jogs/Browse in the next 10 s, then watch/listen while the actor
performs loop and transient CDJ CUE/PLAY transitions. Persistent Hot Cues are
not edited. The final operator answer confirms **all requested steps were
checked and normal**, with clean MAIN/headphones and fluid waveforms.

| Measurement | Result |
| --- | --- |
| New MIDI packets | 3,658 |
| All 22 present strict counter deltas | 0 |
| Output-late delta / final since-boot maximum | 0 / 0 us |
| Sampled PFL states | D1 on/D2 off; D1 on/D2 on |
| Headphone MIX raw range | 0..16383, CUE through MASTER |
| Headphone LEVEL raw range | 2245..14919, approximately 13.7..91.1% |
| Identity, rates, MT, pitch, zoom | Maintained on this image |
| Final transport | Both stopped, loops OFF |

The PFL states listed are the sampled telemetry, not a claim that every brief
button transition was sampled. Broader physical control behavior is confirmed
by the operator. No 100% LEVEL sweep or club loudness measurement is added.

| Actor transition | Observed elapsed |
| --- | ---: |
| Both 4-beat loops active | 27.375 s |
| Both loops cleared | 37.078 s |
| D1 CUE parks only D1 / D1 PLAY resumes | 44.281 / 44.515 s |
| D2 CUE parks only D2 / D2 PLAY resumes | 51.250 / 51.469 s |

The offline all-sample audit checks every identity, strict counter, rate,
MT/pitch, healthy stream and track, plus initial/final zoom and final stopped
state. It passes. Nonzero startup/stopped baseline counters are retained and
do not become new playback losses. Original runner, all samples, final
firmware/UI/USB snapshots, separate operator confirmation and hashes are in
[the combined-smoke evidence](evidence/2026-10-18-flx4-headphones-dual/audit.json).
This closes the requested short combined JC4880 smoke on the installed image.
The long soak remains explicitly waived. Other boards, electrical qualification,
long reconnect campaigns and actual club use retain their separate scope.

## References and working evidence

- [Audio-Technica ATH-M20x specifications](https://www.audio-technica.com.hk/?lang=eng&op=productdetails&pid=924).
- [Manufacturer FLX4 manual, mirrored PDF](https://manuals.plus/m/6da2c55a0f9fa8c040588dafe283205fdc3a71a0bf7675dcdc95a352ec0546d7_optim.pdf),
  pages 34/38 (separate power connection), 153 (insufficient-power headphone limit).
- [JC4880 wiring](../HARDWARE_WIRING.md): old electrical acceptance applies only
  to the unchanged measured wiring; no new electrical PASS is inferred here.
- Local `.cache/jc4880-headphone-level-20261018/`: original status/firmware,
  reconnect status/controller descriptors, profile catalog, failing/passing
  production-stream test logs and subsequent build/qualification records.
