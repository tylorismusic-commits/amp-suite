# Changelog

## 0.2.0-alpha.4 — 2026-11-18

Fixed

- The LV2 bundle did not include the Jost font, so the editor fell back to a
  generic system font. The font and its OFL licence are now bundled and
  installed with the LV2 plugin, matching the CLAP build.

## 0.2.0-alpha.3 — 2026-11-18

- Added a chromatic input tuner page (TUNE pill) with note, octave, cents and
  frequency display.

## 0.2.0-alpha.2 — 2026-11-17

- Hide NAM's internal symbols so Amp Suite can coexist with Bass Suite (or
  another NAM-based suite) in one host session.
- Retune the 808 to the TS-9 reference tone range (400–1000 Hz), centre its
  level near unity and start it bypassed.
- Move the 808 footswitch down to align with the delay and reverb switches.
- Clicking the already-selected amp while another page is open returns to the
  amp page; clicking a different amp keeps the current page open.

## 0.2.0-alpha.1 — 2026-11-17

Follow-up alpha with crossfade/state fixes and the README screenshot.

- Only reset the NAM DSP on an engine that is actually using it; the idle
  crossfade engine no longer wipes the active NAM state.
- NAM cab/model/IR changes use a short fade-out, block-boundary apply and
  fade-in because two engines cannot process one NAM model concurrently.
- Asynchronous NAM model loads trigger the same mute.
- The processed path fades back in after a true-bypass power-on.
- Docs cleanup and the Amp Suite amp-page screenshot.

## 0.2.0-alpha — 2026-11-17

First public alpha release for Linux x86_64 (CLAP + LV2).

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
- Output safety limiter (threshold −24…0 dBFS) with a Comp Suite-style
  pill and threshold knob in the right column.
- Comp Suite-style panel pass: preset arrows and centered preset label,
  centered footer, output LED column, dB ticks and labels on the side meters,
  inset cab-window header strip, and numeric knob scales.
- Per-amp user IR library: a watched `$XDG_DATA_HOME/amp-suite/irs/user`
  folder, sorted filename list on the Cab page, seven stepped selection
  parameters and direct convolution of the chosen user IR.
- Cab-page drag-and-drop and a built-in file browser for importing WAVs;
  Factory/Alt clears the active user IR selection. Mic/cab labels moved clear
  of the mic buttons.
- Portable user IR state: CLAP embeds the selected WAV bytes after the
  parameter block; LV2 stores a portable blob and copies the files into the
  host state directory via `state:makePath`.
- 2×/4× half-band FIR oversampling around the power stage: HQ uses 4×, Eco
  uses 2×. The power-amp feedback and supply sag now keep state across process
  blocks instead of resetting every block.
- Click-free model switching: a 10 ms fade out, recipe/cab swap at silence and
  a 10 ms fade in.
- Manual `tempo` control (40–300 BPM) for LV2 and hosts without transport;
  CLAP transport and an LV2 `time:Position` atom-sequence input port override
  it when the host supplies a timeline.
- 24-preset factory bank across Clean, Crunch, Rhythm, Lead, Doom and Ambient,
  with a ten-row scrolling preset browser.
- Factory cab set regenerated with grouped 1/12-octave minimum-phase synthesis
  (57 / ribbon / blend groups); mean target error is now 0.11–0.33 dB, max
  0.7–2.3 dB. Local Kazrog AmpCraft IR sets are used as unshipped analysis
  references for Hot Rod, Crunch, Recto and the new Hot Rod Greenback
  alternate (eight factory cabs total).
- Amp-section bypass (`amp_bypass`): the 808, delay/reverb loop, cab IR,
  output trim and limiter remain active; the preamp, tone stack and power amp
  are skipped.
- NAM loader: vendored MIT NeuralAmpModelerCore, Eigen, nlohmann/json and
  libsamplerate; dedicated NAM page with a file browser, drag-and-drop,
  worker-thread model loading, host-rate resampling and a level trim.
- UI polish: Comp Suite-style power glyph, right-aligned header pills with the
  NAM page in sequence, centered master/output, aligned cab mic/knob rows,
  symmetrical pedal cards and a wider user-IR/NAM list.
- High-gain voicing: the Gain knob now reaches saturation around 6 and is
  heavily saturated by 10 with active pickups; the high-gain recipes got
  more base drive.
- Input noise gate (`gate_on`, `gate_thresh`, `gate_decay`) on the amp page,
  enabled by the high-gain presets.
- DC blockers around the nonlinear amp stages and a slow high-gain sustain
  compressor; this removes the bias offset that made high-gain tails feel
  gate-like.
- Fixed a sign error in the power-amp feedback filter.  At HQ/4x rate the
  loop exceeded unity gain and self-oscillated near Nyquist, which was heard
  as warble and a premature tail cutoff below Gain 4.5.
- High-gain retune: more power-amp headroom at low gain, a tight 105 Hz
  pre-gain high-pass, a 1.2 kHz focus and a gentle 5 kHz air shelf, so the
  Block 120 / Stealth / Recto models clean up instead of staying fuzzy and
  align better with a 5150 Block Letter NAM reference.
- The sustain compressor now blends in between Gain 5 and 6 instead of
  switching on at a threshold.
- Power button is now a true bypass: the dry input is passed through
  untouched, independent of the host bypass.
- First-load fix: the cab IR is re-installed after the host's extra
  `prepare()`/activate pass, and the factory cabs reload at the project
  sample rate.  A fresh instance no longer starts without its cabinet until
  an amp switch.
- Amp-model and cab/mic/IR changes use a 15 ms fade-out, 20 ms silent hold
  and 15 ms fade-in.  The IR swap happens inside the hold, so the cab no
  longer clicks when a model changes.
- Delay tempo sync is covered by a regression test (1/4 note at 120 BPM =
  500 ms, at 60 BPM = 1000 ms), and the pedal page names the sync state,
  ping-pong mode, selected division and reverb pre-delay.
- The amp faceplate grille now shows the active amp's name in the Jost
  ExtraBold logo style instead of generic speaker circles.
- Factory cab selection is now independent of the amp model.  The cab page
  has a central list of all eight cabs; `cab_set` stores the choice and
  `cab` still selects Default/Custom/User.  Older sessions with 65 values
  load with `cab_set` at Auto.
- The input knob is aligned with the gate column and the master knob sits
  lower so its label clears the EQ row.
- Model and cab/IR changes now use a dual-engine 40 ms linear crossfade
  instead of a mute.  The standby engine is configured and run alongside the
  active one, so there is no silent gap and the IR swap is blended.
- A 50 ms startup fade-in after prepare/activate masks the first block and
  the cabinet IR onset on first launch.
- The cab-page factory list is centred in the middle column and aligned with
  the user-IR box; the mic buttons are centred over the level/cut controls.
- Removed unused engine state and the unused WAV `readU16` helper.
- Tone3000 A2 comparisons for the JC-120, AC30, Rockerverb and 1959 Super
  Lead led to small post-preamp body/focus/air trims on Chime, Crunch and
  Hot Rod.  The clean model was already very close.
- Startup now applies the tuned Block 120 — Rhythm preset rather than the
  neutral parameter defaults. The Gain knob now drives the tube preamp over a
  36 dB range (previously it only affected the solid-state path), high-gain
  amps reach saturation at normal settings, and the 808 defaults to a bright
  clean boost instead of a dark distortion box. High-gain models also get a
  pre-gain bright shelf, a wider presence shelf and a bassier/scooped startup
  EQ. Factory preset output levels are normalized and high-gain masters raised.
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
