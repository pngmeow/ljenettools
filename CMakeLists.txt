cmake_minimum_required(VERSION 3.15)

project(ljenettools LANGUAGES C CXX)

set(SOURCES
    src/main.cpp
    src/sigscan.cpp

    # MinHook
    src/buffer.c
    src/hook.c
    src/trampoline.c
    src/hde/hde64.c
)

add_library(ljenettools SHARED ${SOURCES})

target_include_directories(ljenettools PRIVATE
    include
    src
    src/hde
)

target_compile_definitions(ljenettools PRIVATE
    MINHOOK_IMPLEMENTATION
    WIN32_LEAN_AND_MEAN
    _CRT_SECURE_NO_WARNINGS
)

set_target_properties(ljenettools PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    OUTPUT_NAME ljenettools
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/out
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/out
    WINDOWS_EXPORT_ALL_SYMBOLS ON
)

if (MSVC)
    # ---- Debug ----
    target_compile_options(ljenettools PRIVATE
        $<$<CONFIG:Debug>:/Zi /Od /RTC1>
        $<$<CONFIG:Debug>:/MDd>
    )

    target_link_options(ljenettools PRIVATE
        $<$<CONFIG:Debug>:/DEBUG /INCREMENTAL>
    )

    # ---- Release (safe for hooks) ----
    target_compile_options(ljenettools PRIVATE
        $<$<CONFIG:Release>:/O2 /Oi /Gy>
    )

    target_link_options(ljenettools PRIVATE
        $<$<CONFIG:Release>:/OPT:NOICF /OPT:NOREF>
    )
endif()
