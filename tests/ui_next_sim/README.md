# ui_next simulator

Headless renderer of the new DJ UI (`firmware/p4-core/components/ui_next`).
It draws the pages built so far (Perform, Boot, Browse, Search, Alerts over Browse, Track info, Source, Browse edit, FX) on demo data at the six
board sizes (night, and day for 800x480 and 480x800) and writes one PPM per frame. No window, no hardware,
no core code. It does not replace the P4 panel checks (timing, touch, distance).

```bash
cmake -S tests/ui_next_sim -B build/ui_next_sim -G Ninja -DCMAKE_BUILD_TYPE=Release \
      -DLVGL_DIR=<LVGL 9.5 checkout>     # firmware/main-deck-p4/managed_components/lvgl__lvgl works
cmake --build build/ui_next_sim
build/ui_next_sim/ui_next_sim <out dir> [tick] [perform|boot|browse|search|alerts|trackinfo|source|browseedit|fx]
```

`tick` moves the demo (60 ms per tick, default 100). The captures are for visual
review against `docs/reference/new-ui`. They carry no hash gate yet.
