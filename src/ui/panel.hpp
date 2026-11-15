// Amp Suite — panel.hpp
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <cairo.h>

#include <string>
#include <vector>

#include "params.hpp"

namespace amp {

constexpr int kPanelWidth = 1040;
constexpr int kPanelHeight = 640;

// UI-only state.  The page is not a host parameter and is not saved.
struct PanelState {
  int page = 0;  // 0 amp, 1 pedals, 2 cab
  int presetIndex = 0;
  bool presetDirty = false;
  bool presetOpen = false;
  int presetScroll = 0;
  float inLevel = 0.0f;
  float outLevel = 0.0f;
  float inLevelL = 0.0f;
  float inLevelR = 0.0f;
  float outLevelL = 0.0f;
  float outLevelR = 0.0f;
  float outPeakHold = 0.0f;
  float bpm = 120.0f;
  bool hostTempo = false;
};

class Panel {
public:
  Panel();
  ~Panel();
  Panel(const Panel&) = delete;
  Panel& operator=(const Panel&) = delete;

  void loadFont(const std::string& dir);
  void tick(const Shared& shared, PanelState& state) const;
  void draw(cairo_t* cr, const Shared& shared, const PanelState& state);

  bool pointerDown(Shared& shared, PanelState& state, double x, double y,
                   bool fine);
  bool pointerMove(Shared& shared, PanelState& state, double x, double y,
                   bool fine);
  void pointerUp();
  bool handlePresetClick(Shared& shared, PanelState& state, double x, double y);
  void applyPreset(Shared& shared, PanelState& state, int index) const;
  void drawPresetBar(cairo_t* cr, const Shared& shared,
                     const PanelState& state) const;
  void drawPresetList(cairo_t* cr, const Shared& shared,
                      const PanelState& state) const;
  void markEdited(PanelState& state) const { state.presetDirty = true; }
  bool wheel(Shared& shared, PanelState& state, double x, double y, double dy);

  bool dragging() const { return dragParam_ >= 0; }

  struct WidgetRect {
    uint32_t param = 0;
    double x = 0.0;
    double y = 0.0;
    double w = 0.0;
    double h = 0.0;
    double r = 0.0;
    const char* label = "";
    bool knob = false;
    bool toggle = false;
  };
  std::vector<WidgetRect> widgetRects() const;

private:
  enum class Kind {
    Knob,
    Button,
    Toggle,
    ModelTab,
    Power,
    PagePill,
    Footswitch,
    PresetBar,
    Pill,
  };

  struct Widget {
    Kind kind;
    uint32_t param = 0;
    double value = 0.0;
    double x = 0.0;
    double y = 0.0;
    double w = 0.0;
    double h = 0.0;
    double r = 0.0;
    const char* label = "";
    std::vector<const char*> scale;
    bool labelBelow = false;
    std::vector<const char*> buttonText;
    int steps = 0;
    double scaleRadius = 1.58;
  };

  void layout(const Shared& shared, const PanelState& state);
  void drawHeader(cairo_t* cr, const Shared& shared,
                  const PanelState& state) const;
  void drawTabs(cairo_t* cr, const Shared& shared) const;
  void drawAmpPage(cairo_t* cr, const Shared& shared,
                   const PanelState& state) const;
  void drawPedalPage(cairo_t* cr, const Shared& shared,
                     const PanelState& state) const;
  void drawCabPage(cairo_t* cr, const Shared& shared,
                   const PanelState& state) const;
  void drawFooter(cairo_t* cr, const Shared& shared,
                  const PanelState& state) const;
  void drawWidget(cairo_t* cr, const Widget& w, const Shared& shared,
                  const PanelState& state, int model) const;
  void drawKnob(cairo_t* cr, const Widget& w, double value, int model) const;
  void drawButton(cairo_t* cr, const Widget& w, bool lit, int model) const;
  void drawModelTab(cairo_t* cr, const Widget& w, bool lit, int model) const;
  void drawToggle(cairo_t* cr, const Widget& w, bool on, int model) const;
  void drawPill(cairo_t* cr, const Widget& w, bool on, int model) const;
  void drawFootswitch(cairo_t* cr, const Widget& w, bool on, int model) const;
  void drawLevelMeter(cairo_t* cr, double x, double y, double w, double h,
                      double dbL, double dbR, int model,
                      bool labelsRight) const;
  void drawCabWindow(cairo_t* cr, const Shared& shared,
                     const PanelState& state, int model) const;
  void text(cairo_t* cr, const char* s, double x, double y, double size,
            int align, double r, double g, double b) const;

  double knobValue(const Shared& shared, const Widget& w) const;
  bool isLit(const Shared& shared, const PanelState& state,
             const Widget& w) const;
  bool hitWidget(const Widget& w, double x, double y) const;

  std::vector<Widget> widgets_;
  int dragParam_ = -1;
  double dragStartY_ = 0.0;
  double dragStartValue_ = 0.0;

  cairo_surface_t* grain_ = nullptr;
  void* ft_ = nullptr;
  void* ftFace_ = nullptr;
  cairo_font_face_t* font_ = nullptr;
  cairo_font_face_t* fallbackFont_ = nullptr;
};

} // namespace amp
