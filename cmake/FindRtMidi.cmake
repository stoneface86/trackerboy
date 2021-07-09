#
#  cmake/FindRtMidi.cmake
#
# Attempts to find rtmidi library installed on the system
#
# adds an imported library target RtMidi::rtmidi if found
#

find_package(PkgConfig QUIET)
pkg_check_modules(PC_RTMIDI QUIET rtmidi)
set(RTMIDI_DEFINITIONS ${PC_RTMIDI_CFLAGS_OTHER})

find_path(
    RTMIDI_INCLUDE_DIR RtMidi.h
    HINTS ${PC_RTMIDI_INCLUDEDIR} ${PC_RTMIDI_INCLUDE_DIRS}
    PATH_SUFFIXES rtmidi
)

find_library(
    RTMIDI_LIBRARY NAMES rtmidi
    HINTS ${PC_RTMIDI_LIBDIR} ${PC_RTMIDI_LIBRARY_DIRS}
)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(
    RtMidi
    REQUIRED_VARS RTMIDI_LIBRARY RTMIDI_INCLUDE_DIR
    VERSION_VAR RTMIDI_VERSION_STRING
)

mark_as_advanced(RTMIDI_INCLUDE_DIR RTMIDI_LIBRARY)

if(RTMIDI_FOUND)
    set(RTMIDI_LIBRARIES ${RTMIDI_LIBRARY})
    set(RTMIDI_INCLUDE_DIRS ${RTMIDI_INCLUDE_DIR})

    add_library(RtMidi::rtmidi UNKNOWN IMPORTED)
    set_target_properties(RtMidi::rtmidi PROPERTIES IMPORTED_LOCATION ${RTMIDI_LIBRARIES})
    target_include_directories(RtMidi::rtmidi INTERFACE ${RTMIDI_INCLUDE_DIRS})
endif()
