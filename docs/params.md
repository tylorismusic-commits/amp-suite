# Amp Suite — parameter and state contract

**Status:** pre-1.0 ABI. This file is the host ABI: parameter ids, LV2 port
symbols and state layout are permanent after 1.0.0.

Source of truth: `src/params.hpp`. `tools/gen_ttl.cpp` emits the LV2 metadata
from the same table; `amp_ttl --check` fails if the committed TTL is stale.

## Conventions

- CLAP parameter id = `ParamId` value.
- LV2 control port = `4 + ParamId` (audio ports are 0–3).
- Meter/output ports are appended after the control block:
  - `4 + P_COUNT + 0` — `out_peak_l`
  - `+1` — `out_peak_r`
  - `+2` — `in_peak_l`
  - `+3` — `in_peak_r`
  - `+4` — `out_vu`
  - `+5` — `latency` (samples)
  - `+6` — `time` (LV2 atom sequence input, `time:Position`)
- Stepped parameters use integer values. Toggled parameters use 0/1.
- Values are clamped to `[min, max]`; non-finite host values are ignored.
- Model-specific controls stay at fixed ids and are flagged hidden in the
  host when they do not apply to the selected model.

## Parameters

| ID | Symbol | Name | Range | Default | Flags | Notes |
|---:|---|---|---|---:|---|---|
| 0 | `model` | Model | 0–6 | 4 | stepped | 0 Chorus Clean, 1 Chime, 2 Crunch, 3 Hot Rod, 4 Block 120, 5 Stealth, 6 Recto |
| 1 | `power` | Power | 0–1 | 1 | toggle | |
| 2 | `input` | Input | 0–1 | 0.5 | | ±12 dB trim around 0.5 |
| 3 | `gain` | Gain | 0–1 | 0.55 | | preamp gain |
| 4 | `bass` | Bass | 0–1 | 0.5 | | flat at 0.5 |
| 5 | `mid` | Mid | 0–1 | 0.45 | | flat at 0.5 |
| 6 | `treble` | Treble | 0–1 | 0.55 | | flat at 0.5 |
| 7 | `presence` | Presence | 0–1 | 0.5 | | NFB high shelf |
| 8 | `resonance` | Resonance | 0–1 | 0.5 | | NFB low shelf |
| 9 | `master` | Master | 0–1 | 0.45 | | |
| 10 | `output` | Output | 0–1 | 0.5 | | ±12 dB |
| 11 | `bright` | Bright | 0–1 | 0 | toggle | Chorus Clean |
| 12 | `cut` | Cut | 0–1 | 0.5 | | Chime |
| 13 | `voice` | Voice | 0–2 | 0 | stepped | Hot Rod / Stealth / Recto |
| 14 | `rectifier` | Rectifier | 0–1 | 0 | stepped | Recto: 0 Si, 1 Tube |
| 15 | `crunch` | Crunch | 0–1 | 1 | stepped, toggle | Block 120: 1 Crunch, 0 Lead |
| 16 | `power_scale` | Power Scale | 0–1 | 0 | stepped | Crunch: 0 = 50 W, 1 = 100 W |
| 17 | `cab` | Cabinet | 0–2 | 0 | stepped | 0 default, 1 alternate, 2 user |
| 18 | `cab_mic` | Cab Mic | 0–2 | 0 | stepped | 0 dynamic, 1 ribbon, 2 blend |
| 19 | `cab_level` | Cab Level | 0–1 | 0.6 | | `-24 + 30v` dB |
| 20 | `cab_lowcut` | Cab Low Cut | 0–1 | 0 | | `20 * 15^v` Hz |
| 21 | `cab_highcut` | Cab High Cut | 0–1 | 1 | | `2000 * 10^v` Hz |
| 22 | `cab_bypass` | Cab Bypass | 0–1 | 0 | toggle | |
| 23 | `od_on` | 808 On | 0–1 | 1 | toggle | |
| 24 | `od_drive` | 808 Drive | 0–1 | 0.22 | | |
| 25 | `od_tone` | 808 Tone | 0–1 | 0.5 | | |
| 26 | `od_level` | 808 Level | 0–1 | 0.50 | | |
| 27 | `delay_on` | Delay On | 0–1 | 0 | toggle | |
| 28 | `delay_sync` | Delay Sync | 0–1 | 1 | toggle | 0 Free, 1 Host |
| 29 | `delay_div` | Delay Division | 0–8 | 8 | stepped | 1/1, 1/2, 1/4, 1/8, 1/8T, 1/16, 1/16T, 1/32, 1/8D |
| 30 | `delay_time` | Delay Time | 0–1 | 0.18 | | `20 + 1980v²` ms (free mode) |
| 31 | `delay_feedback` | Delay Feedback | 0–1 | 0.3 | | |
| 32 | `delay_mix` | Delay Mix | 0–1 | 0.25 | | |
| 33 | `delay_tone` | Delay Tone | 0–1 | 0.5 | | |
| 34 | `delay_mod` | Delay Mod | 0–1 | 0.2 | | |
| 35 | `delay_pingpong` | Delay Ping-Pong | 0–1 | 0 | toggle | |
| 36 | `reverb_on` | Reverb On | 0–1 | 0 | toggle | |
| 37 | `reverb_mode` | Reverb Mode | 0–1 | 0 | stepped | 0 Plate, 1 Spring |
| 38 | `reverb_mix` | Reverb Mix | 0–1 | 0.2 | | |
| 39 | `reverb_decay` | Reverb Decay | 0–1 | 0.4 | | |
| 40 | `reverb_tone` | Reverb Tone | 0–1 | 0.5 | | |
| 41 | `reverb_predelay` | Reverb Pre-delay | 0–1 | 0.1 | | 0–200 ms |
| 42 | `hq` | HQ | 0–1 | 1 | toggle | 1 HQ (4×), 0 Eco (2×) |
| 43 | `loop_pos` | Loop Position | 0–1 | 0 | stepped | 0 pre-power, 1 post-cab |
| 44 | `chorus_on` | Chorus On | 0–1 | 1 | toggle | Chorus Clean |
| 45 | `chorus_rate` | Chorus Rate | 0–1 | 0.35 | | Chorus Clean |
| 46 | `chorus_depth` | Chorus Depth | 0–1 | 0.5 | | Chorus Clean |
| 47 | `meter` | Meter | 0–1 | 0 | stepped, hidden | UI-only |
| 48 | `limiter_on` | Limiter | 0–1 | 0 | toggle | output safety limiter |
| 49 | `limiter_threshold` | Limiter Threshold | 0–1 | 1 | | `-24 + 24v` dBFS |
| 50 | `user_ir_chorus` | Chorus Clean User IR | 0–63 | 0 | stepped | 0 none, 1–63 sorted user WAV |
| 51 | `user_ir_chime` | Chime User IR | 0–63 | 0 | stepped | per-amp user IR |
| 52 | `user_ir_crunch` | Crunch User IR | 0–63 | 0 | stepped | per-amp user IR |
| 53 | `user_ir_hot_rod` | Hot Rod User IR | 0–63 | 0 | stepped | per-amp user IR |
| 54 | `user_ir_block_120` | Block 120 User IR | 0–63 | 0 | stepped | per-amp user IR |
| 55 | `user_ir_stealth` | Stealth User IR | 0–63 | 0 | stepped | per-amp user IR |
| 56 | `user_ir_recto` | Recto User IR | 0–63 | 0 | stepped | per-amp user IR |
| 57 | `tempo` | Delay Tempo | 40–300 | 120 | | bpm; manual tempo for LV2 and non-transport hosts |
| 58 | `amp_bypass` | Amp Bypass | 0–1 | 0 | toggle | bypass preamp/tone/power; 808, loop, cab, output and limiter stay active |
| 59 | `nam_on` | NAM On | 0–1 | 0 | toggle | use the selected NAM model instead of the amp |
| 60 | `nam_model` | NAM Model | 0–63 | 0 | stepped | 0 none, 1–63 sorted NAM model file |
| 61 | `nam_level` | NAM Level | 0–1 | 0.5 | | `-12 + 24v` dB trim |

