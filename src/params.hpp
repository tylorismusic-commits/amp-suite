// Amp Suite — params.hpp
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstddef>
#include <cstdint>

#include "dsp/controls.hpp"

namespace amp {

// Parameter ids are the CLAP parameter ids and the index into Shared::values.
// They are also the ordering of the LV2 control ports (port = 4 + id), so this
// enum must only ever be appended to.
enum ParamId : uint32_t {
  P_MODEL = 0,
  P_POWER,
  P_INPUT,
  P_GAIN,
  P_BASS,
  P_MID,
  P_TREBLE,
  P_PRESENCE,
  P_RESONANCE,
  P_MASTER,
  P_OUTPUT,
  P_BRIGHT,
  P_CUT,
  P_VOICE,
  P_RECTIFIER,
  P_CRUNCH,
  P_POWER_SCALE,
  P_CAB,
  P_CAB_MIC,
  P_CAB_LEVEL,
  P_CAB_LOWCUT,
  P_CAB_HIGHCUT,
  P_CAB_BYPASS,
  P_OD_ON,
  P_OD_DRIVE,
  P_OD_TONE,
  P_OD_LEVEL,
  P_DELAY_ON,
  P_DELAY_SYNC,
  P_DELAY_DIV,
  P_DELAY_TIME,
  P_DELAY_FEEDBACK,
  P_DELAY_MIX,
  P_DELAY_TONE,
  P_DELAY_MOD,
  P_DELAY_PINGPONG,
  P_REVERB_ON,
  P_REVERB_MODE,
  P_REVERB_MIX,
  P_REVERB_DECAY,
  P_REVERB_TONE,
  P_REVERB_PREDELAY,
  P_HQ,
  P_LOOP_POS,
  P_CHORUS_ON,
  P_CHORUS_RATE,
  P_CHORUS_DEPTH,
  P_METER,
  P_LIMITER_ON,
  P_LIMITER_THRESH,
  P_USER_IR_0,
  P_USER_IR_1,
  P_USER_IR_2,
  P_USER_IR_3,
  P_USER_IR_4,
  P_USER_IR_5,
  P_USER_IR_6,
  P_BPM,
  P_AMP_BYPASS,
  P_NAM_ON,
  P_NAM_MODEL,
  P_NAM_LEVEL,
  P_GATE_ON,
  P_GATE_THRESH,
  P_GATE_DECAY,
  P_CAB_SET,
  P_COUNT
};

// Scale-point labels for a stepped parameter.  Values are always 0..count-1.
struct Labels {
  const char* const* names = nullptr;
  int count = 0;
};

template <size_t N>
constexpr Labels labels(const char* const (&arr)[N]) noexcept {
  return Labels{arr, static_cast<int>(N)};
}

// One table drives the DSP, CLAP params, editor and LV2 metadata (via
// tools/gen_ttl.cpp).  `symbol` is the LV2 port symbol and part of the saved
// session format: never rename it.
struct ParamSpec {
  const char* symbol;
  const char* name;
  const char* module;
  double min;
  double max;
  double def;
  bool stepped;
  bool hidden;
  bool toggled;
  const char* unit;
  Labels labels;
};

inline constexpr const char* kModelLabels[] = {
    "Chorus Clean", "Chime", "Crunch", "Hot Rod",
    "Block 120",   "Stealth", "Recto"};
inline constexpr const char* kCabLabels[] = {"Default", "Custom", "User"};
inline constexpr const char* kCabSetLabels[] = {
    "Auto", "Chorus 2×12", "Alnico 2×12", "V30 4×12",
    "G12T-75 4×12", "1200 4×12", "G12-EVH 4×12",
    "OS V30 4×12", "Greenback 4×12"};
inline constexpr const char* kCabMicLabels[] = {"Dynamic", "Ribbon", "Blend"};
inline constexpr const char* kDelaySyncLabels[] = {"Free", "Host"};
inline constexpr const char* kDelayDivLabels[] = {
    "1/1", "1/2", "1/4", "1/8", "1/8T", "1/16", "1/16T", "1/32", "1/8D"};
inline constexpr const char* kReverbModeLabels[] = {"Plate", "Spring"};
inline constexpr const char* kLoopLabels[] = {"Pre", "Post"};
inline constexpr const char* kMeterLabels[] = {"In/Out", "Off"};
inline constexpr const char* kPowerScaleLabels[] = {"50", "100"};
inline constexpr const char* kRectifierLabels[] = {"Si", "Tube"};
inline constexpr const char* kVoiceLabels[] = {"A", "B", "C"};

inline constexpr ParamSpec kParamSpecs[P_COUNT] = {
    {"model", "Model", "Amp", 0.0, 6.0, 4.0, true, false, false, nullptr,
     labels(kModelLabels)},
    {"power", "Power", "Amp", 0.0, 1.0, 1.0, true, false, true, nullptr, {}},
    {"input", "Input", "Amp", 0.0, 1.0, 0.5, false, false, false, nullptr, {}},
    {"gain", "Gain", "Amp", 0.0, 1.0, 0.55, false, false, false, nullptr, {}},
    {"bass", "Bass", "Amp", 0.0, 1.0, 0.5, false, false, false, nullptr, {}},
    {"mid", "Mid", "Amp", 0.0, 1.0, 0.45, false, false, false, nullptr, {}},
    {"treble", "Treble", "Amp", 0.0, 1.0, 0.55, false, false, false, nullptr,
     {}},
    {"presence", "Presence", "Amp", 0.0, 1.0, 0.5, false, false, false,
     nullptr, {}},
    {"resonance", "Resonance", "Amp", 0.0, 1.0, 0.5, false, false, false,
     nullptr, {}},
    {"master", "Master", "Amp", 0.0, 1.0, 0.45, false, false, false, nullptr,
     {}},
    {"output", "Output", "Amp", 0.0, 1.0, 0.5, false, false, false, nullptr,
     {}},
    {"bright", "Bright", "Amp", 0.0, 1.0, 0.0, true, false, true, nullptr, {}},
    {"cut", "Cut", "Amp", 0.0, 1.0, 0.5, false, false, false, nullptr, {}},
    {"voice", "Voice", "Amp", 0.0, 2.0, 0.0, true, false, false, nullptr,
     labels(kVoiceLabels)},
    {"rectifier", "Rectifier", "Amp", 0.0, 1.0, 0.0, true, false, false,
     nullptr, labels(kRectifierLabels)},
    {"crunch", "Crunch", "Amp", 0.0, 1.0, 1.0, true, false, true, nullptr, {}},
    {"power_scale", "Power Scale", "Amp", 0.0, 1.0, 0.0, true, false, false,
     nullptr, labels(kPowerScaleLabels)},
    {"cab", "Cabinet", "Cab", 0.0, 2.0, 0.0, true, false, false, nullptr,
     labels(kCabLabels)},
    {"cab_mic", "Cab Mic", "Cab", 0.0, 2.0, 0.0, true, false, false, nullptr,
     labels(kCabMicLabels)},
    {"cab_level", "Cab Level", "Cab", 0.0, 1.0, 0.6, false, false, false,
     nullptr, {}},
    {"cab_lowcut", "Cab Low Cut", "Cab", 0.0, 1.0, 0.0, false, false, false,
     nullptr, {}},
    {"cab_highcut", "Cab High Cut", "Cab", 0.0, 1.0, 1.0, false, false, false,
     nullptr, {}},
    {"cab_bypass", "Cab Bypass", "Cab", 0.0, 1.0, 0.0, true, false, true,
     nullptr, {}},
    {"od_on", "808 On", "808", 0.0, 1.0, 0.0, true, false, true, nullptr, {}},
    {"od_drive", "808 Drive", "808", 0.0, 1.0, 0.50, false, false, false,
     nullptr, {}},
    {"od_tone", "808 Tone", "808", 0.0, 1.0, 0.5, false, false, false,
     nullptr, {}},
    {"od_level", "808 Level", "808", 0.0, 1.0, 0.50, false, false, false,
     nullptr, {}},
    {"delay_on", "Delay On", "Delay", 0.0, 1.0, 0.0, true, false, true,
     nullptr, {}},
    {"delay_sync", "Delay Sync", "Delay", 0.0, 1.0, 1.0, true, false, true,
     nullptr, labels(kDelaySyncLabels)},
    {"delay_div", "Delay Division", "Delay", 0.0, 8.0, 8.0, true, false,
     false, nullptr, labels(kDelayDivLabels)},
    {"delay_time", "Delay Time", "Delay", 0.0, 1.0, 0.18, false, false, false,
     nullptr, {}},
    {"delay_feedback", "Delay Feedback", "Delay", 0.0, 1.0, 0.3, false, false,
     false, nullptr, {}},
    {"delay_mix", "Delay Mix", "Delay", 0.0, 1.0, 0.25, false, false, false,
     nullptr, {}},
    {"delay_tone", "Delay Tone", "Delay", 0.0, 1.0, 0.5, false, false, false,
     nullptr, {}},
    {"delay_mod", "Delay Mod", "Delay", 0.0, 1.0, 0.2, false, false, false,
     nullptr, {}},
    {"delay_pingpong", "Delay Ping-Pong", "Delay", 0.0, 1.0, 0.0, true, false,
     true, nullptr, {}},
    {"reverb_on", "Reverb On", "Reverb", 0.0, 1.0, 0.0, true, false, true,
     nullptr, {}},
    {"reverb_mode", "Reverb Mode", "Reverb", 0.0, 1.0, 0.0, true, false, false,
     nullptr, labels(kReverbModeLabels)},
    {"reverb_mix", "Reverb Mix", "Reverb", 0.0, 1.0, 0.2, false, false, false,
     nullptr, {}},
    {"reverb_decay", "Reverb Decay", "Reverb", 0.0, 1.0, 0.4, false, false,
     false, nullptr, {}},
    {"reverb_tone", "Reverb Tone", "Reverb", 0.0, 1.0, 0.5, false, false,
     false, nullptr, {}},
    {"reverb_predelay", "Reverb Pre-delay", "Reverb", 0.0, 1.0, 0.1, false,
     false, false, nullptr, {}},
    {"hq", "HQ", "Global", 0.0, 1.0, 1.0, true, false, true, nullptr, {}},
    {"loop_pos", "Loop Position", "Global", 0.0, 1.0, 0.0, true, false, false,
     nullptr, labels(kLoopLabels)},
    {"chorus_on", "Chorus On", "Amp", 0.0, 1.0, 1.0, true, false, true, nullptr,
     {}},
    {"chorus_rate", "Chorus Rate", "Amp", 0.0, 1.0, 0.35, false, false, false,
     nullptr, {}},
    {"chorus_depth", "Chorus Depth", "Amp", 0.0, 1.0, 0.5, false, false, false,
     nullptr, {}},
    {"meter", "Meter", "Meter", 0.0, 1.0, 0.0, true, true, false, nullptr,
     labels(kMeterLabels)},
    {"limiter_on", "Limiter", "Limiter", 0.0, 1.0, 0.0, true, false, true,
     nullptr, {}},
    {"limiter_threshold", "Limiter Threshold", "Limiter", 0.0, 1.0, 1.0,
     false, false, false, nullptr, {}},
    {"user_ir_chorus", "Chorus Clean User IR", "Cab", 0.0, 63.0, 0.0, true,
     false, false, nullptr, {}},
    {"user_ir_chime", "Chime User IR", "Cab", 0.0, 63.0, 0.0, true, false,
     false, nullptr, {}},
    {"user_ir_crunch", "Crunch User IR", "Cab", 0.0, 63.0, 0.0, true, false,
     false, nullptr, {}},
    {"user_ir_hot_rod", "Hot Rod User IR", "Cab", 0.0, 63.0, 0.0, true, false,
     false, nullptr, {}},
    {"user_ir_block_120", "Block 120 User IR", "Cab", 0.0, 63.0, 0.0, true,
     false, false, nullptr, {}},
    {"user_ir_stealth", "Stealth User IR", "Cab", 0.0, 63.0, 0.0, true, false,
     false, nullptr, {}},
    {"user_ir_recto", "Recto User IR", "Cab", 0.0, 63.0, 0.0, true, false,
     false, nullptr, {}},
    {"tempo", "Delay Tempo", "Delay", 40.0, 300.0, 120.0, false, false, false,
     "bpm", {}},
    {"amp_bypass", "Amp Bypass", "Amp", 0.0, 1.0, 0.0, true, false, true, nullptr,
     {}},
    {"nam_on", "NAM On", "NAM", 0.0, 1.0, 0.0, true, false, true, nullptr, {}},
    {"nam_model", "NAM Model", "NAM", 0.0, 63.0, 0.0, true, false, false,
     nullptr, {}},
    {"nam_level", "NAM Level", "NAM", 0.0, 1.0, 0.5, false, false, false,
     nullptr, {}},
    {"gate_on", "Gate On", "Gate", 0.0, 1.0, 0.0, true, false, true, nullptr, {}},
    {"gate_thresh", "Gate Threshold", "Gate", 0.0, 1.0, 0.55, false, false,
     false, nullptr, {}},
    {"gate_decay", "Gate Decay", "Gate", 0.0, 1.0, 0.4, false, false, false,
     nullptr, {}},
    {"cab_set", "Factory Cab", "Cab", 0.0, 8.0, 0.0, true, false, false, nullptr,
     labels(kCabSetLabels)},
};

static_assert(sizeof(kParamSpecs) / sizeof(kParamSpecs[0]) == P_COUNT,
              "kParamSpecs must cover every parameter id");

// The control values, meters and dirty flags shared between the audio thread,
// the host thread and the editor.  Everything is atomic so the editor can run
// on the GUI thread without locks.
class Shared {
public:
  Shared() {
    for (uint32_t p = 0; p < P_COUNT; ++p) {
      values_[p].store(kParamSpecs[p].def, std::memory_order_relaxed);
      dirty_[p].store(false, std::memory_order_relaxed);
    }
    inPeakDb.store(-120.0f, std::memory_order_relaxed);
    outPeakDb.store(-120.0f, std::memory_order_relaxed);
    inPeakLDb.store(-120.0f, std::memory_order_relaxed);
    inPeakRDb.store(-120.0f, std::memory_order_relaxed);
    outPeakLDb.store(-120.0f, std::memory_order_relaxed);
    outPeakRDb.store(-120.0f, std::memory_order_relaxed);
    outVuDb.store(-120.0f, std::memory_order_relaxed);
    bpm.store(120.0f, std::memory_order_relaxed);
    hostTempo.store(false, std::memory_order_relaxed);
    meterSerial.store(0, std::memory_order_relaxed);
    tunerOn.store(false, std::memory_order_relaxed);
    tunerHz.store(0.0f, std::memory_order_relaxed);
    tunerCents.store(0.0f, std::memory_order_relaxed);
    tunerConfidence.store(0.0f, std::memory_order_relaxed);
    tunerNote.store(-1, std::memory_order_relaxed);
  }

