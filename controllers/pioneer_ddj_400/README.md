# Pioneer DDJ-400 — software profile

Adapted from collaborator [kayrozen](https://github.com/kayrozen)'s
[Pajoniiir fork](https://github.com/kayrozen/Pajoniiir), commit
`428b97dd4a175f03d3a172c8db9c4d5ed94195fb`,
`controllers/pioneer_ddj_400/profile.json`. Recompiled using our S3CP v4 ABI;
the donor's extended v2 binary is deliberately not imported.

USB identity and MIDI addresses come from the donor. Its hardware reports
are provenance, not local qualification. **Hardware acceptance: NOT RUN.**
UAC format/routing/pacing software support is integrated by package F;
real DDJ-400 MIDI/LED/audio/reconnect acceptance is still NOT RUN.

Beat FX target notes 94/10, 94/11 and 94/14 emit CH1, CH2 and BOTH on
press only. The DDJ-400 MASTER switch maps to our existing both-deck FX,
not a post-fader master bus. Channel filter bypasses the Smart CFX enable
gate using a profile capability; no controller-name checks are used.
Initial SysEx is emitted once per activation before LED feedback; VU scale
150 is applied using saturated integer arithmetic.

CUE/LOOP CALL notes 90/51, 90/53, 91/51 and 91/53 map to the corresponding
deck's loop halve/double. Shared loop admission refuses ranges beyond the
known track length. Four memory-store/delete inputs still lack semantic
actions and remain in `omitted_inputs`; source memory cues/loops are available
through the library/UI implementation.

The v4 binary has 284 inputs, 127 outputs and 6,112 bytes. Its compile identity,
production parser/storage validator and the four press/release mappings pass
host tests. Kayrosen's separate JC1060/DDJ-400 branch has scoped hardware
evidence recorded in the [contributor review](../../docs/reviews/2026-10-14-jc1060-contributor-review.md).
Those results do not qualify the newly integrated canonical firmware; its
physical MIDI/LED/audio/reconnect acceptance remains NOT RUN.

Regenerate with:
`python tools/controller_profile/compile_profile.py controllers/pioneer_ddj_400/profile.json -o controllers/pioneer_ddj_400/profile.s3bin`.
