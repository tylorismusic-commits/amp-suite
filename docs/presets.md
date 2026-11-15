# Amp Suite — presets

**Status:** M0 scaffold. The eight presets below are compiled into
`src/presets.hpp` and emitted into `lv2/presets.ttl` by `tools/gen_ttl.cpp`.

## Startup

- Model: **Block 120**
- Preset: **Block 120 — Rhythm**
- 808: **on** (Drive 0.28, Tone 0.5, Level 0.55)
- Delay/reverb: off
- Cab: 1200 4×12

## Scaffold bank

| Preset | Category | Model | Intent |
|---|---|---|---|
| Block 120 — Rhythm | Rhythm | Block 120 | boosted 5150 rhythm, stock 1200 cab |
| Block 120 — OS | Lead | Block 120 | same amp, late-90s OS V30 cab |
| Chorus Clean | Clean | Chorus Clean | solid-state clean with chorus |
| Chime | Crunch | Chime | AC30 top boost edge |
| Crunch | Crunch | Crunch | Rockerverb dirty channel |
| Hot Rod | Crunch | Hot Rod | modded British lead |
| Stealth Lead | Lead | Stealth EL34 | modern tight high gain |
| Recto Modern | Lead | Recto | scooped modern Recto |

## Target 1.0 bank

- Four per amp: Clean / Crunch / Rhythm / Lead (or model equivalents).
- Six rig presets combining amp, pedals and cab, including:
  - "Track Clean" — Chorus Clean, chorus on, plate reverb
  - "Track Crunch" — Chime, 808 off, spring reverb
  - "Track Rhythm" — Block 120, 808 on, stock cab
  - "Track Lead" — Block 120 OS or Stealth, delay + plate
  - "Track Doom" — Crunch, fuzz-adjacent gain, dark cab
  - "Track Ambient" — Chorus Clean, long delay + plate
- Bank categories: Clean, Crunch, Rhythm, Lead, Doom, Ambient.
- Factory presets use factory cabs only. User presets may reference user IRs;
  if the IR is missing, the amp falls back to its factory cab and the Cab page
  reports the missing file.
- Presets store `delay_sync`, `delay_div` and `delay_time` so host-synced
  presets follow the project tempo and free-time presets keep their ms value.
