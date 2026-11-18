// Amp Suite — CLAP host smoke test
// SPDX-License-Identifier: GPL-3.0-or-later

#include <clap/clap.h>

#include <dlfcn.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#include "params.hpp"

namespace {

const void* hostGetExtension(const clap_host_t*, const char*) { return nullptr; }
void hostRequestRestart(const clap_host_t*) {}
void hostRequestProcess(const clap_host_t*) {}
void hostRequestCallback(const clap_host_t*) {}

uint32_t inSize(const clap_input_events_t*) { return 0; }
const clap_event_header_t* inGet(const clap_input_events_t*, uint32_t) {
  return nullptr;
}
bool outPush(const clap_output_events_t*, const clap_event_header_t*) {
  return true;
}

using EventList = std::vector<clap_event_param_value_t>;
uint32_t evSize(const clap_input_events_t* ev) {
  const auto* list = static_cast<const EventList*>(ev->ctx);
  return list ? static_cast<uint32_t>(list->size()) : 0u;
}
const clap_event_header_t* evGet(const clap_input_events_t* ev,
                                 uint32_t index) {
  const auto* list = static_cast<const EventList*>(ev->ctx);
  if (!list || index >= list->size()) {
    return nullptr;
  }
  return &(*list)[index].header;
}

int64_t streamWrite(const clap_ostream_t* stream, const void* buffer,
                    uint64_t size) {
  auto* out = static_cast<std::vector<unsigned char>*>(stream->ctx);
  const auto* p = static_cast<const unsigned char*>(buffer);
  out->insert(out->end(), p, p + size);
  return static_cast<int64_t>(size);
}

int64_t streamRead(const clap_istream_t* stream, void* buffer, uint64_t size) {
  auto* in = static_cast<std::vector<unsigned char>*>(stream->ctx);
  if (in->empty()) {
    return 0;
  }
  const uint64_t n = std::min<uint64_t>(size, in->size());
  std::memcpy(buffer, in->data(), n);
  in->erase(in->begin(), in->begin() + static_cast<long>(n));
  return static_cast<int64_t>(n);
}

} // namespace

