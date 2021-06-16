#
# File: CMakeLists.txt
#
# Main cmakelists for the trackerboy repo.
# See README.md for details on building
#

cmake_minimum_required(VERSION 3.8)

# removes default /W3 flag for MSVC 
# https://stackoverflow.com/questions/45995784/how-to-set-compiler-options-with-cmake-in-visual-studio-2017
set(CMAKE_USER_MAKE_RULES_OVERRIDE "${CMAKE_CURRENT_LIST_DIR}/CompilerOverrides.cmake")

project (
    trackerboy 
    VERSION "0.2.0"
    LANGUAGES C CXX
)

# combines the project name and version, ie trackerboy-v1.1.2
set(PROJECT_AND_VERSION_NAME "${PROJECT_NAME}-v${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake ${CMAKE_MODULE_PATH})

option(TRACKERBOY_ENABLE_DEMO "Enable building of demo programs (requires portaudio)" OFF)
option(TRACKERBOY_ENABLE_TESTS "Enable unit tests" ON)
option(TRACKERBOY_ENABLE_UI "Enable building of the main trackerboy application" ON)
option(TRACKERBOY_COVERAGE "Enable code coverage" OFF)

include_directories(
    "include"
)

if (TRACKERBOY_ENABLE_DEMO OR TRACKERBOY_ENABLE_UI)
    set(SOUND_REQUIRED TRUE)
else ()
    set(SOUND_REQUIRED FALSE)
endif ()

if (NOT DEFINED CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE MATCHES "Debug")
    set(DEBUG_BUILD TRUE)
else ()
    set(DEBUG_BUILD FALSE)
endif ()


# abort if compiler is 32-bit
if (${CMAKE_SIZEOF_VOID_P} EQUAL 4)
    message(FATAL_ERROR "32-bit builds are not supported")
endif ()

# ============================================================================
# Dependencies
# ============================================================================

# C++17 is required
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# sound related dependencies for demo and ui projects
if (SOUND_REQUIRED)
    add_library(miniaudio STATIC "external/miniaudio/extras/miniaudio_split/miniaudio.c")
    #target_compile_definitions(miniaudio PRIVATE
    #    MA_NO_DECODING
    #    MA_NO_GENERATION
    #)
    if (DEBUG_BUILD)
        # MA_LOG_LEVEL_INFO for debug builds
        target_compile_definitions(miniaudio PRIVATE MA_LOG_LEVEL=3)
    endif ()
    target_include_directories(miniaudio PUBLIC "external/miniaudio/extras/miniaudio_split")
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

if (TRACKERBOY_ENABLE_TESTS)
    enable_testing()
    # catch object library (compiles the header once, used by test programs)
    set(CATCH "catch")
    set(CATCH_SRC "catch.cpp")
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

add_library(trackerboyCoverage INTERFACE)
if (TRACKERBOY_COVERAGE)
    if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(trackerboyCoverage INTERFACE
            -O0
            -g
            --coverage
        )
        if (CMAKE_VERSION VERSION_GREATER_EQUAL 3.13)
            target_link_options(trackerboyCoverage INTERFACE --coverage)
        else ()
            target_link_libraries(trackerboyCoverage INTERFACE --coverage)
        endif ()
    endif()
endif()

include(TestBigEndian)
TEST_BIG_ENDIAN(IS_BIG_ENDIAN)

#add_subdirectory(external/blip_buf-1.1.0)

add_subdirectory(external/gbapu)

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
if (TRACKERBOY_ENABLE_UI)
    add_subdirectory(ui)
endif()

#
# Demo programs (optional)
#
if (TRACKERBOY_ENABLE_DEMO)
    add_subdirectory(demo)
endif ()
