// Amp Suite — panel.cpp
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ui/panel.hpp"

#include "presets.hpp"
#include "version.hpp"
#include "dsp/user_ir.hpp"

#include <cairo-ft.h>
#include <ft2build.h>
#include FT_FREETYPE_H

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>

namespace amp {

namespace {

struct Rgb {
  double r, g, b;
};

struct Skin {
  const char* caption;
  Rgb bodyTop, bodyBottom;
  Rgb legend, legendDim;
  Rgb headerBg;
  Rgb tabOff, tabOn, tabTextOff, tabTextOn;
  Rgb knobCap, knobCapEdge, knobSkirt, knobIndicator;
  Rgb btnOff, btnOn, btnText, btnTextOn;
  Rgb accent;
  bool knurled;
};

Skin makeSkin(const char* caption, Rgb bodyTop, Rgb bodyBottom, Rgb legend,
              Rgb legendDim, Rgb headerBg, Rgb tabOff, Rgb tabOn,
              Rgb tabTextOff, Rgb tabTextOn, Rgb knobCap, Rgb knobCapEdge,
              Rgb knobSkirt, Rgb knobIndicator, Rgb btnOff, Rgb btnOn,
              Rgb btnText, Rgb btnTextOn, Rgb accent, bool knurled) {
  Skin s;
  s.caption = caption;
  s.bodyTop = bodyTop;
  s.bodyBottom = bodyBottom;
  s.legend = legend;
  s.legendDim = legendDim;
  s.headerBg = headerBg;
  s.tabOff = tabOff;
  s.tabOn = tabOn;
  s.tabTextOff = tabTextOff;
  s.tabTextOn = tabTextOn;
  s.knobCap = knobCap;
  s.knobCapEdge = knobCapEdge;
  s.knobSkirt = knobSkirt;
  s.knobIndicator = knobIndicator;
  s.btnOff = btnOff;
  s.btnOn = btnOn;
  s.btnText = btnText;
  s.btnTextOn = btnTextOn;
  s.accent = accent;
  s.knurled = knurled;
  return s;
}

Skin skinFor(int model) {
  switch (model) {
    case 0:  // Chorus Clean — silver / blue
      return makeSkin("Chorus Clean — solid-state 2×12",
                      {0.80, 0.82, 0.84}, {0.58, 0.61, 0.64},
                      {0.10, 0.10, 0.12}, {0.28, 0.30, 0.34},
                      {0.16, 0.18, 0.22},
                      {0.42, 0.46, 0.52}, {0.90, 0.92, 0.94},
                      {0.12, 0.13, 0.15}, {0.02, 0.02, 0.02},
                      {0.86, 0.88, 0.90}, {0.48, 0.52, 0.56},
                      {0.16, 0.18, 0.22}, {0.20, 0.48, 0.78},
                      {0.10, 0.12, 0.16}, {0.24, 0.52, 0.82},
                      {0.90, 0.92, 0.94}, {0.98, 0.99, 1.0},
                      {0.20, 0.48, 0.78}, false);
    case 1:  // Chime — black / cream / copper
      return makeSkin("Chime — British top boost 2×12",
                      {0.13, 0.12, 0.11}, {0.05, 0.05, 0.05},
                      {0.93, 0.89, 0.80}, {0.55, 0.52, 0.46},
                      {0.09, 0.08, 0.07},
                      {0.24, 0.21, 0.17}, {0.72, 0.30, 0.18},
                      {0.55, 0.52, 0.46}, {1.0, 0.96, 0.88},
                      {0.88, 0.84, 0.74}, {0.48, 0.45, 0.39},
                      {0.10, 0.09, 0.08}, {0.86, 0.48, 0.22},
                      {0.10, 0.09, 0.08}, {0.70, 0.31, 0.18},
                      {0.92, 0.88, 0.80}, {1.0, 0.97, 0.90},
                      {0.86, 0.48, 0.22}, true);
    case 2:  // Crunch — orange tolex / black
      return makeSkin("Crunch — British EL34 crunch",
                      {0.78, 0.42, 0.12}, {0.52, 0.25, 0.06},
                      {0.06, 0.05, 0.04}, {0.28, 0.18, 0.10},
                      {0.10, 0.08, 0.06},
                      {0.54, 0.30, 0.10}, {0.96, 0.62, 0.18},
                      {0.20, 0.13, 0.06}, {0.02, 0.02, 0.02},
                      {0.16, 0.15, 0.14}, {0.05, 0.05, 0.05},
                      {0.20, 0.16, 0.12}, {0.95, 0.72, 0.30},
                      {0.16, 0.13, 0.10}, {0.92, 0.52, 0.12},
                      {0.92, 0.88, 0.80}, {1.0, 0.98, 0.92},
                      {0.95, 0.55, 0.14}, true);
    case 3:  // Hot Rod — black / gold
      return makeSkin("Hot Rod — modded British lead",
                      {0.10, 0.10, 0.10}, {0.03, 0.03, 0.03},
                      {0.93, 0.89, 0.72}, {0.56, 0.52, 0.40},
                      {0.05, 0.05, 0.05},
                      {0.24, 0.22, 0.17}, {0.80, 0.66, 0.30},
                      {0.58, 0.54, 0.42}, {1.0, 0.97, 0.84},
                      {0.83, 0.82, 0.78}, {0.42, 0.42, 0.40},
                      {0.09, 0.09, 0.08}, {0.88, 0.70, 0.28},
                      {0.06, 0.06, 0.05}, {0.78, 0.62, 0.26},
                      {0.93, 0.89, 0.76}, {1.0, 0.98, 0.90},
                      {0.88, 0.70, 0.28}, true);
    case 4:  // Block 120 — black / steel / red
      return makeSkin("Block 120 — American high-gain 4×12",
                      {0.13, 0.14, 0.16}, {0.05, 0.06, 0.07},
                      {0.90, 0.92, 0.94}, {0.55, 0.58, 0.62},
                      {0.06, 0.07, 0.09},
                      {0.22, 0.24, 0.28}, {0.80, 0.85, 0.90},
                      {0.52, 0.55, 0.60}, {0.02, 0.02, 0.02},
                      {0.82, 0.84, 0.87}, {0.42, 0.45, 0.49},
                      {0.09, 0.10, 0.11}, {0.78, 0.20, 0.17},
                      {0.08, 0.09, 0.11}, {0.74, 0.20, 0.17},
                      {0.90, 0.92, 0.94}, {1.0, 0.98, 0.98},
                      {0.78, 0.20, 0.17}, true);
    case 5:  // Stealth EL34 — black / white / red
      return makeSkin("Stealth EL34 — modern high-gain 4×12",
                      {0.16, 0.16, 0.17}, {0.06, 0.06, 0.07},
                      {0.94, 0.94, 0.94}, {0.58, 0.58, 0.60},
                      {0.08, 0.08, 0.09},
                      {0.28, 0.28, 0.30}, {0.86, 0.86, 0.86},
                      {0.58, 0.58, 0.60}, {0.02, 0.02, 0.02},
                      {0.90, 0.90, 0.90}, {0.46, 0.46, 0.48},
                      {0.10, 0.10, 0.11}, {0.82, 0.18, 0.18},
                      {0.09, 0.09, 0.10}, {0.80, 0.20, 0.20},
                      {0.92, 0.92, 0.92}, {1.0, 1.0, 1.0},
                      {0.82, 0.18, 0.18}, true);
    default:  // Recto — black / chrome / red
      return makeSkin("Recto — modern American 4×12",
                      {0.10, 0.10, 0.11}, {0.03, 0.03, 0.04},
                      {0.90, 0.90, 0.90}, {0.55, 0.55, 0.57},
                      {0.05, 0.05, 0.06},
                      {0.24, 0.24, 0.26}, {0.78, 0.78, 0.80},
                      {0.56, 0.56, 0.58}, {0.02, 0.02, 0.02},
                      {0.86, 0.86, 0.88}, {0.42, 0.42, 0.44},
                      {0.08, 0.08, 0.09}, {0.82, 0.16, 0.14},
                      {0.07, 0.07, 0.08}, {0.78, 0.16, 0.14},
                      {0.90, 0.90, 0.90}, {1.0, 1.0, 1.0},
                      {0.82, 0.16, 0.14}, true);
  }
}

constexpr double kPi = 3.14159265358979323846;

constexpr double kPresetX = 72.0;
constexpr double kPresetY = 14.0;
constexpr double kPresetW = 330.0;
constexpr double kPresetH = 36.0;
constexpr double kListY = 54.0;
constexpr double kListRow = 22.0;

constexpr double kUserBoxX = 60.0;
constexpr double kUserBoxY = 300.0;
constexpr double kUserBoxW = 360.0;
constexpr double kUserBoxH = 260.0;
constexpr double kUserRowY = 352.0;
constexpr double kUserRowH = 20.0;
constexpr int kUserRows = 10;

double knobAngle(double f) {
  return (135.0 + 270.0 * f) * kPi / 180.0;
}

double clamp01(double v) { return std::min(std::max(v, 0.0), 1.0); }

double dbToNorm(double db, double minDb, double maxDb) {
  if (!std::isfinite(db)) {
    return 0.0;
  }
  return clamp01((db - minDb) / (maxDb - minDb));
}

void roundRect(cairo_t* cr, double x, double y, double w, double h, double r) {
  cairo_new_sub_path(cr);
  cairo_arc(cr, x + w - r, y + r, r, -kPi / 2, 0);
  cairo_arc(cr, x + w - r, y + h - r, r, 0, kPi / 2);
  cairo_arc(cr, x + r, y + h - r, r, kPi / 2, kPi);
  cairo_arc(cr, x + r, y + r, r, kPi, 3 * kPi / 2);
  cairo_close_path(cr);
}

void setRgb(cairo_t* cr, Rgb c, double a = 1.0) {
  cairo_set_source_rgba(cr, c.r, c.g, c.b, a);
}

Rgb mix(Rgb a, Rgb b, double t) {
  return {a.r + (b.r - a.r) * t, a.g + (b.g - a.g) * t,
          a.b + (b.b - a.b) * t};
}

bool hasPresence(int model) { return model != 0; }
bool hasResonance(int model) {
  return model == 2 || model == 4 || model == 5 || model == 6;
}
bool hasBright(int model) { return model == 0; }
bool hasCut(int model) { return model == 1; }
bool hasPowerScale(int model) { return model == 2; }
bool hasCrunch(int model) { return model == 4; }
bool hasRectifier(int model) { return model == 6; }
bool hasVoice(int model) {
  return model == 3 || model == 5 || model == 6;
}
bool hasAlternate(int model) { return model == 3 || model == 4; }

const char* cabName(int model, int cab) {
  if (cab == 2) {
    return "User IR";
  }
  switch (model) {
    case 0: return "Chorus 2×12";
    case 1: return "Alnico 2×12";
    case 2: return "V30 4×12";
    case 3: return cab == 1 ? "Greenback 4×12" : "G12T-75 4×12";
    case 4: return cab == 1 ? "OS V30 4×12" : "1200 4×12";
    case 5: return "G12-EVH 4×12";
    default: return "OS V30 4×12";
  }
}

const char* cabSubtitle(int model, int cab) {
  if (cab == 2) {
    return "user impulse response · factory fallback";
  }
  switch (model) {
    case 0: return "open-back · chorus stereo";
    case 1: return "alnico blue · open-back";
    case 2: return "closed-back · V30";
    case 3: return cab == 1 ? "closed-back · greenback" : "closed-back · G12T-75";
    case 4: return cab == 1 ? "oversized · late-90s V30" : "closed-back · 1200";
    case 5: return "closed-back · G12-EVH";
    default: return "oversized · late-90s V30";
  }
}

const char* modelName(int model) {
  const int m = std::min(std::max(model, 0), 6);
  return kModelLabels[m];
}

std::string shorten(std::string s, size_t maxLen = 40) {
  if (s.size() > maxLen) {
    s.resize(maxLen > 3 ? maxLen - 3 : 0);
    s += "...";
  }
  return s;
}

std::string selectedUserIrName(const Shared& shared, const PanelState& state) {
  const int model =
      std::min(std::max(static_cast<int>(shared.get(P_MODEL) + 0.5), 0), 6);
  const int index =
      static_cast<int>(shared.get(P_USER_IR_0 + model) + 0.5);
  if (index > 0 && index <= static_cast<int>(state.userIrs.size())) {
    return shorten(state.userIrs[static_cast<size_t>(index - 1)], 30);
  }
  return {};
}

} // namespace

Panel::Panel() {
  grain_ = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 64, 64);
  unsigned char* data = cairo_image_surface_get_data(grain_);
  const int stride = cairo_image_surface_get_stride(grain_);
  unsigned int seed = 22222u;
  for (int y = 0; y < 64; ++y) {
    for (int x = 0; x < 64; ++x) {
      seed = seed * 1103515245u + 12345u;
      const unsigned v = (seed >> 16) & 0xff;
      const unsigned a = (v < 128) ? 16u : 0u;
      const unsigned c = (v < 128) ? 0u : 255u;
      unsigned char* px = data + y * stride + x * 4;
      px[0] = static_cast<unsigned char>(c & 0xff);
      px[1] = static_cast<unsigned char>(c & 0xff);
      px[2] = static_cast<unsigned char>(c & 0xff);
      px[3] = static_cast<unsigned char>(a);
    }
  }
  cairo_surface_mark_dirty(grain_);
  fallbackFont_ = cairo_toy_font_face_create(
      "sans-serif", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
  font_ = fallbackFont_;
}

Panel::~Panel() {
  if (grain_) {
    cairo_surface_destroy(grain_);
  }
  if (font_ && font_ != fallbackFont_) {
    cairo_font_face_destroy(font_);
  }
  if (fallbackFont_) {
    cairo_font_face_destroy(fallbackFont_);
  }
  if (ftFace_) {
    FT_Done_Face(static_cast<FT_Face>(ftFace_));
  }
  if (ft_) {
    FT_Done_FreeType(static_cast<FT_Library>(ft_));
  }
}

void Panel::loadFont(const std::string& dir) {
  if (ft_) {
    return;
  }
  std::string path = dir;
  if (!path.empty() && path.back() != '/') {
    path += '/';
  }
  path += "Jost-ExtraBold.ttf";
  FT_Library lib = nullptr;
  if (FT_Init_FreeType(&lib) != 0) {
    return;
  }
  FT_Face face = nullptr;
  if (FT_New_Face(lib, path.c_str(), 0, &face) != 0) {
    FT_Done_FreeType(lib);
    return;
  }
  cairo_font_face_t* font = cairo_ft_font_face_create_for_ft_face(face, 0);
  if (cairo_font_face_status(font) != CAIRO_STATUS_SUCCESS) {
    cairo_font_face_destroy(font);
    FT_Done_Face(face);
    FT_Done_FreeType(lib);
    return;
  }
  ft_ = lib;
  ftFace_ = face;
  font_ = font;
}

void Panel::text(cairo_t* cr, const char* s, double x, double y, double size,
                 int align, double r, double g, double b) const {
  cairo_set_font_face(cr, font_);
  cairo_set_font_size(cr, size);
  cairo_set_source_rgb(cr, r, g, b);
  cairo_text_extents_t e;
  cairo_text_extents(cr, s, &e);
  double tx = x;
  if (align == 1) {
    tx = x - e.x_advance * 0.5;
  } else if (align == 2) {
    tx = x - e.x_advance;
  }
  cairo_move_to(cr, tx, y);
  cairo_show_text(cr, s);
}

// ---------------------------------------------------------------------------
// Layout
// ---------------------------------------------------------------------------

void Panel::layout(const Shared& shared, const PanelState& state) {
  widgets_.clear();
  const int model = static_cast<int>(shared.get(P_MODEL) + 0.5);

  auto knob = [&](uint32_t param, double x, double y, double r,
                  const char* label, std::vector<const char*> scale = {},
                  bool below = false, int steps = 0) {
    Widget k{};
    k.kind = Kind::Knob;
    k.param = param;
    k.x = x;
    k.y = y;
    k.r = r;
    k.label = label;
    k.scale = std::move(scale);
    k.labelBelow = below;
    k.steps = steps;
    widgets_.push_back(k);
  };
  auto button = [&](uint32_t param, double x, double y, double w, double h,
                    double value, const char* label) {
    Widget b{};
    b.kind = Kind::Button;
    b.param = param;
    b.x = x;
    b.y = y;
    b.w = w;
    b.h = h;
    b.value = value;
    b.label = label;
    widgets_.push_back(b);
  };
  auto pill = [&](uint32_t param, double x, double y, double w, double h,
                  const char* label) {
    Widget p{};
    p.kind = Kind::Pill;
    p.param = param;
    p.x = x;
    p.y = y;
    p.w = w;
    p.h = h;
    p.value = 1.0;
    p.label = label;
    widgets_.push_back(p);
  };
  auto pagePill = [&](int page, double x, double y, double w, double h,
                      const char* label) {
    Widget p{};
    p.kind = Kind::PagePill;
    p.param = P_COUNT;
    p.value = page;
    p.x = x;
    p.y = y;
    p.w = w;
    p.h = h;
    p.label = label;
    widgets_.push_back(p);
  };
  auto toggle = [&](uint32_t param, double x, double y, double value,
                    const char* top, const char* bottom) {
    Widget t{};
    t.kind = Kind::Toggle;
    t.param = param;
    t.x = x;
    t.y = y;
    t.w = 30;
    t.h = 56;
    t.value = value;
    t.label = top;
    t.buttonText = {top, bottom};
    widgets_.push_back(t);
  };
  auto foot = [&](uint32_t param, double x, double y, double r,
                  const char* label) {
    Widget f{};
    f.kind = Kind::Footswitch;
    f.param = param;
    f.x = x;
    f.y = y;
    f.r = r;
    f.label = label;
    widgets_.push_back(f);
  };

  // Header: power, preset bar, page/toggle pills.
  {
    Widget p{};
    p.kind = Kind::Power;
    p.param = P_POWER;
    p.x = 34;
    p.y = 30;
    p.r = 14;
    widgets_.push_back(p);
  }
  {
    Widget b{};
    b.kind = Kind::PresetBar;
    b.param = P_COUNT;
    b.x = kPresetX;
    b.y = kPresetY;
    b.w = kPresetW;
    b.h = kPresetH;
    b.label = "Preset";
    widgets_.push_back(b);
  }
  pagePill(1, 690, 20, 76, 24, "PEDALS");
  pagePill(2, 774, 20, 58, 24, "CAB");
  pill(P_HQ, 840, 20, 58, 24, "HQ");
  pill(P_LOOP_POS, 906, 20, 96, 24, "LOOP");

  // Model tabs.
  const int tabCount = static_cast<int>(Model::Count);
  const double tabW = (980.0 - (tabCount - 1) * 8.0) / tabCount;
  for (int i = 0; i < tabCount; ++i) {
    Widget t{};
    t.kind = Kind::ModelTab;
    t.param = P_MODEL;
    t.value = i;
    t.x = 30 + i * (tabW + 8.0);
    t.y = 70;
    t.w = tabW;
    t.h = 40;
    t.label = kModelLabels[i];
    widgets_.push_back(t);
  }

  if (state.page == 0) {
    // Amp page.
    static const std::vector<const char*> kToneScale = {"0", "5", "10"};
    static const std::vector<const char*> kTrimScale = {"-12", "0", "+12"};
    knob(P_GAIN, 190, 420, 32, "GAIN", kToneScale);
    knob(P_BASS, 300, 420, 28, "BASS", kToneScale);
    knob(P_MID, 410, 420, 28, "MID", kToneScale);
    knob(P_TREBLE, 520, 420, 28, "TREBLE", kToneScale);
    if (hasPresence(model)) {
      knob(P_PRESENCE, 630, 420, 26, "PRESENCE", kToneScale);
    }
    if (hasResonance(model)) {
      knob(P_RESONANCE, 740, 420, 26, "RESONANCE", kToneScale);
    }
    knob(P_INPUT, 90, 520, 22, "INPUT", kTrimScale, true);
    knob(P_MASTER, 300, 520, 26, "MASTER", kToneScale);
    knob(P_OUTPUT, 940, 535, 30, "OUTPUT", kTrimScale);

    // Output limiter, in the right column like Comp Suite.
    pill(P_LIMITER_ON, 852, 270, 96, 24, "LIMITER");
    knob(P_LIMITER_THRESH, 900, 372, 26, "THRESH",
         {"-24", "-12", "-6", "0"});

    if (hasCut(model)) {
      knob(P_CUT, 820, 520, 26, "CUT", kToneScale);
    }
    if (hasPowerScale(model)) {
      toggle(P_POWER_SCALE, 820, 400, 1.0, "100", "50");
    }
    if (hasCrunch(model)) {
      toggle(P_CRUNCH, 820, 400, 1.0, "CRUNCH", "LEAD");
    }
    if (hasRectifier(model)) {
      toggle(P_RECTIFIER, 820, 400, 0.0, "TUBE", "SI");
    }
    if (hasVoice(model)) {
      knob(P_VOICE, 820, 520, 22, "VOICE", {"A", "B", "C"}, true, 3);
    }
    if (model == 0) {
      toggle(P_BRIGHT, 750, 380, 0.0, "BRIGHT", "OFF");
      toggle(P_CHORUS_ON, 810, 380, 1.0, "CHORUS", "OFF");
      knob(P_CHORUS_RATE, 770, 490, 20, "RATE", {}, true);
      knob(P_CHORUS_DEPTH, 830, 490, 20, "DEPTH", {}, true);
    }
  } else if (state.page == 1) {
    // Pedalboard page.  Three cards: 808, delay, reverb.
    const double cx[3] = {40, 370, 700};
    const double cy = 185;
    for (int card = 0; card < 3; ++card) {
      (void)card;
      (void)cy;
    }

    // 808 card.
    knob(P_OD_DRIVE, cx[0] + 60, cy + 110, 26, "DRIVE", {}, true);
    knob(P_OD_TONE, cx[0] + 150, cy + 110, 26, "TONE", {}, true);
    knob(P_OD_LEVEL, cx[0] + 240, cy + 110, 26, "LEVEL", {}, true);
    foot(P_OD_ON, cx[0] + 150, cy + 300, 26, "808");

    // Delay card.
    pill(P_DELAY_SYNC, cx[1] + 20, cy + 42, 88, 24, "SYNC");
    pill(P_DELAY_PINGPONG, cx[1] + 192, cy + 42, 88, 24, "PING");
    if (shared.get(P_DELAY_SYNC) > 0.5) {
      knob(P_DELAY_DIV, cx[1] + 120, cy + 120, 26, "DIVISION", {}, true, 9);
    } else {
      knob(P_DELAY_TIME, cx[1] + 120, cy + 120, 26, "TIME", {}, true);
    }
    knob(P_DELAY_FEEDBACK, cx[1] + 220, cy + 120, 26, "FEEDBACK", {}, true);
    knob(P_DELAY_MIX, cx[1] + 60, cy + 215, 26, "MIX", {}, true);
    knob(P_DELAY_TONE, cx[1] + 150, cy + 215, 26, "TONE", {}, true);
    knob(P_DELAY_MOD, cx[1] + 240, cy + 215, 26, "MOD", {}, true);
    knob(P_BPM, cx[1] + 250, cy + 290, 18, "BPM", {}, true);
    foot(P_DELAY_ON, cx[1] + 150, cy + 330, 26, "DELAY");

    // Reverb card.
    pill(P_REVERB_MODE, cx[2] + 20, cy + 42, 88, 24, "MODE");
    knob(P_REVERB_MIX, cx[2] + 90, cy + 115, 26, "MIX", {}, true);
    knob(P_REVERB_DECAY, cx[2] + 210, cy + 115, 26, "DECAY", {}, true);
    knob(P_REVERB_TONE, cx[2] + 60, cy + 215, 26, "TONE", {}, true);
    knob(P_REVERB_PREDELAY, cx[2] + 180, cy + 215, 26, "PRE-DLY", {}, true);
    foot(P_REVERB_ON, cx[2] + 150, cy + 330, 26, "REVERB");

  } else {
    // Cabinet page.
    const bool alt = hasAlternate(model);
    button(P_CAB, 60, 200, 110, 28, 0.0, "DEFAULT");
    if (alt) {
      button(P_CAB, 180, 200, 110, 28, 1.0, "ALT");
    }
    button(P_CAB, alt ? 300 : 180, 200, 110, 28, 2.0, "USER");

    button(P_CAB_MIC, 760, 210, 80, 26, 0.0, "DYNAMIC");
    button(P_CAB_MIC, 850, 210, 80, 26, 1.0, "RIBBON");
    button(P_CAB_MIC, 940, 210, 70, 26, 2.0, "BLEND");

    knob(P_CAB_LEVEL, 760, 340, 28, "LEVEL", {}, true);
    knob(P_CAB_LOWCUT, 850, 340, 26, "LOW CUT", {}, true);
    knob(P_CAB_HIGHCUT, 940, 340, 26, "HIGH CUT", {}, true);
    toggle(P_CAB_BYPASS, 780, 440, 0.0, "BYPASS", "CAB");
  }
}

// ---------------------------------------------------------------------------
// Hit testing and interaction
// ---------------------------------------------------------------------------

bool Panel::hitWidget(const Widget& w, double x, double y) const {
  switch (w.kind) {
    case Kind::Knob: {
      const double dx = x - w.x;
      const double dy = y - w.y;
      return dx * dx + dy * dy <= w.r * w.r * 1.3;
    }
    case Kind::Power: {
      const double dx = x - w.x;
      const double dy = y - w.y;
      return dx * dx + dy * dy <= w.r * w.r * 1.8;
    }
    case Kind::Footswitch: {
      const double dx = x - w.x;
      const double dy = y - w.y;
      return dx * dx + dy * dy <= w.r * w.r * 1.7;
    }
    case Kind::Toggle:
      return x >= w.x - w.w * 0.5 && x <= w.x + w.w * 0.5 &&
             y >= w.y && y <= w.y + w.h;
    default:
      return x >= w.x && x <= w.x + w.w && y >= w.y && y <= w.y + w.h;
  }
}

double Panel::knobValue(const Shared& shared, const Widget& w) const {
  return shared.get(w.param);
}

bool Panel::isLit(const Shared& shared, const PanelState& state,
                  const Widget& w) const {
  if (w.kind == Kind::PagePill) {
    return state.page == static_cast<int>(w.value);
  }
  if (w.param >= P_COUNT) {
    return false;
  }
  return shared.get(w.param) > 0.5;
}

bool Panel::pointerDown(Shared& shared, PanelState& state, double x, double y,
                        bool fine) {
  (void)fine;
  if (handlePresetClick(shared, state, x, y)) {
    return true;
  }
  if (state.page == 2 && x >= kUserBoxX && x <= kUserBoxX + kUserBoxW &&
      y >= kUserBoxY && y <= kUserBoxY + kUserBoxH) {
    if (y >= kUserRowY) {
      const int row = static_cast<int>((y - kUserRowY) / kUserRowH) +
                      state.userIrScroll;
      const int files = static_cast<int>(state.userIrs.size());
      const int model =
          std::min(std::max(static_cast<int>(shared.get(P_MODEL) + 0.5), 0), 6);
      if (row == 0) {
        shared.set(P_USER_IR_0 + model, 0.0);
        shared.set(P_CAB, 2.0);
        markEdited(state);
      } else if (row >= 1 && row <= files) {
        shared.set(P_USER_IR_0 + model, static_cast<double>(row));
        shared.set(P_CAB, 2.0);
        markEdited(state);
      }
    }
    return true;
  }
  for (auto it = widgets_.rbegin(); it != widgets_.rend(); ++it) {
    const Widget& w = *it;
    if (!hitWidget(w, x, y)) {
      continue;
    }
    switch (w.kind) {
      case Kind::Knob:
        dragParam_ = static_cast<int>(w.param);
        dragStartY_ = y;
        dragStartValue_ = shared.get(w.param);
        return true;
      case Kind::ModelTab:
        shared.set(P_MODEL, w.value);
        state.presetIndex = -1;
        state.presetDirty = false;
        return true;
      case Kind::PagePill:
        state.page = (state.page == static_cast<int>(w.value))
                         ? 0
                         : static_cast<int>(w.value);
        return true;
      case Kind::PresetBar:
        state.presetOpen = !state.presetOpen;
        return true;
      case Kind::Power:
        shared.set(P_POWER, shared.get(P_POWER) > 0.5 ? 0.0 : 1.0);
        markEdited(state);
        return true;
      case Kind::Toggle:
      case Kind::Footswitch:
        shared.set(w.param,
                   shared.get(w.param) > 0.5 ? 0.0 : 1.0);
        markEdited(state);
        return true;
      case Kind::Button:
        if (w.param >= P_COUNT) {
          return true;
        }
        shared.set(w.param, w.value);
        markEdited(state);
        return true;
      case Kind::Pill:
        if (w.param >= P_COUNT) {
          return true;
        }
        if (w.param == P_LOOP_POS) {
          shared.set(P_LOOP_POS, shared.get(P_LOOP_POS) > 0.5 ? 0.0 : 1.0);
        } else if (w.param == P_REVERB_MODE) {
          shared.set(P_REVERB_MODE,
                     shared.get(P_REVERB_MODE) > 0.5 ? 0.0 : 1.0);
        } else if (kParamSpecs[w.param].toggled) {
          shared.set(w.param, shared.get(w.param) > 0.5 ? 0.0 : 1.0);
        } else {
          shared.set(w.param, w.value);
        }
        markEdited(state);
        return true;
    }
  }
  return false;
}

bool Panel::pointerMove(Shared& shared, PanelState& state, double x, double y,
                        bool fine) {
  if (dragParam_ < 0) {
    return false;
  }
  const ParamSpec& spec = kParamSpecs[dragParam_];
  const double delta = (dragStartY_ - y) * (fine ? 0.0009 : 0.0045);
  double v = dragStartValue_ + delta;
  v = std::min(std::max(v, spec.min), spec.max);
  if (spec.stepped) {
    v = std::round(v);
  }
  shared.set(static_cast<uint32_t>(dragParam_), v);
  markEdited(state);
  return true;
}

void Panel::pointerUp() { dragParam_ = -1; }

bool Panel::wheel(Shared& shared, PanelState& state, double x, double y,
                  double dy) {
  if (state.page == 2 && x >= kUserBoxX && x <= kUserBoxX + kUserBoxW &&
      y >= kUserBoxY && y <= kUserBoxY + kUserBoxH) {
    const int rows = static_cast<int>(state.userIrs.size()) + 1;
    const int maxScroll = std::max(0, rows - kUserRows);
    state.userIrScroll += dy > 0.0 ? -1 : 1;
    state.userIrScroll = std::min(std::max(state.userIrScroll, 0), maxScroll);
    return true;
  }
  for (auto it = widgets_.rbegin(); it != widgets_.rend(); ++it) {
    const Widget& w = *it;
    if (!hitWidget(w, x, y)) {
      continue;
    }
    if (w.kind == Kind::Knob) {
      const ParamSpec& spec = kParamSpecs[w.param];
      const double step = spec.stepped ? 1.0 : 0.02;
      double v = shared.get(w.param) + (dy > 0.0 ? step : -step);
      v = std::min(std::max(v, spec.min), spec.max);
      if (spec.stepped) {
        v = std::round(v);
      }
      shared.set(w.param, v);
      markEdited(state);
      return true;
    }
  }
  return false;
}

void Panel::tick(const Shared& shared, PanelState& state) const {
  auto norm = [](float db, double minDb, double maxDb) {
    return static_cast<float>(dbToNorm(db, minDb, maxDb));
  };
  const float inL = norm(shared.inPeakLDb.load(std::memory_order_relaxed), -60, 0);
  const float inR = norm(shared.inPeakRDb.load(std::memory_order_relaxed), -60, 0);
  const float outL = norm(shared.outPeakLDb.load(std::memory_order_relaxed), -60, 3);
  const float outR = norm(shared.outPeakRDb.load(std::memory_order_relaxed), -60, 3);
  state.inLevelL = 0.65f * state.inLevelL + 0.35f * inL;
  state.inLevelR = 0.65f * state.inLevelR + 0.35f * inR;
  state.outLevelL = 0.65f * state.outLevelL + 0.35f * outL;
  state.outLevelR = 0.65f * state.outLevelR + 0.35f * outR;
  state.inLevel = std::max(state.inLevelL, state.inLevelR);
  state.outLevel = std::max(state.outLevelL, state.outLevelR);
  state.bpm = shared.bpm.load(std::memory_order_relaxed);
  state.hostTempo = shared.hostTempo.load(std::memory_order_relaxed);
  if (!state.hostTempo) {
    state.bpm = static_cast<float>(shared.get(P_BPM));
  }
  if (++state.userIrScanTicks >= 60) {
    state.userIrScanTicks = 0;
    state.userIrs = listUserIrFiles(userIrDir());
    const int rows = static_cast<int>(state.userIrs.size()) + 1;
    const int maxScroll = std::max(0, rows - 7);
    state.userIrScroll =
        std::min(std::max(state.userIrScroll, 0), maxScroll);
  }
}

// ---------------------------------------------------------------------------
// Presets
// ---------------------------------------------------------------------------

void Panel::drawPresetBar(cairo_t* cr, const Shared& shared,
                          const PanelState& state) const {
  const int model = static_cast<int>(shared.get(P_MODEL) + 0.5);
  const Skin skin = skinFor(model);
  roundRect(cr, kPresetX, kPresetY, kPresetW, kPresetH, 4);
  setRgb(cr, mix(skin.headerBg, {0, 0, 0}, 0.15));
  cairo_fill(cr);
  setRgb(cr, {1, 1, 1}, 0.12);
  cairo_set_line_width(cr, 1.0);
  roundRect(cr, kPresetX + 0.5, kPresetY + 0.5, kPresetW - 1, kPresetH - 1, 4);
  cairo_stroke(cr);

  // Previous / next arrows, as in Comp Suite.
  text(cr, "\u2039", kPresetX + 18, kPresetY + 24, 20, 1, 0.72, 0.75, 0.78);
  text(cr, "\u203a", kPresetX + kPresetW - 18, kPresetY + 24, 20, 1, 0.72,
       0.75, 0.78);

  char buf[160];
  if (state.presetIndex < 0) {
    std::snprintf(buf, sizeof(buf), "Manual — %s", modelName(model));
  } else {
    const int index =
        std::min(std::max(state.presetIndex, 0), kPresetCount - 1);
    if (model != kPresets[index].model) {
      std::snprintf(buf, sizeof(buf), "Manual — %s", modelName(model));
    } else {
      std::snprintf(buf, sizeof(buf), "%s%s", kPresets[index].name,
                    state.presetDirty ? " *" : "");
    }
  }
  text(cr, buf, kPresetX + kPresetW * 0.5, kPresetY + 24, 13.5, 1, 0.90,
       0.92, 0.94);
}

void Panel::drawPresetList(cairo_t* cr, const Shared& shared,
                           const PanelState& state) const {
  const int model = static_cast<int>(shared.get(P_MODEL) + 0.5);
  const Skin skin = skinFor(model);
  const double h = kPresetCount * kListRow + 10.0;
  roundRect(cr, kPresetX, kListY, kPresetW, h, 5);
  setRgb(cr, mix(skin.headerBg, {0, 0, 0}, 0.15));
  cairo_fill(cr);
  setRgb(cr, {1, 1, 1}, 0.14);
  cairo_set_line_width(cr, 1.0);
  roundRect(cr, kPresetX + 0.5, kListY + 0.5, kPresetW - 1, h - 1, 5);
  cairo_stroke(cr);

  for (int i = 0; i < kPresetCount; ++i) {
    const double y = kListY + 5 + i * kListRow;
    if (i == state.presetIndex) {
      roundRect(cr, kPresetX + 3, y, kPresetW - 6, kListRow - 1, 3);
      setRgb(cr, skin.accent, 0.35);
      cairo_fill(cr);
    }
    text(cr, kPresets[i].name, kPresetX + 12, y + 15, 11.5, 0,
         skin.btnText.r, skin.btnText.g, skin.btnText.b);
    text(cr, kPresets[i].category, kPresetX + kPresetW - 10, y + 15, 9.5, 2,
         skin.legendDim.r, skin.legendDim.g, skin.legendDim.b);
  }
}

bool Panel::handlePresetClick(Shared& shared, PanelState& state, double x,
                              double y) {
  const bool inBar = x >= kPresetX && x <= kPresetX + kPresetW &&
                     y >= kPresetY && y <= kPresetY + kPresetH;
  if (!state.presetOpen) {
    if (!inBar) {
      return false;
    }
    if (x <= kPresetX + 36) {
      const int next = state.presetIndex <= 0 ? 0 : state.presetIndex - 1;
      applyPreset(shared, state, next);
      return true;
    }
    if (x >= kPresetX + kPresetW - 36) {
      const int next = state.presetIndex < 0
                           ? 0
                           : std::min(kPresetCount - 1, state.presetIndex + 1);
      applyPreset(shared, state, next);
      return true;
    }
    state.presetOpen = true;
    return true;
  }
  for (int i = 0; i < kPresetCount; ++i) {
    const double rowY = kListY + 5 + i * kListRow;
    if (x >= kPresetX && x <= kPresetX + kPresetW && y >= rowY &&
        y < rowY + kListRow) {
      applyPreset(shared, state, i);
      state.presetOpen = false;
      return true;
    }
  }
  if (inBar || x < kPresetX || x > kPresetX + kPresetW || y < kListY ||
      y > kListY + kPresetCount * kListRow + 10) {
    state.presetOpen = false;
  }
  return true;
}

void Panel::applyPreset(Shared& shared, PanelState& state, int index) const {
  if (index < 0 || index >= kPresetCount) {
    return;
  }
  const Preset& p = kPresets[index];
  shared.set(P_MODEL, static_cast<double>(p.model));
  for (const PresetValue& v : p.values) {
    if (v.param >= 0 && v.param < static_cast<int>(P_COUNT)) {
      shared.set(static_cast<uint32_t>(v.param), v.value);
    }
  }
  state.presetIndex = index;
  state.presetDirty = false;
}

// ---------------------------------------------------------------------------
// Drawing
// ---------------------------------------------------------------------------

void Panel::draw(cairo_t* cr, const Shared& shared, const PanelState& state) {
  layout(shared, state);
  const int model = static_cast<int>(shared.get(P_MODEL) + 0.5);
  const Skin skin = skinFor(model);

  cairo_save(cr);
  cairo_set_source_rgb(cr, 0.055, 0.055, 0.06);
  cairo_paint(cr);

  drawHeader(cr, shared, state);
  drawTabs(cr, shared);

  // Faceplate.
  cairo_pattern_t* body = cairo_pattern_create_linear(0, 122, 0, 604);
  cairo_pattern_add_color_stop_rgb(body, 0.0, skin.bodyTop.r,
                                   skin.bodyTop.g, skin.bodyTop.b);
  cairo_pattern_add_color_stop_rgb(body, 1.0, skin.bodyBottom.r,
                                   skin.bodyBottom.g, skin.bodyBottom.b);
  roundRect(cr, 16, 122, 1008, 482, 6);
  cairo_set_source(cr, body);
  cairo_fill(cr);
  cairo_pattern_destroy(body);

  if (grain_) {
    cairo_pattern_t* grain = cairo_pattern_create_for_surface(grain_);
    cairo_pattern_set_extend(grain, CAIRO_EXTEND_REPEAT);
    cairo_save(cr);
    cairo_rectangle(cr, 17, 123, 1006, 480);
    cairo_clip(cr);
    cairo_set_source(cr, grain);
    cairo_paint_with_alpha(cr, 0.018);
    cairo_restore(cr);
    cairo_pattern_destroy(grain);
  }

  // Screws.
  for (double sx : {32.0, 1008.0}) {
    for (double sy : {138.0, 588.0}) {
      cairo_arc(cr, sx, sy, 4.5, 0, 2 * kPi);
      setRgb(cr, mix(skin.bodyBottom, {0, 0, 0}, 0.25), 0.9);
      cairo_fill(cr);
    }
  }

  if (state.page == 0) {
    drawAmpPage(cr, shared, state);
  } else if (state.page == 1) {
    drawPedalPage(cr, shared, state);
  } else {
    drawCabPage(cr, shared, state);
  }

  drawFooter(cr, shared, state);

  for (const Widget& w : widgets_) {
    drawWidget(cr, w, shared, state, model);
  }

  if (state.presetOpen) {
    drawPresetList(cr, shared, state);
  }

  cairo_restore(cr);
}

void Panel::drawHeader(cairo_t* cr, const Shared& shared,
                       const PanelState& state) const {
  (void)state;
  const int model = static_cast<int>(shared.get(P_MODEL) + 0.5);
  const Skin skin = skinFor(model);
  cairo_pattern_t* bar = cairo_pattern_create_linear(0, 0, 0, 62);
  cairo_pattern_add_color_stop_rgb(bar, 0.0, skin.headerBg.r * 1.35,
                                   skin.headerBg.g * 1.35,
                                   skin.headerBg.b * 1.35);
  cairo_pattern_add_color_stop_rgb(bar, 1.0, skin.headerBg.r * 0.75,
                                   skin.headerBg.g * 0.75,
                                   skin.headerBg.b * 0.75);
  cairo_rectangle(cr, 0, 0, kPanelWidth, 62);
  cairo_set_source(cr, bar);
  cairo_fill(cr);
  cairo_pattern_destroy(bar);

  setRgb(cr, {1, 1, 1}, 0.08);
  cairo_set_line_width(cr, 1.0);
  cairo_move_to(cr, 0, 0.5);
  cairo_line_to(cr, kPanelWidth, 0.5);
  cairo_stroke(cr);
  setRgb(cr, {0, 0, 0}, 0.55);
  cairo_move_to(cr, 0, 61.5);
  cairo_line_to(cr, kPanelWidth, 61.5);
  cairo_stroke(cr);

  text(cr, "AMP SUITE", 540, 38, 19, 1, 0.82, 0.84, 0.86);
}

void Panel::drawTabs(cairo_t* cr, const Shared& shared) const {
  (void)shared;
  cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 0.35);
  cairo_rectangle(cr, 0, 62, kPanelWidth, 54);
  cairo_fill(cr);
}

