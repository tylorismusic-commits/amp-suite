# Generated factory cabinet IRs

These are **original CC0** impulse responses synthesized from grouped,
level-normalized reference magnitudes. The references are listed in
`docs/ir-provenance.md`; no TONE3000 or commercial IR WAV is stored here.

The current factory set uses `tools/ir_synth.py`: each reference group is
smoothed to 1/12-octave, averaged in dB, converted to a minimum-phase IR, then
high-passed at 25 Hz, low-passed at 20 kHz and tail-faded. `tools/ir_fit.py` is
kept for parametric experiments; `tools/ir_validate.py` produces independent
comparison plots.

Generate one cab:

```sh
python3 -m venv .venv
.venv/bin/pip install -r tools/requirements.txt
.venv/bin/python tools/ir_synth.py --cab block-120
```

Generate the full set:

```sh
.venv/bin/python tools/ir_synth.py --all
```

Validate a variant against its reference group:

```sh
.venv/bin/python tools/ir_validate.py \
  --ref refs/irs/block-120/57 \
  --ir assets/irs/block-120/cab_block-120_57.wav \
  --out docs/ir/block-120-57.png
```

References are grouped under `refs/irs/<cab>/57`, `121` and `blend`; those
directories are gitignored. The generated files are `cab_<name>_57`, `_121`,
`_blend`, at 48 kHz (2048 taps) and 96 kHz (4096 taps). Each cab directory has
a `manifest.json` with its reference groups, weights, target EQ, synthesis
method and metrics; the aggregate is `manifest.json` at this level.

Generated set: `chorus-clean`, `chime`, `crunch`, `hot-rod`, `hot-rod-green`,
`block-120`, `stealth`, `recto`. The `recto` set is shared by the Recto tab and
the Block 120 OS alternate; `hot-rod-green` is the Hot Rod alternate. Each IR is
normalized to unity at 1 kHz by the engine for level consistency.
