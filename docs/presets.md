# Amp Suite — presets

**Status:** factory bank. The presets below are compiled into
`src/presets.hpp` and emitted into `lv2/presets.ttl` by `tools/gen_ttl.cpp`.

## Startup

- Model: **Block 120**
- Preset: **Block 120 — Rhythm**
- 808: **on** (Drive 0.28, Tone 0.5, Level 0.55)
- Delay/reverb: off
- Cab: **1200 4×12**, dynamic mic, cab level 0.60

## Factory bank

| # | Preset | Category | Model | Intent |
|---:|---|---|---|---|
| 1 | Block 120 — Rhythm | Rhythm | Block 120 | boosted 5150 rhythm, stock 1200 cab |
| 2 | Block 120 — Crunch | Rhythm | Block 120 | hotter rhythm, more upper-mid bite |
| 3 | Block 120 — Lead | Lead | Block 120 | lead mode, dotted-8th delay + plate |
| 4 | Block 120 — OS | Lead | Block 120 | same amp, late-90s OS V30 cab |
| 5 | Chorus Clean — Clean | Clean | Chorus Clean | solid-state clean, chorus on |
| 6 | Chorus Clean — Chorus | Clean | Chorus Clean | bright, deeper chorus |
| 7 | Chorus Clean — Ambient | Ambient | Chorus Clean | slow chorus, quarter-note delay, plate |
| 8 | Chime — Edge | Crunch | Chime | AC30 top-boost edge |
| 9 | Chime — Top Boost | Crunch | Chime | hotter top boost, 808 light |
| 10 | Crunch — Dirty | Crunch | Crunch | Rockerverb dirty channel, 100 W |
| 11 | Crunch — Doom | Doom | Crunch | dark, thick, 50 W, plate |
| 12 | Hot Rod — Rhythm | Rhythm | Hot Rod | modded British rhythm, 808 light |
| 13 | Hot Rod — Lead | Lead | Hot Rod | lead voice, dotted-8th delay |
| 14 | Stealth — Rhythm | Rhythm | Stealth EL34 | tight modern rhythm |
| 15 | Stealth — Lead | Lead | Stealth EL34 | lead voice, delay + plate |
| 16 | Recto — Modern | Lead | Recto | scooped modern Recto, silicon |
| 17 | Recto — Raw | Crunch | Recto | lower-gain raw mode, voice B |
| 18 | Recto — Vintage | Lead | Recto | tube rectifier, voice C, delay + plate |
| 19 | Track Clean | Ambient | Chorus Clean | tracking clean with plate |
| 20 | Track Crunch | Rhythm | Chime | tracking crunch with spring |
| 21 | Track Rhythm | Rhythm | Block 120 | tracking rhythm, blend mic |
| 22 | Track Lead | Lead | Block 120 | tracking lead, OS cab, delay + plate |

The preset browser shows ten rows at a time and scrolls with the mouse wheel.
Categories are shown on the right of each row. The 0.2.0 cab pass added two
more:

| Preset | Category | Model | Intent |
|---|---|---|---|
| Block 120 — Chug | Rhythm | Block 120 | Kazrog-inspired scooped, resonant rhythm |
| Hot Rod — Green | Crunch | Hot Rod | Greenback alternate cab |

## Applying presets

A preset sets the parameters listed in `src/presets.hpp`; unlisted parameters
are left at their current values. The model and all listed controls are
updated together. Presets store host-sync divisions and free-time values, so a
host-synced lead preset follows the project tempo.

User IR selections are not part of the factory presets. If a preset uses a
factory cab and the current amp has a user IR selected, applying the preset
sets `cab` back to 0 or 1 as listed.

## Target 1.0 bank

The 22-preset bank above is the 1.0 candidate. Additional per-amp clean/crunch
variants can be appended later without changing the parameter ABI.