void Panel::drawAmpPage(cairo_t* cr, const Shared& shared,
                        const PanelState& state) const {
  const int model = static_cast<int>(shared.get(P_MODEL) + 0.5);
  const int cab = static_cast<int>(shared.get(P_CAB) + 0.5);
  drawCabWindow(cr, shared, state, model);
  drawLeds(cr, shared, model);

  // Cab label in the right column.
  const std::string userIr = selectedUserIrName(shared, state);
  const char* cabLabel = cab == 2 && !userIr.empty() ? userIr.c_str()
                                                     : cabName(model, cab);
  text(cr, "CABINET", 900, 180, 10, 1, 0.45, 0.47, 0.50);
  text(cr, cabLabel, 900, 202, 13, 1, 0.86, 0.88, 0.90);
  text(cr, cabSubtitle(model, cab), 900, 220, 9.5, 1, 0.42, 0.44, 0.47);

  // Output limiter caption, above its pill and knob.
  text(cr, "OUTPUT", 900, 256, 9.5, 1, 0.45, 0.47, 0.50);

  // Side level meters.
  drawLevelMeter(cr, 20, 150, 16, 352, state.inLevelL, state.inLevelR, model,
                 true, -60.0, 0.0);
  drawLevelMeter(cr, 986, 150, 16, 352, state.outLevelL, state.outLevelR,
                 model, false, -60.0, 3.0);
}

