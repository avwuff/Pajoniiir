# M3 final local controller candidate

Date: 2026-10-11. Firmware source is clean
`6166d93627304895857b626e719032de3b4f884d`; application identity is
`main-deck-m3`, version `M3-dev-g6166d9362730`.
At the packaging checkpoint this was a locally built and signed development
candidate, not installed, physically accepted or published. No device controls
or profiles were changed during packaging. The subsequently authorized local
OTA and short smoke are recorded below; no public release was published.

## Build and package verification

One M3 bench build was run with ESP-IDF **6.0.2**, reusing the existing
`firmware/main-deck-m3/build_s2_hs` build graph and a dedicated SDKCONFIG copy.
This was an incremental controller build, not another clean three-target matrix.
The ordinary three-target software results remain in the
[final matrix](SHARED_P4_FINAL_BUILD_MATRIX_20261011.md).

The candidate retains the accepted controller topology: **controller USB3/root0,
stick USB2/root1**, normal Wi-Fi power, PCM5102A MAIN and capability-driven USB
headphones. S2 HS/PSRAM DMA and internal SD bounce are enabled; HS-only diagnostic,
low Wi-Fi TX, recorder, alternative UI and DJ Link experiments remain disabled.
This package does not use the ordinary FLX4 USB2/controller and USB3/storage roles.

The generated `sdkconfig.h` is byte-for-byte identical to the frozen accepted
Hercules `e16ecfb5` configuration, SHA-256
`63785c0ec17fc9b820305734c339cae74700a9784078a0c9b903b2aa24b05367`.
Matching configuration does not transfer physical acceptance between images.

Build, experimental board verifier, signed bundle verification and manifest
signature verification all returned exit 0. Actual linked ELF symbols contain
the DWC interrupt decoder wrapper and all three SD wrappers; the unwrapped DWC
decoder is absent. The production audio resampler object has no double-precision
runtime helpers. Debug files and manifest sizes/hashes are independently audited.
Dependency locks and tracked firmware files were unchanged during this preparation.

| Artifact | Bytes | SHA-256 |
| --- | ---: | --- |
| Application BIN | 2,605,008 | `7110d3124dadd0edf06e9008a9247d0cfce9843b02859176b83e722e2d38a15a` |
| Signed DDJOTA | 2,605,196 | `c1e9861778a83dc7c7fb427f45685631ff72b17523ab56174c8d207b3d6e2ce4` |
| ELF | 29,261,268 | `b0778c33f32757a80c59630cd9fd2426600da51167d0c4eb2678ff8f31aea448` |

The image is below the `0x380000` application budget and fits the existing
`0x400000` OTA slot. Signing uses ECDSA-P256-SHA256 and trusted key `rel-001`.
The development version is for local signed upload; it is not a public pull-update
release. Private key material is not included in any delivery artifact.

## Both controller profiles

The same shared firmware supports both profiles. Each was compiled from its
tracked JSON and compared against its tracked binary: both are deterministic
matches. Copies of each JSON, binary and README are included under the package's
`controllers/` directory; these profile files are separate from the signed OTA
payload and require separate profile installation if needed.

| Profile | Binary bytes | SHA-256 |
| --- | ---: | --- |
| `ni_traktor_s2_mk3_full` | 1,624 | `ab9259c51c2b18cf5b67e2e279e3a0fbf9ee167a489f345b813a9ef37f40725f` |
| `hercules_djcontrol_inpulse_500` | 5,592 | `570b59ecca6baa7319ef85ca92d7d3a11fbe11ea1f9c9c580a8779031a40f549` |

S2 uses MIDI mode (hold left FLX while connecting). The bench's full S2 profile
occupies SD storage id `ni_traktor_s2_mk3_basic`; do not add a second profile with
the same S2 VID/PID. If reinstallation is necessary, atomically replace that
existing id. Hercules uses its distinct `hercules_djcontrol_inpulse_500` id.
No profile was uploaded or overwritten during this preparation.

The operator closed the
[S2 campaign](S2_MK3_PROFILE_PREPARATION_20261007.md) with waivers and requested
no further S2 tests. Exact-image 48/44.1-kHz hour tests and documented control
checks remain historical evidence. Waived physical checks remain NOT RUN; this
candidate does not reopen that campaign or turn its waivers into PASS.

