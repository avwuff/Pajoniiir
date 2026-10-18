# Ordinary M3 headphone-repair OTA candidate

Prepared locally on 2026-10-18 at the operator's request. This step builds and
signs the ordinary M3 target only. It performs no device queries, OTA upload,
USB changes, reset, physical test, release or public-channel update.

## Frozen identity

| Field | Candidate |
| --- | --- |
| Source | `71ee534269bf37ffe2314ff0ae8bd2110212cf0d`, clean |
| Version | `M3-dev-g71ee534269bf` |
| Board / project | `m3` / `main-deck-m3` |
| Build | Ordinary ESP-IDF 6.0.2; S2/HS role experiment OFF |
| Application / signed bundle | 2,603,088 B / 2,603,276 B |
| Signature | `rel-001`, ECDSA P-256 / SHA-256 |
| Installed / physical smoke / published | VALID `ota_1` / partial telemetry FAIL below; loudness accepted / NO |

Application SHA-256:
`72063153d541f3227435a0993ffbb4ef0eb45c481077d40ed61ab07993517d15`

Signed bundle SHA-256:
`aaeab422bd4b671d7de17ea43cb1dd2ef1f6b2dd1646f09fc81f436d5b1044ab`

Matching ELF SHA-256:
`aa911155cd5abe00f56e71a69aa91940f191c60efb9889b8b481e9ef5b5c8e88`

Local package directory:
`releases/m3-headphones-71ee5342-20261018/pajoniiir-m3-M3-dev-g71ee534269bf/`.
Use `main-deck-m3.ddjota` for the later signed local OTA. Raw application BIN,
signed manifest and matching debug ELF/map are retained alongside it. No private
signing key is included. This development version has no automatic pull-update
eligibility and is not a numbered release or production approval.

## Change and verification

The candidate contains the shared [FLX4 headphone-level repair](FLX4_HEADPHONE_LEVEL_20261018.md):
dedicated cue PCM no longer loses 12.041 dB after the mixer, and status adds
headphone LEVEL. The PCM5102A MAIN path, FLX4 USB MAIN mirror level, gain control,
stream cadence and generic/native controller behavior retain their prior logic.

All production, test, tooling, controller and CI files are identical between
qualified source `605bcd20` and this source; intervening commits record evidence
and documentation. The previous complete `-Qualification` host run (no SKIP)
and focused JC4880/M3-HS production-stream regressions therefore remain the
software evidence for these unchanged modules. They were not rerun solely for
documentation commits. The combined JC4880 physical smoke confirms the repair
on its own image; it does not qualify this M3 image.

The fresh M3 build completes successfully. The production board verifier checks
BSP/component isolation, LVGL pinning, embedded project/source/cleanliness,
ordinary USB/DMA policy and the `0x380000` application budget. The unmodified
packager verifies the signed bundle, signed manifest, board, source, version and
artifact hashes. The public-key DER is byte-identical to that in the last
recorded installed M3 source `c0ee8152`; its SHA-256 is
`125ef6eb776729e3018ecc564317254febec4a9a2bf34892e99d57f10bc02e43`.
The tracked dependency locks remain unchanged.

Effective SDKCONFIG values are identical to the accepted ordinary M3 `c0ee8152`
build. In particular the candidate retains:

- FLX4 on USB2 / FS / root1 and storage on USB3 / HS / root0;
- PCM5102A MAIN at fixed 48 kHz, with Wi-Fi enabled by default;
- native 800x480 DSI506, existing touch and waveform scheduling policies;
- microphone/NS4150/Ethernet and unqualified network/recorder services disabled.

The [preparation record](evidence/2026-10-18-m3-headphone-package/preparation.json)
retains sizes/hashes, original board/packager verification, manifest/signature,
build identity, trust-key equality, configuration equality and production-diff
provenance. Its byte-preserving inventory is checked against staged Git content.
Original build logs remain under `.cache/m3-headphones-20261018/` and the new
build under `.cache/idf-m3-headphones-20261018/`. Prior build artifacts are not
overwritten. No other board is rebuilt in this preparation.

## Later installation boundary

The last recorded M3 installation remains `c0ee8152`, VALID `ota_0`, in
[the preceding OTA/smoke record](M3_SHARED_INTEGRATION_OTA_SMOKE_20261018.md).
It was not queried during preparation; its state is not a new live observation.
This package is ready locally but has no installed or physical PASS.

For the next separately requested installation, keep the ordinary port roles,
Wi-Fi ON and both decks stopped. Start headphone LEVEL at minimum. After boot,
confirm exact version/source/ELF and VALID state; wake the screen by touch and
move LEVEL slightly up then back to minimum, and MIX away from then back to CUE.
Verify LEVEL=0 / MIX=0 via status before playing. This reads the physical knobs
after boot instead of assuming their positions override initial software state.
Raise LEVEL gradually: the restored cue signal can be four times the old level.