void Panel::drawCabWindow(cairo_t* cr, const Shared& shared,
                          const PanelState& state, int model) const {
  const int cab = static_cast<int>(shared.get(P_CAB) + 0.5);
  const Skin skin = skinFor(model);
  roundRect(cr, 170, 136, 570, 204, 5);
  setRgb(cr, mix(skin.bodyBottom, {0, 0, 0}, 0.55));
  cairo_fill(cr);
  setRgb(cr, {0, 0, 0}, 0.35);
  cairo_set_line_width(cr, 1.0);
  roundRect(cr, 170.5, 136.5, 569, 203, 5);
  cairo_stroke(cr);

  // Grille cloth.
  cairo_save(cr);
  cairo_rectangle(cr, 174, 140, 562, 196);
  cairo_clip(cr);
  cairo_set_source_rgba(cr, skin.accent.r, skin.accent.g, skin.accent.b, 0.10);
  cairo_rectangle(cr, 174, 140, 562, 196);
  cairo_fill(cr);
  cairo_set_source_rgba(cr, 1, 1, 1, 0.035);
  cairo_set_line_width(cr, 1.0);
  for (double x = 176; x < 736; x += 7) {
    cairo_move_to(cr, x, 140);
    cairo_line_to(cr, x, 336);
    cairo_stroke(cr);
  }
  for (double y = 142; y < 336; y += 7) {
    cairo_move_to(cr, 174, y);
    cairo_line_to(cr, 736, y);
    cairo_stroke(cr);
  }
  cairo_restore(cr);

  // Header strip inside the window, like Comp Suite's meter mode bar.
  roundRect(cr, 178, 142, 554, 24, 3);
  setRgb(cr, mix(skin.bodyBottom, {0, 0, 0}, 0.35));
  cairo_fill(cr);
  setRgb(cr, {1, 1, 1}, 0.08);
  cairo_set_line_width(cr, 1.0);
  roundRect(cr, 178.5, 142.5, 553, 23, 3);
  cairo_stroke(cr);
  const std::string userIr = selectedUserIrName(shared, state);
  text(cr, cab == 2 && !userIr.empty() ? userIr.c_str()
                                       : cabName(model, cab),
       190, 159, 12, 0, 0.88, 0.90, 0.92);
  static const char* kMicNames[3] = {"DYNAMIC", "RIBBON", "BLEND"};
  const int mic = std::min(std::max(static_cast<int>(
                                        shared.get(P_CAB_MIC) + 0.5),
                                    0),
                           2);
  char micLabel[32];
  std::snprintf(micLabel, sizeof(micLabel), "%s MIC", kMicNames[mic]);
  text(cr, micLabel, 720, 159, 9, 2, 0.55, 0.57, 0.60);

  const bool two = (model == 0 || model == 1);
  struct Cone { double x, y, r; };
  static const Cone kFour[4] = {{292, 216, 44}, {455, 216, 44},
                                {292, 284, 44}, {455, 284, 44}};
  static const Cone kTwo[2] = {{355, 244, 58}, {545, 244, 58}};
  const Cone* cones = two ? kTwo : kFour;
  const int coneCount = two ? 2 : 4;
  for (int i = 0; i < coneCount; ++i) {
    const Cone& c = cones[i];
    cairo_arc(cr, c.x, c.y, c.r + 3, 0, 2 * kPi);
    setRgb(cr, {0.03, 0.03, 0.035});
    cairo_fill(cr);
    cairo_pattern_t* cone = cairo_pattern_create_radial(
        c.x - c.r * 0.25, c.y - c.r * 0.25, c.r * 0.05, c.x, c.y, c.r);
    cairo_pattern_add_color_stop_rgb(cone, 0.0, 0.22, 0.23, 0.24);
    cairo_pattern_add_color_stop_rgb(cone, 0.6, 0.12, 0.13, 0.14);
    cairo_pattern_add_color_stop_rgb(cone, 1.0, 0.05, 0.05, 0.06);
    cairo_arc(cr, c.x, c.y, c.r, 0, 2 * kPi);
    cairo_set_source(cr, cone);
    cairo_fill(cr);
    cairo_pattern_destroy(cone);
    cairo_arc(cr, c.x, c.y, c.r * 0.38, 0, 2 * kPi);
    setRgb(cr, {0.10, 0.10, 0.11});
    cairo_fill(cr);
    cairo_arc(cr, c.x, c.y, c.r * 0.20, 0, 2 * kPi);
    setRgb(cr, {0.04, 0.04, 0.05});
    cairo_fill(cr);
    cairo_set_source_rgba(cr, skin.accent.r, skin.accent.g, skin.accent.b,
                          0.18);
    cairo_set_line_width(cr, 1.0);
    cairo_arc(cr, c.x, c.y, c.r * 0.72, 0, 2 * kPi);
    cairo_stroke(cr);
  }

  // Input / output bars inside the window.
  auto bar = [&](double x, double y, double w, double v, Rgb c) {
    cairo_rectangle(cr, x, y, w, 5);
    cairo_set_source_rgb(cr, 0.04, 0.04, 0.05);
    cairo_fill(cr);
    cairo_rectangle(cr, x, y, w * clamp01(v), 5);
    setRgb(cr, c);
    cairo_fill(cr);
  };
  text(cr, "IN", 196, 320, 9, 1, 0.55, 0.57, 0.60);
  bar(206, 315, 200, state.inLevel, {0.30, 0.75, 0.42});
  text(cr, "OUT", 516, 320, 9, 1, 0.55, 0.57, 0.60);
  bar(526, 315, 200, state.outLevel, {0.90, 0.70, 0.30});
}

