# Frozen branch-review evidence

Collected 2026-10-19 in the canonical shared-core worktree. Review base:
`584dc0d2b43e8a9deb847b83191a87f40f5a4d0b`; default remote tip:
`8d56f303d13c06daa3d194ae1211785a8cdc33e3`.
See the [review and integration recommendations](../../../reviews/2026-10-19-remaining-branches-merge-review.md).

| Artifact | Meaning |
| --- | --- |
| `inventory.json` | Frozen SHAs, both baseline ancestry/commit counts, patch-equivalence counts, changed-file counts and virtual merge conflict lists. Counts are `[baseline-only, branch-only]`. |
| `refs.txt`, `github-branches.jsonl` | Git refs and independent GitHub branch inventory after fetch/prune. |
| `open-prs.json` | Empty open-PR snapshot. Historical merged PR46 is not a new PR for the current branch. |
| `bounded-checks.json` | Eight pure UI modules compile/run successfully; Numark compiler recreates the 1,999-byte fixture byte-for-byte. |
| `numark-extracted-test.json` | Embedded-patch profile test extracted into an ignored overlay, compiled and run against current production parser/manager. This does not mean the branch installs the test in its runners. |
| `github-check-runs.json` | No attached check runs on the five queried remote development tips; three historical successful checks on the local APTA tip. This is a check-runs API snapshot, not a general CI certification. |
| `source-proofs.json` | Exact Git-blob SHA-256 and line-numbered source excerpts supporting the reported blockers. |
| `idf-component-selection.json` | Installed ESP-IDF 6.0.2 component discovery/override source excerpts with hashes. Static build-selection evidence; no merged configure/build executed. |
| `inventory.py`, `run_bounded_checks.py`, `final_checks.py` | Review commands and focused-check orchestration. Use the frozen inventory when reproducing; live branch refs can move. These export blobs into ignored cache and do not apply a branch patch to production source. |
| `SHA256SUMS.txt` | SHA-256 for the data and orchestration files above. |

The directory-local `.gitattributes` preserves evidence bytes across Git line
ending settings; hashes are verified against the staged Git blobs as well as
the working files.

The extracted Numark test exercises compiled mappings, parser/manager validation,
replay and LEDs, not the production PFL actor or the missing claimed USB stream
fixture. The UI tests exercise pure calculations, not live touch/core binding,
panel timing or a simulator screenshot acceptance gate. No firmware builds,
full host suite, hardware tests, CI runs, merges, branch deletion, OTA or
publication were performed during this review.

Original exports, complete branch logs/diffs and virtual merge output remain
in ignored `.cache/branch-review-20261019/`. The committed inventory retains
the decision-relevant results without copying generated UI fonts or the
unlicensed vendor driver into the canonical tree.
