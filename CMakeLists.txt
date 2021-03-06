﻿#
# File: CMakeLists.txt
#
# Main cmakelists for the trackerboy repo.
# See README.md for details on building
#

cmake_minimum_required(VERSION 3.8)

# use our custom triplet settings
# - static libraries except for Qt
# - dynamic CRT
set(VCPKG_OVERLAY_TRIPLETS "${CMAKE_SOURCE_DIR}/triplets")
set(VCPKG_OVERLAY_PORTS "${CMAKE_SOURCE_DIR}/ports")

# removes default /W3 flag for MSVC 
# https://stackoverflow.com/questions/45995784/how-to-set-compiler-options-with-cmake-in-visual-studio-2017
set(CMAKE_USER_MAKE_RULES_OVERRIDE "${CMAKE_CURRENT_LIST_DIR}/CompilerOverrides.cmake")

project (
    trackerboy 
    VERSION "0.4.0"
    LANGUAGES C CXX
)

# combines the project name and version, ie trackerboy-v1.1.2
set(PROJECT_AND_VERSION_NAME "${PROJECT_NAME}-v${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")

# uncomment if we need some cmake modules (none currently)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake ${CMAKE_MODULE_PATH})

option(ENABLE_DEMO "Enable building of demo programs (requires portaudio)" OFF)
option(ENABLE_TESTS "Enable unit tests" ON)
option(ENABLE_UI "Enable building of the main trackerboy application" ON)

if (${CMAKE_SIZEOF_VOID_P} EQUAL 4)
    set(BUILD_IS_32BIT TRUE)
else ()
    set(BUILD_IS_32BIT FALSE)
endif ()

include_directories(
    "include"
)

set(PACKAGE_MODE "CONFIG")

if (VCPKG_TOOLCHAIN)
    # hack
    # the toolchain should do this but it does not for some reason ?
    # might just be a problem with the rtmidi port
    include_directories("${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include")
    unset(USE_FIND_SCRIPTS)
else ()
    # not using vcpkg, unset this setting
    unset(VCPKG_OVERLAY_TRIPLETS)
    unset(VCPKG_OVERLAY_PORTS)

    if (DEFINED USE_FIND_SCRIPTS)
        set(PACKAGE_MODE "MODULE")
    endif ()
endif ()

if (ENABLE_DEMO OR ENABLE_UI)
    set(SOUND_REQUIRED TRUE)
else ()
    set(SOUND_REQUIRED FALSE)
endif ()

set(RTMIDI_REQUIRED ${ENABLE_UI})

if (NOT DEFINED CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE MATCHES "Debug")
    set(DEBUG_BUILD TRUE)
else ()
    set(DEBUG_BUILD FALSE)
endif ()

# ============================================================================
# Dependencies
# ============================================================================

# C++17 is required
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# gbapu is required for libtrackerboy, which is always built
add_subdirectory(external/gbapu)

if (RTMIDI_REQUIRED)
    find_package(RtMidi ${PACKAGE_MODE} REQUIRED)
endif ()

# sound related dependencies for demo and ui projects
if (SOUND_REQUIRED)

    # Miniaudio doesn't provide a CMakeLists.txt as it's a single-header library
    # we'll have to DIY it then.
    # The split version is used to speed up compilation, the single c file gets compiled
    # into a library, miniaudio.

    add_library(miniaudio STATIC "external/miniaudio.c")
    #
    # add some definitions to cut out stuff we don't need
    #
    target_compile_definitions(miniaudio PUBLIC
        MA_NO_MP3           # No MP3 decoding
        MA_NO_FLAC          # No FLAC decoding
        MA_NO_GENERATION    # No waveform generators
        MA_NO_CUSTOM        # No custom backend
    )
    if (DEBUG_BUILD)
        # MA_LOG_LEVEL_INFO for debug builds
        target_compile_definitions(miniaudio PRIVATE MA_LOG_LEVEL=3)
    endif ()
    # miniaudio dependencies
    # windows: none
    # mac: none
    # linux: pthreads, libm, libdl
    # bsd: pthreads, libm
    if (UNIX AND NOT APPLE)
       set(THREADS_PREFER_PTHREAD_FLAG ON)
       find_package(Threads REQUIRED)
       find_library(LIB_M m)
       if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
           find_library(LIB_DL dl)
           target_link_libraries(miniaudio PRIVATE ${LIB_DL})
       endif ()
       target_link_libraries(miniaudio PRIVATE Threads::Threads ${LIB_M})
    endif ()
endif ()

if (ENABLE_TESTS)
    enable_testing()
    # catch object library (compiles the header once, used by test programs)
    set(CATCH "catch")
    set(CATCH_SRC "external/catch.cpp")
    set(CATCH_OPTIONS "")
    add_library(${CATCH} OBJECT EXCLUDE_FROM_ALL ${CATCH_SRC})
    target_compile_definitions(${CATCH} PUBLIC ${CATCH_OPTIONS})

    function (add_catch_test targetVar)
        set(targetName "Catch_test_${PROJECT_NAME}")
        set(${targetVar} ${targetName} PARENT_SCOPE)
        add_executable(${targetName} ${ARGN} $<TARGET_OBJECTS:${CATCH}>)
        target_include_directories(${targetName} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
    endfunction (add_catch_test)

endif ()

include(TestBigEndian)
TEST_BIG_ENDIAN(IS_BIG_ENDIAN)

# global warning flags
add_library(trackerboyWarnings INTERFACE)
if (MSVC)
    target_compile_options(trackerboyWarnings INTERFACE
        /W4
        /experimental:external /external:W0 /external:anglebrackets
    )
elseif (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    # -Wno-missing-field-initializers prevents warnings when zero-initializing a struct
    target_compile_options(trackerboyWarnings INTERFACE
        -Wall
        -Wextra
        -pedantic
        -Wno-missing-field-initializers
    )
endif ()

# ============================================================================
# Sub-projects
# ============================================================================

#
# libtrackerboy, main library used by ui and demo programs
#
add_subdirectory(libtrackerboy)

#
# Main program (optional)
#
if (ENABLE_UI)
    add_subdirectory(ui)
endif()

#
# Demo programs (optional)
#
if (ENABLE_DEMO)
    add_subdirectory(demo)
endif ()
