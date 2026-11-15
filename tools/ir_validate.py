#!/usr/bin/env python3
"""Compare a generated Amp Suite cab IR with its reference captures.

  python tools/ir_validate.py --ref refs/irs/recto --ir assets/irs/recto/cab_recto_57.wav \
      --out docs/ir/recto-57.png
"""

from __future__ import annotations

import argparse
import glob
import os

import numpy as np
import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt

import ir_model


def refs(path: str) -> list[str]:
    if os.path.isdir(path):
        return sorted(glob.glob(os.path.join(path, "*.wav")) +
                      glob.glob(os.path.join(path, "*.WAV")))
    return [path]


def smoothed(path: str, sr: int = 48000):
    ir = ir_model.trim_reference(ir_model.read_wav_mono(path, sr), sr)
    f, db = ir_model.magnitude_db(ir, sr)
    return ir_model.smooth_db(f, db, ir_model.log_grid())


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--ref", required=True)
    ap.add_argument("--ir", required=True)
    ap.add_argument("--out", required=True)
    ap.add_argument("--title", default="")
    ap.add_argument("--max-refs", type=int, default=8)
    args = ap.parse_args()

    grid = ir_model.log_grid()
    paths = refs(args.ref)[:args.max_refs]
    if not paths:
        raise SystemExit("no reference WAVs")
    targets = [smoothed(p) for p in paths]
    target = np.mean(np.stack(targets, axis=0), axis=0)
    gen = smoothed(args.ir)

    # Align the mean level in the cab's useful band, so the plot compares shape.
    band = (grid >= 100.0) & (grid <= 8000.0)
    offset = float(np.mean(target[band] - gen[band]))
    gen_aligned = gen + offset
    diff = gen_aligned - target
    metrics = {
        "mean_abs_db": float(np.mean(np.abs(diff[band]))),
        "max_abs_db": float(np.max(np.abs(diff[band]))),
        "rms_db": float(np.sqrt(np.mean(diff[band] ** 2))),
        "offset_db": offset,
    }
    print(metrics)

    os.makedirs(os.path.dirname(os.path.abspath(args.out)), exist_ok=True)
    fig, ax = plt.subplots(figsize=(10, 5))
    for t in targets:
        ax.semilogx(grid, t, color="0.7", lw=0.8)
    ax.semilogx(grid, target, color="C0", lw=2.0, label="reference mean")
    ax.semilogx(grid, gen_aligned, color="C1", lw=2.0, label="generated")
    ax.set_xlim(ir_model.F_MIN, ir_model.F_MAX)
    ax.set_ylim(np.min(target) - 12, np.max(target) + 12)
    ax.set_xlabel("Hz")
    ax.set_ylabel("dB")
    ax.grid(True, which="both", alpha=0.25)
    ax.legend(loc="best")
    ax.set_title(args.title or os.path.basename(args.ir))
    fig.tight_layout()
    fig.savefig(args.out, dpi=140)
    print("wrote", args.out)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
