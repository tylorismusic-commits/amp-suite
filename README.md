# Amp Suite

**Version 0.2.0 — M1 in progress** · GPL-3.0-or-later

Seven guitar amps in one plugin, with an 808 boost, delay, reverb and a cab
section:

| Tab | Reference |
|---|---|
| Chorus Clean | JC-120-style solid-state clean |
| Chime | AC30-style top boost |
| Crunch | Rockerverb-style EL34 crunch |
| Hot Rod | Dookie-mod 1959-style lead |
| Block 120 | 5150 block-letter-style high gain |
| Stealth EL34 | 5150III-style EL34 high gain |
| Recto | Dual Rectifier-style modern high gain |

The panel follows the Comp Suite layout: amp tabs across the top, a shared amp
control grid, and header pills for **PEDALS**, **CAB**, **HQ** and **LOOP**.
The pedal and cabinet pages replace the body while open; the plugin never
changes size (1040×640, locked aspect, 0.5×–2× resize).

The plugin starts on **Block 120 — Rhythm** with the 808 on, delay and reverb
off, and the stock 1200 4×12 cab.

## Status

This is the M0 scaffold:

- ✅ CLAP and LV2 bundles build and load.
- ✅ 48-parameter host contract; CLAP state save/load.
- ✅ 7 amp tabs, amp page, PEDALS page, CAB page, preset browser.
- ✅ Default preset and 8 scaffold presets.
- ✅ LV2 metadata generated from the parameter table.
- ✅ **M1:** Guitarix tube tables, `TubeStage`, passive tone stacks and
  per-model recipes, level-calibrated across all seven tabs.
- ✅ **M1:** Guitarix Plexi EL34 power-amp table, per-recipe supply sag and a
  clamped feedback loop; presence has a practical post-power shelf until the
  full NFB pass.
- ⏳ M1: oversampling, full NFB presence/resonance.
- ⏳ M1: delay/reverb DSP and the 808 circuit model.
- ✅ **M1:** parametric cab-IR fitter (`tools/ir_fit.py`, `tools/ir_validate.py`).
- ⏳ M1: generate the seven factory IRs from the references, then the user IR
  loader.

The current preamp uses the Guitarix-derived 12AX7/EL34/EL84/6L6 stages and
the Guitarix/CAPS tone-stack networks. EL34 models use the Guitarix Plexi
power-amp table; the others use a symmetric push-pull shaper until dedicated
tables are generated. The cab is a fixed speaker filter until the IR pipeline
lands.
See `docs/recipes.md` for the topology and `docs/params.md` for the host ABI.

## Formats

- **CLAP** — `build/clap/AmpSuite.clap` + `AmpSuite.resources/`.
- **LV2** — `build/lv2/amp-suite.lv2/` (plugin + X11 UI + generated preset
  banks).

The plugin URI is
`https://github.com/tylorismusic-commits/amp-suite`. It is part of the saved
session format: do not rename the repository or GitHub account after 1.0.0.

## Building

Requires CMake 3.16+, a C++17 compiler, and the Cairo, FreeType and X11
development packages. `lilv` is optional but enables the LV2 host test.

```sh
cmake -S . -B build
cmake --build build -j
cd build && ctest --output-on-failure
```

Install for the current user. `LV2_DIR` matters: `~/.local/lib/lv2` is not on
the default LV2 search path.

```sh
cmake -S . -B build -DLV2_DIR="$HOME/.lv2" -DCLAP_DIR="$HOME/.clap"
cmake --build build -j
cmake --install build
```

Or copy the bundles by hand:

```sh
cp -r build/clap/AmpSuite.clap build/clap/AmpSuite.resources ~/.clap/
cp -r build/lv2/amp-suite.lv2 ~/.lv2/
```

Render every tab and page to PNG without a host:

```sh
build/amp_ui_preview assets/fonts /tmp/ampsuite-preview 1.0
```

## Cab IRs and TONE3000

Factory cabs are original CC0 IRs generated from a parametric model fitted to
smoothed reference captures. Reference WAVs are **not** shipped and are
gitignored under `refs/irs/`.

TONE3000 captures are T3K-licensed: free to use, but not redistributable
without the author's permission. Amp Suite does not download, bundle or
redistribute them. The user IR loader will read local WAV files in M1; the Cab
page lists the folder and the manual documents the reference links.

See `docs/ir-provenance.md` for the reference set and generation method.
The fitter is offline-only; its Python dependencies are in
`tools/requirements.txt`. Generated factory IRs are CC0.

## Documentation

- `docs/params.md` — host parameter and state contract
- `docs/recipes.md` — amp topology and first-pass targets
- `docs/presets.md` — factory preset plan
- `docs/ir-provenance.md` — cab reference and CC0 generation policy

## License

GPL-3.0-or-later. The Jost font is OFL. See `THIRD_PARTY_NOTICES.md`.
