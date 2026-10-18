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
Both JC4880 and M3-HS host policy variants pass the initial focused run.
Full host qualification and a clean JC4880 ESP-IDF 6.0.2 build are pending.
Only JC4880 firmware will be built for the next physical test.

## Next physical comparison

Candidate installation/listening is **NOT RUN**. Preserve the original bench
and record source/ELF plus headphone LEVEL/MIX and PFL. Begin headphone LEVEL
fully down: the candidate's dedicated USB cue signal can be four times the old
amplitude. Raise it gradually while comparing the same track, TRIM/EQ and
ATH-M20x. Check both PFLs, MIX sweep, mute/minimum, distortion and MAIN isolation.
The firmware does not measure headphone voltage, sound pressure or VBUS.
No claim that supply limitations or club audibility are solved precedes that
operator comparison. No public assets/channels are changed; no soak is reopened.

## References and working evidence

- [Audio-Technica ATH-M20x specifications](https://www.audio-technica.com.hk/?lang=eng&op=productdetails&pid=924).
- [Manufacturer FLX4 manual, mirrored PDF](https://manuals.plus/m/6da2c55a0f9fa8c040588dafe283205fdc3a71a0bf7675dcdc95a352ec0546d7_optim.pdf),
  pages 34/38 (separate power connection), 153 (insufficient-power headphone limit).
- [JC4880 wiring](../HARDWARE_WIRING.md): old electrical acceptance applies only
  to the unchanged measured wiring; no new electrical PASS is inferred here.
- Local `.cache/jc4880-headphone-level-20261018/`: original status/firmware,
  reconnect status/controller descriptors, profile catalog, failing/passing
  production-stream test logs and subsequent build/qualification records.
