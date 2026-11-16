#!/usr/bin/env python3
"""Synthesize Amp Suite factory cab IRs from grouped reference magnitudes.

The factory IRs are original CC0 content.  The reference WAVs are only read
locally to estimate a smoothed magnitude curve; no reference audio, phase or
time-domain data is copied into the output.

Method:
  1. Trim, resample and window each reference capture.
  2. Estimate its magnitude, smooth to 1/12-octave and average in dB with
     optional weights (the Recto 1993 cab is the anchor, 2002 is a cross-check).
  3. Build a minimum-phase IR from the averaged magnitude via the real
     cepstrum, high-pass at 25 Hz, low-pass at 20 kHz and fade the tail.
  4. Normalize the peak and write 48 kHz / 96 kHz PCM-24 WAVs.

Examples:
  /tmp/opencode/amp-venv/bin/python tools/ir_synth.py --all
  /tmp/opencode/amp-venv/bin/python tools/ir_synth.py --cab block-120
  /tmp/opencode/amp-venv/bin/python tools/ir_synth.py --self-test
"""

from __future__ import annotations

import argparse
import glob
import json
import math
import os
import sys
from datetime import date

import numpy as np
from scipy import signal

import ir_model

IR_ROOT = "assets/irs"
REF_ROOT = "refs/irs"
SMOOTH_OCT = 1.0 / 12.0
TAPS_48 = 2048
TAPS_96 = 4096
FFT_48 = 16384
FFT_96 = 32768


CABS = {
    "chorus-clean": {
        "urls": ["https://www.tone3000.com/tones/jc120-cab-impulses-1644"],
        "variants": {
            "57": {},
            "121": {"eq": [("high_shelf", 6000.0, -2.0, 0.7)]},
            "blend": {},
        },
    },
    "chime": {
        "urls": ["https://www.tone3000.com/tones/vox-alnico-blue-2x12-98463"],
        "variants": {
            "57": {},
            "121": {
                "eq": [
                    ("high_shelf", 5500.0, -2.5, 0.7),
                    ("low_shelf", 120.0, 0.8, 0.7),
                ]
            },
            "blend": {"mix": {"57": 0.65, "121": 0.35}},
        },
    },
    "crunch": {
        "urls": [
            "https://www.tone3000.com/tones/drps-orange-4x12-v30-impulse-response-80534"
        ],
        "variants": {
            "57": {},
            "121": {"eq": [("high_shelf", 6000.0, -1.0, 0.7)]},
            "blend": {},
        },
    },
    "hot-rod": {
        "urls": [
            "https://www.tone3000.com/tones/marshall-jcm800-lead-1960-4x12-g12t-75-104387"
        ],
        "variants": {
            "57": {},
            "121": {},
            "blend": {"mix": {"57": 0.65, "121": 0.35}},
        },
    },
    "block-120": {
        "urls": [
            "https://www.tone3000.com/tones/peavey-412ms-sheffield-1290-46881"
        ],
        "variants": {
            "57": {},
            "121": {"eq": [("high_shelf", 6000.0, -1.0, 0.7)]},
            "blend": {},
        },
    },
    "stealth": {
        "urls": [
            "https://www.tone3000.com/tones/celestion-g12-evh-evh-5150iii-4x12-40738"
        ],
        "variants": {
            "57": {},
            "121": {
                "eq": [
                    ("high_shelf", 5500.0, -2.5, 0.7),
                    ("low_shelf", 120.0, 0.8, 0.7),
                ]
            },
            "blend": {"mix": {"57": 0.65, "121": 0.35}},
        },
    },
    "recto": {
        "urls": [
            "https://www.tone3000.com/tones/1993-mesa-rectifier-os-standard-slant-4x12-v30s-97689",
            "https://www.tone3000.com/tones/mesa-rectifier-standard-2002-4x12-slanted-v30-multi-mics-67544",
        ],
        "variants": {"57": {}, "121": {}, "blend": {}},
    },
}


def variant_files(cab: str, variant: str, ref_root: str) -> tuple[list[str], dict]:
    folder = os.path.join(ref_root, cab, variant)
    files = sorted(glob.glob(os.path.join(folder, "*.wav")) +
                   glob.glob(os.path.join(folder, "*.WAV")))
    weights = {}
    weights_path = os.path.join(folder, "weights.json")
    if os.path.exists(weights_path):
        with open(weights_path, "r", encoding="utf-8") as fh:
            raw = json.load(fh)
        for name, value in raw.items():
            weights[name] = float(value)
    return files, weights


