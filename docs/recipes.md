# Amp Suite — amp recipes

**Status:** M0 scaffold. The engine currently uses a broad voicing scaffold;
M1 replaces it with the Guitarix-derived stage/tone-stack/power-amp blocks.
This file is the recipe spec the implementation must satisfy.

## Shared architecture

```
input trim → 808 → preamp stages → tone stack → FX loop
  → phase inverter → power amp (sag, OT, NFB) → speaker load → cab IR → output
```

Each recipe controls:

- preamp stage count, tube table, plate/cathode values, coupling, cold clipper
- tone-stack topology and component values
- phase inverter type
- power tube, count, bias, supply, rectifier, NFB
- sag and blocking behaviour
- factory cab and alternate
- model-specific switches

## The seven recipes

| Tab | Reference | Preamp | Tone stack | Power | Default cab | Switches |
|---|---|---|---|---|---|---|
| **Chorus Clean** | JC-120 | solid-state op-amp clean | active 3-band | SS, no sag | Chorus 2×12 | Bright, Chorus on/rate/depth |
| **Chime** | AC30 Top Boost | 12AX7 Top Boost, 2–3 stages | `ac30` | 4×EL84, cathode bias, no global NFB | Alnico 2×12 | Cut |
| **Crunch** | Rockerverb 50/100 | 12AX7 cascade + Orange preamp table | `jcm800`/`bogner` tuned | 4×6V6 (50) / 4×EL34 (100) | V30 4×12 | 50/100 |
| **Hot Rod** | Dookie-mod 1959 | 1959 cascade + pre-PI master | `jtm45`/`bassman` | 4×EL34 Plexi table | G12T-75 4×12 | Voice |
| **Block 120** | Peavey 5150 block letter | 6×12AX7, cold clipper, cathode follower | `peavey` | 4×6L6, stiff supply | 1200 4×12 (alt OS V30) | Crunch/Lead |
| **Stealth EL34** | EVH 5150III 50S Stealth | 5150-family 6-stage, tighter filtering | `peavey`/`soldano` | 2×EL34 | G12-EVH 4×12 | Voice |
| **Recto** | Dual Rectifier | 12AX7 cascade, pre-gain stack | `mesa`/`mlead` | 4×6L6, Si/Tube rectifier | OS V30 4×12 | Rectifier, Voice |

## First-pass targets

- **Chorus Clean:** high headroom, bright 3-band, BBD chorus antiphase L/R,
  stereo 2×12; no power-amp breakup. Default chorus on for the Clean preset.
- **Chime:** early breakup, chimey upper mids, no global NFB, Cut from ~1–10 kHz;
  EL84 cathode-bias compression.
- **Crunch:** thick lower mids, saturated but loose low end; 6V6 50 W is the
  base, EL34 100 W the alternate.
- **Hot Rod:** darker/looser than a JCM800, fuzzy upper mids, cascaded stages and
  pre-PI master; 1959 tone stack values.
- **Block 120:** tight lows, aggressive upper mids, cold-clipper asymmetry,
  resonance/presence in the NFB loop; stock 1200 cab.
- **Stealth EL34:** tighter and brighter than Block 120, more filtering, EL34
  power amp; G12-EVH cab.
- **Recto:** huge lows, scooped mids, pre-gain tone stack, Modern mode; tube
  rectifier adds sag, silicon is tighter; OS V30 late-90s cab.

## Model switches and defaults

| Param | Default | Notes |
|---|---|---|
| `model` | 4 (Block 120) | startup tab |
| `bright` | 0 | Chorus Clean |
| `cut` | 0.5 | Chime |
| `voice` | 0 | A/B/C, model-specific |
| `rectifier` | 0 (Si) | Recto |
| `crunch` | 1 (Crunch) | Block 120 |
| `power_scale` | 0 (50 W) | Crunch |

## M1 implementation notes

- Port Guitarix `tubestageF/P`, `tonestack`, `gxpoweramp` and `trany.h` to C++
  under `src/dsp/`, preserving GPL notices.
- Remove the aggressive SR/2.1 anti-alias low-pass once HIIR 2×/4× is in place;
  measure the alias floor before/after.
- Add the supply-sag and grid-conduction module; tune per recipe.
- Presence/resonance must act in the NFB loop, not as post-EQ.
- Calibrate each recipe's output so model switching does not jump level.
- Validate each recipe against the reference captures listed in
  `docs/ir-provenance.md` and the NAM reference set.
