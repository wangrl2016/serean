cmake_minimum_required(VERSION 3.20)

project(AudioTutorials VERSION 1.0.0)

# C++版本支持
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)


set(EXAMPLES
        wav_format
        )

foreach (EXAMPLE ${EXAMPLES})
    add_executable(${EXAMPLE})
    target_sources(${EXAMPLE}
            PRIVATE
            examples/${EXAMPLE}.cc)

    target_link_libraries(${EXAMPLE}
            PRIVATE
            ${CONAN_LIBS})
endforeach ()