One bounded mixed-rate dual-deck smoke should check both PFLs, MIX/LEVEL,
MAIN/headphones, MT/loop/CUE and waveforms on the exact installed M3 image.
Historical M3 soaks, JC4880 listening acceptance, S2 closeout and the JC4880
soak waiver retain their original scope. No new long-soak requirement or public
publication is introduced by preparing this package.

## Subsequent authorized local OTA, 2026-10-18

After preparation, the operator reported connection to `Pajoniiir-M3` and
explicitly requested installation. Live preflight confirms the preceding clean
`c0ee8152` on VALID `ota_0`, idle OTA service, both decks stopped, FLX4 active
and ordinary USB roles. The signed bundle hash was rechecked before upload.

The unchanged 2,603,276-byte bundle uploads with HTTP 200 in 16.209 seconds;
the response is `{"ok":true,"rebooting":true}`. After reconnecting the host to
the returning SoftAP, `/api/firmware` confirms:

- clean source `71ee534269bf37ffe2314ff0ae8bd2110212cf0d`;
- `M3-dev-g71ee534269bf`, board `m3`, project `main-deck-m3`;
- VALID `ota_1`, OTA service idle and empty `last_error`;
- exact matching ELF `aa911155cd5abe00f56e71a69aa91940f191c60efb9889b8b481e9ef5b5c8e88`.

Boot 427 records SW reset, FLX4 reconnect and profile activation, storage mount,
260 library tracks and Wi-Fi startup. The early controller disconnect/reconnect
recovers within startup; the final MIDI In/Out and 44.1-kHz/16-bit FS UAC stream
are active and non-faulted. The existing 7,776-byte crash summary (task
`ota_reboot`, old ELF prefix `38c637996`) is unchanged from preflight; it is not
evidence of a new crash on this image. Both decks remain stopped. No playback,
new soak or public update is performed in this installation step.

The [installation evidence](evidence/2026-10-18-m3-headphone-ota/installation.json)
and its hashed raw snapshots distinguish this installed identity from the
earlier preparation-only record. The operator confirms GUI/touch, minimum
headphone LEVEL and MIX at CUE. A subsequent live readback confirms LEVEL=0,
MIX=0 and both decks stopped at 105.452 seconds uptime, with the same VALID
identity. The subsequently attempted smoke is recorded below; installation
alone does not qualify audio or controls.

## Subsequent mixed-rate MT smoke: telemetry FAIL, volume accepted

The operator explicitly confirms readiness with MT on both decks and zoom 8.
One bounded 60-second runner starts with D1 `Casanova Action {320}` at
44.1 kHz/+5% and D2 `Dino Dvornik - Ritam moj` at 48 kHz/-5%. Both are
prepared at 15 seconds, initially stopped, with PCM5102A MAIN at 48 kHz and
FLX4 FS UAC headphones at 44.1 kHz/16-bit. No firmware, profile, USB or Wi-Fi
change precedes playback.

The runner stops after **40.642 seconds** on **3,387 new PCM underrun counts
on D2**. All other 21 present strict counter deltas, including USB/UAC loss
and reconnect counters, are zero; output-late delta is zero. It receives
1,482 MIDI packets. Both 4-beat loops are observed active at 27.789 seconds;
the last recorded pre-fault sample at 35.498 seconds still has both loops
active and all strict deltas zero. The error is raised while waiting for
loop-clear completion, so the CUE/PLAY phases are not reached. Cleanup
confirms both decks stopped and both loops OFF, on the same VALID image.

The operator reports **no perceived audio or visual disturbance and sufficient
headphone loudness**. This accepts volume for the observed M3 bench window;
it does not override the telemetry failure or qualify the uncompleted smoke.
LEVEL spans 0..10071 (about 61.5%), MIX stays at CUE, and sampled PFL states
include both off and both on. No complete MIX sweep or every requested physical
control sequence is claimed from these samples.

The diagnostic journal also records an internal largest-free-block query of
436,751 us wall time before the PCM2 loss. The production query includes
preemption; this number is not measured IRQ-disabled time and does not establish
the cause of the underrun. Ordinary FS builds still perform internal/DMA heap
walks, whereas the HS diagnostic configuration skips them. The next bounded
investigation should distinguish loop-exit refill from observer interference.
No speculative production fix, new image or automatic rerun is performed.

The [failure audit](evidence/2026-10-18-m3-headphone-smoke/audit.json) preserves
the runner, preparation, all 43 successful pre-fault samples, final snapshots,
raw diagnostic log and separate operator response with hashes. The runner
validates before appending each response: the failing live response itself was
not written; the final stopped snapshot retains the exact PCM2 delta. This
limitation is explicit. No new reset, disconnect or output-late event explains
the stop, and no full combined-smoke PASS or long-soak result is claimed.