void Panel::drawPedalPage(cairo_t* cr, const Shared& shared,
                          const PanelState& state) const {
  (void)shared;
  const int model = static_cast<int>(shared.get(P_MODEL) + 0.5);
  const int cab = static_cast<int>(shared.get(P_CAB) + 0.5);
  const Skin skin = skinFor(model);
  text(cr, "PEDALBOARD", 520, 150, 18, 1, skin.legend.r, skin.legend.g,
       skin.legend.b);
  char sub[160];
  std::snprintf(sub, sizeof(sub), "%s  ·  %s", modelName(model),
                cabName(model, cab));
  text(cr, sub, 520, 169, 10.5, 1, skin.legendDim.r, skin.legendDim.g,
       skin.legendDim.b);

  struct Card {
    double x;
    const char* title;
    Rgb color;
  };
  const Card cards[3] = {{40, "808", {0.18, 0.42, 0.20}},
                         {370, "DELAY", {0.12, 0.23, 0.20}},
                         {700, "REVERB", {0.14, 0.20, 0.34}}};
  for (const Card& c : cards) {
    roundRect(cr, c.x, 185, 300, 380, 8);
    cairo_pattern_t* pat = cairo_pattern_create_linear(0, 185, 0, 565);
    cairo_pattern_add_color_stop_rgb(pat, 0.0, c.color.r * 1.35,
                                     c.color.g * 1.35, c.color.b * 1.35);
    cairo_pattern_add_color_stop_rgb(pat, 1.0, c.color.r * 0.55,
                                     c.color.g * 0.55, c.color.b * 0.55);
    cairo_set_source(cr, pat);
    cairo_fill(cr);
    cairo_pattern_destroy(pat);
    setRgb(cr, {0, 0, 0}, 0.35);
    cairo_set_line_width(cr, 1.0);
    roundRect(cr, c.x + 0.5, 185.5, 299, 379, 8);
    cairo_stroke(cr);
    text(cr, c.title, c.x + 150, 218, 15, 1, 0.93, 0.93, 0.90);
  }

  text(cr, "GUITAR  >  808  >  AMP  >  DELAY  >  REVERB  >  CAB", 520, 592,
       11, 1, skin.legend.r, skin.legend.g, skin.legend.b);
  char tempo[64];
  if (state.hostTempo) {
    std::snprintf(tempo, sizeof(tempo), "HOST / %.1f BPM", state.bpm);
  } else {
    std::snprintf(tempo, sizeof(tempo), "FREE / %.0f BPM", state.bpm);
  }
  text(cr, tempo, 980, 592, 10, 2, skin.legendDim.r, skin.legendDim.g,
       skin.legendDim.b);
}

