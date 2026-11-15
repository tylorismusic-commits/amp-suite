// Amp Suite — panel render smoke test
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ui/panel.hpp"

#include <cairo.h>

int main() {
  amp::Panel panel;
  amp::Shared shared;
  amp::PanelState state;

  cairo_surface_t* surface =
      cairo_image_surface_create(CAIRO_FORMAT_ARGB32, amp::kPanelWidth,
                                 amp::kPanelHeight);
  cairo_t* cr = cairo_create(surface);

  for (int page = 0; page < 3; ++page) {
    state.page = page;
    panel.tick(shared, state);
    panel.draw(cr, shared, state);
  }

  // Every model, every page, must draw without a cairo error.
  for (int model = 0; model < 7; ++model) {
    shared.set(amp::P_MODEL, static_cast<double>(model));
    for (int page = 0; page < 3; ++page) {
      state.page = page;
      panel.draw(cr, shared, state);
    }
  }

  cairo_destroy(cr);
  cairo_surface_destroy(surface);
  return 0;
}