int main(int argc, char** argv) {
  const char* path = argc > 1 ? argv[1] : "build/clap/AmpSuite.clap";
  void* lib = dlopen(path, RTLD_NOW | RTLD_LOCAL);
  if (!lib) {
    std::fprintf(stderr, "dlopen failed: %s\n", dlerror());
    return 1;
  }
  auto* entry = static_cast<const clap_plugin_entry_t*>(
      dlsym(lib, "clap_entry"));
  if (!entry || !entry->init || !entry->get_factory) {
    std::fprintf(stderr, "clap_entry missing\n");
    return 1;
  }
  if (!entry->init(path)) {
    std::fprintf(stderr, "entry init failed\n");
    return 1;
  }
  auto* factory = static_cast<const clap_plugin_factory_t*>(
      entry->get_factory(CLAP_PLUGIN_FACTORY_ID));
  if (!factory) {
    std::fprintf(stderr, "factory missing\n");
    return 1;
  }
  if (factory->get_plugin_count(factory) != 1) {
    std::fprintf(stderr, "unexpected plugin count\n");
    return 1;
  }
  const clap_plugin_descriptor_t* desc =
      factory->get_plugin_descriptor(factory, 0);
  if (!desc || std::strcmp(desc->id, "com.tylorismusic.ampsuite") != 0) {
    std::fprintf(stderr, "unexpected descriptor\n");
    return 1;
  }

  clap_host_t host{};
  host.clap_version = CLAP_VERSION;
  host.name = "amp-suite-test";
  host.vendor = "tylorismusic";
  host.url = "https://example.invalid";
  host.version = "0.1.0";
  host.get_extension = hostGetExtension;
  host.request_restart = hostRequestRestart;
  host.request_process = hostRequestProcess;
  host.request_callback = hostRequestCallback;

  const clap_plugin_t* plugin = factory->create_plugin(factory, &host, desc->id);
  if (!plugin || !plugin->init(plugin)) {
    std::fprintf(stderr, "create/init failed\n");
    return 1;
  }

  auto* params = static_cast<const clap_plugin_params_t*>(
      plugin->get_extension(plugin, CLAP_EXT_PARAMS));
  if (!params || params->count(plugin) != amp::P_COUNT) {
    std::fprintf(stderr, "params extension missing or wrong count\n");
    return 1;
  }
  clap_param_info_t info{};
  if (!params->get_info(plugin, amp::P_MODEL, &info) ||
      std::strcmp(info.name, "Model") != 0) {
    std::fprintf(stderr, "model parameter info wrong\n");
    return 1;
  }
  char text[64];
  if (!params->value_to_text(plugin, amp::P_MODEL, 4.0, text, sizeof(text)) ||
      std::strcmp(text, "Block 120") != 0) {
    std::fprintf(stderr, "model value text wrong: %s\n", text);
    return 1;
  }

  auto* state = static_cast<const clap_plugin_state_t*>(
      plugin->get_extension(plugin, CLAP_EXT_STATE));
  if (!state) {
    std::fprintf(stderr, "state extension missing\n");
    return 1;
  }
  std::vector<unsigned char> saved;
  clap_ostream_t os{&saved, streamWrite};
  if (!state->save(plugin, &os)) {
    std::fprintf(stderr, "state save failed\n");
    return 1;
  }
  clap_istream_t is{&saved, streamRead};
  if (!state->load(plugin, &is)) {
    std::fprintf(stderr, "state load failed\n");
    return 1;
  }

  // A session saved before cab_set was added has 65 values.  It must still
  // load, with the new parameter taking its default.
  {
    struct OldHeader {
      uint32_t version;
      uint32_t count;
    };
    const OldHeader header{1u, 65u};
    std::vector<unsigned char> oldState;
    auto append = [&](const void* p, size_t n) {
      const auto* b = static_cast<const unsigned char*>(p);
      oldState.insert(oldState.end(), b, b + n);
    };
    append(&header, sizeof(header));
    for (uint32_t i = 0; i < 65u; ++i) {
      double v = amp::kParamSpecs[i].def;
      if (i == amp::P_GAIN) {
        v = 0.2;
      }
      append(&v, sizeof(v));
    }
    const double scale = 1.0;
    append(&scale, sizeof(scale));
    const uint32_t irVersion = 1u;
    const uint32_t irCount = 0u;
    append(&irVersion, sizeof(irVersion));
    append(&irCount, sizeof(irCount));
    clap_istream_t oldStream{&oldState, streamRead};
    if (!state->load(plugin, &oldStream)) {
      std::fprintf(stderr, "old 65-parameter state failed to load\n");
      return 1;
    }
    double gain = 0.0;
    double cabSet = 1.0;
    if (!params->get_value(plugin, amp::P_GAIN, &gain) ||
        !params->get_value(plugin, amp::P_CAB_SET, &cabSet) ||
        std::fabs(gain - 0.2) > 1e-6 || std::fabs(cabSet) > 1e-6) {
      std::fprintf(stderr, "old state did not map correctly (gain %.3f, cabSet %.3f)\n",
                   gain, cabSet);
      return 1;
    }
  }

  if (!plugin->activate(plugin, 48000.0, 1, 512)) {
    std::fprintf(stderr, "activate failed\n");
    return 1;
  }
  if (!plugin->start_processing(plugin)) {
    std::fprintf(stderr, "start_processing failed\n");
    return 1;
  }

  std::vector<float> l(256, 0.0f);
  std::vector<float> r(256, 0.0f);
  for (int i = 0; i < 256; ++i) {
    l[i] = 0.1f * std::sin(2.0 * 3.14159265 * 220.0 * i / 48000.0);
    r[i] = l[i];
  }
  float* inCh[2] = {l.data(), r.data()};
  float* outCh[2] = {l.data(), r.data()};
  clap_audio_buffer_t inBuf{};
  inBuf.data32 = inCh;
  inBuf.channel_count = 2;
  clap_audio_buffer_t outBuf{};
  outBuf.data32 = outCh;
  outBuf.channel_count = 2;
  clap_input_events_t inEvents{nullptr, inSize, inGet};
  clap_output_events_t outEvents{nullptr, outPush};
  clap_process_t proc{};
  proc.frames_count = 256;
  proc.audio_inputs = &inBuf;
  proc.audio_inputs_count = 1;
  proc.audio_outputs = &outBuf;
  proc.audio_outputs_count = 1;
  proc.in_events = &inEvents;
  proc.out_events = &outEvents;

  for (int block = 0; block < 8; ++block) {
    if (plugin->process(plugin, &proc) == CLAP_PROCESS_ERROR) {
      std::fprintf(stderr, "process failed\n");
      return 1;
    }
  }
  for (int i = 0; i < 256; ++i) {
    if (!std::isfinite(l[i]) || !std::isfinite(r[i])) {
      std::fprintf(stderr, "non-finite output\n");
      return 1;
    }
  }

  // End-to-end amp switch through the real CLAP parameter-event path.  The
  // output must not step even though the model changes mid-note.
  {
    EventList events;
    inEvents.ctx = &events;
    inEvents.size = evSize;
    inEvents.get = evGet;
    double phase = 0.0;
    double steadyBefore = 0.0;
    double steadyAfter = 0.0;
    double maxDelta = 0.0;
    bool audibleAfter = false;
    float previous = 0.0f;
    for (int block = 0; block < 320; ++block) {
      for (int i = 0; i < 256; ++i) {
        const float s = 0.12f * std::sin(phase);
        phase += 2.0 * 3.14159265358979 * 220.0 / 48000.0;
        l[i] = r[i] = s;
      }
      events.clear();
      if (block == 120) {
        clap_event_param_value_t ev{};
        ev.header.size = sizeof(ev);
        ev.header.time = 0;
        ev.header.space_id = CLAP_CORE_EVENT_SPACE_ID;
        ev.header.type = CLAP_EVENT_PARAM_VALUE;
        ev.param_id = amp::P_MODEL;
        ev.note_id = -1;
        ev.port_index = -1;
        ev.channel = -1;
        ev.key = -1;
        ev.value = 6.0;
        events.push_back(ev);
      }
      if (plugin->process(plugin, &proc) == CLAP_PROCESS_ERROR) {
        std::fprintf(stderr, "process failed during switch\n");
        return 1;
      }
      double energy = 0.0;
      for (int i = 0; i < 256; ++i) {
        if (!std::isfinite(l[i]) || !std::isfinite(r[i])) {
          std::fprintf(stderr, "non-finite output during switch\n");
          return 1;
        }
        const double d = std::fabs(double(l[i]) - double(previous));
        previous = l[i];
        energy += static_cast<double>(l[i]) * l[i];
        if (block >= 121 && block <= 150) {
          maxDelta = std::max(maxDelta, d);
        } else if (block >= 112 && block < 120) {
          steadyBefore = std::max(steadyBefore, d);
        } else if (block >= 200) {
          steadyAfter = std::max(steadyAfter, d);
        }
      }
      if (block >= 200 && std::sqrt(energy / 256.0) > 1.0e-4) {
        audibleAfter = true;
      }
    }
    const double allowed =
        std::max(steadyBefore, steadyAfter) * 1.5 + 0.005;
    if (!audibleAfter || maxDelta > allowed) {
      std::fprintf(stderr,
                   "CLAP amp switch stepped: audible %d maxDelta %.6f "
                   "allowed %.6f\n",
                   audibleAfter ? 1 : 0, maxDelta, allowed);
      return 1;
    }
    inEvents.ctx = nullptr;
    inEvents.size = inSize;
    inEvents.get = inGet;
  }

  plugin->stop_processing(plugin);
  plugin->deactivate(plugin);
  plugin->destroy(plugin);
  entry->deinit();
  dlclose(lib);
  std::printf("amp_clap_host: ok\n");
  return 0;
}
