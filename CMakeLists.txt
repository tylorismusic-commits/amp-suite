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
# The editor host is X11 today.  macOS builds the portable targets (DSP,
# LV2, panel, tests) until the Cocoa view lands, so X11 stays optional.
option(SUITE_NO_X11 "Skip X11 editor targets (portable subset)" OFF)
if(APPLE OR SUITE_NO_X11)
  set(SUITE_X11 OFF)
else()
  pkg_check_modules(X11 REQUIRED IMPORTED_TARGET x11)
  set(SUITE_X11 ON)
endif()
pkg_check_modules(SAMPLERATE REQUIRED IMPORTED_TARGET samplerate)

file(GLOB AMP_NAM_SOURCES
  "${CMAKE_SOURCE_DIR}/third_party/nam/NAM/*.cpp"
  "${CMAKE_SOURCE_DIR}/third_party/nam/NAM/wavenet/*.cpp")
add_library(nam_core OBJECT ${AMP_NAM_SOURCES})
target_include_directories(nam_core PUBLIC
  "${CMAKE_SOURCE_DIR}/third_party/nam/NAM"
  "${CMAKE_SOURCE_DIR}/third_party/nam/Dependencies/nlohmann"
  "${CMAKE_SOURCE_DIR}/third_party/nam/Dependencies/eigen")
target_compile_features(nam_core PUBLIC cxx_std_20)
target_compile_definitions(nam_core PUBLIC NAM_SAMPLE_FLOAT)
set_target_properties(nam_core PROPERTIES POSITION_INDEPENDENT_CODE ON)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  # NAM contains process-global registries.  Two suites loaded in one host
  # must not interpose on each other's copy, or the second dlopen aborts with
  # "Config parser already registered".
  target_compile_options(nam_core PRIVATE
    -fvisibility=hidden -fvisibility-inlines-hidden)
endif()

set(AMP_UI_SOURCES src/ui/panel.cpp)
set(AMP_DSP_SOURCES
  src/dsp/guitarix_tables.cpp
  src/dsp/power_amp.cpp
  src/dsp/ts9.cpp)
set(AMP_GUITARIX_INCLUDES
  "${CMAKE_SOURCE_DIR}/third_party/guitarix/DSP"
  "${CMAKE_SOURCE_DIR}/third_party/guitarix/DSP/tube_tables"
  "${CMAKE_SOURCE_DIR}/third_party/guitarix/plugins")

set(AMP_RESOURCE_FILES
  "${CMAKE_SOURCE_DIR}/assets/fonts/Jost-ExtraBold.ttf"
  "${CMAKE_SOURCE_DIR}/assets/fonts/OFL.txt")
file(GLOB AMP_IR_FILES "${CMAKE_SOURCE_DIR}/assets/irs/*/*.wav")

# ---------------------------------------------------------------------------
# CLAP plugin: seven amps, one cairo editor (X11 on Linux, Cocoa on macOS).
# ---------------------------------------------------------------------------
if(APPLE)
enable_language(OBJCXX)
add_library(amp_suite MODULE
  src/clap/amp_suite_clap.cpp
  ${AMP_DSP_SOURCES}
  ${AMP_UI_SOURCES}
  src/ui/cocoa_view.mm)
target_include_directories(amp_suite PRIVATE
  "${CMAKE_SOURCE_DIR}/src"
  "${CMAKE_SOURCE_DIR}/third_party/clap/include"
  ${AMP_GUITARIX_INCLUDES}
  ${CAIRO_INCLUDE_DIRS})
target_compile_definitions(amp_suite PRIVATE SUITE_MACOS_COCOA=1)
target_compile_options(amp_suite PRIVATE -fobjc-arc)
target_link_libraries(amp_suite PRIVATE
  PkgConfig::CAIRO PkgConfig::FREETYPE PkgConfig::SAMPLERATE nam_core
  "-framework Cocoa" "-framework CoreGraphics" "-framework QuartzCore"
  Threads::Threads ${CMAKE_DL_LIBS})
