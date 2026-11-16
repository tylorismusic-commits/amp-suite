// Amp Suite — controls.hpp
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

namespace amp {

// Display order of the amp tabs.  The model value is part of the saved-session
// format: only ever append here, never renumber.
enum class Model : int {
  ChorusClean = 0,  // solid-state clean with BBD chorus
  Chime = 1,        // AC30 Top Boost
  Crunch = 2,       // Rockerverb
  HotRod = 3,       // Dookie-mod 1959 Super Lead
  Block120 = 4,     // Peavey 5150 block letter
  Stealth = 5,      // EVH 5150III 50S Stealth EL34
  Recto = 6,        // Dual Rectifier
  Count = 7
};

// Every host control is normalised 0..1 unless the parameter table says
// otherwise.  Model-specific switches live in shared slots and are shown only
// on the models that have them.
struct Controls {
  int model = static_cast<int>(Model::Block120);
  bool enabled = true;
  float input = 0.5f;       // input trim
  float gain = 0.55f;       // preamp gain / drive
  float bass = 0.5f;
  float mid = 0.45f;
  float treble = 0.55f;
  float presence = 0.5f;
  float resonance = 0.5f;
  float master = 0.45f;
  float output = 0.5f;
  bool ampBypass = false;   // skip preamp, tone stack and power amp
  bool namOn = false;       // use a loaded NAM model in place of the amp
  int namModel = 0;         // 1-based NAM library index
  float namLevel = 0.5f;    // -12..+12 dB trim around the model normalization

  // Model-specific switches, in fixed slots.
  bool bright = false;      // Chorus Clean
  bool chorusOn = true;     // Chorus Clean
  float chorusRate = 0.35f;
  float chorusDepth = 0.5f;
  float cut = 0.5f;         // Chime
  int voice = 0;            // channel / voicing where a model has one
  int rectifier = 0;        // Recto: 0 silicon, 1 tube
  bool crunch = true;       // Block 120
  int powerScale = 0;       // Crunch: 0 = 50, 1 = 100

  // Cab.
  int cab = 0;              // 0 factory default, 1 alternate, 2 user
  int cabMic = 0;           // 0 dynamic, 1 ribbon, 2 blend
  int userIr[7] = {0, 0, 0, 0, 0, 0, 0};  // 1-based user library index
  float cabLevel = 0.6f;    // mapped to -24..+6 dB
  float cabLowCut = 0.0f;   // mapped to 20..300 Hz
  float cabHighCut = 1.0f;  // mapped to 2k..20k Hz
  bool cabBypass = false;

  // 808.
  bool odOn = true;
  float odDrive = 0.28f;
  float odTone = 0.5f;
  float odLevel = 0.55f;

  // Delay.
  bool delayOn = false;
  bool delaySync = true;
  int delayDiv = 8;         // index into the division table
  float delayTime = 0.18f;  // free mode, mapped to 20..2000 ms
  float delayFeedback = 0.3f;
  float delayMix = 0.25f;
  float delayTone = 0.5f;
  float delayMod = 0.2f;
  bool delayPingPong = false;
  float bpm = 120.0f;       // manual tempo when the host has no transport

  // Reverb.
  bool reverbOn = false;
  int reverbMode = 0;       // 0 plate, 1 spring
  float reverbMix = 0.2f;
  float reverbDecay = 0.4f;
  float reverbTone = 0.5f;
  float reverbPreDelay = 0.1f;

  // Global.
  bool hq = true;
  int loopPos = 0;          // 0 pre-power, 1 post-cab
  bool limiterOn = false;
  float limiterThreshold = 1.0f;  // 1.0 = 0 dB
};

// Metering published by the engine, in dB.
struct Meters {
  float inPeakDb = -120.0f;
  float outPeakDb = -120.0f;
  float inPeakLDb = -120.0f;
  float inPeakRDb = -120.0f;
  float outPeakLDb = -120.0f;
  float outPeakRDb = -120.0f;
  float outVuDb = -120.0f;
  float bpm = 120.0f;
  bool hostTempo = false;
};

} // namespace amp
