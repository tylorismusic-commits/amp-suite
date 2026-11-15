# Changelog

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
