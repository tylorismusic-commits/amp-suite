// Amp Suite — DSP smoke test
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dsp/crossfade_engine.hpp"
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

  // A model change must pass through a brief muted crossfade instead of
  // swapping the recipe while the signal is live.
  {
    amp::Controls c;
    c.model = 4;
    c.gain = 0.6f;
    c.cabBypass = true;
    c.odOn = false;
    c.gateOn = false;
    c.limiterOn = false;
    amp::Engine e;
    e.prepare(48000.0f);
    e.setControls(c);
    std::vector<float> l(512), r(512);
    amp::Meters m;
    bool sawMute = false;
    bool sawSignal = false;
    for (int b = 0; b < 40; ++b) {
      if (b == 20) {
        c.model = 5;
      }
      for (int i = 0; i < 512; ++i) {
        l[i] = r[i] =
            0.2f * std::sin(2.0 * 3.14159265 * 220.0 * (b * 512 + i) / 48000.0);
      }
      e.setControls(c);
      e.process(l.data(), r.data(), 512, m);
      for (int i = 0; i < 512; ++i) {
        const float v = std::fabs(l[i]);
        if (!std::isfinite(v)) {
          std::fprintf(stderr, "model change produced a non-finite sample\n");
          return 1;
        }
        if (b >= 20 && b < 24 && v < 1.0e-6f) sawMute = true;
        if (b >= 26 && v > 1.0e-4f) sawSignal = true;
      }
    }
    if (!sawMute || !sawSignal) {
      std::fprintf(stderr, "model change did not mute then resume\n");
      return 1;
    }
  }

  // The dual-engine wrapper fades the old amp out, swaps at silence and
  // fades the new one in.  The switch must not step the waveform, and the
  // gap must be short.  (An equal-gain crossfade used to sum two
  // differently-phased amps, producing a deep cancellation dip that was
  // heard as a pop.)
  {
    amp::CrossfadeEngine e;
    e.prepare(48000.0f);
    amp::Controls c;
    c.model = 4;
    c.gain = 0.6f;
    c.cabBypass = true;
    c.odOn = false;
    c.gateOn = false;
    c.limiterOn = false;
    std::vector<float> l(512), r(512);
    amp::Meters m;
    auto fill = [&](int block) {
      for (int i = 0; i < 512; ++i) {
        l[i] = r[i] =
            0.2f * std::sin(2.0 * 3.14159265 * 220.0 *
                            (block * 512 + i) / 48000.0);
      }
    };
    fill(0);
    e.setControls(c);
    e.process(l.data(), r.data(), 512, m);
    if (std::fabs(l[0]) > 1.0e-6f) {
      std::fprintf(stderr, "crossfade engine did not fade in after prepare\n");
      return 1;
    }
    for (int b = 1; b < 20; ++b) {
      fill(b);
      e.setControls(c);
      e.process(l.data(), r.data(), 512, m);
    }
    c.model = 5;
    double steadyBefore = 0.0;
    for (int b = 16; b < 20; ++b) {
      for (int i = 1; i < 512; ++i) {
        steadyBefore = std::max(steadyBefore,
                                double(std::fabs(l[i] - l[i - 1])));
      }
    }
    double minRms = 1.0e9;
    double maxDelta = 0.0;
    double steadyAfter = 0.0;
    bool finite = true;
    bool swapped = false;
    float previous = l[511];
    for (int b = 20; b < 40; ++b) {
      fill(b);
      e.setControls(c);
      e.process(l.data(), r.data(), 512, m);
      finite = finite && finiteBuffer(l, r);
      for (int i = 0; i < 512; ++i) {
        const double d = std::fabs(l[i] - previous);
        maxDelta = std::max(maxDelta, d);
        if (std::fabs(l[i]) < 1.0e-5f) swapped = true;
        previous = l[i];
      }
      if (b <= 30) {
        double energy = 0.0;
        for (float v : l) {
          energy += static_cast<double>(v) * v;
        }
        minRms = std::min(minRms, std::sqrt(energy / l.size()));
      }
      if (b >= 35) {
        for (int i = 1; i < 512; ++i) {
          steadyAfter = std::max(steadyAfter,
                                 double(std::fabs(l[i] - l[i - 1])));
        }
      }
    }
    double tailEnergy = 0.0;
    for (float v : l) {
      tailEnergy += static_cast<double>(v) * v;
    }
    const double tailRms = std::sqrt(tailEnergy / l.size());
    const double allowed =
        std::max(steadyBefore, steadyAfter) * 1.5 + 0.002;
    if (!finite || !swapped || tailRms < 1.0e-3 || maxDelta > allowed) {
      std::fprintf(stderr,
                   "model switch was not clean: finite %d swapped %d "
                   "tailRms %.6f maxDelta %.6f allowed %.6f\n",
                   finite ? 1 : 0, swapped ? 1 : 0, tailRms, maxDelta,
                   allowed);
      return 1;
    }
  }

  // A preset or host-state load replaces many controls at once while the amp
  // stays the same.  requestReload() must route that through the same clean
  // switch instead of stepping gain/tone coefficients in place.
  {
    amp::CrossfadeEngine e;
    e.prepare(48000.0f);
    amp::Controls c;
    c.model = 4;
    c.gain = 0.3f;
    c.master = 0.3f;
    c.cabBypass = true;
    c.odOn = false;
    c.gateOn = false;
    c.limiterOn = false;
    std::vector<float> l(512), r(512);
    amp::Meters m;
    auto fill = [&](int block) {
      for (int i = 0; i < 512; ++i) {
        l[i] = r[i] =
            0.2f * std::sin(2.0 * 3.14159265 * 220.0 *
                            (block * 512 + i) / 48000.0);
      }
    };
    for (int b = 0; b < 20; ++b) {
      fill(b);
      e.setControls(c);
      e.process(l.data(), r.data(), 512, m);
    }
    c.gain = 0.85f;
    c.master = 0.7f;
    c.bass = 0.8f;
    c.treble = 0.2f;
    double steadyBefore = 0.0;
    for (int i = 1; i < 512; ++i) {
      steadyBefore = std::max(steadyBefore,
                              double(std::fabs(l[i] - l[i - 1])));
    }
    e.requestReload();
    double maxDelta = 0.0;
    double steadyAfter = 0.0;
    bool finite = true;
    float previous = l[511];
    for (int b = 20; b < 40; ++b) {
      fill(b);
      e.setControls(c);
      e.process(l.data(), r.data(), 512, m);
      finite = finite && finiteBuffer(l, r);
      for (int i = 0; i < 512; ++i) {
        maxDelta = std::max(maxDelta,
                            double(std::fabs(l[i] - previous)));
        previous = l[i];
      }
      if (b >= 35) {
        for (int i = 1; i < 512; ++i) {
          steadyAfter = std::max(steadyAfter,
                                 double(std::fabs(l[i] - l[i - 1])));
        }
      }
    }
    const double allowed =
        std::max(steadyBefore, steadyAfter) * 1.5 + 0.002;
    if (!finite || maxDelta > allowed) {
      std::fprintf(stderr,
                   "preset reload stepped the output: maxDelta %.6f "
                   "allowed %.6f\n",
                   maxDelta, allowed);
      return 1;
    }
  }

  // With NAM on, a cab/model change uses the short mute path instead of
  // sharing the NAM DSP between two engines.
  {
    amp::CrossfadeEngine e;
    e.prepare(48000.0f);
    amp::Controls c;
    c.model = 4;
    c.gain = 0.6f;
    c.cabBypass = true;
    c.odOn = false;
    c.gateOn = false;
    c.limiterOn = false;
    c.namOn = true;
    std::vector<float> l(512), r(512);
    amp::Meters m;
    auto fill = [&](int block) {
      for (int i = 0; i < 512; ++i) {
        l[i] = r[i] =
            0.2f * std::sin(2.0 * 3.14159265 * 220.0 *
                            (block * 512 + i) / 48000.0);
      }
    };
    for (int b = 0; b < 20; ++b) {
      fill(b);
      e.setControls(c);
      e.process(l.data(), r.data(), 512, m);
    }
    c.cab = 1.0f;
    c.cabSet = 1.0;
    bool sawMute = false;
    bool sawSignal = false;
    bool finite = true;
    for (int b = 20; b < 40; ++b) {
      fill(b);
      e.setControls(c);
      e.process(l.data(), r.data(), 512, m);
      finite = finite && finiteBuffer(l, r);
      for (float v : l) {
        if (std::fabs(v) < 1.0e-7f) sawMute = true;
        if (b >= 30 && std::fabs(v) > 1.0e-4f) sawSignal = true;
      }
    }
    if (!finite || !sawMute || !sawSignal) {
      std::fprintf(stderr, "NAM change mute did not complete\n");
      return 1;
    }
  }

  // Tempo sync: a 1/4-note delay must follow the supplied BPM.
  {
    auto echoMs = [](float bpm) {
      amp::Controls c;
      c.delayOn = true;
      c.delaySync = true;
      c.delayDiv = 2;  // 1/4 note
      c.delayFeedback = 0.0f;
      c.delayMix = 1.0f;
      c.delayTone = 1.0f;
      c.delayMod = 0.0f;
      amp::Delay d;
      d.prepare(48000.0f);
      d.setTargets(c, bpm);
      for (int i = 0; i < 24000; ++i) {  // let the time smoother settle
        float l = 0.0f, r = 0.0f;
        d.process(l, r);
      }
      float l = 0.0f, r = 0.0f;
      int peakIdx = 0;
      float peak = 0.0f;
      for (int i = 0; i < 72000; ++i) {
        if (i == 0) {
          l = r = 1.0f;
        } else {
          l = r = 0.0f;
        }
        d.process(l, r);
        if (std::fabs(l) > peak) {
          peak = std::fabs(l);
          peakIdx = i;
        }
      }
      return static_cast<float>(peakIdx) * 1000.0f / 48000.0f;
    };
    const float t120 = echoMs(120.0f);
    const float t60 = echoMs(60.0f);
    if (std::fabs(t120 - 500.0f) > 20.0f) {
      std::fprintf(stderr, "1/4 delay at 120 BPM is %.1f ms, expected 500\n", t120);
      return 1;
    }
    if (std::fabs(t60 - 1000.0f) > 30.0f) {
      std::fprintf(stderr, "1/4 delay at 60 BPM is %.1f ms, expected 1000\n", t60);
      return 1;
    }
  }

  // The host calls prepare() again in activate() after the constructor has
  // already applied controls.  That second prepare clears the convolver and
  // must re-install the current IR, otherwise the first session loads without
  // a cabinet until a model switch forces a new IR pointer.
  {
    amp::Engine e;
    e.prepare(48000.0f);
    if (!e.loadFactoryCabs(AMP_TEST_ASSETS)) {
      std::fprintf(stderr, "factory cabs did not load from %s\n", AMP_TEST_ASSETS);
      return 1;
    }
    amp::Shared s;
    e.setControls(amp::controlsFrom(s));
    e.prepare(48000.0f);
    e.setControls(amp::controlsFrom(s));
    if (!e.cabReady()) {
      std::fprintf(stderr, "cab IR was not restored after the second prepare\n");
      return 1;
    }
  }

  // Power off is a true bypass: the dry input reaches the output untouched.
  {
    amp::Controls c;
    c.model = 4;
    c.enabled = false;
    c.gain = 0.7f;
    c.odOn = true;
    c.cabBypass = false;
    amp::Engine e;
    e.prepare(48000.0f);
    e.setControls(c);
    std::vector<float> l(1024), r(1024), dry(1024);
    for (int i = 0; i < 1024; ++i) {
      dry[i] = 0.3f * std::sin(2.0 * 3.14159265 * 220.0 * i / 48000.0);
      l[i] = r[i] = dry[i];
    }
    amp::Meters m;
    e.process(l.data(), r.data(), 1024, m);
    for (int i = 0; i < 1024; ++i) {
      if (l[i] != dry[i] || r[i] != dry[i]) {
        std::fprintf(stderr, "power off was not a true bypass at sample %d\n", i);
        return 1;
      }
    }
    if (!(m.inPeakDb > -20.0f) || !(m.outPeakDb > -20.0f)) {
      std::fprintf(stderr, "true bypass did not meter the dry signal\n");
      return 1;
    }
  }

  // Powering back on after changing the model must not leave the engine
  // stuck in bypass while a model fade is pending.
  {
    amp::Controls c;
    c.model = 4;
    c.enabled = false;
    c.cabBypass = true;
    c.odOn = false;
    amp::Engine e;
    e.prepare(48000.0f);
    e.setControls(c);
    std::vector<float> l(512), r(512), dry(512);
    for (int i = 0; i < 512; ++i) {
      dry[i] = 0.1f * std::sin(2.0 * 3.14159265 * 220.0 * i / 48000.0);
      l[i] = r[i] = dry[i];
    }
    amp::Meters m;
    e.process(l.data(), r.data(), 512, m);
    bool bypassed = true;
    for (int i = 0; i < 512; ++i) {
      if (l[i] != dry[i]) bypassed = false;
    }
    if (!bypassed) {
      std::fprintf(stderr, "power off was not a true bypass\n");
      return 1;
    }
    c.model = 5;
    c.enabled = true;
    bool processed = false;
    for (int b = 0; b < 40; ++b) {
      for (int i = 0; i < 512; ++i) {
        l[i] = r[i] = 0.1f * std::sin(2.0 * 3.14159265 * 220.0 * (b * 512 + i) / 48000.0);
      }
      e.setControls(c);
      e.process(l.data(), r.data(), 512, m);
      if (b == 39) {
        for (int i = 0; i < 512; ++i) {
          if (std::fabs(l[i]) > 1.0e-6f) processed = true;
        }
      }
    }
    if (!processed) {
      std::fprintf(stderr, "engine stayed bypassed after power-on\n");
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
      b.set(amp::P_GAIN, 0.4);
      b.set(amp::P_GATE_ON, gateOn ? 1.0 : 0.0);
      b.set(amp::P_GATE_THRESH, 0.8);
      b.set(amp::P_GATE_DECAY, 0.0);
      amp::Engine e;
      e.prepare(48000.0f);
      e.setControls(amp::controlsFrom(b));
      std::vector<float> l(19200), r(19200);
      for (int i = 0; i < 19200; ++i) {
        l[i] = r[i] = ampLevel * std::sin(2.0 * 3.14159265 * 220.0 * i / 48000.0);
      }
      amp::Meters m;
      e.process(l.data(), r.data(), 19200, m);
      double energy = 0.0;
      for (int i = 14400; i < 19200; ++i) {
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


  // Tuner: a 110 Hz input reads as A2, and silence produces no note.
  {
    amp::Tuner tuner;
    tuner.prepare(48000.0f);
    std::vector<float> l(48000), r(48000);
    for (int i = 0; i < 48000; ++i) {
      l[i] = r[i] = 0.2f * std::sin(2.0 * 3.14159265 * 110.0 * i / 48000.0);
    }
    tuner.process(l.data(), r.data(), 48000, true);
    if (!tuner.hasPitch() || std::fabs(tuner.hz() - 110.0f) > 2.0f ||
        tuner.note() != 45) {
      std::fprintf(stderr, "tuner failed on 110 Hz: %.2f Hz note %d\n",
                   tuner.hz(), tuner.note());
      return 1;
    }
    std::fill(l.begin(), l.end(), 0.0f);
    std::fill(r.begin(), r.end(), 0.0f);
    tuner.process(l.data(), r.data(), 48000, true);
    if (tuner.hasPitch()) {
      std::fprintf(stderr, "tuner latched onto silence\n");
      return 1;
    }
  }

  std::printf("amp_dsp: ok\n");
  return 0;
}
