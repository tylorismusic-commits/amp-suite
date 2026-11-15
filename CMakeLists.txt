cmake_minimum_required(VERSION 3.16)
project(amp_suite VERSION 0.2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()
include(GNUInstallDirs)

enable_testing()

set(CLAP_DIR "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/clap"
    CACHE PATH "Directory where CLAP bundles are installed")
set(LV2_DIR "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/lv2"
    CACHE PATH "Directory where the LV2 bundle is installed")

find_package(PkgConfig REQUIRED)
find_package(Threads REQUIRED)
pkg_check_modules(LV2 REQUIRED IMPORTED_TARGET lv2)
pkg_check_modules(CAIRO REQUIRED IMPORTED_TARGET cairo)
pkg_check_modules(FREETYPE REQUIRED IMPORTED_TARGET freetype2)
pkg_check_modules(X11 REQUIRED IMPORTED_TARGET x11)

set(AMP_UI_SOURCES src/ui/panel.cpp)
set(AMP_DSP_SOURCES
  src/dsp/guitarix_tables.cpp
  src/dsp/power_amp.cpp)
set(AMP_GUITARIX_INCLUDES
  "${CMAKE_SOURCE_DIR}/third_party/guitarix/DSP"
  "${CMAKE_SOURCE_DIR}/third_party/guitarix/DSP/tube_tables")

# ---------------------------------------------------------------------------
# CLAP plugin: seven amps, one X11/Cairo editor.
# ---------------------------------------------------------------------------
add_library(amp_suite MODULE
  src/clap/amp_suite_clap.cpp
  ${AMP_DSP_SOURCES}
  ${AMP_UI_SOURCES})
target_include_directories(amp_suite PRIVATE
  "${CMAKE_SOURCE_DIR}/src"
  "${CMAKE_SOURCE_DIR}/third_party/clap/include"
  ${AMP_GUITARIX_INCLUDES})
target_link_libraries(amp_suite PRIVATE
  PkgConfig::CAIRO PkgConfig::FREETYPE PkgConfig::X11
  Threads::Threads ${CMAKE_DL_LIBS})
target_compile_features(amp_suite PRIVATE cxx_std_17)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(amp_suite PRIVATE -Wall -Wextra -fvisibility=hidden)
endif()
set_target_properties(amp_suite PROPERTIES
  PREFIX ""
  OUTPUT_NAME "AmpSuite"
  SUFFIX ".clap"
  LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/clap")

set(AMP_RESOURCE_FILES
  "${CMAKE_SOURCE_DIR}/assets/fonts/Jost-ExtraBold.ttf"
  "${CMAKE_SOURCE_DIR}/assets/fonts/OFL.txt")

add_custom_target(amp_suite_bundle ALL
  COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/clap/AmpSuite.resources"
  COMMAND ${CMAKE_COMMAND} -E copy_if_different ${AMP_RESOURCE_FILES}
          "${CMAKE_BINARY_DIR}/clap/AmpSuite.resources/"
  DEPENDS amp_suite ${AMP_RESOURCE_FILES})

install(TARGETS amp_suite LIBRARY DESTINATION "${CLAP_DIR}")
install(FILES ${AMP_RESOURCE_FILES}
        DESTINATION "${CLAP_DIR}/AmpSuite.resources")

# ---------------------------------------------------------------------------
# LV2 plugin and X11 UI
# ---------------------------------------------------------------------------
add_library(amp_suite_lv2 MODULE
  src/amp_suite_lv2.cpp
  ${AMP_DSP_SOURCES})
target_include_directories(amp_suite_lv2 PRIVATE
  "${CMAKE_SOURCE_DIR}/src"
  ${AMP_GUITARIX_INCLUDES})
target_link_libraries(amp_suite_lv2 PRIVATE PkgConfig::LV2)
target_compile_features(amp_suite_lv2 PRIVATE cxx_std_17)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(amp_suite_lv2 PRIVATE -Wall -Wextra -fvisibility=hidden)
endif()
set_target_properties(amp_suite_lv2 PROPERTIES
  PREFIX ""
  OUTPUT_NAME "amp_suite"
  SUFFIX ".so"
  LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lv2/amp-suite.lv2")

add_library(amp_suite_ui MODULE
  src/ui/lv2_ui.cpp
  ${AMP_UI_SOURCES})
target_include_directories(amp_suite_ui PRIVATE
  "${CMAKE_SOURCE_DIR}/src"
  "${CMAKE_SOURCE_DIR}/third_party/clap/include")
target_link_libraries(amp_suite_ui PRIVATE
  PkgConfig::LV2 PkgConfig::CAIRO PkgConfig::FREETYPE PkgConfig::X11
  Threads::Threads ${CMAKE_DL_LIBS})
target_compile_features(amp_suite_ui PRIVATE cxx_std_17)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(amp_suite_ui PRIVATE -Wall -Wextra -fvisibility=hidden)
endif()
set_target_properties(amp_suite_ui PROPERTIES
  PREFIX ""
  OUTPUT_NAME "amp_suite_ui"
  SUFFIX ".so"
  LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lv2/amp-suite.lv2")

# Metadata is generated from params.hpp/presets.hpp.
add_executable(amp_ttl_gen tools/gen_ttl.cpp)
target_include_directories(amp_ttl_gen PRIVATE "${CMAKE_SOURCE_DIR}/src")
target_compile_features(amp_ttl_gen PRIVATE cxx_std_17)

add_custom_target(amp_lv2_ttl ALL
  COMMAND $<TARGET_FILE:amp_ttl_gen> --root "${CMAKE_SOURCE_DIR}"
  DEPENDS amp_ttl_gen)
add_custom_target(amp_lv2_bundle ALL
  COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/lv2/amp-suite.lv2"
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
          "${CMAKE_SOURCE_DIR}/lv2/amp_suite.ttl"
          "${CMAKE_SOURCE_DIR}/lv2/manifest.ttl"
          "${CMAKE_SOURCE_DIR}/lv2/presets.ttl"
          "${CMAKE_BINARY_DIR}/lv2/amp-suite.lv2/"
  DEPENDS amp_suite_lv2 amp_suite_ui amp_lv2_ttl)

install(TARGETS amp_suite_lv2 amp_suite_ui
        LIBRARY DESTINATION "${LV2_DIR}/amp-suite.lv2")
install(FILES
          "${CMAKE_SOURCE_DIR}/lv2/amp_suite.ttl"
          "${CMAKE_SOURCE_DIR}/lv2/manifest.ttl"
          "${CMAKE_SOURCE_DIR}/lv2/presets.ttl"
        DESTINATION "${LV2_DIR}/amp-suite.lv2")

# ---------------------------------------------------------------------------
# Tests and tools
# ---------------------------------------------------------------------------
add_executable(amp_dsp_test test/test_dsp.cpp ${AMP_DSP_SOURCES})
target_include_directories(amp_dsp_test PRIVATE
  "${CMAKE_SOURCE_DIR}/src"
  ${AMP_GUITARIX_INCLUDES})
target_compile_features(amp_dsp_test PRIVATE cxx_std_17)
add_test(NAME amp_dsp COMMAND amp_dsp_test)

add_executable(amp_panel_test test/test_panel.cpp ${AMP_UI_SOURCES})
target_include_directories(amp_panel_test PRIVATE "${CMAKE_SOURCE_DIR}/src")
target_link_libraries(amp_panel_test PRIVATE PkgConfig::CAIRO PkgConfig::FREETYPE)
target_compile_features(amp_panel_test PRIVATE cxx_std_17)
add_test(NAME amp_panel COMMAND amp_panel_test)

add_executable(amp_layout_test test/test_layout.cpp ${AMP_UI_SOURCES})
target_include_directories(amp_layout_test PRIVATE "${CMAKE_SOURCE_DIR}/src")
target_link_libraries(amp_layout_test PRIVATE PkgConfig::CAIRO PkgConfig::FREETYPE)
target_compile_features(amp_layout_test PRIVATE cxx_std_17)
add_test(NAME amp_layout COMMAND amp_layout_test)

add_executable(amp_geometry_test test/test_geometry.cpp)
target_include_directories(amp_geometry_test PRIVATE "${CMAKE_SOURCE_DIR}/src")
target_link_libraries(amp_geometry_test PRIVATE PkgConfig::CAIRO)
target_compile_features(amp_geometry_test PRIVATE cxx_std_17)
add_test(NAME amp_geometry COMMAND amp_geometry_test)

add_executable(amp_ui_preview test/ui_preview.cpp ${AMP_UI_SOURCES})
target_include_directories(amp_ui_preview PRIVATE "${CMAKE_SOURCE_DIR}/src")
target_link_libraries(amp_ui_preview PRIVATE PkgConfig::CAIRO PkgConfig::FREETYPE)
target_compile_features(amp_ui_preview PRIVATE cxx_std_17)

add_test(NAME amp_ttl
  COMMAND amp_ttl_gen --check --root "${CMAKE_SOURCE_DIR}")

pkg_check_modules(LILV IMPORTED_TARGET lilv-0)
if(LILV_FOUND)
  add_executable(amp_lv2_host_test test/lv2_host_test.cpp)
  target_include_directories(amp_lv2_host_test PRIVATE "${CMAKE_SOURCE_DIR}/src")
  target_link_libraries(amp_lv2_host_test PRIVATE PkgConfig::LILV PkgConfig::LV2)
  target_compile_features(amp_lv2_host_test PRIVATE cxx_std_17)
  add_dependencies(amp_lv2_host_test amp_lv2_bundle)
  add_test(NAME amp_lv2_host
    COMMAND amp_lv2_host_test "${CMAKE_BINARY_DIR}/lv2/amp-suite.lv2")
endif()

add_executable(amp_clap_host_test test/clap_host_test.cpp)
target_include_directories(amp_clap_host_test PRIVATE
  "${CMAKE_SOURCE_DIR}/src"
  "${CMAKE_SOURCE_DIR}/third_party/clap/include")
target_link_libraries(amp_clap_host_test PRIVATE ${CMAKE_DL_LIBS})
target_compile_features(amp_clap_host_test PRIVATE cxx_std_17)
add_dependencies(amp_clap_host_test amp_suite_bundle)
add_test(NAME amp_clap_host
  COMMAND amp_clap_host_test "${CMAKE_BINARY_DIR}/clap/AmpSuite.clap")

add_executable(amp_tone_probe tools/tone_probe.cpp)
target_include_directories(amp_tone_probe PRIVATE "${CMAKE_SOURCE_DIR}/src")
target_compile_features(amp_tone_probe PRIVATE cxx_std_17)

add_executable(amp_probe tools/amp_probe.cpp ${AMP_DSP_SOURCES})
target_include_directories(amp_probe PRIVATE
  "${CMAKE_SOURCE_DIR}/src"
  ${AMP_GUITARIX_INCLUDES})
target_compile_features(amp_probe PRIVATE cxx_std_17)