  double get(uint32_t p) const noexcept {
    return values_[p].load(std::memory_order_relaxed);
  }

  // Clamps to the parameter range and marks the value for the host.
  void set(uint32_t p, double v) noexcept {
    if (!std::isfinite(v)) {
      return;
    }
    const ParamSpec& s = kParamSpecs[p];
    v = std::min(std::max(v, s.min), s.max);
    if (s.stepped) {
      v = static_cast<double>(static_cast<int>(v + (v >= 0 ? 0.5 : -0.5)));
    }
    values_[p].store(v, std::memory_order_relaxed);
    dirty_[p].store(true, std::memory_order_release);
  }

  // Stores a value that came from the host, without marking it dirty.
  void setFromHost(uint32_t p, double v) noexcept {
    if (!std::isfinite(v)) {
      return;
    }
    const ParamSpec& s = kParamSpecs[p];
    v = std::min(std::max(v, s.min), s.max);
    values_[p].store(v, std::memory_order_relaxed);
  }

  bool takeDirty(uint32_t p) noexcept {
    return dirty_[p].exchange(false, std::memory_order_acquire);
  }

  // A preset or host-state load replaces many controls at once.  The audio
  // thread turns this into a full engine switch so the running amp never
  // steps its gain/tone coefficients in place.
  void requestReload() noexcept {
    reload_.store(true, std::memory_order_release);
  }
  bool takeReload() noexcept {
    return reload_.exchange(false, std::memory_order_acquire);
  }