void Panel::drawCabPage(cairo_t* cr, const Shared& shared,
                        const PanelState& state) const {
  const int model = static_cast<int>(shared.get(P_MODEL) + 0.5);
  const int cab = static_cast<int>(shared.get(P_CAB) + 0.5);
  const Skin skin = skinFor(model);
  text(cr, "CABINET", 520, 150, 18, 1, skin.legend.r, skin.legend.g,
       skin.legend.b);
  char sub[160];
  std::snprintf(sub, sizeof(sub), "%s  ·  %s", modelName(model),
                cabName(model, cab));
  text(cr, sub, 520, 169, 10.5, 1, skin.legendDim.r, skin.legendDim.g,
       skin.legendDim.b);

  text(cr, "FACTORY", 60, 190, 10, 0, skin.legendDim.r, skin.legendDim.g,
       skin.legendDim.b);
  text(cr, "USER", hasAlternate(model) ? 300 : 180, 190, 10, 0,
       skin.legendDim.r, skin.legendDim.g, skin.legendDim.b);

  roundRect(cr, kUserBoxX, kUserBoxY, kUserBoxW, kUserBoxH, 6);
  setRgb(cr, mix(skin.bodyBottom, {0, 0, 0}, 0.35));
  cairo_fill(cr);
  setRgb(cr, {1, 1, 1}, 0.10);
  cairo_set_line_width(cr, 1.0);
  roundRect(cr, kUserBoxX + 0.5, kUserBoxY + 0.5, kUserBoxW - 1,
            kUserBoxH - 1, 6);
  cairo_stroke(cr);
  text(cr, "USER IR FILES", 76, kUserBoxY + 20, 11, 0, skin.legend.r,
       skin.legend.g, skin.legend.b);
  text(cr, userIrDir().c_str(), 76, kUserBoxY + 35, 8, 0, skin.legendDim.r,
       skin.legendDim.g, skin.legendDim.b);

  const int selected = static_cast<int>(
      shared.get(P_USER_IR_0 + std::min(std::max(model, 0), 6)) + 0.5);
  const int files = static_cast<int>(state.userIrs.size());
  const int rows = std::min(kUserRows, files + 1);
  for (int row = 0; row < rows; ++row) {
    const int index = row + state.userIrScroll;
    if (index > files) {
      break;
    }
    const double ry = kUserRowY + row * kUserRowH;
    if (index == selected) {
      roundRect(cr, kUserBoxX + 4, ry + 1, kUserBoxW - 8, kUserRowH - 2, 3);
      setRgb(cr, skin.accent, 0.35);
      cairo_fill(cr);
    }
    char name[80];
    if (index == 0) {
      std::snprintf(name, sizeof(name), "None");
    } else {
      std::snprintf(name, sizeof(name), "%s",
                    shorten(state.userIrs[static_cast<size_t>(index - 1)], 38)
                        .c_str());
    }
    text(cr, name, kUserBoxX + 12, ry + 14, 9.5, 0,
         index == selected ? skin.btnTextOn.r : skin.legend.r,
         index == selected ? skin.btnTextOn.g : skin.legend.g,
         index == selected ? skin.btnTextOn.b : skin.legend.b);
  }
  if (files == 0) {
    text(cr, "Drop WAV files in this folder.", 76, kUserRowY + 26, 9, 0,
         skin.legendDim.r, skin.legendDim.g, skin.legendDim.b);
  }

  text(cr, "MIC", 760, 196, 10, 0, skin.legendDim.r, skin.legendDim.g,
       skin.legendDim.b);

  if (cab == 2) {
    const std::string userIr = selectedUserIrName(shared, state);
    text(cr, userIr.empty() ? "User IR — none" : userIr.c_str(), 900, 246, 13,
         1, skin.legend.r, skin.legend.g, skin.legend.b);
  } else {
    text(cr, cabName(model, cab), 900, 246, 13, 1, skin.legend.r,
         skin.legend.g, skin.legend.b);
  }
  text(cr, "Factory cabs are CC0 and embedded.", 900, 266, 9.5, 1,
       skin.legendDim.r, skin.legendDim.g, skin.legendDim.b);
}

