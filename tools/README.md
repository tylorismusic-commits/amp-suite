# Developer tools

## AMP DSP probes

```sh
cmake --build build -j --target amp_probe amp_tone_probe
./build/amp_probe          # levels/THD with the 808 off
./build/amp_probe --od     # levels/THD with the 808 on
./build/amp_tone_probe     # tone-stack insertion loss per stack
```

## Cab IR model and fitter

The fitter is offline-only and needs Python 3.10+ with:

```sh
python3 -m venv .venv
.venv/bin/pip install -r tools/requirements.txt
```

Put manually downloaded reference WAVs under `refs/irs/<amp>/` (T3K license,
gitignored), then:

```sh
.venv/bin/python tools/ir_fit.py \
  --ref refs/irs/block-120 \
  --out assets/irs/block-120 \
  --name block-120 \
  --ref-url https://www.tone3000.com/tones/peavey-412ms-sheffield-1290-46881
```

`--quick` is a fast smoke fit. The full fit takes a few minutes per cab.

Validate:

```sh
.venv/bin/python tools/ir_validate.py \
  --ref refs/irs/block-120 \
  --ir assets/irs/block-120/cab_block-120_57.wav \
  --out docs/ir/block-120-57.png
```

Self-test (no references needed):

```sh
.venv/bin/python tools/ir_fit.py --self-test --quick
```

The generated WAVs are CC0; the reference WAVs are not redistributed.

## Metadata

```sh
./build/amp_ttl_gen --check --root .
```

Regenerates `lv2/{amp_suite,manifest,presets}.ttl` from `src/params.hpp` and
`src/presets.hpp`.
