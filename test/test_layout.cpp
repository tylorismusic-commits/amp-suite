// Amp Suite — layout and interaction test
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ui/panel.hpp"

#include <cairo.h>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>

#include <unistd.h>

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
  if (!panel.pointerDown(shared, state, 676.0, 32.0, false)) {
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
  panel.pointerDown(shared, state, 676.0, 32.0, false);
  if (state.page != 0) {
    std::fprintf(stderr, "PEDALS pill did not close the page\n");
    return 1;
  }

  // CAB pill opens the cabinet page.
  panel.pointerDown(shared, state, 751.0, 32.0, false);
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

  // Cab page: the BROWSE/header area opens the built-in IR browser, a WAV
  // row imports it, and Factory/Alt clear the per-amp user IR selection.
  {
    const std::string base =
        "/tmp/amp-suite-layout-ir-" + std::to_string(::getpid());
    const std::string userDir = base + "/amp-suite/irs/user";
    std::error_code ec;
    std::filesystem::remove_all(base, ec);
    std::filesystem::create_directories(userDir, ec);
    setenv("XDG_DATA_HOME", base.c_str(), 1);
    const std::string wav = userDir + "/Test IR.wav";
    {
      std::ofstream out(wav, std::ios::binary);
      const uint16_t channels = 1;
      const uint16_t bits = 16;
      const uint32_t dataSize = 64;
      const uint32_t riffSize = 36 + dataSize;
      const uint32_t fmtSize = 16;
      const uint16_t format = 1;
      const uint32_t rate = 48000;
      const uint32_t byteRate = rate * channels * bits / 8;
      const uint16_t align = channels * bits / 8;
      out.write("RIFF", 4);
      out.write(reinterpret_cast<const char*>(&riffSize), 4);
      out.write("WAVEfmt ", 8);
      out.write(reinterpret_cast<const char*>(&fmtSize), 4);
      out.write(reinterpret_cast<const char*>(&format), 2);
      out.write(reinterpret_cast<const char*>(&channels), 2);
      out.write(reinterpret_cast<const char*>(&rate), 4);
      out.write(reinterpret_cast<const char*>(&byteRate), 4);
      out.write(reinterpret_cast<const char*>(&align), 2);
      out.write(reinterpret_cast<const char*>(&bits), 2);
      out.write("data", 4);
      out.write(reinterpret_cast<const char*>(&dataSize), 4);
      for (int i = 0; i < 32; ++i) {
        const int16_t zero = 0;
        out.write(reinterpret_cast<const char*>(&zero), 2);
      }
    }
    shared.set(amp::P_MODEL, 4.0);
    shared.set(amp::P_CAB, 0.0);
    shared.set(amp::P_USER_IR_4, 1.0);
    state.page = 2;
    panel.draw(cr, shared, state);

    // Header/title area of the user IR box opens the browser.
    if (!panel.pointerDown(shared, state, 120.0, 320.0, false) ||
        !state.fileBrowserOpen) {
      std::fprintf(stderr, "user IR box did not open the browser\n");
      return 1;
    }
    panel.draw(cr, shared, state);
    // Row 0 is "..", row 1 is the WAV.
    if (!panel.pointerDown(shared, state, 240.0, 160.0, false)) {
      std::fprintf(stderr, "file browser click was not consumed\n");
      return 1;
    }
    if (std::fabs(shared.get(amp::P_USER_IR_4) - 1.0) > 0.01 ||
        std::fabs(shared.get(amp::P_CAB) - 2.0) > 0.01 ||
        state.fileBrowserOpen) {
      std::fprintf(stderr, "file browser import did not select the WAV\n");
      return 1;
    }

    // Default clears the per-amp user IR selection.
    panel.draw(cr, shared, state);
    panel.pointerDown(shared, state, 90.0, 210.0, false);
    if (std::fabs(shared.get(amp::P_CAB) - 0.0) > 0.01 ||
        std::fabs(shared.get(amp::P_USER_IR_4)) > 0.01) {
      std::fprintf(stderr, "Default did not clear the user IR selection\n");
      return 1;
    }
    // Alt also clears it.
    shared.set(amp::P_USER_IR_4, 1.0);
    panel.draw(cr, shared, state);
    panel.pointerDown(shared, state, 200.0, 210.0, false);
    if (std::fabs(shared.get(amp::P_CAB) - 1.0) > 0.01 ||
        std::fabs(shared.get(amp::P_USER_IR_4)) > 0.01) {
      std::fprintf(stderr, "Alt did not clear the user IR selection\n");
      return 1;
    }

    std::filesystem::remove_all(base, ec);
  }

  // NAM page: browse/import a model and toggle it.
  {
    const std::string base =
        "/tmp/amp-suite-layout-nam-" + std::to_string(::getpid());
    const std::string namDir = base + "/amp-suite/nam";
    std::error_code ec;
    std::filesystem::remove_all(base, ec);
    std::filesystem::create_directories(namDir, ec);
    setenv("XDG_DATA_HOME", base.c_str(), 1);
    {
      std::ofstream out(namDir + "/Test Model.nam", std::ios::binary);
      out << "{\"version\":\"0.5.4\",\"architecture\":\"WaveNet\","
             "\"config\":{}}";
    }
    shared.set(amp::P_MODEL, 4.0);
    shared.set(amp::P_NAM_MODEL, 0.0);
    shared.set(amp::P_NAM_ON, 0.0);
    state.page = 3;
    panel.draw(cr, shared, state);
    if (!panel.pointerDown(shared, state, 120.0, 320.0, false) ||
        !state.fileBrowserOpen || !state.fileBrowserNam) {
      std::fprintf(stderr, "NAM box did not open the NAM browser\n");
      return 1;
    }
    panel.draw(cr, shared, state);
    panel.pointerDown(shared, state, 240.0, 160.0, false);
    if (std::fabs(shared.get(amp::P_NAM_MODEL) - 1.0) > 0.01 ||
        shared.get(amp::P_NAM_ON) < 0.5 || state.fileBrowserOpen) {
      std::fprintf(stderr, "NAM browser import did not select the model\n");
      return 1;
    }
    panel.draw(cr, shared, state);
    panel.pointerDown(shared, state, 900.0, 312.0, false);
    if (shared.get(amp::P_NAM_ON) > 0.5) {
      std::fprintf(stderr, "NAM pill did not toggle off\n");
      return 1;
    }
    // The NAM header pill closes the page back to the amp.
    panel.pointerDown(shared, state, 817.0, 32.0, false);
    if (state.page != 0) {
      std::fprintf(stderr, "NAM header pill did not close the page\n");
      return 1;
    }
    std::filesystem::remove_all(base, ec);
  }

  // The cab page can pair any factory cab with any amp.
  {
    shared.set(amp::P_MODEL, 2.0);
    shared.set(amp::P_CAB, 0.0);
    shared.set(amp::P_CAB_SET, 0.0);
    state.page = 2;
    state.presetOpen = false;
    panel.draw(cr, shared, state);
    // Fourth factory-cab row: G12T-75 4x12.
    panel.pointerDown(shared, state, 620.0, 403.0, false);
    panel.pointerUp();
    if (shared.get(amp::P_CAB) < 0.5 ||
        std::fabs(shared.get(amp::P_CAB_SET) - 4.0) > 0.01) {
      std::fprintf(stderr, "factory cab selection did not apply: cab %.1f set %.1f\n",
                   shared.get(amp::P_CAB), shared.get(amp::P_CAB_SET));
      return 1;
    }
    panel.draw(cr, shared, state);
    // DEFAULT resets to the model's own cab.
    panel.pointerDown(shared, state, 115.0, 214.0, false);
    panel.pointerUp();
    if (shared.get(amp::P_CAB) > 0.5 ||
        std::fabs(shared.get(amp::P_CAB_SET)) > 0.01) {
      std::fprintf(stderr, "default cab did not reset the factory set\n");
      return 1;
    }
    state.page = 0;
  }

  // When the host supplies tempo, the manual BPM knob is hidden and the
  // delay page shows the host tempo instead.
  {
    state.page = 1;
    state.presetOpen = false;
    state.hostTempo = true;
    state.bpm = 90.0;
    shared.set(amp::P_DELAY_SYNC, 1.0);
    panel.draw(cr, shared, state);
    bool hasBpm = false;
    for (const auto& r : panel.widgetRects()) {
      if (r.param == amp::P_BPM) hasBpm = true;
    }
    if (hasBpm) {
      std::fprintf(stderr, "manual BPM knob shown while the host supplies tempo\n");
      return 1;
    }
    state.hostTempo = false;
    panel.draw(cr, shared, state);
    hasBpm = false;
    for (const auto& r : panel.widgetRects()) {
      if (r.param == amp::P_BPM) hasBpm = true;
    }
    if (!hasBpm) {
      std::fprintf(stderr, "manual BPM knob missing without host tempo\n");
      return 1;
    }
    state.page = 0;
  }

  // Double-click resets a knob to its preset value; right-click/typed entry
  // accepts a value (including dB suffixes).
  {
    shared.set(amp::P_MODEL, 4.0);
    shared.set(amp::P_GAIN, 0.2);
    state.page = 0;
    state.presetIndex = 0;
    panel.draw(cr, shared, state);
    panel.pointerDown(shared, state, 190.0, 420.0, false);
    panel.pointerUp();
    panel.pointerDown(shared, state, 190.0, 420.0, false);
    panel.pointerUp();
    if (std::fabs(shared.get(amp::P_GAIN) - 0.55) > 0.01) {
      std::fprintf(stderr, "double-click did not restore the preset gain: %.3f\n",
                   shared.get(amp::P_GAIN));
      return 1;
    }
    panel.beginTextEntry(shared, state, 190.0, 420.0);
    panel.textKey(shared, state, 0, '3');
    panel.textKey(shared, state, amp::Panel::kTextEnter, 0);
    if (std::fabs(shared.get(amp::P_GAIN) - 0.3) > 0.01) {
      std::fprintf(stderr, "typed gain was not applied: %.3f\n",
                   shared.get(amp::P_GAIN));
      return 1;
    }
    panel.beginTextEntry(shared, state, 900.0, 520.0);
    for (const char* p = "-6 dB"; *p; ++p) {
      panel.textKey(shared, state, 0, *p);
    }
    panel.textKey(shared, state, amp::Panel::kTextEnter, 0);
    if (std::fabs(shared.get(amp::P_OUTPUT) - 0.25) > 0.01) {
      std::fprintf(stderr, "typed output dB was not applied: %.3f\n",
                   shared.get(amp::P_OUTPUT));
      return 1;
    }
    panel.draw(cr, shared, state);
  }

  cairo_destroy(cr);
  cairo_surface_destroy(surface);
  std::printf("amp_layout: ok\n");
  return 0;
}