void Panel::drawFooter(cairo_t* cr, const Shared& shared,
                       const PanelState& state) const {
  const int model = static_cast<int>(shared.get(P_MODEL) + 0.5);
  const int cab = static_cast<int>(shared.get(P_CAB) + 0.5);
  const Skin skin = skinFor(model);
  cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 0.55);
  cairo_rectangle(cr, 0, 604, kPanelWidth, 36);
  cairo_fill(cr);
  cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 0.10);
  cairo_set_line_width(cr, 1.0);
  cairo_move_to(cr, 0, 604.5);
  cairo_line_to(cr, kPanelWidth, 604.5);
  cairo_stroke(cr);

  char buf[256];
  if (state.page == 1) {
    std::snprintf(buf, sizeof(buf), "PEDALBOARD  ·  %s", modelName(model));
  } else if (state.page == 2) {
    std::snprintf(buf, sizeof(buf), "CABINET  ·  %s  ·  %s", modelName(model),
                  cabName(model, cab));
  } else {
    std::snprintf(buf, sizeof(buf), "%s  ·  %s", skin.caption,
                  cabName(model, cab));
  }
  text(cr, buf, kPanelWidth * 0.5, 629, 13.5, 1, 0.86, 0.88, 0.90);
  text(cr, "v" AMP_SUITE_VERSION, 1016, 631, 9, 2, 0.38, 0.40, 0.43);
}

