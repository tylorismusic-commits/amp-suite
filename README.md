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

M1 is in progress:

- ✅ CLAP and LV2 bundles build and load.
- ✅ 62-parameter host contract; CLAP state save/load.
- ✅ 7 amp tabs, amp page, PEDALS page, CAB page, preset browser.
- ✅ Default preset and a 24-preset factory bank with a scrolling browser.
- ✅ LV2 metadata generated from the parameter table.
- ✅ **M1:** Guitarix tube tables, `TubeStage`, passive tone stacks and
  per-model recipes, level-calibrated across all seven tabs.
- ✅ **M1:** Guitarix Plexi EL34 power-amp table, per-recipe supply sag and a
  clamped feedback loop; presence has a practical post-power shelf until the
  full NFB pass.
- ✅ **M1:** 2×/4× half-band oversampling around the power stage (HQ = 4×,
  Eco = 2×); the power-amp feedback and sag now keep state across blocks.
- ⏳ M1: full NFB/blocking and per-model reference tuning.
- ✅ **M1:** Guitarix TS9/808 circuit model; stereo tape-style delay; FDN
  reverb; pre-power/post-cab loop switch; free/host delay sync.
- ✅ **M1:** eight generated CC0 factory cabs (Hot Rod has a Greenback
  alternate), convolved in the engine (57 / ribbon / blend variants, 48/96 kHz).
- ✅ Comp Suite-style preset arrows, centered footer, LED output column,
  metering ticks, inset cab window and output limiter.
- ✅ Per-amp user IR loading from `$XDG_DATA_HOME/amp-suite/irs/user`, with a
  watched folder, filename selection, drag-and-drop, a built-in file browser,
  and CLAP state embedding; LV2 state copies selected WAVs into the host
  session via `state:makePath`. Factory/Alt clear the active user IR selection.
- ✅ Manual delay tempo (`tempo` control) for LV2 and non-transport hosts;
  CLAP transport and the LV2 `time:Position` atom port override it when the
  host supplies a timeline.
- ✅ Amp-section bypass: keep the 808, FX loop, cab IR and output/limiter
  active while skipping the preamp, tone stack and power amp.
- ✅ NAM loader: `.nam` and NAM-compatible WAV linear models in
  `~/.local/share/amp-suite/nam`, with a dedicated NAM page, file browser,
  drag-and-drop, worker-thread loading and host-rate resampling.
- ✅ **M1:** user IR loading and portable user IR state.

The current preamp uses the Guitarix-derived 12AX7/EL34/EL84/6L6 stages and
the Guitarix/CAPS tone-stack networks. EL34 models use the Guitarix Plexi
power-amp table; the others use a symmetric push-pull shaper until dedicated
tables are generated. The 808 is the Guitarix TS9 nonlinear model. Delay and
reverb are original implementations. The cab section loads the generated CC0
factory IRs from the bundle and convolves them; the fixed speaker filter is
only a fallback for builds without the assets.
See `docs/recipes.md` for the topology and `docs/params.md` for the host ABI.

## Formats

- **CLAP** — `build/clap/AmpSuite.clap` + `AmpSuite.resources/`.
- **LV2** — `build/lv2/amp-suite.lv2/` (plugin + X11 UI + generated preset
  banks).

The plugin URI is
`https://github.com/tylorismusic-commits/amp-suite`. It is part of the saved
session format: do not rename the repository or GitHub account after 1.0.0.

## Building

Requires CMake 3.16+, a C++17 compiler, and the Cairo, FreeType, X11,
libsamplerate, LV2 and lilv development packages. The NAM core needs a C++20
compiler.

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

Factory cabs are original CC0 IRs synthesized from grouped, level-normalized
reference magnitudes. The current set uses 1/12-octave minimum-phase synthesis
with 57 / ribbon / blend groups, not a copy of any reference capture. Reference
WAVs are **not** shipped and are gitignored under `refs/irs/`. Local Kazrog
AmpCraft IR sets are used only as unshipped analysis references for the Hot Rod,
Crunch, Recto and Hot Rod Greenback cabs.

TONE3000 captures are T3K-licensed: free to use, but not redistributable
without the author's permission. Amp Suite does not download, bundle or
redistribute them. The user IR loader reads local WAV files from
`$XDG_DATA_HOME/amp-suite/irs/user`; the Cab page lists the folder. Selected
user IRs are embedded in CLAP sessions and copied into LV2 state.

See `docs/ir-provenance.md` for the reference set and generation method.
The synthesizer and validator are offline-only; their Python dependencies are
in `tools/requirements.txt`. Generated factory IRs are CC0.

## Documentation

- `docs/params.md` — host parameter and state contract
- `docs/recipes.md` — amp topology and first-pass targets
- `docs/presets.md` — factory preset plan
- `docs/nam.md` — NAM model loader, folders and sample-rate handling
- `docs/ir-provenance.md` — cab reference and CC0 generation policy

## License

GPL-3.0-or-later. The Jost font is OFL. See `THIRD_PARTY_NOTICES.md`.
