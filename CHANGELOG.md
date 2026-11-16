# Changelog

## 0.2.0 — 2026-11-15

M1: tube-stage and tone-stack engine.

### Added

- Vendored Guitarix's tube tables, `valve.h`, `tubestageF/P` and the
  Guitarix/CAPS tone-stack values under `third_party/guitarix`, with notices.
- `TubeStage` C++ port (table lookup, cathode feedback, HPF, anti-alias).
- `ToneStack` C++ port (third-order IIR; Bassman, JTM45, JCM800, AC30, Peavey,
  Mesa, Soldano, Bogner, Roland, MLead).
- First-pass `AmpRecipe` table for all seven amps; Block 120 uses six stages
  and the Peavey stack.
- Level calibration: all models now land around −20 dBFS peak on a −12 dBFS
  sine at the default master, instead of −26…−49 dBFS.
- Per-model tone-stack makeup and a solid-state clean path for Chorus Clean.
- Extreme-setting stress test (max gain/master/EQ/808) and a tone-stack probe.
- Guitarix Plexi EL34 power-amp table, per-recipe supply sag, a clamped
  feedback loop and a practical presence shelf.
- Guitarix TS9/808 nonlinear table and op-amp/clipping network replacing the
  scaffold boost.
- Original stereo tape-style delay: fractional read, feedback low-pass,
  modulation, ping-pong, host/free sync, bypass trails.
- Original four-line FDN reverb: pre-delay, damping, plate/spring voicing,
  bypass trails.
- FX loop routing: pre-power (default) or post-cab.
- Stress test for the loop at maximum feedback/decay/wet.
- Seven generated CC0 factory cabs, fitted from the reference captures and
  embedded in the CLAP/LV2 bundles.
- Direct-FIR cab convolution with 57/ribbon/blend variants and 1 kHz level
  normalization; fixed speaker filter retained as a missing-asset fallback.
- Cab page copy now reflects embedded factory IRs; user IR loading is the next
  pass.
- `amp_probe` and `amp_tone_probe` development tools.
- Parametric cab-IR model/fitter (`tools/ir_model.py`, `tools/ir_fit.py`,
  `tools/ir_validate.py`) with a synthetic self-test and CC0 output policy.

### Notes

- The power amp is a simple push-pull saturation; the Plexi/sag/NFB model is
  the next M1 step.
- The 808 is still the scaffold boost; the TS9/808 circuit model lands with
  the pedal pass.
- Cab IRs are not generated yet; the cab section uses a fixed speaker filter.
- The tube-stage port is table-driven from Guitarix and GPL-compatible.

---

## 0.1.0 — 2026-11-15

Initial M0 scaffold.

### Added

- New `amp-suite` repository, forked from the Comp Suite shell.
- CLAP and LV2 builds with the shared Cairo/X11 editor.
- 1040×640 panel with seven amp tabs and header pills:
  **PEDALS**, **CAB**, **HQ**, **LOOP**.
- Amp page with the shared gain/EQ/master/output grid and model-specific
  switch slots.
- PEDALS page: 808, delay and reverb cards; host/free delay sync; loop pill.
- CAB page: factory/alternate/user selection, mic, level, low/high cut,
  bypass. User IR loading lands in M1.
- 48-parameter host contract; CLAP state save/load; generated LV2 metadata.
- Startup on Block 120 — Rhythm with the 808 on.
- Eight scaffold presets and LV2 preset banks.
- Host smoke tests: lilv LV2 load/process and CLAP load/params/state/process.
- UI preview renderer.

### Notes

- The DSP engine is a scaffold voicing, not the final amp recipes. M1 replaces
  it with the Guitarix-derived stage/tone-stack/power-amp blocks.
- Factory IRs are not generated yet; the cab section uses a fixed speaker
  filter until the IR pipeline lands.
- TONE3000 reference captures are analysis targets only and are not shipped.
