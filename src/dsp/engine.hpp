// Amp Suite — engine.hpp
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Scaffold engine.  The recipe-driven Guitarix DSP lands in M1; this file keeps
// the host/UI contract real and produces a usable, model-dependent sound so
// the plugin can be built, loaded and tested end to end.

#pragma once

#include <algorithm>
#include <cmath>

#include "biquad.hpp"
#include "common.hpp"
#include "params.hpp"

namespace amp {

class Engine {
public:
  void prepare(float sampleRate) noexcept {
    sr_ = sampleRate;
    odHpL_.reset();
    odHpR_.reset();
    odLpL_.reset();
    odLpR_.reset();
    cabHpL_.reset();
    cabHpR_.reset();
    cabLpL_.reset();
    cabLpR_.reset();
    cabHpL_.set(70.0f, sr_);
    cabHpR_.set(70.0f, sr_);
    cabLpL_.lowPass(6500.0f, 0.707f, sr_);
    cabLpR_.lowPass(6500.0f, 0.707f, sr_);
    inputSmooth_.setTime(20.0f, sr_);
    outputSmooth_.setTime(20.0f, sr_);
    inputSmooth_.reset(0.5f);
    outputSmooth_.reset(0.5f);
    vuCoeff_ = timeCoeff(300.0f, sr_);
    vuPower_ = 0.0f;
    active_ = -1;
    lastModel_ = -1;
  }

  void setControls(const Controls& c) noexcept {
    if (c.model != lastModel_) {
      lastModel_ = c.model;
      resetTone(c.model);
    }
    if (c.model != active_) {
      active_ = c.model;
    }
    controls_ = c;
    inputSmooth_.setTarget(c.input);
    outputSmooth_.setTarget(c.output);
    updateTone(c);
  }

  void setTempo(float bpm, bool valid) noexcept {
    bpm_ = (std::isfinite(bpm) && bpm > 1.0f) ? bpm : 120.0f;
    hostTempo_ = valid;
  }

  void process(float* left, float* right, int frames, Meters& meters) noexcept {
    float inPeakL = 0.0f;
    float inPeakR = 0.0f;
    float outPeakL = 0.0f;
    float outPeakR = 0.0f;
    const Controls c = controls_;
    const float ampGain = dbToGain(-6.0f + c.gain * 36.0f);
    const float master = dbToGain(-18.0f + c.master * 18.0f);
    const float odGain = c.odOn ? dbToGain(-3.0f + c.odDrive * 33.0f) : 1.0f;
    const float odLevel = c.odOn ? dbToGain((c.odLevel - 0.5f) * 24.0f) : 1.0f;
    const float cabLevel = c.cabBypass ? 1.0f : dbToGain(cabLevelDb(c.cabLevel));
    const float enabled = c.enabled ? 1.0f : 0.0f;

    for (int i = 0; i < frames; ++i) {
      const float dryL = left[i];
      const float dryR = right[i];
      inPeakL = std::max(inPeakL, std::fabs(dryL));
      inPeakR = std::max(inPeakR, std::fabs(dryR));

      const float in = inputSmooth_.next();
      const float trim = dbToGain((in - 0.5f) * 24.0f);
      float l = dryL * trim;
      float r = dryR * trim;

      // 808: bass cut, mid-forward clipping, tone low-pass.
      if (c.odOn) {
        l = odHpL_.process(l);
        r = odHpR_.process(r);
        l = std::tanh(l * odGain);
        r = std::tanh(r * odGain);
        l = odLpL_.process(l) * odLevel;
        r = odLpR_.process(r) * odLevel;
      }

      // Scaffold amp: gain, asymmetric-ish saturation, tone stack, master.
      l = std::tanh(l * ampGain + 0.06f) - 0.06f;
      r = std::tanh(r * ampGain + 0.06f) - 0.06f;
      l = toneL_[0].process(l);
      r = toneR_[0].process(r);
      l = toneL_[1].process(l);
      r = toneR_[1].process(r);
      l = toneL_[2].process(l);
      r = toneR_[2].process(r);
      l *= master;
      r *= master;

      // Scaffold cab: a fixed speaker-ish band-pass.  Replaced by IR
      // convolution in M1.
      if (!c.cabBypass) {
        l = cabHpL_.process(l);
        r = cabHpR_.process(r);
        l = cabLpL_.process(l);
        r = cabLpR_.process(r);
        l *= cabLevel;
        r *= cabLevel;
      }

      const float out = outputSmooth_.next();
      const float outGain = dbToGain((out - 0.5f) * 24.0f);
      l *= enabled * outGain;
      r *= enabled * outGain;

      left[i] = l;
      right[i] = r;
      outPeakL = std::max(outPeakL, std::fabs(l));
      outPeakR = std::max(outPeakR, std::fabs(r));
      const float power = 0.5f * (l * l + r * r);
      vuPower_ = vuCoeff_ * vuPower_ + (1.0f - vuCoeff_) * power;
    }

    meters.inPeakLDb = gainToDb(inPeakL);
    meters.inPeakRDb = gainToDb(inPeakR);
    meters.inPeakDb = std::max(meters.inPeakLDb, meters.inPeakRDb);
    meters.outPeakLDb = gainToDb(outPeakL);
    meters.outPeakRDb = gainToDb(outPeakR);
    meters.outPeakDb = std::max(meters.outPeakLDb, meters.outPeakRDb);
    meters.outVuDb = gainToDb(std::sqrt(vuPower_)) + 18.0f;
    meters.bpm = bpm_;
    meters.hostTempo = hostTempo_;
  }

