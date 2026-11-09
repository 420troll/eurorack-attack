cmake_minimum_required(VERSION 3.16)
project(ChakraPlaitsReference LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(PLAITS_ROOT "" CACHE PATH "Path to pichenettes/eurorack checkout with initialized stmlib")
if(NOT EXISTS "${PLAITS_ROOT}/plaits/dsp/voice.h" OR NOT EXISTS "${PLAITS_ROOT}/stmlib/utils/buffer_allocator.h")
  message(FATAL_ERROR "Set -DPLAITS_ROOT=/path/to/eurorack after git clone --recursive https://github.com/pichenettes/eurorack.git")
endif()
# Firmware sources are intentionally excluded; the synth engine runs natively.
file(GLOB_RECURSE PLAITS_DSP CONFIGURE_DEPENDS
  "${PLAITS_ROOT}/plaits/dsp/*.cc"
  "${PLAITS_ROOT}/plaits/resources/*.cc"
  "${PLAITS_ROOT}/stmlib/dsp/*.cc"
  "${PLAITS_ROOT}/stmlib/utils/*.cc")
add_library(chakra_plaits STATIC ${PLAITS_DSP})
target_include_directories(chakra_plaits PUBLIC "${PLAITS_ROOT}")
# Plaits' user_data.h uses a desktop mock under TEST rather than MCU flash.
target_compile_definitions(chakra_plaits PUBLIC TEST)
target_compile_options(chakra_plaits PRIVATE $<$<CXX_COMPILER_ID:GNU,Clang>:-Wno-unused-parameter>)
add_executable(chakra_native_render src/render.cpp)
target_link_libraries(chakra_native_render PRIVATE chakra_plaits)
if(UNIX AND NOT APPLE)
  target_link_libraries(chakra_native_render PRIVATE m)
endif()
enable_testing()
add_test(NAME render_chord COMMAND chakra_native_render "${CMAKE_CURRENT_BINARY_DIR}/chord.wav" 639 chord 1)
add_test(NAME render_additive COMMAND chakra_native_render "${CMAKE_CURRENT_BINARY_DIR}/additive.wav" 528 additive 1)
add_test(NAME render_wavetable COMMAND chakra_native_render "${CMAKE_CURRENT_BINARY_DIR}/wavetable.wav" 396 wavetable 1)
