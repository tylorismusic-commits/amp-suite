// Amp Suite — engine.hpp
// SPDX-License-Identifier: GPL-3.0-or-later
//
// First-pass recipe engine.  The preamp uses Guitarix-derived tube stages and
// tone stacks; the power amp is a simple push-pull saturation for now.  M1
// replaces the power amp with the Plexi/sag/NFB model.

#pragma once

#include <algorithm>
#include <atomic>
#include <cmath>
#include <memory>

#include "biquad.hpp"
#include "cab_convolver.hpp"
#include "common.hpp"
#include "delay.hpp"
#include "params.hpp"
#include "reverb.hpp"
#include "oversample.hpp"
#include "power_amp.hpp"
#include "ts9.hpp"
#include "recipe.hpp"
#include "user_ir.hpp"

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
    odL_.prepare(sr_);
    odR_.prepare(sr_);
    cabConvolver_.prepare(4096);
    delay_.prepare(sr_);
    reverb_.prepare(sr_);
    cabHpL_.set(70.0f, sr_);
    cabHpR_.set(70.0f, sr_);
    cabLpL_.lowPass(6500.0f, 0.707f, sr_);
    cabLpR_.lowPass(6500.0f, 0.707f, sr_);
    vuCoeff_ = timeCoeff(300.0f, sr_);
    vuPower_ = 0.0f;
    limiterAttack_ = timeCoeff(0.5f, sr_);
    limiterRelease_ = timeCoeff(80.0f, sr_);
    limiterGain_ = 1.0f;
    active_ = -1;
    lastModel_ = -1;
    powerOs_ = 0;
    pendingModel_ = -1;
    fadeDir_ = 0;
    switchGain_ = 1.0f;
    switchStep_ = 1.0f / std::max(1.0f, sr_ * 0.010f);
  }

  void setControls(const Controls& c) noexcept {
    if (lastModel_ < 0) {
      lastModel_ = c.model;
      prepareRecipe(c.model);
      pendingModel_ = -1;
      fadeDir_ = 0;
    }
    if (c.model != lastModel_ && fadeDir_ != -1) {
      pendingModel_ = c.model;
      fadeDir_ = -1;
    }
    const int activeModel = (fadeDir_ == -1 && pendingModel_ >= 0)
                                ? lastModel_
                                : c.model;
    controls_ = c;
    const AmpRecipe& r = recipeFor(activeModel);
    PowerAmpSpec ps;
    ps.drive = r.powerDrive;
    ps.feedback = r.powerFeedback;
    ps.sag = r.powerSag;
    ps.output = r.outputTrim;
    ps.useTable = r.powerTable;
    const int os = c.hq ? 4 : 2;
    if (os != powerOs_) {
      powerL_.prepare(sr_ * static_cast<float>(os), ps);
      powerR_.prepare(sr_ * static_cast<float>(os), ps);
      os1L_.prepare();
      os1R_.prepare();
      os2L_.prepare();
      os2R_.prepare();
      powerOs_ = os;
    } else {
      powerL_.setSpec(ps);
      powerR_.setSpec(ps);
    }
    toneSpec_ = toneStackSpec(r.tone);
    toneL_.set(toneSpec_, c.bass, c.mid, c.treble, sr_);
    toneR_.set(toneSpec_, c.bass, c.mid, c.treble, sr_);
    powerL_.setPresence(c.presence);
    powerR_.setPresence(c.presence);
    setPresenceResonance(c);
    inputSmooth_.setTarget(c.input);
    outputSmooth_.setTarget(c.output);
    odL_.setControls(c.odDrive, c.odTone, c.odLevel);
    odR_.setControls(c.odDrive, c.odTone, c.odLevel);
    syncUserIrBank();
    updateCab(c);
    updateCabIR();
  }

  void setUserIrBank(std::shared_ptr<const UserIrBank> bank) noexcept {
    std::atomic_store(&userBankPending_, std::move(bank));
  }

  bool loadFactoryCabs(const std::string& dir) noexcept {
    const bool ok = cabBank_.load(dir, sr_);
    updateCabIR();
    return ok;
  }

  void setTempo(float bpm, bool valid) noexcept {
    bpm_ = (std::isfinite(bpm) && bpm > 1.0f) ? bpm : 120.0f;
    hostTempo_ = valid;
  }

  void process(float* left, float* right, int frames, Meters& meters) noexcept {
    const Controls c = controls_;
    const int procModel =
        (fadeDir_ == -1 && lastModel_ >= 0) ? lastModel_ : c.model;
    const AmpRecipe& rec = recipeFor(procModel);
    float inPeakL = 0.0f;
    float inPeakR = 0.0f;
    float outPeakL = 0.0f;
    float outPeakR = 0.0f;
    const float master = dbToGain(-18.0f + c.master * 18.0f);
    const float makeup = dbToGain(rec.makeupDb);
    const float cabLevel = c.cabBypass ? 1.0f : dbToGain(cabLevelDb(c.cabLevel));
    const float enabled = c.enabled ? 1.0f : 0.0f;

    const float tempo = hostTempo_ ? bpm_ : c.bpm;
    delay_.setTargets(c, tempo);
    reverb_.setTargets(c);

    for (int i = 0; i < frames; ++i) {
      const float dryL = left[i];
      const float dryR = right[i];
      inPeakL = std::max(inPeakL, std::fabs(dryL));
      inPeakR = std::max(inPeakR, std::fabs(dryR));

      const float in = inputSmooth_.next();
      const float trim = dbToGain((in - 0.5f) * 24.0f);
      float l = dryL * trim;
      float r = dryR * trim;

      // 808: TS9 input network, feedback clipper and tone low-pass.
      if (c.odOn) {
        l = odL_.process(l);
        r = odR_.process(r);
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
      }

      // FX loop: authentic default is between the preamp and the power amp.
      if (c.loopPos == 0) {
        delay_.process(l, r);
        reverb_.process(l, r);
      }

      if (!rec.solidState) {
        if (powerOs_ == 4) {
          float a0 = 0.0f, a1 = 0.0f;
          float b0 = 0.0f, b1 = 0.0f;
          os1L_.up(l, a0, a1);
          float p0 = powerL_.process(a0);
          float p1 = powerL_.process(a1);
          float q0 = 0.0f, q1 = 0.0f, q2 = 0.0f, q3 = 0.0f;
          os2L_.up(p0, q0, q1);
          os2L_.up(p1, q2, q3);
          q0 = powerL_.process(q0);
          q1 = powerL_.process(q1);
          q2 = powerL_.process(q2);
          q3 = powerL_.process(q3);
          l = os1L_.down(os2L_.down(q0, q1), os2L_.down(q2, q3));

          os1R_.up(r, a0, a1);
          p0 = powerR_.process(a0);
          p1 = powerR_.process(a1);
          os2R_.up(p0, q0, q1);
          os2R_.up(p1, q2, q3);
          q0 = powerR_.process(q0);
          q1 = powerR_.process(q1);
          q2 = powerR_.process(q2);
          q3 = powerR_.process(q3);
          r = os1R_.down(os2R_.down(q0, q1), os2R_.down(q2, q3));
        } else if (powerOs_ == 2) {
          float a0 = 0.0f, a1 = 0.0f;
          os1L_.up(l, a0, a1);
          a0 = powerL_.process(a0);
          a1 = powerL_.process(a1);
          l = os1L_.down(a0, a1);
          os1R_.up(r, a0, a1);
          a0 = powerR_.process(a0);
          a1 = powerR_.process(a1);
          r = os1R_.down(a0, a1);
        } else {
          l = powerL_.process(l);
          r = powerR_.process(r);
        }
      }

      l = presenceL_.process(l);
      r = presenceR_.process(r);
      l = resonanceL_.process(l);
      r = resonanceR_.process(r);
      l *= master;
      r *= master;

      // Cab: factory IR convolution when available, fixed speaker filter as
      // a fallback for builds without generated assets.
      if (!c.cabBypass) {
        l = cabHpL_.process(l);
        r = cabHpR_.process(r);
        if (cabConvolver_.active()) {
          cabConvolver_.process(l, r);
          l = cabLpL_.process(l);
          r = cabLpR_.process(r);
        } else {
          l = cabLpL_.process(l);
          r = cabLpR_.process(r);
        }
        l *= cabLevel;
        r *= cabLevel;
      }

      // Post-cab loop option.
      if (c.loopPos == 1) {
        delay_.process(l, r);
        reverb_.process(l, r);
      }

      const float out = outputSmooth_.next();
      const float outGain = dbToGain((out - 0.5f) * 24.0f);
      l *= enabled * outGain;
      r *= enabled * outGain;

      // Brief zero-crossfade when the model changes: fade out, swap the
      // recipe at silence, then fade in.  Ten milliseconds each way avoids
      // clicks without a second full engine.
      if (fadeDir_ < 0) {
        switchGain_ -= switchStep_;
        if (switchGain_ <= 0.0f) {
          switchGain_ = 0.0f;
          if (pendingModel_ >= 0) {
            lastModel_ = pendingModel_;
            prepareRecipe(pendingModel_);
            pendingModel_ = -1;
          }
          fadeDir_ = 1;
        }
      } else if (fadeDir_ > 0) {
        switchGain_ += switchStep_;
        if (switchGain_ >= 1.0f) {
          switchGain_ = 1.0f;
          fadeDir_ = 0;
        }
      }
      l *= switchGain_;
      r *= switchGain_;

      if (c.limiterOn) {
        const float thresh = dbToGain(-24.0f + 24.0f * c.limiterThreshold);
        const float peakNow = std::max(std::fabs(l), std::fabs(r));
        const float target = peakNow > thresh ? thresh / peakNow : 1.0f;
        const float coeff =
            target < limiterGain_ ? limiterAttack_ : limiterRelease_;
        limiterGain_ = coeff * limiterGain_ + (1.0f - coeff) * target;
        l = std::tanh(l * limiterGain_);
        r = std::tanh(r * limiterGain_);
      }

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
    meters.bpm = hostTempo_ ? bpm_ : c.bpm;
    meters.hostTempo = hostTempo_;
  }

  void reset() noexcept {
    for (auto& s : stages_) {
      s.reset();
    }
    toneL_.reset();
    toneR_.reset();
    odL_.reset();
    odR_.reset();
    cabConvolver_.reset();
    delay_.reset();
    reverb_.reset();
    cabHpL_.reset();
    cabHpR_.reset();
    cabLpL_.reset();
    cabLpR_.reset();
    powerL_.reset();
    powerR_.reset();
    os1L_.reset();
    os1R_.reset();
    os2L_.reset();
    os2R_.reset();
    presenceL_.reset();
    presenceR_.reset();
    resonanceL_.reset();
    resonanceR_.reset();
    vuPower_ = 0.0f;
    limiterGain_ = 1.0f;
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
    const float presenceDb = (c.presence - 0.5f) * 10.0f;
    const float resonanceDb = (c.resonance - 0.5f) * 12.0f;
    presenceL_.highShelf(3200.0f, presenceDb, 0.7f, sr_);
    presenceR_.highShelf(3200.0f, presenceDb, 0.7f, sr_);
    resonanceL_.lowShelf(120.0f, resonanceDb, 0.7f, sr_);
    resonanceR_.lowShelf(120.0f, resonanceDb, 0.7f, sr_);
  }

  void syncUserIrBank() noexcept {
    auto bank = std::atomic_load(&userBankPending_);
    if (bank != activeUserBank_) {
      activeUserBank_ = std::move(bank);
    }
  }

  void updateCabIR() noexcept {
    // While fading down for a model switch, keep the old model's cab until
    // the recipe swap at silence.
    const int cabModel =
        (fadeDir_ == -1 && lastModel_ >= 0) ? lastModel_ : controls_.model;
    int set = cabModel;
    if (cabModel == 4 && controls_.cab == 1) {
      set = CabBank::Recto;
    }
    if (cabModel == 6) {
      set = CabBank::Recto;
    }
    const CabIR* ir = nullptr;
    if (controls_.cab == 2 && activeUserBank_) {
      const int model = std::min(std::max(cabModel, 0), kAmpCount - 1);
      const int index = controls_.userIr[model];
      if (index > 0 &&
          index <= static_cast<int>(activeUserBank_->entries.size())) {
        const CabIR* candidate =
            &activeUserBank_->entries[static_cast<size_t>(index - 1)].ir;
        if (!candidate->h.empty()) {
          ir = candidate;
        }
      }
    }
    int mic = controls_.cabMic;
    if (!ir) {
      ir = cabBank_.get(set, mic);
    }
    if (!ir) {
      ir = cabBank_.get(set, 0);
    }
    if (ir != activeCab_) {
      activeCab_ = ir;
      cabConvolver_.setIR(ir);
    }
  }

  void updateCab(const Controls& c) noexcept {
    cabHpL_.set(cabLowCutHz(c.cabLowCut), sr_);
    cabHpR_.set(cabLowCutHz(c.cabLowCut), sr_);
    cabLpL_.lowPass(cabHighCutHz(c.cabHighCut), 0.707f, sr_);
    cabLpR_.lowPass(cabHighCutHz(c.cabHighCut), 0.707f, sr_);
  }

  float sr_ = 48000.0f;
  Controls controls_;
  int lastModel_ = -1;
  int active_ = -1;
  int pendingModel_ = -1;
  int fadeDir_ = 0;
  float switchGain_ = 1.0f;
  float switchStep_ = 1.0f;
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
  Ts9 odL_;
  Ts9 odR_;
  CabBank cabBank_;
  CabConvolver cabConvolver_;
  const CabIR* activeCab_ = nullptr;
  std::shared_ptr<const UserIrBank> userBankPending_;
  std::shared_ptr<const UserIrBank> activeUserBank_;
  Delay delay_;
  Reverb reverb_;
  HighPass2 cabHpL_;
  HighPass2 cabHpR_;
  Biquad cabLpL_;
  Biquad cabLpR_;
  PowerAmp powerL_;
  PowerAmp powerR_;
  Oversampler2x os1L_;
  Oversampler2x os1R_;
  Oversampler2x os2L_;
  Oversampler2x os2R_;
  int powerOs_ = 0;
  Biquad presenceL_;
  Biquad presenceR_;
  Biquad resonanceL_;
  Biquad resonanceR_;
  float limiterGain_ = 1.0f;
  float limiterAttack_ = 0.0f;
  float limiterRelease_ = 0.0f;
};

} // namespace amp
