# JC4880 controller lifecycle candidate

Date: 2026-10-12. The operator requested continuing the short joint Hercules
Pad FX/global FX/LED/VU block after the controller lifecycle software review.
The available board was explicitly confirmed as JC4880. This record separates
the installed candidate, short playback evidence and remaining control coverage.
Global FX follow-up completed on 2026-10-14 without another firmware installation.

## Exact candidate and installation

Source: `c7d2e9cda2b7bcb99637907694e0ebbe00894783`, clean and pushed to
`codex/shared-p4-core-m3`; remote SHA independently matches local. The
[software review](../reviews/2026-10-12-controller-lifecycle-review.md) records
full required-fixture host qualification, negative reproductions and the fixes
for retired input work and generation-bound LED output. Only JC4880 was built
for this installation; there is no new three-board build or physical matrix.

| Artifact | Identity |
| --- | --- |
| Project / board | `main-deck-p4` / `jc4880` |
| Version | `M2.5-221-gc7d2e9cd` |
| Application | 2,607,824 bytes |
| Application SHA-256 | `38dd5581e358c032b4b612f0f6f34d82bb8a595b5a445c1d84c183177780a18c` |
| Signed bundle | 2,608,012 bytes |
| Bundle SHA-256 | `6542efb696fb420625fc2a6d61619b1070d727bb274b991cb524edc3c7985366` |
| ELF / embedded image hash | `a4562f257dbc1db092a529ebce5f41cde77a17a40add6ef447514e29c5990e7d` |
| Installed slot / state | `ota_0` / VALID, firmware API idle |

A fresh ordinary ESP-IDF v6.0.2 build, isolated BSP/project/source/budget check,
unchanged dependency lock, signed bundle and manifest verification pass. Local
marked `POST /api/ota/p4` completed HTTP 200 with exactly one upload attempt.
After reconnection the firmware API confirmed the exact clean source/project/ELF
and VALID startup. The prior `e22ff334` remains the historical acceptance source.

The host Wi-Fi adapter had DHCP disabled before access and again after OTA
reconnection; the operator restored it with administrator commands. Those host
connectivity gaps are not described as board rollback, PANIC or a proven firmware
network defect. GUI/touch restoration was explicitly confirmed before playback.
Hercules `06F8:B12B`, its unchanged profile, MIDI/UAC1 FS 44.1-kHz packed-24 audio
and the 260-track library returned. Both decks were stopped during installation.

## Short playback and control scope

D1 `Casanova Action {320}` is 44.1 kHz; D2 `Dino Dvornik - Ritam moj` is 48 kHz.
Both start at 15 s, pitch zero, without MT or loops. Faders/mixer are centered,
both PFLs enabled and native Hercules headphone controls retained. This is an FX
follow-up workload, not a repeated MT/loop qualification or long soak.

The first **150.359-s / 341-sample** window passes all **22 present strict deltas
at zero** and output-late delta zero. Every stored sample was audited for identity,
uptime, playback, track identity, controller/audio/storage health and counters.
Final snapshots confirm both decks stopped and the exact VALID image retained.
The operator confirms clean MAIN/Hercules headphones, fluid waveforms and a
stable display throughout this window.

Four `HEAP_WALK_SLOW` journal entries occur in the first approximately 9.5 s
of this window (maximum 2,685 us). They are retained resource-monitoring findings;
there is no new strict loss/output-late event or operator-reported symptom.
The test did not call `/api/resources` or read the diagnostic log during playback.
This is not a claim of zero diagnostic overhead.

Only **2 MIDI packets** arrived during playback and the sampled global FX state
did not change. The operator explicitly confirmed that the requested controls
had **not yet been exercised**. Therefore this window is short audio/display
PASS, with **Pad FX/global FX/LED/VU control coverage NOT RUN**. It is not a
successful active-control sweep.

The subsequent **90.438-s / 206-sample** supplement receives **48 MIDI packets**
and passes all 22 strict deltas and output-late delta zero. The operator confirms
SLICER and Shift+LOOP pads 1-4 on **both decks**, momentary release with effect
tail, the observed pad colours/VU response, clean MAIN/headphones and fluid
waveforms. This closes that selected Pad FX 1/2 and observed LED/VU block on
the exact installed JC4880 image. It does not measure exact echo delay/filter
response, test unsupported pads 5-8 or qualify every LED output/address.

The operator did not perform the six global FX commands. No sampled global FX
change was recorded in those first windows. A separate 30-s supplement was
prepared, then deferred because the operator was not yet ready. Those windows
remain NOT RUN for global FX; their results are not relabelled.

## Global FX command supplement — 2026-10-14

The same clean `c7d2e9cd` image was independently reverified as VALID `ota_0`
with the matching project/ELF, Hercules profile/UAC and mounted storage. The
tracks were reloaded at 15 s, pitch zero, without MT/loops; both PFLs were enabled.
No new build, OTA, profile upload, USB reconnect or soak was performed.

The **30.203-s / 74-sample** window receives **16 MIDI packets** and passes all
22 present strict deltas and output-late delta zero. All six ordered state
changes are captured, with these elapsed times:

| Physical command | Sampled result | Elapsed |
| --- | --- | ---: |
| FX1 | previous effect: filter → delay | 2.625 s |
| FX2 | next effect: delay → filter | 4.266 s |
| FX3 | beat index 2 → 1 | 5.484 s |
| FX4 | beat index 1 → 2 | 7.125 s |
| Shift+FX1 | enabled false → true | 11.609 s |
| Shift+FX2 | enabled true → false | 16.062 s |

The operator explicitly confirms all six commands responded as expected, MAIN
and Hercules headphones remained clean, and waveforms/display were fluid.
Every sample passes the independent identity/playback/health/counter audit.
Final snapshots retain the exact VALID image and confirm both decks stopped,
global FX disabled and no new strict losses. This closes the six-command global
FX adaptation and the requested joint Pad FX/observed LED/VU block on JC4880.
The enabled effect in this supplement was **filter**: selecting delay while
disabled does not qualify its audible global DSP behavior or every effect type.

## Evidence and retained boundaries

Evidence root: `.cache/jc4880-controller-c7d2e9cd-20261012/`, including clean build
and package logs, signed artifacts/matching debug ELF, admission/installed
snapshots, upload/installation audits and the full operator-test evidence.
First window: `pads-fx-led-20261012T143910Z/`, with all samples, baseline/final
snapshots, raw post-stop diagnostic log, runner, audit, operator confirmation
and SHA-256 inventory. The selected Pad FX supplement is
`pads-fx-led-20261012T173238Z/`, with its own original runner, samples, audit,
operator confirmation and hashes. Missing counters are not treated as zero.
The global command supplement is retained under
`.cache/jc4880-global-fx-20261014/global-fx-20261014T173236Z/`, including original
runner, all samples, baseline/final firmware/UI/USB snapshots, post-stop raw
diagnostic log, independent six-state audit, operator confirmation and hashes.

This does not physically reproduce or qualify the reconnect repairs, establish
arbitrary LED-burst throughput, reopen operator-closed S2 work or transfer old
soaks to the new image. JC4880 long soak and hosted CI remain operator-skipped;
JC1060 physical work is deferred to Kayrosen. Public beta assets, stable channels
and GitHub Pages remain unchanged by this local candidate installation.
