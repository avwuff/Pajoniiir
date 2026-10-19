# JC1060 / DDJ-400 signed contributor kit

Prepared 2026-10-15 at the operator's request for the contributor handoff only.
This is a local unpublished candidate, not a new GitHub beta, public OTA update,
installation or physical acceptance. No message has been sent to Kayrosen.

## Frozen firmware identity

The kit reuses the already verified clean image from
`c0ee81527b4dbd5d83599140aaa106ebcd27772a`. The
[three-target software record](reviews/2026-10-14-jc1060-integration-verification.md)
and [selective integration scope](reviews/2026-10-14-jc1060-selective-integration.md)
remain its software authority. Preparing the kit did not rebuild the firmware
or change defaults. Packaging ran the unmodified tool from an isolated clean
checkout of that exact commit, against the existing verified build metadata;
the packaging tool's clean/source/board/signature checks remained enabled.
That temporary checkout was removed after the kit and source recovery passed.

| Field | Value |
| --- | --- |
| Board / project | `jc1060` / `main-deck-jc1060` |
| Version | `M2.5-226-gc0ee8152` |
| Build | ESP-IDF 6.0.2, ordinary, `source_dirty=false` |
| Application | 2,668,576 B |
| Signed `.ddjota` | 2,668,764 B |
| Signing | `rel-001`, ECDSA P-256 / SHA-256 |
| Display / USB | 1024x600; controller USB0/root0, storage USB1/root1 |
| MAIN / cue / network | Controller USB audio; Ethernet HTTP; no Wi-Fi provider |
| Hardware, OTA install, rollback | NOT RUN on this candidate |

Application SHA-256:
`b116e043f7a7c73f80e1f5dcc70a36897cc25fc21df52310d145d6b2b9652d44`

Matching ELF SHA-256:
`18e390fdf44224be086616b11cf94b7dc1746bb5c76ead38baca67236281b2c9`

Signed bundle SHA-256:
`7e6241046a937186b5f04b06ba4a73899f150516e87f24762319d5b3f50a788c`

Trusted public DER SHA-256:
`125ef6eb776729e3018ecc564317254febec4a9a2bf34892e99d57f10bc02e43`

## Local delivery

Output directory (ignored build/delivery artifacts, not hosted):
`releases/jc1060-contributor-c0ee8152-20261015/`.

The forwardable archive is `jc1060-candidate-c0ee8152-kit.zip`, 15,790,197 B.
ZIP SHA-256:
`82f7e4cc74386b7a50e9dfe70ee78d463b0999fe29439888e59ad9c2e01c7d02`.
The adjacent `SHA256SUMS.txt` and detached `SHA256SUMS.sig` authenticate the ZIP.
The archive itself contains an independently signed inventory: 40 hashed files
plus the inventory and its signature, 42 files total. Verify using a trusted
public key or compare the bundled public-key fingerprint through a trusted
channel; the presence of a bundled key alone does not establish trust.

The archive includes:

- signed OTA bundle, signed manifest and raw application BIN;
- exact ELF/map, source identity, configuration and dependency lock;
- DDJ-400 and FLX4 compiled profiles and source JSON, with profile provenance;
- original bootloader/partition/otadata/application files and full-build flash
  metadata under `recovery/full-build/`, explicitly separated from normal OTA;
- `README.md`, `RECOVERY.md`, short combined `TEST-CHECKLIST.md` and an editable
  copy-out `REPORT-TEMPLATE.md`;
- offline `Verify-Kit.ps1`, existing OTA signing verifier and public DER only;
- `MESSAGE-FOR-KAYROSEN.md`, a handoff draft that has not been sent;
- source Git bundle and source-recovery instructions, plus software evidence.

The 95,791-B incremental Git bundle requires canonical base
`8d56f303d13c06daa3d194ae1211785a8cdc33e3`, then provides HEAD at the exact
candidate source. Restoring from a repository containing only that base was
tested; it does not require the candidate commit to be public. Module-specific
source licenses remain in the recovered tree. The source bundle is not a
complete standalone clone. Compiling at another path/tool installation need
not reproduce the supplied ELF bytes; qualify a rebuild under its own hashes.

The manifest's `development_candidate=false` denotes the ordinary packaging
class; it does not establish production approval for this contributor kit.
No private signing-key file is supplied. The offline artifact scan also checks
for actual private PEM payloads and the signing key's DER representation; PEM
parser marker strings in the compiled TLS library are not private-key payloads.

## Offline verification

The original [delivery record](reviews/evidence/2026-10-15-jc1060-kit/delivery-record.json)
freezes all artifact identities. The
[offline result](reviews/evidence/2026-10-15-jc1060-kit/offline-verification.json)
records 11 passing cases:

1. ZIP CRC/path uniqueness/extraction, complete signed inventory, manifest,
   bundle, source/version and exact matching ELF.
2. Modified `.ddjota` rejected by inventory verification.
3. Modified manifest signature rejected.
4. Modified inventory rejected by its signature.
5. Unlisted extra file rejected.
6. Missing controller profile rejected.
7. Wrong external public verification key rejected.
8. Source recovery from the exact prerequisite plus incremental bundle.
9. No private-key file, PEM payload or signing-key DER in the unpacked kit.
10. Production bundle verifier rejects a modified signed header independently
    of the inventory.
11. Production bundle verifier rejects modified image bytes independently of
    the inventory; the original signed payload equals the raw application.

The existing signing suite also passes all 10 tests, and OTA release helper
tests pass. Outer ZIP-checksum signing and verification pass. These operations
made no network calls to a device and exercised no flash/OTA/USB actions.
No new three-board firmware builds were needed for this packaging-only step.

Local verification:

```powershell
# From the unpacked kit, with ESP-IDF Python/cryptography available:
.\Verify-Kit.ps1
# Prefer an independently trusted DER when available:
.\Verify-Kit.ps1 -PublicKey <TRUSTED_PUBLIC_DER>
```

## Contributor next action and recovery

The kit groups boot/routing, one short mixed-rate active-control/audio block,
USB MAIN reconnect on both decks without LOAD, three SD warm resets and one
cold boot. Ethernet startup/controlled pending-image failure and rollback are
a distinct physical gate after recovery access is confirmed. Return measured
and operator observations, timed counters and explicit NOT RUN items. Missing
telemetry is not zero; isolated output-late events are recorded separately.

Read the enclosed recovery document before installation. The supplied full-build
files are candidate artifacts, not a backup of the contributor's device or a
known bootable previous image. The measured layout/selector/security state must
match before any wired write. Preserve NVS and crash/slot evidence. No full-chip
erase, automatic flasher or blind otadata/partition rewrite is supplied.

SD warm reset remains unresolved. Actual hub behavior, DDJ-400 audio/MIDI/LED,
loaded-track USB MAIN recovery, network readiness and OTA rollback remain NOT RUN
on this image. Live Link peer-loss/download/artwork and long timing qualification
remain separate. S2 closeout and the JC4880 soak waiver remain unchanged.

Published release assets, public OTA channels and the stable release are unchanged.
The kit is ready for manual handoff; any later sending/publication or installation
must be recorded separately from this preparation.
