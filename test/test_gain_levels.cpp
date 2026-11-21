// Amp Suite — level consistency across models and the Gain knob
// SPDX-License-Identifier: GPL-3.0-or-later
//
// The high-gain models used to sit ~6 dB above Hot Rod at Gain 5 and then lose
// level as the sustain compressor engaged, so turning the knob up made the amp
// quieter.  This renders a repeating plucked note through each model and checks
// the peak stays close to Hot Rod at both ends of the range.
#include "dsp/engine.hpp"
#include "params.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <vector>

namespace {

constexpr double kSr = 48000.0;
constexpr int kBlock = 256;

// Peak level of a repeating decaying note, in dB.
double peakDb(int model, double gain) {
  amp::Shared shared;
  shared.set(amp::P_MODEL, model);
  shared.set(amp::P_GAIN, gain);
  amp::Engine engine;
  engine.prepare(static_cast<float>(kSr));
  engine.setControls(amp::controlsFrom(shared));

  std::vector<float> l(kBlock), r(kBlock);
  amp::Meters meters;
  float peak = 0.0f;
  const int blocks = 200;  // ~1.1 s
  for (int b = 0; b < blocks; ++b) {
    for (int i = 0; i < kBlock; ++i) {
      const double t = static_cast<double>(b * kBlock + i) / kSr;
      const double phase = std::fmod(t, 0.9);
      const double env = std::exp(-phase * 6.0);
      const double f = 196.0;
      const float x = static_cast<float>(
          0.30 * env * (std::sin(2.0 * M_PI * f * t) +
                        0.35 * std::sin(4.0 * M_PI * f * t)));
      l[i] = x;
      r[i] = x;
    }
    engine.process(l.data(), r.data(), kBlock, meters);
    if (b > 60) {
      for (int i = 0; i < kBlock; ++i) {
        peak = std::max(peak, std::max(std::fabs(l[i]), std::fabs(r[i])));
      }
    }
  }
  return 20.0 * std::log10(static_cast<double>(peak) + 1e-20);
}

} // namespace

int main() {
  const int kHotRod = 3;
  const int kModels[] = {4, 5, 6};  // Block 120, Stealth, Recto
  int failures = 0;
  const double refLow = peakDb(kHotRod, 0.50);
  const double refHigh = peakDb(kHotRod, 0.90);
  std::printf("Hot Rod: gain 5 peak %+.2f dB, gain 9 peak %+.2f dB\n", refLow,
              refHigh);

  for (int model : kModels) {
    const double low = peakDb(model, 0.50);
    const double high = peakDb(model, 0.90);
    std::printf("%-12s gain 5 peak %+.2f dB (%+.2f vs Hot Rod), "
                "gain 9 peak %+.2f dB (%+.2f)\n",
                amp::kModelLabels[model], low, low - refLow, high,
                high - refHigh);
    if (std::fabs(low - refLow) > 3.0) {
      std::printf("  level at gain 5 is more than 3 dB off Hot Rod\n");
      ++failures;
    }
    if (std::fabs(high - refHigh) > 3.0) {
      std::printf("  level at gain 9 is more than 3 dB off Hot Rod\n");
      ++failures;
    }
    if (high < low - 3.0) {
      std::printf("  turning the gain up loses more than 3 dB\n");
      ++failures;
    }
  }
  return failures == 0 ? 0 : 1;
}
