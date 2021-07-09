#
# cmake/FindRtAudio.cmake
#
# Attempts to find rtaudio lib installed on the system
#
# If found, an imported library target RtAudio::rtaudio is added
# the linkage of this library is unknown
#

find_package(PkgConfig)
pkg_check_modules(PC_RTAUDIO rtaudio)

set(RTAUDIO_DEFINITIONS ${PC_RTAUDIO_CFLAGS_OTHER})
set(RTAUDIO_VERSION_STRING ${PC_RTAUDIO_VERSION})

find_path(
  RTAUDIO_INCLUDE_DIR RtAudio.h
  HINTS ${PC_RTAUDIO_INCLUDEDIR} ${PC_RTAUDIO_INCLUDE_DIRS}
  PATH_SUFFIXES rtaudio
)

find_library(
  RTAUDIO_LIBRARIES NAMES rtaudio
  HINTS ${PC_RTAUDIO_LIBDIR} ${PC_RTAUDIO_LIBRARY_DIRS}
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  RtAudio
  REQUIRED_VARS RTAUDIO_LIBRARIES RTAUDIO_INCLUDE_DIR
  VERSION_VAR RTAUDIO_VERSION_STRING
)

mark_as_advanced(RTAUDIO_INCLUDE_DIR RTAUDIO_LIBRARIES)

if (RTAUDIO_FOUND)

    add_library(RtAudio::rtaudio UNKNOWN IMPORTED)
    set_target_properties(RtAudio::rtaudio PROPERTIES IMPORTED_LOCATION ${RTAUDIO_LIBRARIES})
    target_include_directories(RtAudio::rtaudio INTERFACE ${RTAUDIO_INCLUDE_DIRS})

endif ()
