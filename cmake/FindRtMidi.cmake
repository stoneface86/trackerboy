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

if (RTMIDI_INCLUDE_DIR)
    # RtMidi.h has its version as a #define RTMIDI_VERSION "..."
    # attempt to extract this version from the define
    file(
        STRINGS "${RTMIDI_INCLUDE_DIR}/RtMidi.h" rtmidiVersionLineList
        REGEX "[ \t]*#define[ \t]+RTMIDI_VERSION*"
    )
    # if found rtmidiVersionLineList will have 1 item containing the define
    list(GET rtmidiVersionLineList 0 rtmidiVersionLine)
    if (NOT rtmidiVersionLine)
        message(AUTHOR_WARNING "RTMIDI_INCLUDE_DIR found, but RTMIDI_VERSION is missing")
    else ()
        # strip the define and opening quotation
        string(REGEX REPLACE "^[ \t]*#define[ \t]+RTMIDI_VERSION*[ \t]+\"" "" rtmidiVersionLine "${rtmidiVersionLine}")
        # extract the version
        string(REGEX MATCH "^[0-9]+.[0-9]+.[0-9]+" RTMIDI_VERSION_STRING "${rtmidiVersionLine}")
    endif ()
endif ()

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
