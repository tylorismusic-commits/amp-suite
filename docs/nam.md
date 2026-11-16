# NAM models

Amp Suite can replace the built-in amp section with a Neural Amp Modeler
capture.

## Where models live

```
$XDG_DATA_HOME/amp-suite/nam      (usually ~/.local/share/amp-suite/nam)
```

Drop `.nam` files there, or use **NAM → BROWSE**. Drag-and-drop a `.nam` (or a
NAM-compatible WAV linear model) onto the plug-in window. The folder is watched;
new files appear without restarting the host.

## Using a model

1. Open the **NAM** header page.
2. Select a model from the list. The pill changes to **NAM ON**.
3. The model replaces the preamp, tone stack and power amp.
4. The 808, delay/reverb loop, cab IR, output trim and limiter stay active, so
   the NAM amp can feed the same cab/pedal chain as the built-in amps.
5. **AMP ON** switches back to the selected built-in amp without losing the NAM
   selection.

The **LEVEL** knob is a ±12 dB trim around the model's own loudness metadata.

## Sample rates

Models are trained at a fixed rate, normally 48 kHz. Amp Suite runs the model at
its trained rate and resamples both directions with libsamplerate when the host
uses 44.1, 48 or 96 kHz. NAM-compatible WAV linear models can run at arbitrary
rates through the NAM core.

## Session state

The selected index is stored in the host state. The model file itself is **not**
embedded. If a project is opened on another machine, put the same `.nam` files
in the NAM folder or reselect them. This is local user content and is never
redistributed by Amp Suite.

## Attribution

The NAM runtime is the MIT-licensed
[NeuralAmpModelerCore](https://github.com/sdatkinson/NeuralAmpModelerCore),
with Eigen (MPL-2.0) and nlohmann/json (MIT). Resampling uses libsamplerate
(BSD-2-Clause). See `THIRD_PARTY_NOTICES.md`.
