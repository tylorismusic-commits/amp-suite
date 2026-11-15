#!/usr/bin/env python3
"""Fit the Amp Suite parametric cab model to one or more reference IRs.

Examples:
  python tools/ir_fit.py --ref refs/irs/block-120 --out assets/irs/block-120 \
      --name block-120 --ref-url https://www.tone3000.com/tones/...

  python tools/ir_fit.py --self-test --quick

Reference WAVs are only read locally.  The generated WAVs are CC0.
"""

from __future__ import annotations

import argparse
import glob
import os
import sys
import time

import numpy as np

import ir_model


def collect_refs(path: str) -> list[str]:
    if os.path.isdir(path):
        out = sorted(glob.glob(os.path.join(path, "*.wav")) +
                     glob.glob(os.path.join(path, "*.WAV")))
        return out
    return [path]


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--ref", help="reference WAV or directory")
    ap.add_argument("--out", help="output directory")
    ap.add_argument("--name", help="amp/cab name used in filenames")
    ap.add_argument("--ref-url", action="append", default=[],
                    help="reference URL (repeatable)")
    ap.add_argument("--seed", type=int, default=1234)
    ap.add_argument("--max-refs", type=int, default=4)
    ap.add_argument("--quick", action="store_true")
    ap.add_argument("--self-test", action="store_true")
    ap.add_argument("--progress", action="store_true")
    args = ap.parse_args()

    if args.self_test:
        # Generate a synthetic target from a known model and recover it.
        rng = np.random.default_rng(args.seed)
        true = np.array([(lo + hi) / 2 for _, lo, hi, _ in ir_model.PARAM_SPACE])
        # Pick a plausible cab-like point, not the centre of every bound.
        truth = {
            "res_f": 92.0, "res_gain": 4.0, "res_q": 1.4,
            "body_f": 180.0, "body_gain": 2.0, "body_q": 1.2,
            "baffle_f": 380.0, "baffle_gain": -2.0,
            "presence_f": 2600.0, "presence_gain": 3.0, "presence_q": 1.0,
            "breakup1_f": 2200.0, "breakup1_gain": -2.0, "breakup1_q": 5.0,
            "breakup2_f": 4100.0, "breakup2_gain": 2.0, "breakup2_q": 6.0,
            "breakup3_f": 6700.0, "breakup3_gain": -4.0, "breakup3_q": 5.0,
            "hf_f": 7000.0, "hf_gain": -10.0,
            "lp_f": 9500.0, "lp_q": 0.7,
            "mic_low_f": 110.0, "mic_low_gain": -1.0,
            "mic_hi_f": 6500.0, "mic_hi_gain": -4.0,
            "tilt": 0.5, "gain": 0.0,
        }
        true = np.array([truth[name] for name, _, _, _ in ir_model.PARAM_SPACE])
        grid = ir_model.log_grid()
        target = ir_model.smooth_db(
            grid, ir_model.response_db(true, ir_model.SAMPLE_RATE, grid), grid)
        t0 = time.time()
        x, metrics = ir_model.fit(
            target,
            seed=args.seed,
            maxiter=30 if args.quick else 90,
            popsize=8 if args.quick else 10,
            progress=args.progress,
        )
        dt = time.time() - t0
        print(f"self-test: {dt:.1f}s  {metrics}")
        ok = metrics["mean_abs_db"] < 2.0
        print("self-test:", "PASS" if ok else "FAIL")
        return 0 if ok else 1

    if not args.ref or not args.out or not args.name:
        ap.error("--ref, --out and --name are required")

    refs = collect_refs(args.ref)[:args.max_refs]
    if not refs:
        print("no reference WAVs found", file=sys.stderr)
        return 1
    print(f"references ({len(refs)}):")
    for r in refs:
        print("  ", r)

    target = ir_model.target_from_reference(refs)
    t0 = time.time()
    x, metrics = ir_model.fit(
        target,
        seed=args.seed,
        maxiter=30 if args.quick else 140,
        popsize=8 if args.quick else 12,
        progress=args.progress,
    )
    dt = time.time() - t0
    print(f"fit: {dt:.1f}s  {metrics}")
    if metrics["mean_abs_db"] > 3.0:
        print("warning: mean error above 3 dB; add a cleaner reference",
              file=sys.stderr)

    ir_model.write_wavs(x, args.out, args.name, 48000, 2048)
    ir_model.write_wavs(x, args.out, args.name, 96000, 4096,
                        rate_tag="_96k")
    manifest = ir_model.write_manifest(
        args.out, args.name, refs, x, metrics,
        extra={"reference_urls": args.ref_url},
    )
    print("wrote", args.out)
    print("manifest", manifest)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