void Panel::drawWidget(cairo_t* cr, const Widget& w, const Shared& shared,
                       const PanelState& state, int model) const {
  switch (w.kind) {
    case Kind::Knob:
      drawKnob(cr, w, knobValue(shared, w), model);
      break;
    case Kind::Button: {
      bool lit = false;
      if (w.param < P_COUNT) {
        const ParamSpec& spec = kParamSpecs[w.param];
        lit = spec.stepped
                  ? std::fabs(shared.get(w.param) - w.value) < 0.5
                  : shared.get(w.param) > 0.5;
      }
      drawButton(cr, w, lit, model);
      break;
    }
    case Kind::ModelTab:
      drawModelTab(cr, w, model == static_cast<int>(w.value), model);
      break;
    case Kind::Toggle:
      drawToggle(cr, w, shared.get(w.param) > 0.5, model);
      break;
    case Kind::Footswitch:
      drawFootswitch(cr, w, shared.get(w.param) > 0.5, model);
      break;
    case Kind::Pill: {
      bool on = isLit(shared, state, w);
      Widget ww = w;
      if (w.param == P_HQ) {
        ww.label = on ? "HQ" : "ECO";
      } else if (w.param == P_LOOP_POS) {
        ww.label = on ? "LOOP POST" : "LOOP PRE";
      } else if (w.param == P_DELAY_SYNC) {
        ww.label = on ? "HOST" : "FREE";
      } else if (w.param == P_DELAY_PINGPONG) {
        ww.label = on ? "PING ON" : "PING OFF";
      } else if (w.param == P_REVERB_MODE) {
        ww.label = on ? "SPRING" : "PLATE";
      } else if (w.param == P_POWER_SCALE) {
        ww.label = on ? "100" : "50";
      } else if (w.param == P_RECTIFIER) {
        ww.label = on ? "TUBE" : "SI";
      }
      drawPill(cr, ww, on, model);
      break;
    }
    case Kind::PagePill:
      drawPill(cr, w, state.page == static_cast<int>(w.value), model);
      break;
    case Kind::PresetBar:
      drawPresetBar(cr, shared, state);
      break;
    case Kind::Power: {
      const bool on = shared.get(P_POWER) > 0.5;
      const Skin skin = skinFor(model);
      cairo_arc(cr, w.x, w.y, w.r, 0, 2 * kPi);
      setRgb(cr, on ? skin.btnOn : skin.btnOff);
      cairo_fill(cr);
      cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 0.45);
      cairo_set_line_width(cr, 1.0);
      cairo_arc(cr, w.x, w.y, w.r - 0.5, 0, 2 * kPi);
      cairo_stroke(cr);
      cairo_arc(cr, w.x, w.y, w.r * 0.35, 0, 2 * kPi);
      setRgb(cr, on ? skin.btnTextOn : skin.btnText, 0.9);
      cairo_fill(cr);
      break;
    }
  }
}

void Panel::drawKnob(cairo_t* cr, const Widget& w, double value,
                     int model) const {
  const Skin skin = skinFor(model);
  const double cx = w.x;
  const double cy = w.y;
  const double r = w.r;

  setRgb(cr, skin.legendDim, 0.55);
  cairo_set_line_width(cr, 1.0);
  for (int i = 0; i <= 10; ++i) {
    const double a = knobAngle(i / 10.0);
    cairo_move_to(cr, cx + std::cos(a) * r * 1.10,
                  cy + std::sin(a) * r * 1.10);
    cairo_line_to(cr, cx + std::cos(a) * r * 1.19,
                  cy + std::sin(a) * r * 1.19);
    cairo_stroke(cr);
  }

  const ParamSpec& spec = kParamSpecs[w.param];
  if (w.scale.empty() && !spec.stepped) {
    const double a0 = knobAngle(0.0);
    const double a1 = knobAngle(1.0);
    text(cr, "\u2212", cx + std::cos(a0) * (r * 1.44),
         cy + std::sin(a0) * (r * 1.44) + 4, 12, 1, skin.legendDim.r,
         skin.legendDim.g, skin.legendDim.b);
    text(cr, "+", cx + std::cos(a1) * (r * 1.44),
         cy + std::sin(a1) * (r * 1.44) + 4, 12, 1, skin.legendDim.r,
         skin.legendDim.g, skin.legendDim.b);
  }

  for (size_t i = 0; i < w.scale.size(); ++i) {
    const double f = w.scale.size() > 1
                         ? static_cast<double>(i) / (w.scale.size() - 1)
                         : 0.0;
    const double a = knobAngle(f);
    text(cr, w.scale[i], cx + std::cos(a) * (r * w.scaleRadius),
         cy + std::sin(a) * (r * w.scaleRadius) + 4, 10, 1, skin.legend.r,
         skin.legend.g, skin.legend.b);
  }

  cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 0.18);
  cairo_arc(cr, cx + 0.5, cy + 1.5, r * 1.03, 0, 2 * kPi);
  cairo_fill(cr);

  cairo_pattern_t* body = cairo_pattern_create_linear(0, cy - r, 0, cy + r);
  const Rgb top = mix(skin.knobSkirt, {1, 1, 1}, 0.16);
  const Rgb bot = mix(skin.knobSkirt, {0, 0, 0}, 0.30);
  cairo_pattern_add_color_stop_rgb(body, 0.0, top.r, top.g, top.b);
  cairo_pattern_add_color_stop_rgb(body, 1.0, bot.r, bot.g, bot.b);
  cairo_arc(cr, cx, cy, r, 0, 2 * kPi);
  cairo_set_source(cr, body);
  cairo_fill(cr);
  cairo_pattern_destroy(body);

  cairo_set_line_width(cr, 1.0);
  cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 0.45);
  cairo_arc(cr, cx, cy, r - 0.5, 0, 2 * kPi);
  cairo_stroke(cr);

  cairo_pattern_t* cap = cairo_pattern_create_radial(
      cx - r * 0.35, cy - r * 0.40, r * 0.05, cx, cy, r * 0.98);
  const Rgb hi = mix(skin.knobCap, {1, 1, 1}, 0.24);
  cairo_pattern_add_color_stop_rgb(cap, 0.0, hi.r, hi.g, hi.b);
  cairo_pattern_add_color_stop_rgb(cap, 0.75, skin.knobCap.r,
                                   skin.knobCap.g, skin.knobCap.b);
  cairo_pattern_add_color_stop_rgb(cap, 1.0, skin.knobCapEdge.r,
                                   skin.knobCapEdge.g, skin.knobCapEdge.b);
  cairo_arc(cr, cx, cy, r * 0.82, 0, 2 * kPi);
  cairo_set_source(cr, cap);
  cairo_fill(cr);
  cairo_pattern_destroy(cap);

  if (skin.knurled) {
    cairo_set_line_width(cr, 0.8);
    cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 0.22);
    for (int k = 0; k < 60; ++k) {
      const double a = k * 2 * kPi / 60.0;
      cairo_move_to(cr, cx + std::cos(a) * r * 0.86,
                    cy + std::sin(a) * r * 0.86);
      cairo_line_to(cr, cx + std::cos(a) * r * 0.99,
                    cy + std::sin(a) * r * 0.99);
      cairo_stroke(cr);
    }
  }

  const double vmax = w.steps > 1 ? w.steps - 1 : spec.max;
  const double span = vmax - spec.min;
  const double frac =
      span > 0.0 ? (std::min(value, vmax) - spec.min) / span : 0.0;
  const double a = knobAngle(clamp01(frac));
  cairo_set_line_width(cr, 2.0);
  setRgb(cr, skin.knobIndicator);
  cairo_move_to(cr, cx + std::cos(a) * r * 0.26,
                cy + std::sin(a) * r * 0.26);
  cairo_line_to(cr, cx + std::cos(a) * r * 0.78,
                cy + std::sin(a) * r * 0.78);
  cairo_stroke(cr);

  const double labelY =
      w.labelBelow ? cy + r * 1.44 + 20 : cy - r * 1.44 - 18;
  text(cr, w.label, cx, labelY, 10.5, 1, skin.legend.r, skin.legend.g,
       skin.legend.b);
}

void Panel::drawButton(cairo_t* cr, const Widget& w, bool lit,
                       int model) const {
  const Skin skin = skinFor(model);
  const Rgb base = lit ? skin.btnOn : skin.btnOff;
  cairo_pattern_t* pat = cairo_pattern_create_linear(0, w.y, 0, w.y + w.h);
  const Rgb hi = mix(base, {1, 1, 1}, lit ? 0.10 : 0.07);
  const Rgb lo = mix(base, {0, 0, 0}, 0.10);
  cairo_pattern_add_color_stop_rgb(pat, 0.0, hi.r, hi.g, hi.b);
  cairo_pattern_add_color_stop_rgb(pat, 1.0, lo.r, lo.g, lo.b);
  roundRect(cr, w.x, w.y, w.w, w.h, 3);
  cairo_set_source(cr, pat);
  cairo_fill(cr);
  cairo_pattern_destroy(pat);
  setRgb(cr, mix(base, lit ? Rgb{1, 1, 1} : Rgb{0, 0, 0}, 0.35), 0.9);
  cairo_set_line_width(cr, 1.0);
  roundRect(cr, w.x + 0.5, w.y + 0.5, w.w - 1, w.h - 1, 3);
  cairo_stroke(cr);
  const Rgb tc = lit ? skin.btnTextOn : skin.btnText;
  text(cr, w.label, w.x + w.w * 0.5, w.y + w.h * 0.5 + 4, 10.5, 1, tc.r,
       tc.g, tc.b);
}

