# Amp Suite — amp recipes

**Status:** M1 in progress. The Guitarix-derived `TubeStage`, `ToneStack`,
power-amp table, sag/NFB, 2×/4× oversampling, generated factory cabs and user
IRs are implemented. Full grid-blocking and per-model reference tuning remain.
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

- The Guitarix tube tables, `tubestageF/P`, tone stacks and Plexi EL34
  power-amp table are ported under `src/dsp/` with GPL notices.
- The EL34 models (Crunch, Hot Rod, Stealth) use the Guitarix Plexi power-amp
  table. Chime, Block 120 and Recto use a symmetric push-pull shaper until
  dedicated 6L6/EL84 tables are generated.
- Supply sag is a per-recipe envelope (`powerSag`); Recto has the most,
  Block 120 the least. Grid-conduction/blocking is still to come.
- The power stage runs at 4× (HQ) or 2× (Eco) through a half-band FIR pair.
- Presence uses the Guitarix feedback filter inside the power stage plus a
  practical post-power high shelf. Resonance remains a post-power low shelf;
  a fully coupled NFB resonance loop is still to come.
- Each recipe is level-calibrated; `amp_probe` reports peak/RMS/THD.
- The 2×/4× power-stage oversampling is in place; the alias floor still needs a
  formal measurement in the release pass.
- Validate each recipe against the reference captures listed in
  `docs/ir-provenance.md` and the NAM reference set.
