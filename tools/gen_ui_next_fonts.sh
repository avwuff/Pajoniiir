#!/usr/bin/env bash
# Regenerates the LVGL font files of components/ui_next/fonts.
# usage: tools/gen_ui_next_fonts.sh <dir with Roboto-*.ttf (2.138)> <dir with Cousine-*.ttf (1.21)> [lv_font_conv]
# Both families are Apache-2.0. Do not use the newer OFL builds of Roboto or Cousine.
set -euo pipefail
roboto=${1:?Roboto 2.x directory}
cousine=${2:?Cousine 1.21 directory}
conv=${3:-lv_font_conv}
out="$(cd "$(dirname "$0")/.." && pwd)/firmware/p4-core/components/ui_next/fonts"

# ASCII, Latin-1 (French accents), dashes, quotes, bullet, ellipsis, minus.
range="0x20-0x7E,0xA0-0xFF,0x2013,0x2014,0x2018,0x2019,0x201C,0x201D,0x2022,0x2026,0x2212"

gen() { # name ttf size
    "$conv" --font "$2" --size "$3" --format lvgl --bpp 4 --range "$range" \
        --no-compress --no-prefilter --force-fast-kern-format --lv-include lvgl.h \
        -o "$out/$1_$3.c"
    # Keep developer paths out of the generated header comment.
    sed -i -E '/^ \* Opts:/ s# (--font|-o) [^ ]*/([^ /]+)# \1 \2#g' "$out/$1_$3.c"
}

for s in 12 14 16 18 24; do gen ui_next_roboto_reg "$roboto/Roboto-Regular.ttf" "$s"; done
for s in 14 16 18 24 28 30 34; do gen ui_next_roboto_med "$roboto/Roboto-Medium.ttf" "$s"; done
for s in 10 12 14 16 20; do gen ui_next_cousine_reg "$cousine/Cousine-Regular.ttf" "$s"; done
for s in 10 12 14; do gen ui_next_cousine_bold "$cousine/Cousine-Bold.ttf" "$s"; done
