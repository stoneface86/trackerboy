#
# File: CMakeLists.txt
#
# Main cmakelists for the trackerboy repo.
# See README.md for details on building
#

cmake_minimum_required(VERSION 3.9)

# use our custom triplet settings
# - static libraries except for Qt
# - dynamic CRT
set(VCPKG_OVERLAY_TRIPLETS "${CMAKE_SOURCE_DIR}/vcpkg-overlays/triplets")
set(VCPKG_OVERLAY_PORTS "${CMAKE_SOURCE_DIR}/vcpkg-overlays/ports")

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

option(ENABLE_DEMO "Enable building of demo programs (requires miniaudio)" OFF)
option(ENABLE_TESTS "Enable unit tests" ON)
option(ENABLE_UI "Enable building of the main trackerboy application" ON)
option(ENABLE_UNITY "Enable unity builds" OFF)

if (${CMAKE_SIZEOF_VOID_P} EQUAL 4)
    set(BUILD_ARCH "x86")
    set(BUILD_IS_32BIT TRUE)
else ()
    set(BUILD_ARCH "x64")
    set(BUILD_IS_32BIT FALSE)
endif ()

#
# Sets the UNITY_BUILD property to ON for the given target if:
#  1. ENABLE_UNITY is ON
#  2. CMake version is >= 3.16
#
function (unityBuildable target)
    if (${CMAKE_VERSION} VERSION_GREATER "3.15")
        if (ENABLE_UNITY)
            set_target_properties(${target} PROPERTIES UNITY_BUILD ON)
        endif ()
    endif ()
endfunction ()

if (NOT VCPKG_TOOLCHAIN)
    # not using vcpkg, unset this setting
    unset(VCPKG_OVERLAY_TRIPLETS)
    unset(VCPKG_OVERLAY_PORTS)
endif ()

if (ENABLE_DEMO OR ENABLE_UI)
    set(SOUND_REQUIRED TRUE)
else ()
    set(SOUND_REQUIRED FALSE)
endif ()

# assume Debug if CMAKE_BUILD_TYPE was not provided
if (NOT DEFINED CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE MATCHES "Debug")
    set(DEBUG_BUILD TRUE)
else ()
    set(DEBUG_BUILD FALSE)
endif ()

# enable LTO for non-debug builds
if (NOT DEBUG_BUILD)
    message(CHECK_START "checking if LTO is supported")
    include(CheckIPOSupported)
    check_ipo_supported(RESULT ipoSupported OUTPUT output)
    if (ipoSupported)
        message(CHECK_PASS "yes")
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
    else ()
        message(CHECK_FAIL "no")
    endif ()
endif()


# ============================================================================
# Dependencies
# ============================================================================

# C++17 is required
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# gbapu is required for libtrackerboy, which is always built
add_subdirectory(external/gbapu)

# sound related dependencies for demo and ui projects
if (SOUND_REQUIRED)

    # Miniaudio doesn't provide a CMakeLists.txt as it's a single-header library
    # we'll have to DIY it then.
    # The split version is used to speed up compilation, the single c file gets compiled
    # into a library, miniaudio.

    add_library(miniaudio STATIC "external/miniaudio/extras/miniaudio_split/miniaudio.c")
    target_include_directories(miniaudio PUBLIC "external/miniaudio/extras/miniaudio_split")
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
    find_package(Catch2 CONFIG REQUIRED)
    include(CTest)
    include(Catch)
    add_library(Catch2Main STATIC "external/catch.cpp")
    target_link_libraries(Catch2Main PUBLIC Catch2::Catch2)
    enable_testing()
endif ()

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

# ============================================================================
# Information
# ============================================================================

message(
    "\n"
    "Configuration summary\n"
    "---------------------\n"
    " * Install directory           : ${CMAKE_INSTALL_PREFIX}\n"
    " * Build type                  : ${CMAKE_BUILD_TYPE}\n"
    " * Architecture                : ${BUILD_ARCH}\n"
    " * Demos                       : ${ENABLE_DEMO}\n"
    " * Tests                       : ${ENABLE_TESTS}\n"
    " * UI                          : ${ENABLE_UI}\n"
    " * Unity build                 : ${ENABLE_UNITY}\n"
)
