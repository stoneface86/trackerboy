#
# File: CMakeLists.txt
#
# Main cmakelists for the trackerboy repo.
# See README.md for details on building
#

cmake_minimum_required(VERSION 3.8)

set(CMAKE_USER_MAKE_RULES_OVERRIDE "${CMAKE_CURRENT_LIST_DIR}/CompilerOverrides.cmake")

project (
    trackerboy 
    VERSION "0.0.2"
    DESCRIPTION "Gameboy tracker"
    LANGUAGES C CXX
)

option(TRACKERBOY_ENABLE_DEMO "Enable building of demo programs (requires portaudio)" OFF)
option(TRACKERBOY_ENABLE_TESTS "Enable unit tests" ON)
option(TRACKERBOY_ENABLE_UI "Enable building of the main trackerboy application" ON)
option(TRACKERBOY_COVERAGE "Enable code coverage" OFF)

include_directories(
    "include"
)

if (TRACKERBOY_ENABLE_DEMO OR TRACKERBOY_ENABLE_UI)
    set(PORTAUDIO_REQUIRED TRUE)
else ()
    set(PORTAUDIO_REQUIRED FALSE)
endif ()

# ============================================================================
# Dependencies
# ============================================================================

# C++11 is required
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# portaudio is required for the demo and ui projects
if (PORTAUDIO_REQUIRED)
    add_subdirectory(portaudio EXCLUDE_FROM_ALL)
    # PaUtilRingBuffer isn't included in the library, so add it here separately
    add_library(portaudio_ringbuffer STATIC EXCLUDE_FROM_ALL "portaudio/src/common/pa_ringbuffer.c")
    target_include_directories(portaudio_ringbuffer PUBLIC "portaudio/src/common")
endif ()

# qt is required for the ui project
macro(use_qt)
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTOUIC ON)
    set(CMAKE_AUTORCC ON)
    find_package(Qt5 COMPONENTS ${ARGV} REQUIRED)
    if (WIN32)
        set(GUI_TYPE WIN32)
    elseif (APPLE)
        set(GUI_TYPE MACOSX_BUNDLE)
    else ()
        set(GUI_TYPE "")
    endif ()
endmacro(use_qt)

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

# ============================================================================
# Sub-projects
# ============================================================================

#
# libtrackerboy, main library used by ui and demo programs
#
add_subdirectory(libtrackerboy)

if (PORTAUDIO_REQUIRED)
    add_subdirectory(audio)
endif()

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
