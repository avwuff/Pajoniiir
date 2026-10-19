# Remaining branches: merge review, 2026-10-19

The first integration candidate is the current `codex/shared-p4-core-m3`
branch into `master`. The other branches should not be merged wholesale into
the qualified shared core. `kay/new-ui` can be retained as an explicitly
disabled prototype; Numark needs a PFL semantic repair and real test wiring;
JC8012 needs a separate board integration with driver provenance and component
isolation resolved. The JC1060 donor has already been selectively integrated.
The local APTA branch contains no outstanding commits.

This is a read-only branch/code review. No branch was merged, deleted or
rebased; no firmware was built, flashed or tested on hardware. Existing beta2
assets, tags, public OTA and operator acceptance remain unchanged.

## Frozen repository and evidence

Repository: `dvucinozd/Pajoniiir`, default branch `master`. Origin was fetched
with pruning before comparison. GitHub's branch listing agrees with the
remote-tracking refs. No open PR exists at the review snapshot. PR46 was merged
earlier and does not cover the subsequent 32 shared-branch commits.

| Ref | Frozen tip | Commits ahead of origin/master | Conflicted paths against current shared core |
| --- | --- | ---: | ---: |
| `codex/shared-p4-core-m3` | `584dc0d2b43e8a9deb847b83191a87f40f5a4d0b` | 32 | 0; already the current branch |
| `jc8012p4a1-v3` | `694ecd6a3f59caa99fd0811e11e0c29d77219411` | 97 | 9 |
| `kay/jc1060-hw-bench` | `00041a0dd1c704781af4d991b663c05cdb6bd4c3` | 61 | 8 |
| `kay/new-ui` | `b8dccf260db4c919cae209091a70ef18f89f18cc` | 19 | 0 |
| `numark-dj2go2-touch` | `86d6b412b2ba11908b48518dfaf26d51f6c3e0f6` | 7 | 0 |
| `codex/p4-apta-library` (local only) | `364930b65eceefb562fec597c6c0c681c96084e7` | 0; already an ancestor | 0; already contained |

Frozen `origin/master` is `8d56f303d13c06daa3d194ae1211785a8cdc33e3`.
All five remote development branches descend from that tip. The current shared
branch is 32 commits ahead without divergence; each of the four other remote
development branches lacks those 32 commits. JC8012 contains the complete
61-commit JC1060 donor ancestry and adds another 36 commits.

The local `master` is still `9af99cd2`, 219 commits behind `origin/master`, and
is checked out in a separate worktree. It was not used as the review baseline
or changed. The APTA worktree is clean and remains attached to its owner.
Its branch is 535 commits behind `origin/master`, with no unique commits.

The [evidence inventory](../validation/evidence/2026-10-19-branch-review/README.md)
contains exact counts, virtual merge results, focused test outputs, GitHub
check-run metadata and pinned source excerpts. All counts above refer to the
frozen snapshot, before adding this documentation-only review commit.

## Findings that affect the merge decision

### P1: the JC8012 PPA backport also enters JC4880's component discovery

Commit `eeef2f0d` puts an entire replacement IDF component in
`firmware/p4-core/components/esp_driver_ppa/`. Its CMake guard is
`CONFIG_SOC_PPA_SUPPORTED`, not a JC8012 board guard. The existing JC4880
entrypoint scans the entire `p4-core/components` directory using
`EXTRA_COMPONENT_DIRS`. ESP-IDF 6.0.2 discovers its child components and lets
project components override IDF components of the same name.

Consequently, a complete merge exposes JC4880 to this replacement even though
the donor describes it as board-specific. M3 and JC1060 use explicit component
lists; do not generalize the JC4880 finding to every target. This is a verified
source/build-selection issue; a merged firmware configure/build was not run.

Keep the override inside a board-scoped component directory or provide an
explicit, fail-closed selection contract before integration. Verify the selected
compiled component for every target. Any deliberate shared IDF backport needs
its own pinned diff, provenance, build checks and display/timing regressions.
The donor's polling/display experiment does not qualify our accepted M3 or
JC4880 images with a replacement driver.

### P1: the JC8012 touch driver has an unresolved distribution gate

The donor's `esp_lcd_touch_gsl3680/README.md` explicitly states that no licence
is attached to the code or embedded firmware table. It records a local decision
to use it on the owner's unit and requires a licence or replacement before a
release. This is unresolved provenance/permission evidence, not a conclusion
that the rest of the new board cannot be developed.

Do not incorporate that vendor copy into a distributable canonical target until
its terms are established or a compatible replacement is available. Preserve
the source attribution and original notice. A private bench decision does not
by itself establish permission for the shared repository's release bundle.

### P2: Numark PFL cannot work correctly with the current toggle-only action

