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

  // Extreme settings must stay finite and bounded (no NaN, no runaway).
  for (int model = 0; model < 7; ++model) {
    amp::Shared x;
    x.set(amp::P_MODEL, static_cast<double>(model));
    x.set(amp::P_GAIN, 1.0);
    x.set(amp::P_MASTER, 1.0);
    x.set(amp::P_OUTPUT, 0.7);
    x.set(amp::P_BASS, 1.0);
    x.set(amp::P_MID, 0.0);
    x.set(amp::P_TREBLE, 1.0);
    x.set(amp::P_PRESENCE, 1.0);
    x.set(amp::P_RESONANCE, 1.0);
    x.set(amp::P_OD_ON, 1.0);
    x.set(amp::P_OD_DRIVE, 1.0);
    x.set(amp::P_OD_LEVEL, 1.0);
    amp::Engine e;
    e.prepare(48000.0f);
    e.setControls(amp::controlsFrom(x));
    std::vector<float> xl(1024), xr(1024);
    for (int i = 0; i < 1024; ++i) {
      xl[i] = 0.5f * std::sin(2.0 * 3.14159265 * 220.0 * i / 48000.0);
      xr[i] = xl[i];
    }
    amp::Meters xm;
    e.process(xl.data(), xr.data(), 1024, xm);
    if (!finiteBuffer(xl, xr)) {
      std::fprintf(stderr, "non-finite output at extremes on model %d\n",
                   model);
      return 1;
    }
    float peak = 0.0f;
    for (float v : xl) {
      peak = std::max(peak, std::fabs(v));
    }
    if (peak > 50.0f) {
      std::fprintf(stderr, "runaway output on model %d: %.1f\n", model,
                   static_cast<double>(peak));
      return 1;
    }
  }

  // Delay/reverb extremes: high feedback, full wet, post-cab loop.
  {
    amp::Shared x;
    x.set(amp::P_MODEL, 4.0);
    x.set(amp::P_DELAY_ON, 1.0);
    x.set(amp::P_DELAY_SYNC, 0.0);
    x.set(amp::P_DELAY_TIME, 1.0);
    x.set(amp::P_DELAY_FEEDBACK, 0.9);
    x.set(amp::P_DELAY_MIX, 1.0);
    x.set(amp::P_REVERB_ON, 1.0);
    x.set(amp::P_REVERB_DECAY, 1.0);
    x.set(amp::P_REVERB_MIX, 1.0);
    x.set(amp::P_REVERB_PREDELAY, 1.0);
    x.set(amp::P_LOOP_POS, 1.0);
    amp::Engine e;
    e.prepare(48000.0f);
    e.setControls(amp::controlsFrom(x));
    std::vector<float> xl(48000), xr(48000);
    for (int i = 0; i < 48000; ++i) {
      xl[i] = 0.2f * std::sin(2.0 * 3.14159265 * 220.0 * i / 48000.0);
      xr[i] = xl[i];
    }
    amp::Meters xm;
    e.process(xl.data(), xr.data(), 48000, xm);
    if (!finiteBuffer(xl, xr)) {
      std::fprintf(stderr, "non-finite output in FX loop\n");
      return 1;
    }
    float peak = 0.0f;
    for (float v : xl) {
      peak = std::max(peak, std::fabs(v));
    }
    if (peak > 50.0f) {
      std::fprintf(stderr, "FX loop runaway: %.1f\n",
                   static_cast<double>(peak));
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
