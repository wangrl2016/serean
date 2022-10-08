cmake_minimum_required(VERSION 3.20)

project(SeReAn)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

set(TUTORIALS
        tutorials/android_oboe.cc
        tutorials/audio_property.cc
        tutorials/mac_audio.cc
        tutorials/memory_relate.cc
        tutorials/oscillator_generate.cc
        tutorials/smart_pointer.cc
        )

foreach (TUTORIAL ${TUTORIALS})
    string(REGEX MATCHALL "[0-9A-Za-z_]*.cc" temp ${TUTORIAL})
    string(REGEX REPLACE ".cc" "" EXE ${temp})
    add_executable(${EXE} ${TUTORIAL})
    target_link_libraries(${EXE} ${CONAN_LIBS})
endforeach ()