`P_COUNT = 62`.

User IRs are WAV files in `$XDG_DATA_HOME/amp-suite/irs/user` (usually
`~/.local/share/amp-suite/irs/user`). The list is sorted by filename; the
parameter value is the 1-based index into that list. The plugin watches the
directory, so newly dropped files appear without a restart.

NAM models live in `$XDG_DATA_HOME/amp-suite/nam`. `.nam` models and NAM-compatible
`.wav` linear models are supported. The model is loaded on a worker thread and
runs at the host sample rate (resampled around the model when needed). NAM
replaces the preamp/power amp; the 808, FX loop, cab, output and limiter remain
active. The model file itself is local; the CLAP/LV2 state stores the selection
index but does not embed the model.

## Model-specific visibility

| Control | Visible on |
|---|---|
| `bright`, `chorus_*` | Chorus Clean |
| `cut` | Chime |
| `power_scale` | Crunch |
| `presence` | all except Chorus Clean |
| `resonance` | Crunch, Block 120, Stealth, Recto |
| `crunch` | Block 120 |
| `rectifier` | Recto |
| `voice` | Hot Rod, Stealth, Recto |

## State

CLAP state blob (little-endian, native doubles in practice; the header makes
versioning explicit):

```
struct StateHeader { uint32_t version; uint32_t count; };
StateHeader { 1, P_COUNT }
double values[P_COUNT];
double uiScale;
struct UserIrStateHeader { uint32_t version; uint32_t count; };
UserIrStateHeader { 1, n }
repeat n times:
  uint32_t amp;
  uint32_t nameLen;
  char     name[nameLen];
  uint32_t dataLen;
  uint8_t  wav[dataLen];
```

The user IR section is optional; a session that ends after `uiScale` still
loads. Selected user WAVs are embedded so the session survives moving to a
machine without the original files.

LV2 state uses the same parameter values and a portable user-IR blob.
Selected WAVs are also copied into the host state directory through
`state:makePath` when the host provides it. The UI page is not stored.

## Text formatting

`paramsValueToText` uses the label table for stepped controls and maps
continuous values:
- `delay_time` → milliseconds
- `cab_level` → dB
- `cab_lowcut` / `cab_highcut` → Hz
- `reverb_predelay` → milliseconds
- `limiter_threshold` → dBFS
- `user_ir_*` → filename or `None`
- `tempo` → BPM
- `nam_model` → model filename or `None`
- `nam_level` → dB
- toggles → `On` / `Off`

After 1.0, symbols and ids must never change. Append new parameters at the end
of the enum and table.