The installed Hercules `e16ecfb5` image and its
[accepted campaign](HERCULES_INPULSE_500_PREPARATION_20261010.md) remain separate:
its full hour had zero new strict losses, operator-confirmed clean audio/display,
and 15 output-late events (maximum 12,534 us). This was not a zero-late result.
Neither historical campaign qualifies this newly built image.

## Evidence and next boundary

Local evidence root: `.cache/final-m3-candidate-6166d936/`.
It contains `build.log`, `package.log`, dedicated `sdkconfig`,
`audit_candidate.py`, `candidate-audit.json` and `documentation.log`.
Package directory: `package/pajoniiir-m3-M3-dev-g6166d9362730/`, containing BIN,
DDJOTA, signed manifest, local debug archive, both profiles and usage notes.
The package's `SHA256SUMS.txt` covers all its other files; the external audit
retains the final file hashes.

Full host/no-SKIP and Linux ASan/UBSan results from the completed protocol review
remain applicable to the unchanged production/test code. Those suites and the
three UI simulators were not repeated for this packaging-only step.
Documentation integrity and Git whitespace checks must pass before committing
this record. Build logs retain existing nonfatal upstream warnings.

Next device step is a separately authorized local OTA and brief operator smoke
of GUI, MAIN/headphones and controller responsiveness. No new S2 campaign is
requested. Current CI is operator-deferred due to quota; JC4880/JC1060 physical
acceptance and remaining shared-core gates are not closed by this package.
No push, release tag, public OTA channel or published artifact was changed.

## Authorized local OTA and short smoke

On 2026-10-11 the operator authorized local OTA and a short smoke. One signed
upload returned HTTP 200. Startup reached `valid` / OTA service `idle` in
`ota_1`, with exact source `6166d93627304895857b626e719032de3b4f884d`, clean
identity, version `M3-dev-g6166d9362730` and the expected ELF hash above.
The pre-existing crash summary was unchanged across the restart. Hercules MIDI
In/Out and UAC1 44,100-Hz packed-24 streaming recovered, and the catalog had
260 tracks. The operator confirmed GUI/touch were normal.

An intervening operator report that first PLAY required a second press was
clarified: the first press only woke the visible screensaver. This is the
intentional consume-first-wake behavior, not a reproduced playback defect.
Read-only logs also contain two earlier D1 259-frame underruns (518 total),
USB/controller recovery and storage disconnect/remount history. Their causal
association is unproven. They remain historical observations, not cleared errors
or symptoms attributable to the wake press.

The subsequent bounded mixed-rate dual smoke ran **30.094 seconds / 51 samples**
with D1 Casanova Action 44.1 kHz, D2 Ritam moj 48 kHz, pitch zero, no MT or loops,
MAIN 48 kHz and Hercules headphones 44.1 kHz. All 22 strict counter deltas were
present and zero; output-late delta was zero. The nonzero pre-existing counters,
including 518 D1 underruns, one invalid feedback and stopped headphone underflow,
were retained as the baseline. No heap walk was performed during playback.
Both decks were confirmed stopped at the end. The operator confirmed clean MAIN
and headphones and fluid waveforms. Only two MIDI packets were observed and the
operator did not rotate jogs/Browse: active-control qualification is **NOT RUN**
for this smoke. No extra controller campaign or long soak was started.

Evidence is `.cache/final-m3-candidate-6166d936/ota-20261011T183237Z/` and
`smoke-30s-20261011T202456Z/`, containing before/after identity and status,
result, baseline, all samples and final stopped/firmware/UI/USB snapshots.
The smoke archive also contains the runner, operator confirmation, raw diagnostic
log and SHA-256 inventory. This is a short exact-image audio/display smoke, not
full new-image qualification or evidence that historical underrun causes are fixed.
S2 remains operator-closed with waivers and was not physically connected/tested.

The subsequent active-control supplement completed 20.531 s / 35 samples with
2421 MIDI packets. All 22 strict deltas and output-late delta were zero; both
decks ended stopped. Operator confirmed both jogs/Browse, clean MAIN/headphones
and normal waveforms. This closes the short smoke's active-input gap for this
image, not broader profile or long-soak qualification. Evidence:
`.cache/final-m3-candidate-6166d936/controls-20s-20261011T202900Z/`.
See the [final development handoff](../reviews/2026-10-11-shared-core-final-handoff.md)
for the consolidated completed, waived and open scope.
