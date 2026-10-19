# JC8012 preparation and JC1060 SD adaptation, 2026-10-19

This completes the bounded software preparation approved after the remaining
branch integration. It does not merge the JC8012 donor wholesale, activate a
fourth target, change existing production defaults or qualify new hardware.

Base: clean `b9e3ab3f10b1fa19e0e0a640d72c489bd82a5934` from canonical
`dvucinozd/Pajoniiir/master`. Implementation: clean
`5a005092a3bbe499d2d1c48a14c9d34414484916`, on
`codex/jc-board-preparation`. Frozen donor remains
`694ecd6a3f59caa99fd0811e11e0c29d77219411`. Later records/licence/CI wiring do
not alter the compiled implementation tested below. No donor branch is deleted.

## JC8012: isolated preparation, not an active board

The [board preparation contract](../../firmware/boards/jc8012p4a1/README.md)
records panel geometry, backlight, touch, SD ownership and integration gates.
Inspection finds a second distribution blocker: `jd9365_guition_init.inc`
explicitly says its 204-entry panel table was decoded from factory firmware
without an attached licence. The earlier review already identified the
unlicensed GSL3680 driver/firmware table. Neither payload is imported. A BSP
that silently substitutes another table would not preserve the donor's actual
panel behavior, so no pretend runnable entrypoint is added.

The donor's complete replacement PPA component was compared with installed
IDF6.0.2: six of seven source/build files are identical after newline
normalization. Only the DIG-734 calculation in `ppa_srm.c` changes. The private
generator pins all seven hashes, replaces one exact fragment, verifies the
full resulting SRM hash against the frozen donor and seals generated output.
It copies original Espressif notices/licence and does not modify IDF. Its
CMake helper generates only under the future board's build directory; it is
not selected by any existing entrypoint. A generated component refuses other
boards at configure time. The donor's upstream-fix reference is retained as
attribution, not independently verified upstream equivalence.

The ordinary board verifier now checks both the stock IDF component path and
the actual compiled `ppa_srm.c` path. This protects M3 and JC4880, whose shared
component-root discovery made the original donor override unsafe, and JC1060.
The helper is ready for a future licensed entrypoint; full JC8012 driver and
firmware compilation, simulator geometry, BSP resource cleanup and hardware
display/touch/audio/USB tests remain open.

## JC1060: bounded first-mount experiment

The [experiment instructions](../../firmware/boards/jc1060/SD_COLD_POWER_EXPERIMENT.md)
describe opt-in build and physical gates. The adaptation replaces the donor's
unchecked release/GPIO operations with a production state machine and checked
board callbacks. LDO4 is acquired and released, six slot-0 pins are held low for
150 ms, every pin is restored, then power is reacquired and settled for 150 ms.
It runs before the first slot-0 mount under the existing sole-owner startup
contract. Mount retries cannot repeat it. No global SDMMC reset/deinit is used;
Hosted slot 1 and its separate pins remain untouched.

IDF6.0.2's on-chip SD power driver uses an adjustable LDO handle. Inspection of
`esp_ldo_acquire_channel()` confirms that such a channel rejects a second owner;
`sd_pwr_ctrl_del_on_chip_ldo()` frees its handle only after successful release.
The adapter preserves that ownership on release failure and does not drive
pins afterward. Partial clamp failures still restore all pins. Any failure is
latched; a failed cleanup cannot proceed to power-on or mount. As before,
unavailable internal `/sd` does not prevent USB playback startup.

The experiment is JC1060-only and OFF by default. CMake and the build verifier
require explicit matching admission; an old sdkconfig that masks the requested
option causes a configure error. A new CI matrix variant exercises the enabled
path in future runs. Its hosted result is NOT RUN in this local record.
JC1060 Wi-Fi, Ethernet/provider policy, accepted three-board audio/display
paths and dependency versions are unchanged. The donor's 11/11 mount report
is historical, not a canonical-image hardware PASS.

## Verification

All firmware builds below use clean implementation `5a005092`, IDF6.0.2 and
unchanged committed dependency locks. Ordinary build directories were reused
with explicit reconfiguration; the SD experiment uses a new build directory
and fresh sdkconfig. These are build results, not installed images.

| Check | Result |
| --- | --- |
| Complete `run_p4_host_tests.ps1 -Qualification` | Exit 0; 144 functional build steps, 206 run steps; no explicit optional SKIP |
| SD production state machine | Correct order; 15 injected operation failures; all-pin cleanup; retained release ownership; no second cycle; invalid ownership rejected |
| PPA arithmetic | Stock fragment fails a 90-degree case; patched fragment passes 11,616 dimension/rotation/scale/format/chunk combinations |
| Private PPA generator/configuration | Six regression cases, including drift in every pinned file and wrong-board rejection; real pinned-IDF generation and positive helper/configure smoke pass |
| Board verifier | Nine tests, including redirected compiled PPA source and SD admission rejection |
| JC4880 ordinary | 2,609,728 bytes; identity/isolation/budget pass; SD experiment OFF, stock PPA |
| JC1060 ordinary | 2,668,352 bytes; identity/isolation/budget pass; SD experiment OFF, stock PPA |
| M3 ordinary | 2,602,928 bytes; identity/isolation/budget pass; stock PPA |
| JC1060 SD experiment | 2,669,920 bytes; helper compiled and option ON; explicit verifier pass; ordinary verifier rejects it as intended |

The [evidence inventory](../validation/evidence/2026-10-19-jc-board-preparation/README.md)
retains full local logs, exact hashes, generated-overlay provenance and build
metadata. The positive configure smoke uses IDF registration stubs; it is not
a complete private-driver build. No screenshot baseline changes are needed
for these disabled/opt-in changes.

Open gates: resolve both JC8012 payload terms or replacements, then implement
and compile the real BSP/entrypoint; qualify JC1060 SD rail/pin wiring and
cold/warm resets on Kayrosen's exact board/image; review JC1060 Wi-Fi separately.
Numark hardware and the broader UI-core integration remain separate work.
S2 stays operator-closed, JC4880 soak stays waived, and accepted M3/JC4880 images,
beta2 tags/assets and public OTA stay unchanged. No device API, controls, USB,
OTA upload or physical test was used in this preparation.
