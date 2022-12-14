cmake_minimum_required(VERSION 3.20)

project(SeReAn)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CONAN_INCLUDE_DIRS})

add_library(serean STATIC)
target_sources(serean
        PRIVATE
        audio/source/oscillator.cc
        audio/source/synth_sound.cc
        media/base/bit_reader.cc
        media/base/bit_reader_core.cc
        media/base/container_names.cc
)

set(TUTORIALS
        tutorials/audio_property.cc
        tutorials/mac_audio_unit.cc
        tutorials/memory_relate.cc
        tutorials/opencv_core_func.cc
        tutorials/smart_pointer.cc
        tutorials/ppm_image.cc
        )

foreach (TUTORIAL ${TUTORIALS})
    string(REGEX MATCHALL "[0-9A-Za-z_]*.cc" temp ${TUTORIAL})
    string(REGEX REPLACE ".cc" "" EXE ${temp})
    add_executable(${EXE} ${TUTORIAL})
    target_link_libraries(${EXE} serean ${CONAN_LIBS})
endforeach ()

# 单元测试
enable_testing()

add_executable(
        serean_unittest
        base/containers/queue_unittest.cc
        media/base/bit_reader_unittest.cc
)
target_link_libraries(
        serean_unittest
        serean
        ${CONAN_LIBS}
)

include(GoogleTest)
gtest_discover_tests(serean_unittest)