def apply_target_eq(target: np.ndarray, grid: np.ndarray, eq) -> np.ndarray:
    out = target.copy()
    if not eq:
        return out
    for item in eq:
        kind, f, gain, q = item
        if kind == "high_shelf":
            b, a = ir_model.high_shelf(float(f), float(gain), float(q),
                                       ir_model.SAMPLE_RATE)
        elif kind == "low_shelf":
            b, a = ir_model.low_shelf(float(f), float(gain), float(q),
                                      ir_model.SAMPLE_RATE)
        elif kind == "peaking":
            b, a = ir_model.peaking(float(f), float(gain), float(q),
                                    ir_model.SAMPLE_RATE)
        else:
            continue
        _, h = signal.freqz(b, a, worN=grid, fs=ir_model.SAMPLE_RATE)
        out += 20.0 * np.log10(np.maximum(np.abs(h), 1e-12))
    return out


def load_target(cab: str, variant: str, ref_root: str,
                smoothing: float = SMOOTH_OCT) -> tuple[np.ndarray, list[str], dict]:
    files, weights = variant_files(cab, variant, ref_root)
    if not files:
        raise RuntimeError(f"no references for {cab}/{variant}")
    grid = ir_model.log_grid()
    curves = []
    used = []
    used_weights = []
    for path in files:
        name = os.path.basename(path)
        weight = weights.get(name, 1.0)
        if weight <= 0.0:
            continue
        ir = ir_model.trim_reference(
            ir_model.read_wav_mono(path, ir_model.SAMPLE_RATE),
            ir_model.SAMPLE_RATE)
        if ir.size < 32:
            continue
        f, db = ir_model.magnitude_db(ir, ir_model.SAMPLE_RATE)
        curve = ir_model.smooth_db(f, db, grid, octaves=smoothing)
        # Remove per-capture level differences before averaging; the engine
        # normalizes the final IR at 1 kHz anyway.
        band = (grid >= 100.0) & (grid <= 8000.0)
        curve = curve - float(np.mean(curve[band]))
        curves.append(curve)
        used.append(name)
        used_weights.append(weight)
    if not curves:
        raise RuntimeError(f"no usable references for {cab}/{variant}")
    data = np.stack(curves, axis=0)
    w = np.array(used_weights, dtype=np.float64)
    w /= w.sum()
    target = np.sum(data * w[:, None], axis=0)
    return target, used, {name: float(weight) for name, weight in
                          zip(used, used_weights)}