  void reset() noexcept {
    odHpL_.reset();
    odHpR_.reset();
    odLpL_.reset();
    odLpR_.reset();
    cabHpL_.reset();
    cabHpR_.reset();
    cabLpL_.reset();
    cabLpR_.reset();
    vuPower_ = 0.0f;
  }

  uint32_t latencySamples() const noexcept { return 0; }

private:
  void resetTone(int model) noexcept {
    for (auto& s : toneL_) {
      s.reset();
    }
    for (auto& s : toneR_) {
      s.reset();
    }
    // Model-dependent voicing priors.  These are deliberately broad; the real
    // recipes replace them in M1.
    static const float kGain[7] = {0.55f, 0.62f, 0.72f, 0.76f,
                                   0.82f, 0.78f, 0.80f};
    static const float kLow[7] = {0.48f, 0.45f, 0.58f, 0.52f,
                                  0.55f, 0.50f, 0.62f};
    static const float kMid[7] = {0.45f, 0.58f, 0.50f, 0.47f,
                                  0.46f, 0.47f, 0.38f};
    static const float kHigh[7] = {0.58f, 0.56f, 0.54f, 0.53f,
                                   0.55f, 0.58f, 0.54f};
    const int m = std::min(std::max(model, 0), 6);
    voicing_[0] = kLow[m];
    voicing_[1] = kMid[m];
    voicing_[2] = kHigh[m];
    (void)kGain;
  }

  void updateTone(const Controls& c) noexcept {
    // Target response.  The controls are normalised; 0.5 is flat.
    const float bassDb = (c.bass - 0.5f) * 24.0f;
    const float midDb = (c.mid - 0.5f) * 18.0f;
    const float trebleDb = (c.treble - 0.5f) * 24.0f;
    const float presenceDb = (c.presence - 0.5f) * 12.0f;
    const float resonanceDb = (c.resonance - 0.5f) * 12.0f;

    for (auto* s : {&toneL_[0], &toneR_[0]}) {
      s->lowShelf(120.0f + 80.0f * c.bass, bassDb + resonanceDb, 0.7f, sr_);
    }
    for (auto* s : {&toneL_[1], &toneR_[1]}) {
      s->peak(720.0f, midDb, 0.8f, sr_);
    }
    for (auto* s : {&toneL_[2], &toneR_[2]}) {
      s->highShelf(3200.0f + 1800.0f * c.presence,
                   trebleDb + presenceDb, 0.7f, sr_);
    }

    // 808 tone low-pass: 700 Hz .. 5 kHz.
    const float odCut = 700.0f * std::pow(7.0f, c.odTone);
    odLpL_.lowPass(odCut, 0.707f, sr_);
    odLpR_.lowPass(odCut, 0.707f, sr_);
    odHpL_.set(720.0f, sr_);
    odHpR_.set(720.0f, sr_);

    // Cab filters follow the cut controls.
    cabHpL_.set(cabLowCutHz(c.cabLowCut), sr_);
    cabHpR_.set(cabLowCutHz(c.cabLowCut), sr_);
    cabLpL_.lowPass(cabHighCutHz(c.cabHighCut), 0.707f, sr_);
    cabLpR_.lowPass(cabHighCutHz(c.cabHighCut), 0.707f, sr_);
  }

  float sr_ = 48000.0f;
  Controls controls_;
  int lastModel_ = -1;
  int active_ = -1;
  float voicing_[3] = {0.5f, 0.5f, 0.5f};
  float bpm_ = 120.0f;
  bool hostTempo_ = false;
  float vuCoeff_ = 0.0f;
  float vuPower_ = 0.0f;
  Smoothed inputSmooth_;
  Smoothed outputSmooth_;
  Biquad toneL_[3];
  Biquad toneR_[3];
  HighPass2 odHpL_;
  HighPass2 odHpR_;
  Biquad odLpL_;
  Biquad odLpR_;
  HighPass2 cabHpL_;
  HighPass2 cabHpR_;
  Biquad cabLpL_;
  Biquad cabLpR_;
};

} // namespace amp
