cmake_minimum_required(VERSION 3.20)

project(JuceTutorials VERSION 1.0.0)

# C++版本支持
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_subdirectory(JUCE)

set(EXAMPLES
        advanced_audio_sample
        audio_input
        audio_player
        audio_sample_buffer
        )

foreach (EXAMPLE ${EXAMPLES})
    juce_add_gui_app(${EXAMPLE})
    juce_generate_juce_header(${EXAMPLE})
    target_sources(${EXAMPLE}
            PRIVATE
            examples/${EXAMPLE}.cc)

    target_compile_definitions(${EXAMPLE}
            PRIVATE
            JUCE_WEB_BROWSER=0
            JUCE_USE_CURL=0)

    target_link_libraries(${EXAMPLE}
            PRIVATE
            juce::juce_audio_basics
            juce::juce_audio_devices
            juce::juce_audio_formats
            juce::juce_audio_utils
            juce::juce_core
            juce::juce_events
            juce::juce_gui_extra
            juce::juce_graphics
            PUBLIC
            juce::juce_recommended_config_flags
            juce::juce_recommended_lto_flags
            juce::juce_recommended_warning_flags)
endforeach ()