target_compile_features(amp_suite PRIVATE cxx_std_17)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(amp_suite PRIVATE -Wall -Wextra -fvisibility=hidden)
endif()
set_target_properties(amp_suite PROPERTIES
  PREFIX ""
  OUTPUT_NAME "AmpSuite"
  SUFFIX ".dylib"
  LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/macos")

set(AMP_CLAP_BUNDLE "${CMAKE_BINARY_DIR}/clap/AmpSuite.clap")
find_program(DYLIBBUNDLER dylibbundler)
add_custom_target(amp_suite_bundle ALL
  COMMAND ${CMAKE_COMMAND} -E remove_directory "${AMP_CLAP_BUNDLE}"
  COMMAND ${CMAKE_COMMAND} -E make_directory
          "${AMP_CLAP_BUNDLE}/Contents/MacOS"
  COMMAND ${CMAKE_COMMAND} -E copy
          "$<TARGET_FILE:amp_suite>"
          "${AMP_CLAP_BUNDLE}/Contents/MacOS/AmpSuite"
  COMMAND ${CMAKE_COMMAND} -E copy
          "${CMAKE_SOURCE_DIR}/packaging/macos/Info.plist"
          "${AMP_CLAP_BUNDLE}/Contents/Info.plist"
  COMMAND ${CMAKE_COMMAND} -E make_directory
          "${AMP_CLAP_BUNDLE}/Contents/Resources/AmpSuite.resources"
  COMMAND ${CMAKE_COMMAND} -E copy_if_different ${AMP_RESOURCE_FILES}
          "${AMP_CLAP_BUNDLE}/Contents/Resources/AmpSuite.resources/"
  COMMAND ${CMAKE_COMMAND} -E copy_directory
          "${CMAKE_SOURCE_DIR}/assets/irs"
          "${AMP_CLAP_BUNDLE}/Contents/Resources/AmpSuite.resources/irs"
  DEPENDS amp_suite ${AMP_RESOURCE_FILES} ${AMP_IR_FILES})
if(DYLIBBUNDLER)
  add_custom_command(TARGET amp_suite_bundle POST_BUILD
    COMMAND "${DYLIBBUNDLER}" -od -b
            -x "${AMP_CLAP_BUNDLE}/Contents/MacOS/AmpSuite"
            -d "${AMP_CLAP_BUNDLE}/Contents/Frameworks"
            -p "@loader_path/../Frameworks")
endif()
# ---------------------------------------------------------------------------
# VST3 and AUv2 projections of the same CLAP (clap-wrapper)
#
# Both wrappers load the .clap we just built from their own Contents/PlugIns,
# so the bundles stay self-contained: no copy of the plug-in has to be
# installed for the wrapper to work.
# ---------------------------------------------------------------------------
set(CLAP_WRAPPER_DOWNLOAD_DEPENDENCIES ON)
add_subdirectory(third_party/clap-wrapper EXCLUDE_FROM_ALL)

set(AMP_VST3_BUNDLE "${CMAKE_BINARY_DIR}/vst3/AmpSuite.vst3")
set(AMP_AUV2_BUNDLE "${CMAKE_BINARY_DIR}/au/AmpSuite.component")

add_library(amp_suite_vst3 MODULE)
add_dependencies(amp_suite_vst3 amp_suite_bundle)
target_add_vst3_wrapper(TARGET amp_suite_vst3
  OUTPUT_NAME "AmpSuite"
  BUNDLE_IDENTIFIER "com.tylorismusic.ampsuite"
  BUNDLE_VERSION "${PROJECT_VERSION}"
  ASSET_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/vst3"
  MACOS_EMBEDDED_CLAP_LOCATION "${AMP_CLAP_BUNDLE}")

add_library(amp_suite_auv2 MODULE)
add_dependencies(amp_suite_auv2 amp_suite_bundle)
target_add_auv2_wrapper(TARGET amp_suite_auv2
  OUTPUT_NAME "AmpSuite"
  BUNDLE_IDENTIFIER "com.tylorismusic.ampsuite"
  BUNDLE_VERSION "${PROJECT_VERSION}"
  MANUFACTURER_NAME "tylorismusic"
  MANUFACTURER_CODE "Tylr"
  SUBTYPE_CODE "AsSu"
  INSTRUMENT_TYPE "aufx"
  MACOS_EMBEDDED_CLAP_LOCATION "${AMP_CLAP_BUNDLE}")
# The AUv2 build helper loads the CLAP bundle to read its metadata, and
# clap-wrapper only orders it after the CLAP library -- not after our bundle
# target, whose POST_BUILD runs dylibbundler over the same bundle.  Without
# this the helper can dlopen a bundle that is being rewritten (SIGSEGV).
add_dependencies(amp_suite_auv2-build-helper amp_suite_bundle)
set_target_properties(amp_suite_auv2 PROPERTIES
  LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/au")

# The wrapper generates Contents/Info.plist with its AudioComponents entry at
# build time and copies it PRE_BUILD.  With the Makefile generator CMake then
# writes its own default bundle plist, which has no AudioComponents, so the
# component never registers (auval: "didn't find the component").  Copy the
# generated plist again POST_BUILD; the signing targets run afterwards.
add_custom_command(TARGET amp_suite_auv2 POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy
          "${CMAKE_BINARY_DIR}/amp_suite_auv2-build-helper-output/auv2_Info.plist"
          "$<TARGET_BUNDLE_DIR:amp_suite_auv2>/Contents/Info.plist"
  COMMENT "Installing the generated AUv2 Info.plist")

# Embedding the CLAP happens PRE_BUILD, which invalidates any signature the
# wrapper had, so re-sign the finished bundles.
find_program(CODESIGN codesign)
if(CODESIGN)
  add_custom_target(amp_suite_vst3_sign ALL
    COMMAND "${CODESIGN}" --force --deep --sign - "${AMP_VST3_BUNDLE}"
    COMMENT "Ad-hoc signing ${AMP_VST3_BUNDLE}"
    DEPENDS amp_suite_vst3
    VERBATIM)
  add_custom_target(amp_suite_auv2_sign ALL
    COMMAND "${CODESIGN}" --force --deep --sign - "${AMP_AUV2_BUNDLE}"
    COMMENT "Ad-hoc signing ${AMP_AUV2_BUNDLE}"
    DEPENDS amp_suite_auv2
    VERBATIM)
endif()

install(DIRECTORY "${AMP_CLAP_BUNDLE}" DESTINATION "${CLAP_DIR}")
elseif(SUITE_X11)
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
  PkgConfig::SAMPLERATE nam_core
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

add_custom_target(amp_suite_bundle ALL
  COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/clap/AmpSuite.resources"
  COMMAND ${CMAKE_COMMAND} -E copy_if_different ${AMP_RESOURCE_FILES}
          "${CMAKE_BINARY_DIR}/clap/AmpSuite.resources/"
  COMMAND ${CMAKE_COMMAND} -E copy_directory
          "${CMAKE_SOURCE_DIR}/assets/irs"
          "${CMAKE_BINARY_DIR}/clap/AmpSuite.resources/irs"
  DEPENDS amp_suite ${AMP_RESOURCE_FILES} ${AMP_IR_FILES})

install(TARGETS amp_suite LIBRARY DESTINATION "${CLAP_DIR}")
install(FILES ${AMP_RESOURCE_FILES}
        DESTINATION "${CLAP_DIR}/AmpSuite.resources")
install(DIRECTORY "${CMAKE_SOURCE_DIR}/assets/irs"
        DESTINATION "${CLAP_DIR}/AmpSuite.resources")
endif()  # APPLE / SUITE_X11  # SUITE_X11

# ---------------------------------------------------------------------------
# LV2 plugin and X11 UI
# ---------------------------------------------------------------------------
add_library(amp_suite_lv2 MODULE
  src/amp_suite_lv2.cpp
  ${AMP_DSP_SOURCES})
target_include_directories(amp_suite_lv2 PRIVATE
  "${CMAKE_SOURCE_DIR}/src"
  ${AMP_GUITARIX_INCLUDES})
target_link_libraries(amp_suite_lv2 PRIVATE PkgConfig::LV2 PkgConfig::SAMPLERATE nam_core Threads::Threads)
target_compile_features(amp_suite_lv2 PRIVATE cxx_std_17)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(amp_suite_lv2 PRIVATE -Wall -Wextra -fvisibility=hidden)
endif()
set_target_properties(amp_suite_lv2 PROPERTIES
  PREFIX ""
  OUTPUT_NAME "amp_suite"
  SUFFIX ".so"
  LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lv2/amp-suite.lv2")

if(SUITE_X11)
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
endif()  # SUITE_X11

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
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
          "${CMAKE_SOURCE_DIR}/assets/fonts/Jost-ExtraBold.ttf"
          "${CMAKE_SOURCE_DIR}/assets/fonts/OFL.txt"
          "${CMAKE_BINARY_DIR}/lv2/amp-suite.lv2/"
  COMMAND ${CMAKE_COMMAND} -E copy_directory
          "${CMAKE_SOURCE_DIR}/assets/irs"
          "${CMAKE_BINARY_DIR}/lv2/amp-suite.lv2/irs"
  DEPENDS amp_suite_lv2 amp_lv2_ttl ${AMP_IR_FILES})
if(SUITE_X11)
  add_dependencies(amp_lv2_bundle amp_suite_ui)
endif()

install(TARGETS amp_suite_lv2
        LIBRARY DESTINATION "${LV2_DIR}/amp-suite.lv2")
if(SUITE_X11)
  install(TARGETS amp_suite_ui
          LIBRARY DESTINATION "${LV2_DIR}/amp-suite.lv2")
endif()
install(FILES
          "${CMAKE_SOURCE_DIR}/lv2/amp_suite.ttl"
          "${CMAKE_SOURCE_DIR}/lv2/manifest.ttl"
          "${CMAKE_SOURCE_DIR}/lv2/presets.ttl"
          "${CMAKE_SOURCE_DIR}/assets/fonts/Jost-ExtraBold.ttf"
          "${CMAKE_SOURCE_DIR}/assets/fonts/OFL.txt"
        DESTINATION "${LV2_DIR}/amp-suite.lv2")
install(DIRECTORY "${CMAKE_SOURCE_DIR}/assets/irs"
        DESTINATION "${LV2_DIR}/amp-suite.lv2")

# ---------------------------------------------------------------------------
# Tests and tools
# ---------------------------------------------------------------------------
add_executable(amp_gain_levels_test test/test_gain_levels.cpp ${AMP_DSP_SOURCES})
target_include_directories(amp_gain_levels_test PRIVATE
  "${CMAKE_SOURCE_DIR}/src"
  ${AMP_GUITARIX_INCLUDES})
target_link_libraries(amp_gain_levels_test PRIVATE PkgConfig::SAMPLERATE nam_core Threads::Threads)
target_compile_features(amp_gain_levels_test PRIVATE cxx_std_17)
add_test(NAME amp_gain_levels COMMAND amp_gain_levels_test)

add_executable(amp_dsp_test test/test_dsp.cpp ${AMP_DSP_SOURCES})
target_include_directories(amp_dsp_test PRIVATE
  "${CMAKE_SOURCE_DIR}/src"
  ${AMP_GUITARIX_INCLUDES})
target_link_libraries(amp_dsp_test PRIVATE PkgConfig::SAMPLERATE nam_core Threads::Threads)
target_compile_definitions(amp_dsp_test PRIVATE
  AMP_TEST_ASSETS="${CMAKE_SOURCE_DIR}/assets/irs")
target_compile_features(amp_dsp_test PRIVATE cxx_std_17)
add_test(NAME amp_dsp COMMAND amp_dsp_test)

add_executable(amp_user_ir_test test/test_user_ir.cpp ${AMP_DSP_SOURCES})
target_include_directories(amp_user_ir_test PRIVATE
  "${CMAKE_SOURCE_DIR}/src"
  ${AMP_GUITARIX_INCLUDES})
target_link_libraries(amp_user_ir_test PRIVATE PkgConfig::SAMPLERATE nam_core Threads::Threads)
target_compile_features(amp_user_ir_test PRIVATE cxx_std_17)
add_test(NAME amp_user_ir COMMAND amp_user_ir_test)

add_executable(amp_nam_test test/test_nam.cpp ${AMP_DSP_SOURCES})
target_include_directories(amp_nam_test PRIVATE
  "${CMAKE_SOURCE_DIR}/src"
  ${AMP_GUITARIX_INCLUDES})
target_link_libraries(amp_nam_test PRIVATE PkgConfig::SAMPLERATE nam_core Threads::Threads)
target_compile_definitions(amp_nam_test PRIVATE
  NAM_TEST_MODEL="${CMAKE_SOURCE_DIR}/test/fixtures/wavenet.nam")
target_compile_features(amp_nam_test PRIVATE cxx_std_17)
add_test(NAME amp_nam COMMAND amp_nam_test)

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

if(SUITE_X11)
add_executable(amp_clap_host_test test/clap_host_test.cpp)
target_include_directories(amp_clap_host_test PRIVATE
  "${CMAKE_SOURCE_DIR}/src"
  "${CMAKE_SOURCE_DIR}/third_party/clap/include")
target_link_libraries(amp_clap_host_test PRIVATE ${CMAKE_DL_LIBS})
target_compile_features(amp_clap_host_test PRIVATE cxx_std_17)
add_dependencies(amp_clap_host_test amp_suite_bundle)
add_test(NAME amp_clap_host
  COMMAND amp_clap_host_test "${CMAKE_BINARY_DIR}/clap/AmpSuite.clap")

add_executable(amp_clap_user_ir_test test/clap_user_ir_test.cpp)
target_include_directories(amp_clap_user_ir_test PRIVATE
  "${CMAKE_SOURCE_DIR}/src"
  "${CMAKE_SOURCE_DIR}/third_party/clap/include")
target_link_libraries(amp_clap_user_ir_test PRIVATE ${CMAKE_DL_LIBS})
target_compile_features(amp_clap_user_ir_test PRIVATE cxx_std_17)
add_test(NAME amp_clap_user_ir
  COMMAND amp_clap_user_ir_test "$<TARGET_FILE:amp_suite>")

add_executable(amp_clap_nam_test test/clap_nam_test.cpp)
target_include_directories(amp_clap_nam_test PRIVATE
  "${CMAKE_SOURCE_DIR}/src"
  "${CMAKE_SOURCE_DIR}/third_party/clap/include")
target_link_libraries(amp_clap_nam_test PRIVATE ${CMAKE_DL_LIBS})
target_compile_definitions(amp_clap_nam_test PRIVATE
  NAM_TEST_MODEL="${CMAKE_SOURCE_DIR}/test/fixtures/wavenet.nam")
target_compile_features(amp_clap_nam_test PRIVATE cxx_std_17)
add_test(NAME amp_clap_nam
  COMMAND amp_clap_nam_test "$<TARGET_FILE:amp_suite>")
endif()  # SUITE_X11

add_executable(amp_tone_probe tools/tone_probe.cpp)
target_include_directories(amp_tone_probe PRIVATE "${CMAKE_SOURCE_DIR}/src")
target_link_libraries(amp_tone_probe PRIVATE PkgConfig::SAMPLERATE nam_core Threads::Threads)
target_compile_features(amp_tone_probe PRIVATE cxx_std_17)

add_executable(amp_probe tools/amp_probe.cpp ${AMP_DSP_SOURCES})
target_include_directories(amp_probe PRIVATE
  "${CMAKE_SOURCE_DIR}/src"
  ${AMP_GUITARIX_INCLUDES})
target_link_libraries(amp_probe PRIVATE PkgConfig::SAMPLERATE nam_core Threads::Threads)
target_compile_features(amp_probe PRIVATE cxx_std_17)
