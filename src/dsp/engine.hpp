// Amp Suite — engine.hpp
// SPDX-License-Identifier: GPL-3.0-or-later
//
// First-pass recipe engine.  The preamp uses Guitarix-derived tube stages and
// tone stacks; the power amp is a simple push-pull saturation for now.  M1
// replaces the power amp with the Plexi/sag/NFB model.

#pragma once

#include <algorithm>
#include <cmath>

#include "biquad.hpp"
#include "common.hpp"
#include "params.hpp"
#include "recipe.hpp"

namespace amp {

class Engine {
public:
  void prepare(float sampleRate) noexcept {
    sr_ = sampleRate;
    prepareRecipe(static_cast<int>(Model::Block120));
    inputSmooth_.setTime(20.0f, sr_);
    outputSmooth_.setTime(20.0f, sr_);
    inputSmooth_.reset(0.5f);
    outputSmooth_.reset(0.5f);
    odHpL_.set(720.0f, sr_);
    odHpR_.set(720.0f, sr_);
    odLpL_.lowPass(3000.0f, 0.707f, sr_);
    odLpR_.lowPass(3000.0f, 0.707f, sr_);
    cabHpL_.set(70.0f, sr_);
    cabHpR_.set(70.0f, sr_);
    cabLpL_.lowPass(6500.0f, 0.707f, sr_);
    cabLpR_.lowPass(6500.0f, 0.707f, sr_);
    vuCoeff_ = timeCoeff(300.0f, sr_);
    vuPower_ = 0.0f;
    active_ = -1;
    lastModel_ = -1;
  }

  void setControls(const Controls& c) noexcept {
    if (c.model != lastModel_) {
      lastModel_ = c.model;
      prepareRecipe(c.model);
    }
    controls_ = c;
    const AmpRecipe& r = recipeFor(c.model);
    toneSpec_ = toneStackSpec(r.tone);
    toneL_.set(toneSpec_, c.bass, c.mid, c.treble, sr_);
    toneR_.set(toneSpec_, c.bass, c.mid, c.treble, sr_);
    setPresenceResonance(c);
    inputSmooth_.setTarget(c.input);
    outputSmooth_.setTarget(c.output);
    updateOd(c);
    updateCab(c);
  }

  void setTempo(float bpm, bool valid) noexcept {
    bpm_ = (std::isfinite(bpm) && bpm > 1.0f) ? bpm : 120.0f;
    hostTempo_ = valid;
  }

  void process(float* left, float* right, int frames, Meters& meters) noexcept {
    const Controls c = controls_;
    const AmpRecipe& rec = recipeFor(c.model);
    float inPeakL = 0.0f;
    float inPeakR = 0.0f;
    float outPeakL = 0.0f;
    float outPeakR = 0.0f;
    const float master = dbToGain(-18.0f + c.master * 18.0f);
    const float odGain = c.odOn ? dbToGain(-3.0f + c.odDrive * 33.0f) : 1.0f;
    const float odLevel = c.odOn ? dbToGain((c.odLevel - 0.5f) * 24.0f) : 1.0f;
    const float makeup = dbToGain(rec.makeupDb);
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

      if (rec.solidState) {
        // JC-style clean path: high headroom, then the tone stack.
        l = std::tanh(l * (0.8f + c.gain * 0.6f) + 0.02f) - 0.02f;
        r = std::tanh(r * (0.8f + c.gain * 0.6f) + 0.02f) - 0.02f;
        l *= rec.preGain;
        r *= rec.preGain;
        l = toneL_.process(l);
        r = toneR_.process(r);
        l *= makeup;
        r *= makeup;
        l *= rec.outputTrim;
        r *= rec.outputTrim;
      } else {
        l *= rec.preGain;
        r *= rec.preGain;
        for (int s = 0; s < rec.stages; ++s) {
          l = stages_[s].process(l);
          r = stages_[s].process(r);
        }
        l = toneL_.process(l);
        r = toneR_.process(r);
        l *= makeup;
        r *= makeup;
        l = powerAmp(l, rec.powerDrive);
        r = powerAmp(r, rec.powerDrive);
        l *= rec.outputTrim;
        r *= rec.outputTrim;
      }

      l = presenceL_.process(l);
      r = presenceR_.process(r);
      l = resonanceL_.process(l);
      r = resonanceR_.process(r);
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
    for (auto& s : stages_) {
      s.reset();
    }
    toneL_.reset();
    toneR_.reset();
    odHpL_.reset();
    odHpR_.reset();
    odLpL_.reset();
    odLpR_.reset();
    cabHpL_.reset();
    cabHpR_.reset();
    cabLpL_.reset();
    cabLpR_.reset();
    presenceL_.reset();
    presenceR_.reset();
    resonanceL_.reset();
    resonanceR_.reset();
    vuPower_ = 0.0f;
  }

