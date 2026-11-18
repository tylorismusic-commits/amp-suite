// Amp Suite — TS9/808 overdrive
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Port of Guitarix's `ts9sim` (GPL-2.0-or-later, based on a TS-9 circuit
// analysis by Tamas Kenez).  The nonlinear table is included exactly once in
// ts9.cpp.

#pragma once

#include <cmath>

#include "common.hpp"

namespace amp {

double ts9_nonlinear(double x) noexcept;

class Ts9 {
public:
  void prepare(float sampleRate) noexcept {
    sr_ = sampleRate;
    setControls(0.28f, 0.5f, 0.55f);
    reset();
  }

  void setControls(float drive, float tone, float level) noexcept {
    constexpr float R1 = 4700.0f;
    const float R2 = 51000.0f + 500000.0f * clampf(drive, 0.0f, 1.0f);
    constexpr float C = 0.047e-6f;
    const float a1 = (R1 + R2) * C * 2.0f * sr_;
    const float a2 = R1 * C * 2.0f * sr_;
    const float den = 1.0f + a2;
    if (std::fabs(den) < 1.0e-20f) {
      return;
    }
    b0_ = (1.0f + a1) / den;
    b1_ = (1.0f - a1) / den;
    a1_ = (1.0f - a2) / den;

    // The TS tone control is a first-order low-pass after the clipper.  The
    // guitarix/TS-9 reference range is 400..1000 Hz; the previous 700..4200 Hz
    // range was far too bright and made the 808 sound thin and fizzy.
    const float toneHz = 400.0f * std::pow(2.5f, clampf(tone, 0.0f, 1.0f));
    tone_.set(toneHz, sr_);

    // Level is a usable pedal volume range, not the old -12..+12 dB swing.
    // Centred near unity so switching the 808 on does not feel like a drop.
    level_ = dbToGain(-12.0f + 20.0f * clampf(level, 0.0f, 1.0f));
  }

  void reset() noexcept {
    xPrev_ = 0.0f;
    x2Prev_ = 0.0f;
    tone_.reset();
  }

  float process(float x) noexcept {
    const float x2 = b0_ * x + b1_ * xPrev_ - a1_ * x2Prev_;
    xPrev_ = x;
    x2Prev_ = x2;
    const float clipped = static_cast<float>(ts9_nonlinear(x2 - x));
    float y = x - clipped;
    y = tone_.process(y);
    return y * level_;
  }

private:
  float sr_ = 48000.0f;
  float b0_ = 1.0f;
  float b1_ = 0.0f;
  float a1_ = 0.0f;
  float xPrev_ = 0.0f;
  float x2Prev_ = 0.0f;
  float level_ = 1.0f;
  OnePoleLP tone_;
};

} // namespace amp