The Numark README records a latching PFL button: consecutive physical presses
send Note On 127, Note Off 0, then Note On 127. The profile emits ordinary
`deckN.pfl` button events. Canonical `deck_core.c:on_mixer_control` toggles PFL
only for nonzero values, so the second physical press cannot set PFL OFF.
The donor records an operator-observed LED/audio disagreement from that point.

Dropping Note Off entries or changing a CC inversion does not repair this
contract. Add an explicit profile-selectable level/set semantic through the
shared semantic action path, including validator/compiler/runtime coverage and
LED reconciliation. Preserve momentary-toggle semantics for existing FLX4 and
DDJ-400 profiles. Test ON/OFF/ON on both decks, connection epochs and held-state
reconciliation, then perform a bounded Numark operator check. This review
identifies the required behavior; it does not select or implement a new ABI.

### P2: Numark's documented regression tests are not actually installed

Only four files change on the Numark branch: its README, profile JSON, binary
and an embedded `.patch`. The README says the normal runners execute
`test_numark_dj2go2_touch.c` and `test_numark_touch_fs`; those additions are
absent from the actual branch's test tree/runner changes. A test stored inside
a patch attachment is not automatically executed by CI or the host suite.

The current compiler reproduces the committed 1,999-byte S3CP v4 binary exactly.
For this review, the profile test was extracted from the patch into an ignored
overlay, compiled against the current production parser/manager and passed.
That is useful parser/mapping evidence, but it does not repair test registration,
execute the deck PFL actor, or establish the claimed USB descriptor/stream gate.
Install the tests and runner wiring as real repository files before integration;
add a production actor regression that fails for the latching PFL sequence.

### P2: a full JC1060 donor merge would reintroduce rejected implementations

The exact current donor tip `00041a0d` was already covered by the
[contributor review](2026-10-14-jc1060-contributor-review.md),
[selective integration](2026-10-14-jc1060-selective-integration.md) and
[verification record](2026-10-14-jc1060-integration-verification.md).
Canonical commit `c0ee8152` adapts root admission, bounded HS UAC1 support,
atomic remote CUE, USB MAIN recovery, loop EOF bounds, Ethernet startup health,
dependency byte handling and library worker/progress retirement.

The donor still has the storage admission shortcut that returns true for a
hub configuration after a successful metadata query, without honoring the root
match result. Canonical code resolves ancestry and rejects wrong/unknown roots.
Full-merging and choosing donor conflict resolutions would undo this boundary.
Other adapted repairs likewise require semantic comparison, not choosing the
newer-looking file wholesale. Unsafe Linux HIL/PID-reuse tooling was explicitly
excluded from the previous integration.

There are eight conflicting paths against the current shared core, covering
JC1060 startup, capabilities, USB audio, storage, web Host handling, DDJ-400
documentation and the storage experiment helper. JC8012 inherits those same
eight conflicts and adds `board_capabilities.c`. The complete path lists are
in the inventory. Donor hardware logs remain historical evidence for their
own exact image; they do not close Kayrosen's beta2 physical gate.

## Branch-by-branch disposition

### Current shared core: integrate into master first

The 32-commit fast-forward includes qualified shared lifecycle/controller fixes,
the FLX4 dedicated-headphone level repair, diagnostic snapshot heap-walk removal,
MT fallback re-anchoring, bounded ordinary cache maintenance and the associated
beta2 acceptance/publication records. These changes must be the base for later
work. The published beta2 tag/assets remain immutable.

Recommendation: prepare a new reviewable PR from the current shared branch to
`master`, with the retained board acceptance and open gates summarized. PR46
is historical. Hosted CI remains NOT RUN under the quota waiver; merging is
not a substitute for JC1060 physical acceptance or the waived JC4880 soak.
No PR or merge was created by this review.

### New UI: eligible as an isolated prototype, not a production UI switch

The 19 commits add 82 files, mostly UI code and generated fonts. All eight
focused pure-module tests pass: layout, wave, key, text, view, format, sync
and color. Font licence notices are present. Virtual merging is conflict-free.

However, `CORE_GAPS.md` explicitly describes 15 screens on demo data with no
core binding or touch reaction. `ui_next` has no production component
`CMakeLists.txt` or startup selection. The simulator is a demo renderer, not
the existing accepted screenshot/hash or panel-timing gate. Recording,
four-deck and other visible demo features do not become supported capabilities.

Recommendation: import as a clearly disabled development prototype with its
tests and documentation. First reconcile its inventory against today's core,
then separately implement generation-safe data adapters, semantic actions,
touch/wake behavior and capability-driven availability. Activation needs a
reviewed screen/resource budget and board-specific rendering acceptance;
preserve the accepted M3 layout/default until then. No new baseline should be
silently accepted just because the pure tests pass.

### Numark: fix the bounded blockers, then integrate the profile