  bool isDirty(uint32_t p) const noexcept {
    return dirty_[p].load(std::memory_order_acquire);
  }

  bool anyDirty() const noexcept {
    for (uint32_t p = 0; p < P_COUNT; ++p) {
      if (dirty_[p].load(std::memory_order_acquire)) {
        return true;
      }
    }
    return false;
  }

  std::atomic<float> inPeakDb;
  std::atomic<float> outPeakDb;
  std::atomic<float> inPeakLDb;
  std::atomic<float> inPeakRDb;
  std::atomic<float> outPeakLDb;
  std::atomic<float> outPeakRDb;
  std::atomic<float> outVuDb;
  std::atomic<float> bpm;
  std::atomic<bool> hostTempo;
  std::atomic<uint32_t> meterSerial;
  std::atomic<bool> tunerOn;
  std::atomic<float> tunerHz;
  std::atomic<float> tunerCents;
  std::atomic<float> tunerConfidence;
  std::atomic<int> tunerNote;

private:
  std::atomic<double> values_[P_COUNT];
  std::atomic<bool> dirty_[P_COUNT];
  std::atomic<bool> reload_{false};
};

// Maps the host-facing parameter values to the DSP controls.
inline Controls controlsFrom(const Shared& s) {
  Controls c;
  c.model = static_cast<int>(s.get(P_MODEL) + 0.5);
  c.enabled = s.get(P_POWER) > 0.5;
  c.input = static_cast<float>(s.get(P_INPUT));
  c.gain = static_cast<float>(s.get(P_GAIN));
  c.bass = static_cast<float>(s.get(P_BASS));
  c.mid = static_cast<float>(s.get(P_MID));
  c.treble = static_cast<float>(s.get(P_TREBLE));
  c.presence = static_cast<float>(s.get(P_PRESENCE));
  c.resonance = static_cast<float>(s.get(P_RESONANCE));
  c.master = static_cast<float>(s.get(P_MASTER));
  c.output = static_cast<float>(s.get(P_OUTPUT));
  c.bright = s.get(P_BRIGHT) > 0.5;
  c.chorusOn = s.get(P_CHORUS_ON) > 0.5;
  c.chorusRate = static_cast<float>(s.get(P_CHORUS_RATE));
  c.chorusDepth = static_cast<float>(s.get(P_CHORUS_DEPTH));
  c.cut = static_cast<float>(s.get(P_CUT));
  c.voice = static_cast<int>(s.get(P_VOICE) + 0.5);
  c.rectifier = static_cast<int>(s.get(P_RECTIFIER) + 0.5);
  c.crunch = s.get(P_CRUNCH) > 0.5;
  c.powerScale = static_cast<int>(s.get(P_POWER_SCALE) + 0.5);
  c.cab = static_cast<int>(s.get(P_CAB) + 0.5);
  c.cabSet = static_cast<int>(s.get(P_CAB_SET) + 0.5);
  c.cabMic = static_cast<int>(s.get(P_CAB_MIC) + 0.5);
  c.cabLevel = static_cast<float>(s.get(P_CAB_LEVEL));
  c.cabLowCut = static_cast<float>(s.get(P_CAB_LOWCUT));
  c.cabHighCut = static_cast<float>(s.get(P_CAB_HIGHCUT));
  c.cabBypass = s.get(P_CAB_BYPASS) > 0.5;
  c.tunerOn = s.tunerOn.load(std::memory_order_relaxed);
  c.odOn = s.get(P_OD_ON) > 0.5;
  c.odDrive = static_cast<float>(s.get(P_OD_DRIVE));
  c.odTone = static_cast<float>(s.get(P_OD_TONE));
  c.odLevel = static_cast<float>(s.get(P_OD_LEVEL));
  c.delayOn = s.get(P_DELAY_ON) > 0.5;
  c.delaySync = s.get(P_DELAY_SYNC) > 0.5;
  c.delayDiv = static_cast<int>(s.get(P_DELAY_DIV) + 0.5);
  c.delayTime = static_cast<float>(s.get(P_DELAY_TIME));
  c.delayFeedback = static_cast<float>(s.get(P_DELAY_FEEDBACK));
  c.delayMix = static_cast<float>(s.get(P_DELAY_MIX));
  c.delayTone = static_cast<float>(s.get(P_DELAY_TONE));
  c.delayMod = static_cast<float>(s.get(P_DELAY_MOD));
  c.delayPingPong = s.get(P_DELAY_PINGPONG) > 0.5;
  c.reverbOn = s.get(P_REVERB_ON) > 0.5;
  c.reverbMode = static_cast<int>(s.get(P_REVERB_MODE) + 0.5);
  c.reverbMix = static_cast<float>(s.get(P_REVERB_MIX));
  c.reverbDecay = static_cast<float>(s.get(P_REVERB_DECAY));
  c.reverbTone = static_cast<float>(s.get(P_REVERB_TONE));
  c.reverbPreDelay = static_cast<float>(s.get(P_REVERB_PREDELAY));
  c.hq = s.get(P_HQ) > 0.5;
  c.loopPos = static_cast<int>(s.get(P_LOOP_POS) + 0.5);
  c.limiterOn = s.get(P_LIMITER_ON) > 0.5;
  c.limiterThreshold = static_cast<float>(s.get(P_LIMITER_THRESH));
  for (int i = 0; i < 7; ++i) {
    c.userIr[i] = static_cast<int>(s.get(P_USER_IR_0 + i) + 0.5);
  }
  c.bpm = static_cast<float>(s.get(P_BPM));
  c.ampBypass = s.get(P_AMP_BYPASS) > 0.5;
  c.namOn = s.get(P_NAM_ON) > 0.5;
  c.namModel = static_cast<int>(s.get(P_NAM_MODEL) + 0.5);
  c.namLevel = static_cast<float>(s.get(P_NAM_LEVEL));
  c.gateOn = s.get(P_GATE_ON) > 0.5;
  c.gateThreshold = static_cast<float>(s.get(P_GATE_THRESH));
  c.gateDecay = static_cast<float>(s.get(P_GATE_DECAY));
  return c;
}

// --- Display helpers -------------------------------------------------------

inline float delayTimeMs(float v) noexcept {
  return 20.0f + v * v * 1980.0f;
}

inline float delayDivisionBeats(int index) noexcept {
  static const float kBeats[9] = {4.0f, 2.0f, 1.0f, 0.5f, 1.0f / 3.0f,
                                  0.25f, 1.0f / 6.0f, 0.125f, 0.75f};
  return kBeats[std::min(std::max(index, 0), 8)];
}

inline float cabLowCutHz(float v) noexcept {
  return 20.0f * std::pow(15.0f, v);
}

inline float cabHighCutHz(float v) noexcept {
  return 2000.0f * std::pow(10.0f, v);
}

inline float cabLevelDb(float v) noexcept {
  return -24.0f + 30.0f * v;
}

} // namespace amp