  uint32_t latencySamples() const noexcept { return 0; }

private:
  void prepareRecipe(int model) noexcept {
    const AmpRecipe& r = recipeFor(model);
    for (int i = 0; i < 6; ++i) {
      stages_[i].prepare(r.stage[i], sr_);
    }
    toneSpec_ = toneStackSpec(r.tone);
    toneL_.set(toneSpec_, 0.5f, 0.45f, 0.55f, sr_);
    toneR_.set(toneSpec_, 0.5f, 0.45f, 0.55f, sr_);
    toneL_.reset();
    toneR_.reset();
  }

  void setPresenceResonance(const Controls& c) noexcept {
    const float presenceDb = (c.presence - 0.5f) * 12.0f;
    const float resonanceDb = (c.resonance - 0.5f) * 12.0f;
    presenceL_.highShelf(3200.0f, presenceDb, 0.7f, sr_);
    presenceR_.highShelf(3200.0f, presenceDb, 0.7f, sr_);
    resonanceL_.lowShelf(120.0f, resonanceDb, 0.7f, sr_);
    resonanceR_.lowShelf(120.0f, resonanceDb, 0.7f, sr_);
  }

  void updateOd(const Controls& c) noexcept {
    const float odCut = 700.0f * std::pow(7.0f, c.odTone);
    odLpL_.lowPass(odCut, 0.707f, sr_);
    odLpR_.lowPass(odCut, 0.707f, sr_);
    odHpL_.set(720.0f, sr_);
    odHpR_.set(720.0f, sr_);
  }

  void updateCab(const Controls& c) noexcept {
    cabHpL_.set(cabLowCutHz(c.cabLowCut), sr_);
    cabHpR_.set(cabLowCutHz(c.cabLowCut), sr_);
    cabLpL_.lowPass(cabHighCutHz(c.cabHighCut), 0.707f, sr_);
    cabLpR_.lowPass(cabHighCutHz(c.cabHighCut), 0.707f, sr_);
  }

  static float powerAmp(float x, float drive) noexcept {
    constexpr float bias = 0.05f;
    return std::tanh(x * drive + bias) - std::tanh(bias);
  }

  float sr_ = 48000.0f;
  Controls controls_;
  int lastModel_ = -1;
  int active_ = -1;
  float bpm_ = 120.0f;
  bool hostTempo_ = false;
  float vuCoeff_ = 0.0f;
  float vuPower_ = 0.0f;
  ToneStackSpec toneSpec_;
  TubeStage stages_[6];
  ToneStack toneL_;
  ToneStack toneR_;
  Smoothed inputSmooth_;
  Smoothed outputSmooth_;
  HighPass2 odHpL_;
  HighPass2 odHpR_;
  Biquad odLpL_;
  Biquad odLpR_;
  HighPass2 cabHpL_;
  HighPass2 cabHpR_;
  Biquad cabLpL_;
  Biquad cabLpR_;
  Biquad presenceL_;
  Biquad presenceR_;
  Biquad resonanceL_;
  Biquad resonanceR_;
};

} // namespace amp
