# Amp Suite — cab IR provenance

**Status:** M0. Factory cabs are generated from a parametric model fitted to
smoothed reference magnitudes. Reference WAVs are never shipped.

## Policy

- TONE3000 reference captures are used only as **analysis targets**.
- Reference WAVs live in `refs/irs/` and are gitignored. They must not appear
  in the repository, release tarball or plugin binary.
- No bulk download, scrape, mirror or cache of TONE3000. The user downloads
  references manually.
- Generated factory IRs are original works synthesized from the model and are
  licensed **CC0**. The model/fitter is GPL-3.0-or-later.
- The plugin never fetches, uploads or shares user IRs.

TONE3000's T3K license permits downloading and using the file in software, but
not redistributing it without the author's permission. That is why the
references stay out of the release.

## Reference set

| Amp | Reference | Creator | Notes |
|---|---|---|---|
| Chorus Clean | [JC120 Cab Impulses](https://www.tone3000.com/tones/jc120-cab-impulses-1644) | timr | JC-120 2×12, two condensers, 15 IRs |
| Chime | [Vox Alnico Blue 2×12](https://www.tone3000.com/tones/vox-alnico-blue-2x12-98463) | flavortownstudios | AC30, Celestion Blue, E906 |
| Crunch | [DRPS Orange 4×12 V30](https://www.tone3000.com/tones/drps-orange-4x12-v30-impulse-response-80534) | drprophecystudio | SM57 / 7B / 421 / R121 |
| Hot Rod | [Marshall 1960 4×12 G12T-75](https://www.tone3000.com/tones/marshall-jcm800-lead-1960-4x12-g12t-75-104387) | tenfiftyfour | 75 IRs, lower speakers |
| Block 120 | [Peavey 412MS Sheffield 1290](https://www.tone3000.com/tones/peavey-412ms-sheffield-1290-46881) | guitarscrub | closest IR; exact 1200 only in a NAM capture |
| Stealth EL34 | [Celestion G12-EVH — EVH 5150III 4×12](https://www.tone3000.com/tones/celestion-g12-evh-evh-5150iii-4x12-40738) | OutmodedElectronics | all four speakers, 0.25″ moves |
| Recto / OS | [Mesa Rectifier Standard 2002 4×12 V30](https://www.tone3000.com/tones/mesa-rectifier-standard-2002-4x12-slanted-v30-multi-mics-67544) | kenazmusic | early-00s OS, SM57/MD421/R10, 6 positions |
| OS anchor | [1993 Mesa Rectifier OS Standard Slant 4×12 V30s](https://www.tone3000.com/tones/1993-mesa-rectifier-os-standard-slant-4x12-v30s-97689) | mikesedani | OG Mesa-spec V30s; era anchor |

### Mesa OS notes

- The target is the **Rectifier Standard / oversized** cab, not the smaller
  Traditional.
- Late-90s/early-00s UK-spec Vintage 30 voicing: warmer low-mids, smoother
  3–5 kHz, less fizz than modern Chinese V30s.
- Cross-checks: [2000s straight](https://www.tone3000.com/tones/mesa-os-4x12-ir-1612),
  [SM57/SM58/KSM313](https://www.tone3000.com/tones/mesa-4x12-oversized-rectifier-standard-shure-sm57-sm58-ksm313-irs-6271).
- Exclude the 2012/2013 OS cab and the 2002 Traditional.
- Used by Block 120 (alternate) and Recto (default).

## Generation

The fitter is implemented in `tools/ir_model.py`, `tools/ir_fit.py` and
`tools/ir_validate.py`. It needs numpy/scipy/soundfile/matplotlib; see
`tools/requirements.txt`.

```
refs/irs/<amp>/  →  load/mono/resample  →  1/6-octave |H(f)|
  →  differential_evolution + L-BFGS-B
  →  biquad cab model  →  minimum-phase impulse response
  →  normalize/truncate  →  48k/96k 24-bit WAV + manifest
```

The self-test (`tools/ir_fit.py --self-test --quick`) fits a synthetic target
and recovers it within ~0.1 dB mean error, which verifies the pipeline without
shipping any reference audio.

Model blocks: speaker low resonance, low shelf, presence peak, 3–6 cone-breakup
modes, HF rolloff; cabinet baffle step and internal modes; mic dynamic/ribbon/
condenser, proximity, on/off-axis, distance comb; optional short room tail.

Outputs per amp: `cab_<amp>_57`, `cab_<amp>_121`, `cab_<amp>_blend`;
48 kHz and 96 kHz, 2048 taps, minimum-phase.

## Acceptance

- 1/6-octave magnitude within ±2 dB, 80 Hz–8 kHz; RMS ±1 dB.
- Low resonance ±10%; HF rolloff ±1.5 dB at 4/6/8 kHz.
- Blind A/B: same cab family, not synthetic.
- Low cross-correlation with the reference and no null; the generated IR must
  be a model, not a copy.
- 2048 taps, minimum-phase, no added algorithmic latency.


## Generated factory set (0.2.0)

| Cab | Generated files | Mean / max error (57) | Notes |
|---|---|---:|---|
| Chorus Clean | `assets/irs/chorus-clean/` | 0.47 / 2.36 dB | JC-120 2×12 |
| Chime | `assets/irs/chime/` | 0.75 / 2.89 dB | AC30 alnico 2×12 |
| Crunch | `assets/irs/crunch/` | 1.22 / 5.66 dB | Orange 4×12 V30 |
| Hot Rod | `assets/irs/hot-rod/` | 1.32 / 3.91 dB | Marshall 1960 G12T-75 |
| Block 120 | `assets/irs/block-120/` | 0.83 / 3.78 dB | Peavey 412MS Sheffield 1290 |
| Stealth EL34 | `assets/irs/stealth/` | 0.63 / 4.25 dB | EVH 5150III G12-EVH |
| Recto / OS | `assets/irs/recto/` | 0.55 / 2.02 dB | 1993 OS anchor + 2002 cross-check |

Each set contains `57`, `121` and `blend` variants at 48 kHz (2048 taps) and
96 kHz (4096 taps). The engine normalizes each IR to unity at 1 kHz so cab and
mic switching stays level-consistent; the cab level control still trims the
final level.

The Mesa OS set is shared by the Recto default and the Block 120 alternate.
The Block 120 default remains the Sheffield 1290 cab.

The aggregate manifest is `assets/irs/manifest.json`; per-cab manifests record
the fitted parameters and reference URLs.

## Manifest

`assets/irs/manifest.json` records, per output: model version, fitted
parameters, reference URL, creator, license (T3K), reference hash and fit date.
`assets/irs/LICENSE` is CC0.

## Release gate

No T3K WAV in the repository or tarball. No network in package install
scripts. The user IR folder is `~/.local/share/amp-suite/irs/user/`; the plugin
only reads local files.
