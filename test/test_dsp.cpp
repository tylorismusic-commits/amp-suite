// Amp Suite — DSP smoke test
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dsp/engine.hpp"
#include "params.hpp"

#include <cmath>
#include <cstdio>
#include <vector>

namespace {

bool finiteBuffer(const std::vector<float>& l, const std::vector<float>& r) {
  for (size_t i = 0; i < l.size(); ++i) {
    if (!std::isfinite(l[i]) || !std::isfinite(r[i])) {
      return false;
    }
  }
  return true;
}

} // namespace

int main() {
  amp::Shared shared;
  amp::Engine engine;
  engine.prepare(48000.0f);
  engine.setControls(amp::controlsFrom(shared));

  std::vector<float> l(512, 0.0f);
  std::vector<float> r(512, 0.0f);
  l[0] = 0.5f;
  r[0] = 0.5f;
  amp::Meters meters;
  engine.process(l.data(), r.data(), 512, meters);
  if (!finiteBuffer(l, r)) {
    std::fprintf(stderr, "non-finite output at startup\n");
    return 1;
  }
  if (!std::isfinite(meters.outPeakDb) || !std::isfinite(meters.outVuDb)) {
    std::fprintf(stderr, "non-finite meter\n");
    return 1;
  }

  // Every model must produce finite output and a plausible meter.
  for (int model = 0; model < 7; ++model) {
    shared.set(amp::P_MODEL, static_cast<double>(model));
    engine.setControls(amp::controlsFrom(shared));
    std::vector<float> ml(256, 0.1f);
    std::vector<float> mr(256, -0.1f);
    engine.process(ml.data(), mr.data(), 256, meters);
    if (!finiteBuffer(ml, mr)) {
      std::fprintf(stderr, "non-finite output on model %d\n", model);
      return 1;
    }
  }

  // Tempo state reaches the engine.
  engine.setTempo(140.0f, true);
  shared.set(amp::P_DELAY_ON, 1.0);
  engine.setControls(amp::controlsFrom(shared));
  engine.process(l.data(), r.data(), 512, meters);
  if (meters.bpm < 139.0f || meters.bpm > 141.0f || !meters.hostTempo) {
    std::fprintf(stderr, "tempo not propagated\n");
    return 1;
  }

  std::printf("amp_dsp: ok\n");
  return 0;
}