The mapping is a useful starting point: VID/PID `15e4:003c`, S3CP v4,
100 inputs/30 outputs, pinned Mixxx references and partial operator evidence.
The full-speed synchronous four-channel descriptor described by the donor is
plausible for the existing audio family, but the claimed focused test is absent.

Recommendation: retain the branch, implement PFL semantics and install profile
and actual stream/descriptor regressions, then integrate and qualify the
controller separately. Keep the documented AUTO-pad beat-size differences,
unmapped SAMPLER controls, incomplete LED checks and unknown native/software
level interaction visible. Do not advertise full support from byte parity.

### JC8012: split the new board from inherited repairs and product policy

The branch adds a fourth target with 1280x800 output, a rotated 800x1280 panel,
GSL3680 touch, BSP/entrypoint/configuration, simulator geometry and bring-up
records. It also changes existing JC1060 behavior and shared drivers. The
donor describes a real PPA-related freeze and subsequent instrumented display
progress; this is useful investigation evidence, not a three-target regression
result on the canonical image.

Use separate changesets:

1. Board identity, BSP, entrypoint, build verifier and simulator geometry,
   after resolving the touch-driver and PPA isolation gates above. Candidate
   landmarks are `ef215a1f`, `b3a9bdb9`, `a5ce5a4a`, `00c5a4ea`, `457b445c`,
   `d34abe48`, `c7082c99` and `dde980b2`; inspect/adapt their final combined
   state rather than assuming each intermediate bring-up commit is shippable.
2. The board-owned SD cold-power sequence (`981225e5`, `1e288dbd`) is worth a
   focused follow-up to JC1060's still-open warm-reset problem. It cycles LDO4
   and drives SD pins low before the first mount. Audit rail/pin ownership,
   return/cleanup behavior and interaction with Hosted SDMMC; preserve other
   boards' defaults. The donor reports 11/11 JC1060 mounts, which is not a
   canonical-image PASS. Do not import a global SDMMC reset.
3. JC1060 Wi-Fi (`24b63df9`) is a separate product feature: it changes capability
   from absent to present, enables AP+STA by default, adds Settings behavior
   and changes HTTP lifetime when Ethernet exists. Reconcile this with canonical
   requested-network startup health, Host admission and provider ownership.
   Decide defaults explicitly; it is not required merely to add JC8012.
4. HS FIFO, PPA rotation and any remaining common changes need their own
   narrowly scoped justification against the current shared implementation.

Do not full-merge the 97 commits. Resolve new-board ownership/licensing first,
then qualify its hardware independently. Do not upgrade IDF as a side effect.

### JC1060 donor and APTA: no additional whole-branch integration needed

Keep `kay/jc1060-hw-bench` for contributor provenance and its exact hardware
records. Revisit specific genuinely new findings if its tip changes; at the
frozen tip its useful repairs have already been adapted. There is no reason
to merge its old implementations back into production.

`codex/p4-apta-library` is fully ancestral to both `origin/master` and the
current shared branch. There is nothing left to cherry-pick. Later branch
cleanup can be decided with its worktree owner; this review removes nothing.

## Verification and next integration order

Performed: fetched branch inventory, ancestry and changed-file/commit review,
open-PR check, non-mutating `git merge-tree` checks against both frozen
baselines, eight UI pure-module tests, Numark compiler byte parity and the
separately extracted profile test. No check runs are attached to the five remote
development tips in the queried GitHub check-runs API snapshot. The old APTA
tip has three historical successful checks; those do not qualify new branches.
No CI workflow was triggered. Source excerpts and raw check outputs are retained.
The documentation integrity checker passes (149 Markdown/HTML files, 134 retired
paths); staged whitespace checks pass. All 12 archived evidence hashes also
match the exact staged Git blobs, with directory-scoped CRLF preservation.

NOT RUN for candidate branches: full host qualification, ESP-IDF build matrix,
UI simulator screenshot review, merged-image budgets, signed packaging, OTA,
physical controller/display/audio tests or long timing qualification. Existing
accepted images are described by their own validation records, not re-certified
by this branch review. A clean virtual merge proves textual compatibility only.

Recommended order:

1. Integrate the qualified shared-core branch into `master` through a new PR.
2. If wanted, import the disabled new-UI prototype with its focused tests;
   develop production activation as a separate bounded project.
3. Repair Numark PFL and register real regressions, then integrate its profile.
4. Adapt the small JC1060 SD-power change if the contributor can validate it;
   keep optional JC1060 Wi-Fi policy separate.
5. Integrate the fourth-board BSP only after provenance/isolation gates and
   board-specific validation. Preserve the existing three targets throughout.

For each actual code integration, rerun relevant production host tests first;
build the affected target while iterating and broaden to all three established
targets when shared code/build selection is finalized. Physical PASS belongs
to the exact installed image; explicit waivers and NOT RUN gates remain so.
