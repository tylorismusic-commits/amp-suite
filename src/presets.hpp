// Amp Suite — factory presets
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
  PresetValue values[32];
};

inline constexpr Preset kPresets[] = {
    // --- Block 120 ---------------------------------------------------------
    {"Block 120 — Rhythm", "Rhythm", 4,
     {{P_GAIN, 0.55f}, {P_BASS, 0.56f}, {P_MID, 0.38f}, {P_TREBLE, 0.60f},
      {P_PRESENCE, 0.56f}, {P_RESONANCE, 0.56f}, {P_MASTER, 0.44f},
      {P_OUTPUT, 0.50f}, {P_CRUNCH, 1.0f}, {P_OD_ON, 1.0f},
      {P_OD_DRIVE, 0.30f}, {P_OD_TONE, 0.50f}, {P_OD_LEVEL, 0.55f},
      {P_DELAY_ON, 0.0f}, {P_REVERB_ON, 0.0f}, {P_CAB, 0.0f},
      {P_CAB_MIC, 0.0f}, {P_CAB_LEVEL, 0.60f}, {-1, 0.0f}}},
    {"Block 120 — Crunch", "Rhythm", 4,
     {{P_GAIN, 0.62f}, {P_BASS, 0.52f}, {P_MID, 0.42f}, {P_TREBLE, 0.58f},
      {P_PRESENCE, 0.55f}, {P_RESONANCE, 0.52f}, {P_MASTER, 0.42f},
      {P_OUTPUT, 0.50f}, {P_CRUNCH, 1.0f}, {P_OD_ON, 1.0f},
      {P_OD_DRIVE, 0.30f}, {P_OD_TONE, 0.50f}, {P_OD_LEVEL, 0.55f},
      {P_DELAY_ON, 0.0f}, {P_REVERB_ON, 0.0f}, {P_CAB, 0.0f},
      {P_CAB_MIC, 0.0f}, {P_CAB_LEVEL, 0.60f}, {-1, 0.0f}}},
    {"Block 120 — Lead", "Lead", 4,
     {{P_GAIN, 0.70f}, {P_BASS, 0.55f}, {P_MID, 0.44f}, {P_TREBLE, 0.60f},
      {P_PRESENCE, 0.60f}, {P_RESONANCE, 0.48f}, {P_MASTER, 0.40f},
      {P_OUTPUT, 0.50f}, {P_CRUNCH, 0.0f}, {P_OD_ON, 1.0f},
      {P_OD_DRIVE, 0.35f}, {P_OD_TONE, 0.52f}, {P_OD_LEVEL, 0.55f},
      {P_DELAY_ON, 1.0f}, {P_DELAY_SYNC, 1.0f}, {P_DELAY_DIV, 8.0f},
      {P_DELAY_MIX, 0.18f}, {P_DELAY_FEEDBACK, 0.30f}, {P_DELAY_TONE, 0.55f},
      {P_DELAY_MOD, 0.15f}, {P_REVERB_ON, 1.0f}, {P_REVERB_MODE, 0.0f},
      {P_REVERB_MIX, 0.14f}, {P_REVERB_DECAY, 0.38f}, {P_REVERB_TONE, 0.50f},
      {P_REVERB_PREDELAY, 0.08f}, {P_CAB, 0.0f}, {P_CAB_MIC, 0.0f},
      {P_CAB_LEVEL, 0.60f}, {-1, 0.0f}}},
    {"Block 120 — OS", "Lead", 4,
     {{P_GAIN, 0.68f}, {P_BASS, 0.54f}, {P_MID, 0.43f}, {P_TREBLE, 0.59f},
      {P_PRESENCE, 0.58f}, {P_RESONANCE, 0.50f}, {P_MASTER, 0.40f},
      {P_OUTPUT, 0.50f}, {P_CRUNCH, 0.0f}, {P_OD_ON, 1.0f},
      {P_OD_DRIVE, 0.33f}, {P_OD_TONE, 0.52f}, {P_OD_LEVEL, 0.55f},
      {P_DELAY_ON, 0.0f}, {P_REVERB_ON, 0.0f}, {P_CAB, 1.0f},
      {P_CAB_MIC, 0.0f}, {P_CAB_LEVEL, 0.60f}, {-1, 0.0f}}},

    {"Block 120 — Chug", "Rhythm", 4,
     {{P_GAIN, 0.55f}, {P_BASS, 0.60f}, {P_MID, 0.34f}, {P_TREBLE, 0.62f},
      {P_PRESENCE, 0.60f}, {P_RESONANCE, 0.62f}, {P_MASTER, 0.40f},
      {P_OUTPUT, 0.50f}, {P_CRUNCH, 1.0f}, {P_OD_ON, 1.0f},
      {P_OD_DRIVE, 0.32f}, {P_OD_TONE, 0.50f}, {P_OD_LEVEL, 0.55f},
      {P_DELAY_ON, 0.0f}, {P_REVERB_ON, 0.0f}, {P_CAB, 0.0f},
      {P_CAB_MIC, 0.0f}, {P_CAB_LEVEL, 0.60f}, {-1, 0.0f}}},

    // --- Chorus Clean ------------------------------------------------------
    {"Chorus Clean — Clean", "Clean", 0,
     {{P_GAIN, 0.45f}, {P_BASS, 0.50f}, {P_MID, 0.50f}, {P_TREBLE, 0.60f},
      {P_MASTER, 0.50f}, {P_OUTPUT, 0.50f}, {P_BRIGHT, 0.0f},
      {P_CHORUS_ON, 1.0f}, {P_CHORUS_RATE, 0.35f}, {P_CHORUS_DEPTH, 0.50f},
      {P_OD_ON, 0.0f}, {P_DELAY_ON, 0.0f}, {P_REVERB_ON, 0.0f},
      {P_CAB, 0.0f}, {P_CAB_MIC, 0.0f}, {P_CAB_LEVEL, 0.60f},
      {-1, 0.0f}}},
    {"Chorus Clean — Chorus", "Clean", 0,
     {{P_GAIN, 0.48f}, {P_BASS, 0.48f}, {P_MID, 0.52f}, {P_TREBLE, 0.62f},
      {P_MASTER, 0.50f}, {P_OUTPUT, 0.50f}, {P_BRIGHT, 1.0f},
      {P_CHORUS_ON, 1.0f}, {P_CHORUS_RATE, 0.55f}, {P_CHORUS_DEPTH, 0.70f},
      {P_OD_ON, 0.0f}, {P_DELAY_ON, 0.0f}, {P_REVERB_ON, 0.0f},
      {P_CAB, 0.0f}, {P_CAB_MIC, 0.0f}, {P_CAB_LEVEL, 0.60f},
      {-1, 0.0f}}},
    {"Chorus Clean — Ambient", "Ambient", 0,
     {{P_GAIN, 0.45f}, {P_BASS, 0.52f}, {P_MID, 0.50f}, {P_TREBLE, 0.58f},
      {P_MASTER, 0.50f}, {P_OUTPUT, 0.48f}, {P_BRIGHT, 0.0f},
      {P_CHORUS_ON, 1.0f}, {P_CHORUS_RATE, 0.25f}, {P_CHORUS_DEPTH, 0.80f},
      {P_OD_ON, 0.0f}, {P_DELAY_ON, 1.0f}, {P_DELAY_SYNC, 1.0f},
      {P_DELAY_DIV, 2.0f}, {P_DELAY_MIX, 0.28f}, {P_DELAY_FEEDBACK, 0.45f},
      {P_DELAY_TONE, 0.55f}, {P_DELAY_MOD, 0.30f}, {P_REVERB_ON, 1.0f},
      {P_REVERB_MODE, 0.0f}, {P_REVERB_MIX, 0.30f}, {P_REVERB_DECAY, 0.65f},
      {P_REVERB_TONE, 0.55f}, {P_REVERB_PREDELAY, 0.15f}, {P_CAB, 0.0f},
      {P_CAB_MIC, 0.0f}, {P_CAB_LEVEL, 0.60f}, {-1, 0.0f}}},

    // --- Chime -------------------------------------------------------------
    {"Chime — Edge", "Crunch", 1,
     {{P_GAIN, 0.58f}, {P_BASS, 0.45f}, {P_MID, 0.60f}, {P_TREBLE, 0.58f},
      {P_CUT, 0.45f}, {P_PRESENCE, 0.50f}, {P_MASTER, 0.50f},
      {P_OUTPUT, 0.50f}, {P_OD_ON, 0.0f}, {P_DELAY_ON, 0.0f},
      {P_REVERB_ON, 0.0f}, {P_CAB, 0.0f}, {P_CAB_MIC, 0.0f},
      {P_CAB_LEVEL, 0.60f}, {-1, 0.0f}}},
    {"Chime — Top Boost", "Crunch", 1,
     {{P_GAIN, 0.68f}, {P_BASS, 0.42f}, {P_MID, 0.66f}, {P_TREBLE, 0.62f},
      {P_CUT, 0.35f}, {P_PRESENCE, 0.58f}, {P_MASTER, 0.48f},
      {P_OUTPUT, 0.50f}, {P_OD_ON, 1.0f}, {P_OD_DRIVE, 0.18f},
      {P_OD_TONE, 0.55f}, {P_OD_LEVEL, 0.50f}, {P_DELAY_ON, 0.0f},
      {P_REVERB_ON, 0.0f}, {P_CAB, 0.0f}, {P_CAB_MIC, 0.0f},
      {P_CAB_LEVEL, 0.60f}, {-1, 0.0f}}},

    // --- Crunch ------------------------------------------------------------
    {"Crunch — Dirty", "Crunch", 2,
     {{P_GAIN, 0.65f}, {P_BASS, 0.60f}, {P_MID, 0.50f}, {P_TREBLE, 0.55f},
      {P_PRESENCE, 0.50f}, {P_RESONANCE, 0.50f}, {P_MASTER, 0.45f},
      {P_OUTPUT, 0.50f}, {P_POWER_SCALE, 1.0f}, {P_OD_ON, 0.0f},
      {P_DELAY_ON, 0.0f}, {P_REVERB_ON, 0.0f}, {P_CAB, 0.0f},
      {P_CAB_MIC, 0.0f}, {P_CAB_LEVEL, 0.60f}, {-1, 0.0f}}},
    {"Crunch — Doom", "Doom", 2,
     {{P_GAIN, 0.80f}, {P_BASS, 0.75f}, {P_MID, 0.35f}, {P_TREBLE, 0.45f},
      {P_PRESENCE, 0.45f}, {P_RESONANCE, 0.65f}, {P_MASTER, 0.40f},
      {P_OUTPUT, 0.48f}, {P_POWER_SCALE, 0.0f}, {P_OD_ON, 1.0f},
      {P_OD_DRIVE, 0.20f}, {P_OD_TONE, 0.45f}, {P_OD_LEVEL, 0.55f},
      {P_DELAY_ON, 0.0f}, {P_REVERB_ON, 1.0f}, {P_REVERB_MODE, 0.0f},
      {P_REVERB_MIX, 0.20f}, {P_REVERB_DECAY, 0.45f}, {P_REVERB_TONE, 0.40f},
      {P_REVERB_PREDELAY, 0.05f}, {P_CAB, 0.0f}, {P_CAB_MIC, 0.0f},
      {P_CAB_LEVEL, 0.60f}, {-1, 0.0f}}},

    // --- Hot Rod -----------------------------------------------------------
    {"Hot Rod — Rhythm", "Rhythm", 3,
     {{P_GAIN, 0.68f}, {P_BASS, 0.55f}, {P_MID, 0.48f}, {P_TREBLE, 0.52f},
      {P_PRESENCE, 0.55f}, {P_MASTER, 0.45f}, {P_OUTPUT, 0.50f},
      {P_VOICE, 0.0f}, {P_OD_ON, 1.0f}, {P_OD_DRIVE, 0.18f},
      {P_OD_TONE, 0.50f}, {P_OD_LEVEL, 0.55f}, {P_DELAY_ON, 0.0f},
      {P_REVERB_ON, 0.0f}, {P_CAB, 0.0f}, {P_CAB_MIC, 0.0f},
      {P_CAB_LEVEL, 0.60f}, {-1, 0.0f}}},
    {"Hot Rod — Lead", "Lead", 3,
     {{P_GAIN, 0.76f}, {P_BASS, 0.52f}, {P_MID, 0.50f}, {P_TREBLE, 0.56f},
      {P_PRESENCE, 0.60f}, {P_MASTER, 0.42f}, {P_OUTPUT, 0.50f},
      {P_VOICE, 1.0f}, {P_OD_ON, 1.0f}, {P_OD_DRIVE, 0.28f},
      {P_OD_TONE, 0.52f}, {P_OD_LEVEL, 0.55f}, {P_DELAY_ON, 1.0f},
      {P_DELAY_SYNC, 1.0f}, {P_DELAY_DIV, 8.0f}, {P_DELAY_MIX, 0.16f},
      {P_DELAY_FEEDBACK, 0.28f}, {P_DELAY_TONE, 0.55f}, {P_DELAY_MOD, 0.12f},
      {P_REVERB_ON, 1.0f}, {P_REVERB_MODE, 0.0f}, {P_REVERB_MIX, 0.12f},
      {P_REVERB_DECAY, 0.35f}, {P_REVERB_TONE, 0.50f},
      {P_REVERB_PREDELAY, 0.08f}, {P_CAB, 0.0f}, {P_CAB_MIC, 0.0f},
      {P_CAB_LEVEL, 0.60f}, {-1, 0.0f}}},

    {"Hot Rod — Green", "Crunch", 3,
     {{P_GAIN, 0.66f}, {P_BASS, 0.54f}, {P_MID, 0.52f}, {P_TREBLE, 0.56f},
      {P_PRESENCE, 0.54f}, {P_MASTER, 0.45f}, {P_OUTPUT, 0.50f},
      {P_VOICE, 0.0f}, {P_OD_ON, 1.0f}, {P_OD_DRIVE, 0.16f},
      {P_OD_TONE, 0.52f}, {P_OD_LEVEL, 0.55f}, {P_DELAY_ON, 0.0f},
      {P_REVERB_ON, 0.0f}, {P_CAB, 1.0f}, {P_CAB_MIC, 0.0f},
      {P_CAB_LEVEL, 0.60f}, {-1, 0.0f}}},

    // --- Stealth EL34 ------------------------------------------------------
    {"Stealth — Rhythm", "Rhythm", 5,
     {{P_GAIN, 0.66f}, {P_BASS, 0.50f}, {P_MID, 0.48f}, {P_TREBLE, 0.58f},
      {P_PRESENCE, 0.58f}, {P_RESONANCE, 0.50f}, {P_MASTER, 0.42f},
      {P_OUTPUT, 0.50f}, {P_VOICE, 0.0f}, {P_OD_ON, 1.0f},
      {P_OD_DRIVE, 0.26f}, {P_OD_TONE, 0.52f}, {P_OD_LEVEL, 0.55f},
      {P_DELAY_ON, 0.0f}, {P_REVERB_ON, 0.0f}, {P_CAB, 0.0f},
      {P_CAB_MIC, 0.0f}, {P_CAB_LEVEL, 0.60f}, {-1, 0.0f}}},
    {"Stealth — Lead", "Lead", 5,
     {{P_GAIN, 0.74f}, {P_BASS, 0.52f}, {P_MID, 0.48f}, {P_TREBLE, 0.62f},
      {P_PRESENCE, 0.62f}, {P_RESONANCE, 0.45f}, {P_MASTER, 0.40f},
      {P_OUTPUT, 0.50f}, {P_VOICE, 1.0f}, {P_OD_ON, 1.0f},
      {P_OD_DRIVE, 0.34f}, {P_OD_TONE, 0.54f}, {P_OD_LEVEL, 0.55f},
      {P_DELAY_ON, 1.0f}, {P_DELAY_SYNC, 1.0f}, {P_DELAY_DIV, 8.0f},
      {P_DELAY_MIX, 0.18f}, {P_DELAY_FEEDBACK, 0.32f}, {P_DELAY_TONE, 0.55f},
      {P_DELAY_MOD, 0.18f}, {P_REVERB_ON, 1.0f}, {P_REVERB_MODE, 0.0f},
      {P_REVERB_MIX, 0.14f}, {P_REVERB_DECAY, 0.40f}, {P_REVERB_TONE, 0.50f},
      {P_REVERB_PREDELAY, 0.10f}, {P_CAB, 0.0f}, {P_CAB_MIC, 0.0f},
      {P_CAB_LEVEL, 0.60f}, {-1, 0.0f}}},

    // --- Recto -------------------------------------------------------------
    {"Recto — Modern", "Lead", 6,
     {{P_GAIN, 0.66f}, {P_BASS, 0.62f}, {P_MID, 0.38f}, {P_TREBLE, 0.58f},
      {P_PRESENCE, 0.55f}, {P_RESONANCE, 0.55f}, {P_MASTER, 0.40f},
      {P_OUTPUT, 0.50f}, {P_RECTIFIER, 0.0f}, {P_VOICE, 0.0f},
      {P_OD_ON, 1.0f}, {P_OD_DRIVE, 0.25f}, {P_OD_TONE, 0.52f},
      {P_OD_LEVEL, 0.55f}, {P_DELAY_ON, 0.0f}, {P_REVERB_ON, 0.0f},
      {P_CAB, 0.0f}, {P_CAB_MIC, 0.0f}, {P_CAB_LEVEL, 0.60f},
      {-1, 0.0f}}},
    {"Recto — Raw", "Crunch", 6,
     {{P_GAIN, 0.55f}, {P_BASS, 0.60f}, {P_MID, 0.42f}, {P_TREBLE, 0.54f},
      {P_PRESENCE, 0.52f}, {P_RESONANCE, 0.50f}, {P_MASTER, 0.45f},
      {P_OUTPUT, 0.50f}, {P_RECTIFIER, 0.0f}, {P_VOICE, 1.0f},
      {P_OD_ON, 1.0f}, {P_OD_DRIVE, 0.12f}, {P_OD_TONE, 0.50f},
      {P_OD_LEVEL, 0.55f}, {P_DELAY_ON, 0.0f}, {P_REVERB_ON, 0.0f},
      {P_CAB, 0.0f}, {P_CAB_MIC, 0.0f}, {P_CAB_LEVEL, 0.60f},
      {-1, 0.0f}}},
    {"Recto — Vintage", "Lead", 6,
     {{P_GAIN, 0.62f}, {P_BASS, 0.64f}, {P_MID, 0.36f}, {P_TREBLE, 0.55f},
      {P_PRESENCE, 0.52f}, {P_RESONANCE, 0.60f}, {P_MASTER, 0.40f},
      {P_OUTPUT, 0.50f}, {P_RECTIFIER, 1.0f}, {P_VOICE, 2.0f},
      {P_OD_ON, 1.0f}, {P_OD_DRIVE, 0.22f}, {P_OD_TONE, 0.50f},
      {P_OD_LEVEL, 0.55f}, {P_DELAY_ON, 1.0f}, {P_DELAY_SYNC, 1.0f},
      {P_DELAY_DIV, 8.0f}, {P_DELAY_MIX, 0.15f}, {P_DELAY_FEEDBACK, 0.30f},
      {P_DELAY_TONE, 0.50f}, {P_DELAY_MOD, 0.20f}, {P_REVERB_ON, 1.0f},
      {P_REVERB_MODE, 0.0f}, {P_REVERB_MIX, 0.16f}, {P_REVERB_DECAY, 0.45f},
      {P_REVERB_TONE, 0.50f}, {P_REVERB_PREDELAY, 0.12f}, {P_CAB, 0.0f},
      {P_CAB_MIC, 0.0f}, {P_CAB_LEVEL, 0.60f}, {-1, 0.0f}}},

    // --- Rig presets -------------------------------------------------------
    {"Track Clean", "Ambient", 0,
     {{P_GAIN, 0.44f}, {P_BASS, 0.50f}, {P_MID, 0.50f}, {P_TREBLE, 0.60f},
      {P_MASTER, 0.50f}, {P_OUTPUT, 0.50f}, {P_CHORUS_ON, 1.0f},
      {P_CHORUS_RATE, 0.30f}, {P_CHORUS_DEPTH, 0.55f}, {P_OD_ON, 0.0f},
      {P_DELAY_ON, 0.0f}, {P_REVERB_ON, 1.0f}, {P_REVERB_MODE, 0.0f},
      {P_REVERB_MIX, 0.24f}, {P_REVERB_DECAY, 0.50f}, {P_REVERB_TONE, 0.55f},
      {P_REVERB_PREDELAY, 0.12f}, {P_CAB, 0.0f}, {P_CAB_MIC, 0.0f},
      {P_CAB_LEVEL, 0.60f}, {-1, 0.0f}}},
    {"Track Crunch", "Rhythm", 1,
     {{P_GAIN, 0.62f}, {P_BASS, 0.45f}, {P_MID, 0.62f}, {P_TREBLE, 0.58f},
      {P_CUT, 0.42f}, {P_PRESENCE, 0.54f}, {P_MASTER, 0.48f},
      {P_OUTPUT, 0.50f}, {P_OD_ON, 0.0f}, {P_DELAY_ON, 0.0f},
      {P_REVERB_ON, 1.0f}, {P_REVERB_MODE, 1.0f}, {P_REVERB_MIX, 0.16f},
      {P_REVERB_DECAY, 0.40f}, {P_REVERB_TONE, 0.50f},
      {P_REVERB_PREDELAY, 0.06f}, {P_CAB, 0.0f}, {P_CAB_MIC, 0.0f},
      {P_CAB_LEVEL, 0.60f}, {-1, 0.0f}}},
    {"Track Rhythm", "Rhythm", 4,
     {{P_GAIN, 0.58f}, {P_BASS, 0.50f}, {P_MID, 0.44f}, {P_TREBLE, 0.56f},
      {P_PRESENCE, 0.52f}, {P_RESONANCE, 0.52f}, {P_MASTER, 0.44f},
      {P_OUTPUT, 0.50f}, {P_CRUNCH, 1.0f}, {P_OD_ON, 1.0f},
      {P_OD_DRIVE, 0.30f}, {P_OD_TONE, 0.50f}, {P_OD_LEVEL, 0.55f},
      {P_DELAY_ON, 0.0f}, {P_REVERB_ON, 0.0f}, {P_CAB, 0.0f},
      {P_CAB_MIC, 2.0f}, {P_CAB_LEVEL, 0.64f}, {-1, 0.0f}}},
    {"Track Lead", "Lead", 4,
     {{P_GAIN, 0.70f}, {P_BASS, 0.54f}, {P_MID, 0.44f}, {P_TREBLE, 0.60f},
      {P_PRESENCE, 0.58f}, {P_RESONANCE, 0.46f}, {P_MASTER, 0.40f},
      {P_OUTPUT, 0.50f}, {P_CRUNCH, 0.0f}, {P_OD_ON, 1.0f},
      {P_OD_DRIVE, 0.34f}, {P_OD_TONE, 0.52f}, {P_OD_LEVEL, 0.55f},
      {P_DELAY_ON, 1.0f}, {P_DELAY_SYNC, 1.0f}, {P_DELAY_DIV, 8.0f},
      {P_DELAY_MIX, 0.18f}, {P_DELAY_FEEDBACK, 0.30f}, {P_DELAY_TONE, 0.55f},
      {P_DELAY_MOD, 0.15f}, {P_REVERB_ON, 1.0f}, {P_REVERB_MODE, 0.0f},
      {P_REVERB_MIX, 0.15f}, {P_REVERB_DECAY, 0.40f}, {P_REVERB_TONE, 0.50f},
      {P_REVERB_PREDELAY, 0.10f}, {P_CAB, 1.0f}, {P_CAB_MIC, 0.0f},
      {P_CAB_LEVEL, 0.60f}, {-1, 0.0f}}},
};

constexpr int kPresetCount = static_cast<int>(sizeof(kPresets) / sizeof(kPresets[0]));

// Apply the startup preset to a freshly constructed Shared.  The parameter
// defaults stay neutral for manual model switching; the plugin starts on the
// Block 120 rhythm snapshot shown in the preset bar.
inline void applyStartupPreset(Shared& shared) {
  shared.set(P_MODEL, static_cast<double>(kPresets[0].model));
  for (const PresetValue& v : kPresets[0].values) {
    if (v.param >= 0 && v.param < static_cast<int>(P_COUNT)) {
      shared.set(static_cast<uint32_t>(v.param), static_cast<double>(v.value));
    }
  }
}

} // namespace amp
