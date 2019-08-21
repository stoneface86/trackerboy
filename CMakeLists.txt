#
# File: CMakeLists.txt
#
# Main cmakelists for the trackerboy repo.
# See README.md for details on building
#

cmake_minimum_required(VERSION 3.8)
project (
	trackerboy 
	VERSION "0.1.0"
	DESCRIPTION "Gameboy tracker"
	HOMEPAGE_URL "https://github.com/stoneface86/trackerboy"
	LANGUAGES C CXX
)

option(TRACKERBOY_ENABLE_DEMO "Enable building of demo programs (requires portaudio)" OFF)
option(TRACKERBOY_ENABLE_TESTS "Enable unit tests" ON)
option(TRACKERBOY_ENABLE_UI "Enable building of the main trackerboy application" ON)

if (TRACKERBOY_ENABLE_TESTS)
	enable_testing()
endif ()

include_directories("include")

# ============================================================================
# Dependencies
# ============================================================================

# portaudio is required for the demo and ui projects
if (TRACKERBOY_ENABLE_DEMO OR TRACKERBOY_ENABLE_UI)
	add_subdirectory(portaudio EXCLUDE_FROM_ALL)
	# PaUtilRingBuffer isn't included in the library, so add it here separately
	add_library(portaudio_ringbuffer STATIC EXCLUDE_FROM_ALL "portaudio/src/common/pa_ringbuffer.c")
	target_include_directories(portaudio_ringbuffer PUBLIC "portaudio/src/common")
endif ()

# qt is required for the ui project
function(use_qt components)
	set(CMAKE_AUTOMOC ON)
	set(CMAKE_AUTOUIC ON)
	set(CMAKE_AUTORCC ON)
	find_package(Qt5 COMPONENTS ${components} REQUIRED)
	if (WIN32)
		set(GUI_TYPE WIN32 PARENT_SCOPE)
	elseif (APPLE)
		set(GUI_TYPE MACOSX_BUNDLE PARENT_SCOPE)
	else ()
		set(GUI_TYPE "" PARENT_SCOPE)
	endif ()
endfunction(use_qt)

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
#	add_subdirectory(ui)
endif()

#
# Demo programs (optional)
#
if (TRACKERBOY_ENABLE_DEMO)
	add_subdirectory(demo)
endif ()
