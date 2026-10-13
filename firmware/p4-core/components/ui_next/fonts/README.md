# ui_next fonts

| Family | Version | Source | Licence |
| --- | --- | --- | --- |
| Roboto Regular, Medium | 2.138 | `googlefonts/roboto` release `v2.138` (`roboto-unhinted.zip`) | Apache-2.0 |
| Cousine Regular, Bold | 1.21 | `google/fonts`, `apache/cousine` at commit `46d8a043641ec6f446cddf39749c0b8d0dc71467` | Apache-2.0 |

Do not replace them with the newer OFL builds. The licence plan of the UI
(`docs/reference/new-ui`) uses Apache-2.0 only. Cousine in `google/fonts` moved to
OFL 1.240 in a later commit, so pin the commit above.

The licence files are in `licenses/`. Keep them with the fonts (FW-8, ST-11).

## Content

4 bpp, no compression. Range: ASCII, Latin-1, en and em dash, curly quotes,
bullet, ellipsis, minus sign (U+2212). Icons are not in the fonts: they are drawn
by the UI.

Sizes: Roboto Regular 12 14 16 18 24, Roboto Medium 12 14 16 18 20 22 24 28 30 34, Cousine Regular
10 12 14 16 20, Cousine Bold 10 12 14. Roboto Medium stands in for the weight
500 and 600 of the demo boards.

## Regenerate

```bash
npm install lv_font_conv@1.5.3   # in a scratch directory
tools/gen_ui_next_fonts.sh <Roboto 2.138 dir> <Cousine 1.21 dir> <path to lv_font_conv>
```

## Open items

- Confirm the licence text of each font before the first release (backlog open question).
- Check the flash size of the 23 fonts on the P4 image before the fonts join a build.
