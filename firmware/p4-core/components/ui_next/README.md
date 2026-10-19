# New UI development prototype

This directory is a disabled prototype imported from `kay/new-ui` at
`b8dccf260db4c919cae209091a70ef18f89f18cc`. It intentionally has no ESP-IDF
component `CMakeLists.txt`, production startup hook or default build selection.
JC4880, JC1060 and M3 continue to use the existing accepted UI and board policies.

The screens render demo data. They do not provide production deck/library
bindings or touch actions. Visible recorder, four-deck or other demo controls
do not establish support for those features. Before activation, reconcile
`docs/reference/new-ui/CORE_GAPS.md` with the current shared core, use its
generation/ownership and semantic-action contracts, implement touch/wake safety,
and qualify resource use and rendering on each affected board.

Eight pure-module tests are registered in the shared host runner. The headless
simulator under `tests/ui_next_sim` renders examples; it does not replace the
accepted UI screenshot or physical panel/timing gates. Keep the existing
baseline images until a separate visual review authorizes new ones.

Font origins and licence notices are retained under `fonts/`.
