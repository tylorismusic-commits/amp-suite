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

  // Amp bypass removes the preamp/power amp but keeps input, 808, loop,
  // cab and output active.
  {
    amp::Shared b;
    b.set(amp::P_MODEL, 4.0);
    b.set(amp::P_GAIN, 1.0);
    b.set(amp::P_MASTER, 1.0);
    b.set(amp::P_OUTPUT, 0.5);
    b.set(amp::P_OD_ON, 0.0);
    b.set(amp::P_CAB_BYPASS, 1.0);
    b.set(amp::P_AMP_BYPASS, 1.0);
    amp::Engine e;
    e.prepare(48000.0f);
    e.setControls(amp::controlsFrom(b));
    std::vector<float> bl(512), br(512);
    for (int i = 0; i < 512; ++i) {
      bl[i] = 0.1f * std::sin(2.0 * 3.14159265 * 220.0 * i / 48000.0);
      br[i] = bl[i];
    }
    amp::Meters bm;
    e.process(bl.data(), br.data(), 512, bm);
    if (!finiteBuffer(bl, br)) {
      std::fprintf(stderr, "non-finite amp-bypass output\n");
      return 1;
    }
    double energy = 0.0;
    for (float v : bl) {
      energy += static_cast<double>(v) * v;
    }
    const double rms = std::sqrt(energy / bl.size());
    if (rms < 0.02 || rms > 0.2) {
      std::fprintf(stderr, "amp bypass level is wrong: %.3f\n", rms);
      return 1;
    }
  }

  // Noise gate: a very quiet input is attenuated when the gate is open, while
  // a normal input passes.
  {
    auto measure = [](bool gateOn, float ampLevel) {
      amp::Shared b;
      b.set(amp::P_MODEL, 4.0);
      b.set(amp::P_CAB_BYPASS, 1.0);
      b.set(amp::P_OD_ON, 0.0);
      b.set(amp::P_DELAY_ON, 0.0);
      b.set(amp::P_REVERB_ON, 0.0);
      b.set(amp::P_GAIN, 0.5);
      b.set(amp::P_GATE_ON, gateOn ? 1.0 : 0.0);
      b.set(amp::P_GATE_THRESH, 0.8);
      b.set(amp::P_GATE_DECAY, 0.0);
      amp::Engine e;
      e.prepare(48000.0f);
      e.setControls(amp::controlsFrom(b));
      std::vector<float> l(9600), r(9600);
      for (int i = 0; i < 9600; ++i) {
        l[i] = r[i] = ampLevel * std::sin(2.0 * 3.14159265 * 220.0 * i / 48000.0);
      }
      amp::Meters m;
      e.process(l.data(), r.data(), 9600, m);
      double energy = 0.0;
      for (int i = 4800; i < 9600; ++i) {
        energy += static_cast<double>(l[i]) * l[i];
      }
      return std::sqrt(energy / 4800.0);
    };
    const double quietOff = measure(false, 0.001f);
    const double quietOn = measure(true, 0.001f);
    const double loudOn = measure(true, 0.1f);
    if (!(quietOn < quietOff * 0.25)) {
      std::fprintf(stderr, "gate did not attenuate a quiet input: %.6f vs %.6f\n",
                   quietOn, quietOff);
      return 1;
    }
    if (!(loudOn > 0.0001)) {
      std::fprintf(stderr, "gate blocked a normal input\n");
      return 1;
    }
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

  // Extreme settings must stay finite and bounded (no NaN, no runaway) at
  // both oversampling rates.
  for (int model = 0; model < 7; ++model) {
    for (int hq = 0; hq < 2; ++hq) {
    amp::Shared x;
    x.set(amp::P_MODEL, static_cast<double>(model));
    x.set(amp::P_HQ, static_cast<double>(hq));
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
      std::fprintf(stderr, "runaway output on model %d (hq=%d): %.1f\n", model,
                   hq, static_cast<double>(peak));
      return 1;
    }
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
