// Amp Suite — layout and interaction test
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ui/panel.hpp"

#include <cairo.h>
#include <cmath>
#include <cstdio>

namespace {

bool hasParam(const std::vector<amp::Panel::WidgetRect>& rects,
              uint32_t param) {
  for (const auto& r : rects) {
    if (r.param == param) {
      return true;
    }
  }
  return false;
}

int countParam(const std::vector<amp::Panel::WidgetRect>& rects,
               uint32_t param) {
  int n = 0;
  for (const auto& r : rects) {
    if (r.param == param) {
      ++n;
    }
  }
  return n;
}

} // namespace

int main() {
  if (amp::kPanelWidth != 1040 || amp::kPanelHeight != 640) {
    std::fprintf(stderr, "unexpected panel size\n");
    return 1;
  }

  amp::Panel panel;
  amp::Shared shared;
  amp::PanelState state;

  cairo_surface_t* surface =
      cairo_image_surface_create(CAIRO_FORMAT_ARGB32, amp::kPanelWidth,
                                 amp::kPanelHeight);
  cairo_t* cr = cairo_create(surface);

  // Startup model is Block 120.
  if (std::fabs(shared.get(amp::P_MODEL) - 4.0) > 0.01) {
    std::fprintf(stderr, "startup model is not Block 120\n");
    return 1;
  }
  if (std::fabs(shared.get(amp::P_OD_ON) - 1.0) > 0.01) {
    std::fprintf(stderr, "808 is not on at startup\n");
    return 1;
  }

  // Amp page.
  state.page = 0;
  panel.draw(cr, shared, state);
  auto rects = panel.widgetRects();
  if (countParam(rects, amp::P_MODEL) != 7) {
    std::fprintf(stderr, "expected 7 model tabs\n");
    return 1;
  }
  if (!hasParam(rects, amp::P_GAIN) || !hasParam(rects, amp::P_MASTER) ||
      !hasParam(rects, amp::P_OUTPUT)) {
    std::fprintf(stderr, "amp page is missing core knobs\n");
    return 1;
  }

  // PEDALS pill opens the pedal page; clicking it again closes.
  if (!panel.pointerDown(shared, state, 728.0, 32.0, false)) {
    std::fprintf(stderr, "PEDALS pill did not consume the click\n");
    return 1;
  }
  if (state.page != 1) {
    std::fprintf(stderr, "PEDALS pill did not open the page\n");
    return 1;
  }
  panel.draw(cr, shared, state);
  rects = panel.widgetRects();
  if (!hasParam(rects, amp::P_OD_DRIVE) || !hasParam(rects, amp::P_DELAY_DIV) ||
      !hasParam(rects, amp::P_REVERB_MIX)) {
    std::fprintf(stderr, "pedal page is missing controls\n");
    return 1;
  }
  panel.pointerDown(shared, state, 728.0, 32.0, false);
  if (state.page != 0) {
    std::fprintf(stderr, "PEDALS pill did not close the page\n");
    return 1;
  }

  // CAB pill opens the cabinet page.
  panel.pointerDown(shared, state, 803.0, 32.0, false);
  if (state.page != 2) {
    std::fprintf(stderr, "CAB pill did not open the page\n");
    return 1;
  }
  panel.draw(cr, shared, state);
  rects = panel.widgetRects();
  if (!hasParam(rects, amp::P_CAB) || !hasParam(rects, amp::P_CAB_MIC) ||
      !hasParam(rects, amp::P_CAB_LEVEL)) {
    std::fprintf(stderr, "cab page is missing controls\n");
    return 1;
  }

  // A model tab changes the model while the page stays open.
  panel.draw(cr, shared, state);
  panel.pointerDown(shared, state, 380.0, 90.0, false);
  if (std::fabs(shared.get(amp::P_MODEL) - 2.0) > 0.01) {
    std::fprintf(stderr, "model tab did not switch to Crunch\n");
    return 1;
  }

  cairo_destroy(cr);
  cairo_surface_destroy(surface);
  std::printf("amp_layout: ok\n");
  return 0;
}
