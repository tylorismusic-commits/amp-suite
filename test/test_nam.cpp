// Amp Suite — NAM loader smoke test
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dsp/crossfade_engine.hpp"
#include "dsp/nam_model.hpp"

#include <cmath>
#include <cstdio>
#include <memory>
#include <vector>

#ifndef NAM_TEST_MODEL
#define NAM_TEST_MODEL "test/fixtures/wavenet.nam"
#endif

int main() {
  for (double sr : {44100.0, 48000.0, 96000.0}) {
    amp::NamDSP nam;
    if (!nam.load(NAM_TEST_MODEL, sr, 512)) {
      std::fprintf(stderr, "NAM load failed at %.0f Hz\n", sr);
      return 1;
    }
    if (!nam.valid() || nam.name().empty()) {
      std::fprintf(stderr, "NAM model did not report a name\n");
      return 1;
    }
    std::vector<float> left(512), right(512);
    double energy = 0.0;
    for (int block = 0; block < 8; ++block) {
      for (int i = 0; i < 512; ++i) {
        const double t = (block * 512 + i) / sr;
        left[i] = right[i] =
            static_cast<float>(0.1 * std::sin(2.0 * 3.14159265358979 * 220.0 * t));
      }
      nam.process(left.data(), right.data(), 512);
      for (int i = 0; i < 512; ++i) {
        if (!std::isfinite(left[i]) || !std::isfinite(right[i])) {
          std::fprintf(stderr, "NAM output is not finite\n");
          return 1;
        }
        energy += static_cast<double>(left[i]) * left[i];
      }
    }
    if (energy <= 0.0) {
      std::fprintf(stderr, "NAM output is silent at %.0f Hz\n", sr);
      return 1;
    }
  }
  // With NAM on, a cab/model change goes through the short mute instead of a
  // two-engine switch.  The mute must not step the waveform.
  {
    const float sr = 48000.0f;
    amp::CrossfadeEngine e;
    e.prepare(sr);
    auto nam = std::make_shared<amp::NamDSP>();
    if (!nam->load(NAM_TEST_MODEL, sr, 512)) {
      std::fprintf(stderr, "NAM switch model load failed\n");
      return 1;
    }
    e.setNamModel(nam);
    amp::Controls c;
    c.namOn = true;
    c.cabBypass = true;
    c.odOn = false;
    c.gateOn = false;
    std::vector<float> l(512), r(512);
    amp::Meters m;
    auto fill = [&](int block) {
      for (int i = 0; i < 512; ++i) {
        l[i] = r[i] =
            0.15f * std::sin(2.0 * 3.14159265 * 220.0 *
                             (block * 512 + i) / sr);
      }
    };
    double steady = 0.0;
    for (int b = 0; b < 20; ++b) {
      fill(b);
      e.setControls(c);
      e.process(l.data(), r.data(), 512, m);
      if (b >= 15) {
        for (int i = 1; i < 512; ++i) {
          steady = std::max(steady,
                            double(std::fabs(l[i] - l[i - 1])));
        }
      }
    }
    c.cab = 1;
    c.cabSet = 1;
    double maxDelta = 0.0;
    double tailEnergy = 0.0;
    float previous = l[511];
    for (int b = 20; b < 40; ++b) {
      fill(b);
      e.setControls(c);
      e.process(l.data(), r.data(), 512, m);
      for (int i = 0; i < 512; ++i) {
        if (!std::isfinite(l[i]) || !std::isfinite(r[i])) {
          std::fprintf(stderr, "NAM switch is not finite\n");
          return 1;
        }
        maxDelta = std::max(maxDelta,
                            double(std::fabs(l[i] - previous)));
        previous = l[i];
      }
      if (b >= 35) {
        for (float v : l) tailEnergy += static_cast<double>(v) * v;
      }
    }
    const double tailRms = std::sqrt(tailEnergy / (5.0 * 512.0));
    const double allowed = steady * 1.5 + 0.005;
    if (tailRms < 1.0e-4 || maxDelta > allowed) {
      std::fprintf(stderr,
                   "NAM switch stepped: tailRms %.6f maxDelta %.6f "
                   "allowed %.6f\n",
                   tailRms, maxDelta, allowed);
      return 1;
    }
  }

  std::printf("amp_nam: ok\n");
  return 0;
}
