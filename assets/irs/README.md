# Generated factory cabinet IRs

These are **original CC0** impulse responses synthesized from the parametric
cabinet model in `tools/ir_model.py` and fitted to the 1/6-octave magnitude of
the references listed in `docs/ir-provenance.md`.

No TONE3000 or commercial IR WAV is stored here.

Generate one cab:

```sh
python3 -m venv .venv
.venv/bin/pip install -r tools/requirements.txt
.venv/bin/python tools/ir_fit.py \
  --ref refs/irs/block-120 \
  --out assets/irs/block-120 \
  --name block-120
```

Validate:

```sh
.venv/bin/python tools/ir_validate.py \
  --ref refs/irs/block-120 \
  --ir assets/irs/block-120/cab_block-120_57.wav \
  --out docs/ir/block-120-57.png
```

The generated files are `cab_<name>_57`, `_121`, `_blend`, at 48 kHz and
96 kHz. Each cab directory has a `manifest.json` with its fitted parameters and
metrics; the aggregate is `manifest.json` at this level.

Generated set: `chorus-clean`, `chime`, `crunch`, `hot-rod`, `block-120`,
`stealth`, `recto`. The `recto` set is shared by the Recto tab and the Block 120
OS alternate. Each IR is normalized to unity at 1 kHz for level consistency.
