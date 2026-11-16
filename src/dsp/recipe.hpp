// Amp Suite — first-pass amp recipes
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Stage values are first-pass Guitarix-derived voicings.  M1 tunes them
// against the reference captures; the structure is the production structure.

#pragma once

#include "tube_stage.hpp"
#include "tone_stack.hpp"

namespace amp {

enum class ToneStackKind {
  Bassman,
  JTM45,
  JCM800,
  AC30,
  Peavey,
  Mesa,
  Soldano,
  Bogner,
  Roland,
  MLead
};

inline ToneStackSpec toneStackSpec(ToneStackKind kind) noexcept {
  switch (kind) {
    case ToneStackKind::Bassman:
      return {250000.0f, 1000000.0f, 25000.0f, 56000.0f, 250.0e-12f,
              20.0e-9f, 20.0e-9f};
    case ToneStackKind::JTM45:
      return {250000.0f, 1000000.0f, 25000.0f, 33000.0f, 270.0e-12f,
              22.0e-9f, 22.0e-9f};
    case ToneStackKind::JCM800:
      return {220000.0f, 1000000.0f, 22000.0f, 33000.0f, 470.0e-12f,
              22.0e-9f, 22.0e-9f};
    case ToneStackKind::AC30:
      return {1000000.0f, 1000000.0f, 10000.0f, 100000.0f, 50.0e-12f,
              22.0e-9f, 22.0e-9f};
    case ToneStackKind::Peavey:
      return {250000.0f, 250000.0f, 20000.0f, 68000.0f, 270.0e-12f,
              22.0e-9f, 22.0e-9f};
    case ToneStackKind::Mesa:
      return {250000.0f, 250000.0f, 25000.0f, 100000.0f, 250.0e-12f,
              100.0e-9f, 47.0e-9f};
    case ToneStackKind::Soldano:
      return {250000.0f, 1000000.0f, 25000.0f, 47000.0f, 470.0e-12f,
              20.0e-9f, 20.0e-9f};
    case ToneStackKind::Bogner:
      return {250000.0f, 1000000.0f, 33000.0f, 51000.0f, 220.0e-12f,
              15.0e-9f, 47.0e-9f};
    case ToneStackKind::Roland:
      return {250000.0f, 250000.0f, 10000.0f, 41000.0f, 240.0e-12f,
              33.0e-9f, 82.0e-9f};
    case ToneStackKind::MLead:
      return {250000.0f, 1000000.0f, 25000.0f, 33000.0f, 500.0e-12f,
              22.0e-9f, 22.0e-9f};
  }
  return {};
}

struct AmpRecipe {
  const char* name;
  int stages;
  StageSpec stage[6];
  ToneStackKind tone;
  float preGain;
  float powerDrive;
  float powerFeedback;
  float powerSag;
  float outputTrim;
  float makeupDb;
  bool powerTable;
  bool solidState;
};

inline StageSpec stageSpec(TubeTable tube, float vplus, float divider,
                           float fck, float rk, float vk0) noexcept {
  StageSpec s;
  s.tube = tube;
  s.vplus = vplus;
  s.divider = divider;
  s.fck = fck;
  s.rk = rk;
  s.vk0 = vk0;
  return s;
}

inline const AmpRecipe& recipeFor(int model) noexcept {
  // Common first two stages from Guitarix's amp2.dsp, then a model-specific
  // cascade.  The exact values are M1 tuning targets.
  static const StageSpec s1 =
      stageSpec(TubeTable::k12AX7_68k, 250.0f, 40.0f, 86.0f, 2700.0f,
                1.581656f);
  static const StageSpec s2 =
      stageSpec(TubeTable::k12AX7_250k, 250.0f, 40.0f, 132.0f, 1500.0f,
                1.204285f);
  static const StageSpec s3 =
      stageSpec(TubeTable::k12AX7_68k, 250.0f, 40.0f, 120.0f, 1000.0f,
                1.0f);
  static const StageSpec s4 =
      stageSpec(TubeTable::k12AX7_68k, 250.0f, 40.0f, 100.0f, 820.0f, 0.9f);
  static const StageSpec s5 =
      stageSpec(TubeTable::k12AX7_68k, 250.0f, 40.0f, 100.0f, 1500.0f, 1.2f);
  static const StageSpec s6 =
      stageSpec(TubeTable::k12AX7_250k, 250.0f, 40.0f, 100.0f, 1000.0f,
                1.0f);

  static const AmpRecipe recipes[7] = {
      // Chorus Clean: solid-state path, tone stack used only as a voicing.
      {"Chorus Clean", 0, {}, ToneStackKind::Roland, 2.0f, 0.0f, 0.0f, 0.0f,
       0.8f, 15.0f, false, true},
      // Chime
      {"Chime", 3, {s1, s2, s3}, ToneStackKind::AC30, 0.45f, 1.2f, 0.0f,
       0.14f, 0.8f, 18.0f, false, false},
      // Crunch
      {"Crunch", 4, {s1, s2, s3, s4}, ToneStackKind::JCM800, 0.60f, 1.5f,
       0.06f, 0.10f, 0.8f, 15.0f, true, false},
      // Hot Rod
      {"Hot Rod", 4, {s1, s2, s3, s4}, ToneStackKind::JTM45, 0.65f, 1.6f,
       0.05f, 0.14f, 0.8f, 14.5f, true, false},
      // Block 120
      {"Block 120", 6, {s1, s2, s3, s4, s5, s6}, ToneStackKind::Peavey, 0.95f,
       1.8f, 0.06f, 0.06f, 0.8f, 8.5f, false, false},
      // Stealth EL34
      {"Stealth EL34", 6, {s1, s2, s3, s4, s5, s6}, ToneStackKind::Bogner,
       1.0f, 1.9f, 0.07f, 0.07f, 0.8f, 11.0f, true, false},
      // Recto
      {"Recto", 5, {s1, s2, s3, s4, s5}, ToneStackKind::Mesa, 0.85f, 1.7f,
       0.04f, 0.22f, 0.8f, 11.0f, false, false},
  };
  const int m = model < 0 ? 0 : (model > 6 ? 6 : model);
  return recipes[m];
}

} // namespace amp
