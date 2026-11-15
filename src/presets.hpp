// Amp Suite — factory presets (scaffold set)
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <cstdint>

#include "params.hpp"

namespace amp {

// A preset sets the parameters it cares about; -1 means "leave as is".
struct PresetValue {
  int16_t param = -1;
  float value = 0.0f;
};

struct Preset {
  const char* name;
  const char* category;
  int model;
  PresetValue values[16];
};

inline constexpr Preset kPresets[] = {
    {"Block 120 — Rhythm", "Rhythm", 4,
     {{P_GAIN, 0.55f}, {P_BASS, 0.50f}, {P_MID, 0.45f}, {P_TREBLE, 0.55f},
      {P_PRESENCE, 0.50f}, {P_RESONANCE, 0.50f}, {P_MASTER, 0.45f},
      {P_OUTPUT, 0.50f}, {P_OD_ON, 1.0f}, {P_OD_DRIVE, 0.28f},
      {P_OD_TONE, 0.50f}, {P_OD_LEVEL, 0.55f}, {P_DELAY_ON, 0.0f},
      {P_REVERB_ON, 0.0f}, {P_CAB, 0.0f}, {-1, 0.0f}}},
    {"Block 120 — OS", "Lead", 4,
     {{P_GAIN, 0.62f}, {P_BASS, 0.52f}, {P_MID, 0.42f}, {P_TREBLE, 0.58f},
      {P_PRESENCE, 0.55f}, {P_RESONANCE, 0.45f}, {P_MASTER, 0.42f},
      {P_OUTPUT, 0.50f}, {P_OD_ON, 1.0f}, {P_OD_DRIVE, 0.35f},
      {P_OD_TONE, 0.52f}, {P_OD_LEVEL, 0.55f}, {P_CAB, 1.0f},
      {P_DELAY_ON, 0.0f}, {P_REVERB_ON, 0.0f}, {-1, 0.0f}}},
    {"Chorus Clean", "Clean", 0,
     {{P_GAIN, 0.45f}, {P_BASS, 0.50f}, {P_MID, 0.50f}, {P_TREBLE, 0.60f},
      {P_MASTER, 0.50f}, {P_BRIGHT, 0.0f}, {P_OD_ON, 0.0f},
      {P_DELAY_ON, 0.0f}, {P_REVERB_ON, 0.0f}, {-1, 0.0f}}},
    {"Chime", "Crunch", 1,
     {{P_GAIN, 0.58f}, {P_BASS, 0.45f}, {P_MID, 0.60f}, {P_TREBLE, 0.58f},
      {P_CUT, 0.45f}, {P_MASTER, 0.50f}, {P_OD_ON, 0.0f},
      {P_DELAY_ON, 0.0f}, {P_REVERB_ON, 0.0f}, {-1, 0.0f}}},
    {"Crunch", "Crunch", 2,
     {{P_GAIN, 0.65f}, {P_BASS, 0.60f}, {P_MID, 0.50f}, {P_TREBLE, 0.55f},
      {P_PRESENCE, 0.50f}, {P_MASTER, 0.45f}, {P_OD_ON, 0.0f},
      {P_DELAY_ON, 0.0f}, {P_REVERB_ON, 0.0f}, {-1, 0.0f}}},
    {"Hot Rod", "Crunch", 3,
     {{P_GAIN, 0.70f}, {P_BASS, 0.55f}, {P_MID, 0.48f}, {P_TREBLE, 0.52f},
      {P_PRESENCE, 0.55f}, {P_MASTER, 0.45f}, {P_OD_ON, 1.0f},
      {P_OD_DRIVE, 0.20f}, {P_DELAY_ON, 0.0f}, {P_REVERB_ON, 0.0f},
      {-1, 0.0f}}},
    {"Stealth Lead", "Lead", 5,
     {{P_GAIN, 0.68f}, {P_BASS, 0.52f}, {P_MID, 0.48f}, {P_TREBLE, 0.60f},
      {P_PRESENCE, 0.58f}, {P_RESONANCE, 0.45f}, {P_MASTER, 0.42f},
      {P_OD_ON, 1.0f}, {P_OD_DRIVE, 0.30f}, {P_DELAY_ON, 1.0f},
      {P_DELAY_MIX, 0.20f}, {P_REVERB_ON, 1.0f}, {P_REVERB_MIX, 0.15f},
      {-1, 0.0f}}},
    {"Recto Modern", "Lead", 6,
     {{P_GAIN, 0.66f}, {P_BASS, 0.62f}, {P_MID, 0.38f}, {P_TREBLE, 0.58f},
      {P_PRESENCE, 0.55f}, {P_MASTER, 0.40f}, {P_RECTIFIER, 0.0f},
      {P_VOICE, 0.0f}, {P_OD_ON, 1.0f}, {P_OD_DRIVE, 0.25f},
      {P_REVERB_ON, 1.0f}, {P_REVERB_MIX, 0.15f}, {-1, 0.0f}}},
};

constexpr int kPresetCount = static_cast<int>(sizeof(kPresets) / sizeof(kPresets[0]));

} // namespace amp