void Panel::drawModelTab(cairo_t* cr, const Widget& w, bool lit,
                         int model) const {
  const Skin skin = skinFor(model);
  const Rgb base = lit ? skin.tabOn : skin.tabOff;
  cairo_pattern_t* pat = cairo_pattern_create_linear(0, w.y, 0, w.y + w.h);
  const Rgb hi = mix(base, {1, 1, 1}, lit ? 0.14 : 0.06);
  const Rgb lo = mix(base, {0, 0, 0}, 0.18);
  cairo_pattern_add_color_stop_rgb(pat, 0.0, hi.r, hi.g, hi.b);
  cairo_pattern_add_color_stop_rgb(pat, 1.0, lo.r, lo.g, lo.b);
  roundRect(cr, w.x, w.y, w.w, w.h, 4);
  cairo_set_source(cr, pat);
  cairo_fill(cr);
  cairo_pattern_destroy(pat);
  setRgb(cr, {0, 0, 0}, 0.35);
  cairo_set_line_width(cr, 1.0);
  roundRect(cr, w.x + 0.5, w.y + 0.5, w.w - 1, w.h - 1, 4);
  cairo_stroke(cr);
  const Rgb tc = lit ? skin.tabTextOn : skin.tabTextOff;
  text(cr, w.label, w.x + w.w * 0.5, w.y + w.h * 0.5 + 5, 12, 1, tc.r, tc.g,
       tc.b);
}

void Panel::drawToggle(cairo_t* cr, const Widget& w, bool on,
                       int model) const {
  const Skin skin = skinFor(model);
  roundRect(cr, w.x - w.w * 0.5, w.y, w.w, w.h, 4);
  setRgb(cr, mix(skin.knobSkirt, {0, 0, 0}, 0.3));
  cairo_fill(cr);
  const double leverY = on ? w.y + 12 : w.y + w.h - 12;
  cairo_arc(cr, w.x, leverY, 8, 0, 2 * kPi);
  setRgb(cr, skin.knobCap);
  cairo_fill(cr);
  const char* top = w.buttonText.size() >= 1 ? w.buttonText[0] : w.label;
  const char* bottom = w.buttonText.size() >= 2 ? w.buttonText[1] : "";
  text(cr, top, w.x, w.y - 8, 10, 1, skin.legend.r, skin.legend.g,
       skin.legend.b);
  text(cr, bottom, w.x, w.y + w.h + 16, 10, 1, skin.legend.r, skin.legend.g,
       skin.legend.b);
}

void Panel::drawPill(cairo_t* cr, const Widget& w, bool on, int model) const {
  const Skin skin = skinFor(model);
  roundRect(cr, w.x, w.y, w.w, w.h, 5);
  setRgb(cr, on ? skin.btnOn : skin.btnOff);
  cairo_fill(cr);
  cairo_set_line_width(cr, 1.0);
  setRgb(cr, {0, 0, 0}, 0.35);
  roundRect(cr, w.x + 0.5, w.y + 0.5, w.w - 1, w.h - 1, 5);
  cairo_stroke(cr);
  const Rgb tc = on ? skin.btnTextOn : skin.btnText;
  text(cr, w.label, w.x + w.w * 0.5, w.y + w.h * 0.5 + 3.5, 9.5, 1, tc.r,
       tc.g, tc.b);
}

void Panel::drawFootswitch(cairo_t* cr, const Widget& w, bool on,
                           int model) const {
  const Skin skin = skinFor(model);
  cairo_arc(cr, w.x, w.y, w.r + 4, 0, 2 * kPi);
  setRgb(cr, mix(skin.bodyBottom, {0, 0, 0}, 0.35));
  cairo_fill(cr);
  cairo_pattern_t* pat = cairo_pattern_create_radial(
      w.x - w.r * 0.3, w.y - w.r * 0.35, w.r * 0.1, w.x, w.y, w.r);
  cairo_pattern_add_color_stop_rgb(pat, 0.0, 0.72, 0.74, 0.76);
  cairo_pattern_add_color_stop_rgb(pat, 1.0, 0.24, 0.25, 0.27);
  cairo_arc(cr, w.x, w.y, w.r, 0, 2 * kPi);
  cairo_set_source(cr, pat);
  cairo_fill(cr);
  cairo_pattern_destroy(pat);
  cairo_arc(cr, w.x, w.y, w.r * 0.55, 0, 2 * kPi);
  setRgb(cr, {0.10, 0.10, 0.11});
  cairo_fill(cr);

  cairo_arc(cr, w.x, w.y - w.r - 12, 5, 0, 2 * kPi);
  setRgb(cr, on ? skin.accent : mix(skin.accent, {0, 0, 0}, 0.75));
  cairo_fill(cr);
  text(cr, w.label, w.x, w.y + w.r + 20, 10, 1, skin.legend.r, skin.legend.g,
       skin.legend.b);
}

void Panel::drawLevelMeter(cairo_t* cr, double x, double y, double w,
                           double h, double dbL, double dbR, int model,
                           bool labelsRight, double minDb, double maxDb) const {
  const Skin skin = skinFor(model);
  const double gap = 4.0;
  const double pairWidth = w * 2.0 + gap;
  for (int ch = 0; ch < 2; ++ch) {
    const double bx = x + ch * (w + gap);
    cairo_rectangle(cr, bx, y, w, h);
    cairo_set_source_rgb(cr, 0.045, 0.045, 0.05);
    cairo_fill(cr);
    cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 0.7);
    cairo_set_line_width(cr, 1.0);
    cairo_rectangle(cr, bx + 0.5, y + 0.5, w - 1, h - 1);
    cairo_stroke(cr);

    const double v = dbToNorm(ch == 0 ? dbL : dbR, minDb, maxDb);
    const double top = y + h * (1.0 - v);
    if (v > 0.0) {
      cairo_pattern_t* pat = cairo_pattern_create_linear(0, top, 0, y + h);
      cairo_pattern_add_color_stop_rgb(pat, 0.0, 0.92, 0.25, 0.20);
      cairo_pattern_add_color_stop_rgb(pat, 0.18, 0.92, 0.78, 0.26);
      cairo_pattern_add_color_stop_rgb(pat, 0.45, 0.30, 0.82, 0.42);
      cairo_pattern_add_color_stop_rgb(pat, 1.0, 0.18, 0.62, 0.30);
      cairo_rectangle(cr, bx + 1, top, w - 2, y + h - top);
      cairo_set_source(cr, pat);
      cairo_fill(cr);
      cairo_pattern_destroy(pat);
    }
  }

  // Tick marks and dB labels sit on the inner side of each meter pair, as in
  // Comp Suite.
  const double tickX = labelsRight ? x + pairWidth : x;
  const double labelX = labelsRight ? tickX + 7.0 : tickX - 7.0;
  const int align = labelsRight ? 0 : 2;
  static const double kMarks[] = {3.0, 0.0, -6.0, -12.0,
                                  -24.0, -36.0, -48.0};
  for (double m : kMarks) {
    if (m < minDb - 0.01 || m > maxDb + 0.01) {
      continue;
    }
    const double ty = y + h * (1.0 - dbToNorm(m, minDb, maxDb));
    setRgb(cr, skin.legendDim, 0.55);
    cairo_set_line_width(cr, 1.0);
    cairo_move_to(cr, tickX + (labelsRight ? -3.0 : 3.0), ty);
    cairo_line_to(cr, tickX, ty);
    cairo_stroke(cr);
    char buf[16];
    std::snprintf(buf, sizeof(buf), m > 0.0 ? "+%.0f" : "%.0f", m);
    text(cr, buf, labelX, ty + 3.0, 7.5, align, skin.legendDim.r,
         skin.legendDim.g, skin.legendDim.b);
  }
  const char* label = labelsRight ? "IN" : "OUT";
  text(cr, label, x + w, y + h + 16, 9, 1, skin.legendDim.r, skin.legendDim.g,
       skin.legendDim.b);
}

void Panel::drawLeds(cairo_t* cr, const Shared& shared, int model) const {
  (void)model;
  const double db = shared.outPeakDb.load(std::memory_order_relaxed);
  // Twelve output segments from -30 dBFS to +3 dBFS.  The column echoes the
  // LED ladder in Comp Suite while showing amp output level.
  for (int i = 0; i < 12; ++i) {
    const double threshold = -30.0 + i * 3.0;
    const bool on = db >= threshold;
    Rgb c;
    if (i < 6) {
      c = {0.27, 0.88, 0.42};
    } else if (i < 9) {
      c = {0.96, 0.82, 0.29};
    } else {
      c = {1.0, 0.30, 0.24};
    }
    const double y = 146.0 + static_cast<double>(11 - i) * 16.0;
    roundRect(cr, 754, y, 16, 12, 2);
    if (on) {
      setRgb(cr, c);
    } else {
      setRgb(cr, {c.r * 0.18, c.g * 0.18, c.b * 0.18});
    }
    cairo_fill(cr);
  }
  text(cr, "OUT", 762, 344, 8, 1, 0.52, 0.54, 0.57);
}

std::vector<Panel::WidgetRect> Panel::widgetRects() const {
  std::vector<WidgetRect> out;
  out.reserve(widgets_.size());
  for (const Widget& w : widgets_) {
    WidgetRect r;
    r.param = w.param;
    r.label = w.label;
    r.knob = w.kind == Kind::Knob;
    r.toggle = w.kind == Kind::Toggle || w.kind == Kind::Footswitch;
    r.r = w.r;
    if (w.kind == Kind::Knob) {
      r.x = w.x - w.r;
      r.y = w.y - w.r;
      r.w = w.r * 2.0;
      r.h = w.r * 2.0;
    } else if (w.kind == Kind::Toggle) {
      r.x = w.x - w.w * 0.5;
      r.y = w.y;
      r.w = w.w;
      r.h = w.h;
    } else {
      r.x = w.x;
      r.y = w.y;
      r.w = w.w;
      r.h = w.h;
    }
    out.push_back(r);
  }
  return out;
}

} // namespace amp