def synthesize(target: np.ndarray, sr: int, taps: int, n_fft: int) -> np.ndarray:
    grid = ir_model.log_grid()
    f = np.fft.rfftfreq(n_fft, 1.0 / sr)
    lg = np.log10(np.maximum(grid, 1e-9))
    lf = np.log10(np.maximum(f, 1e-9))
    db = np.interp(lf, lg, target)

    # Extend below the fit range with a 12 dB/octave rolloff and above it with
    # a gentle 6 dB/octave slope.  Getting the sub-60 Hz end right matters:
    # extrapolating *up* toward DC creates a huge DC component and a boomy IR.
    low = f < ir_model.F_MIN
    db[low] = target[0] + 12.0 * np.log2(
        np.maximum(f[low], 1.0e-3) / ir_model.F_MIN)
    high = f > ir_model.F_MAX
    db[high] = target[-1] - 6.0 * np.log2(f[high] / ir_model.F_MAX)
    db = np.maximum(db, -90.0)

    log_mag = db * (math.log(10.0) / 20.0)
    cep = np.fft.irfft(log_mag, n_fft)
    cep_min = np.zeros(n_fft)
    cep_min[0] = cep[0]
    cep_min[1:n_fft // 2] = 2.0 * cep[1:n_fft // 2]
    if n_fft % 2 == 0:
        cep_min[n_fft // 2] = cep[n_fft // 2]
    ir = np.fft.irfft(np.exp(np.fft.rfft(cep_min)), n_fft)

    # Keep the cab band clean and remove subsonic energy before truncating.
    sos = signal.butter(2, 25.0 / (sr / 2.0), "high", output="sos")
    ir = signal.sosfilt(sos, ir)
    sos = signal.butter(4, min(20000.0, 0.45 * sr) / (sr / 2.0), "low",
                        output="sos")
    ir = signal.sosfilt(sos, ir)

    ir = ir[:taps].copy()
    fade = min(taps, 256)
    ir[-fade:] *= np.linspace(0.0, 1.0, fade)[::-1]
    peak = float(np.max(np.abs(ir)))
    if peak > 0.0:
        ir *= (10.0 ** (-0.3 / 20.0)) / peak
    return ir


def metrics_for(ir: np.ndarray, target: np.ndarray,
                sr: int = ir_model.SAMPLE_RATE) -> dict:
    grid = ir_model.log_grid()
    f, db = ir_model.magnitude_db(ir, sr)
    gen = ir_model.smooth_db(f, db, grid, octaves=SMOOTH_OCT)
    band = (grid >= 100.0) & (grid <= 8000.0)
    offset = float(np.mean(target[band] - gen[band]))
    diff = (gen + offset) - target
    return {
        "mean_abs_db": float(np.mean(np.abs(diff[band]))),
        "max_abs_db": float(np.max(np.abs(diff[band]))),
        "rms_db": float(np.sqrt(np.mean(diff[band] ** 2))),
        "offset_db": offset,
    }


def write_cab(cab: str, ref_root: str, out_root: str, name: str) -> dict:
    import soundfile as sf
    out_dir = os.path.join(out_root, cab)
    os.makedirs(out_dir, exist_ok=True)
    manifest_variants = {}
    targets = {}
    for variant, spec in CABS[cab]["variants"].items():
        if "mix" in spec:
            target = np.zeros_like(ir_model.log_grid())
            for source, weight in spec["mix"].items():
                target = target + float(weight) * targets[source]
            refs = []
            weights = {}
        else:
            target, refs, weights = load_target(cab, variant, ref_root)
            target = apply_target_eq(target, ir_model.log_grid(), spec.get("eq"))
        targets[variant] = target
        for sr, taps, n_fft, tag in (
            (48000, TAPS_48, FFT_48, ""),
            (96000, TAPS_96, FFT_96, "_96k"),
        ):
            ir = synthesize(target, sr, taps, n_fft)
            path = os.path.join(out_dir, f"cab_{name}_{variant}{tag}.wav")
            sf.write(path, ir.astype(np.float32), sr, subtype="PCM_24")
        ir48 = synthesize(target, 48000, TAPS_48, FFT_48)
        manifest_variants[variant] = {
            "references": refs,
            "weights": weights,
            "eq": spec.get("eq", []),
            "mix": spec.get("mix", {}),
            "metrics": metrics_for(ir48, target),
            "taps": {"48k": TAPS_48, "96k": TAPS_96},
            "smoothing_octaves": SMOOTH_OCT,
        }
        print(f"{cab}/{variant}: {manifest_variants[variant]['metrics']}")
    manifest = {
        "name": name,
        "model_version": 2,
        "method": "minimum-phase magnitude synthesis",
        "generated": date.today().isoformat(),
        "license": "CC0-1.0",
        "reference_license": "T3K (not redistributed)",
        "reference_urls": CABS[cab]["urls"],
        "variants": manifest_variants,
    }
    with open(os.path.join(out_dir, "manifest.json"), "w", encoding="utf-8") as fh:
        json.dump({name: manifest}, fh, indent=2, sort_keys=True)
        fh.write("\n")
    return manifest


def self_test() -> int:
    # Synthesize a known cab-like curve, fit it, and check the round trip.
    grid = ir_model.log_grid()
    x = np.array([(lo + hi) / 2 for _, lo, hi, _ in ir_model.PARAM_SPACE])
    target = ir_model.smooth_db(
        grid, ir_model.response_db(x, ir_model.SAMPLE_RATE, grid), grid,
        octaves=SMOOTH_OCT)
    ir = synthesize(target, 48000, TAPS_48, FFT_48)
    m = metrics_for(ir, target)
    print("self-test:", m)
    ok = m["mean_abs_db"] < 1.0 and m["max_abs_db"] < 3.0
    print("self-test:", "PASS" if ok else "FAIL")
    return 0 if ok else 1


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--cab", choices=sorted(CABS))
    ap.add_argument("--all", action="store_true")
    ap.add_argument("--ref-root", default=REF_ROOT)
    ap.add_argument("--out-root", default=IR_ROOT)
    ap.add_argument("--self-test", action="store_true")
    args = ap.parse_args()

    if args.self_test:
        return self_test()
    cabs = sorted(CABS) if args.all else ([args.cab] if args.cab else [])
    if not cabs:
        ap.error("use --cab <name> or --all")
    for cab in cabs:
        write_cab(cab, args.ref_root, args.out_root, cab)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
