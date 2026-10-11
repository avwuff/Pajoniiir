# DJ UI presentation provenance

Imported `dj_ui.c` and `include/dj_ui.h` from kayrozen/Pajoniiir commit
`428b97dd4a175f03d3a172c8db9c4d5ed94195fb`, firmware v323.
The donor repository is MIT licensed. The repository LICENSE applies; original
donor comments are retained. Local modifications adapt native board geometry,
LVGL 9.5, feature visibility and input callbacks to the existing P4 state owner.
The donor firmware engine, direct PPA bridge and network state are not imported.

## Musieer font assets

`Musieer_48.c` and `Musieer_80.c` are generated LVGL assets from
`Musieer-L7nD.ttf`, inherited from the M3 splash/screensaver implementation.
Their use is not limited to the experimental DJ Overview. The generated
headers retain the conversion options but omit a developer's absolute path.

These font assets are separate from the MIT-licensed DJ UI code. An original
font licence or permission record has not been retained in this repository;
the repository MIT licence must not be presented as permission for the font.
Public catalogue listings attribute Musieer to 538Fonts, but disagree about
commercial-use permission: [Font Zillion](https://www.fontzillion.com/fonts/538fonts/musieer)
lists commercial use but has no EULA; [Befonts](https://befonts.com/musieer-font.html)
lists personal use only (checked 2026-10-11). These are catalogue statements,
not an original permission record. Font permission and any required branding review
remain an unresolved provenance item; this review does not assert that either
has been completed. No font pixels or established splash layout were changed.
