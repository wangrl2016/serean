cmake_minimum_required(VERSION 3.20)

project(CCPPTutorials VERSION 1.0.0)

set(EXAMPLES
        pointer2pointer
        )

foreach (EXAMPLE ${EXAMPLES})
    add_executable(${EXAMPLE})
    target_sources(${EXAMPLE}
            PRIVATE
            examples/${EXAMPLE}.cc)

#    target_link_libraries(${EXAMPLE}
#            PRIVATE
#            ${CONAN_LIBS})
endforeach ()