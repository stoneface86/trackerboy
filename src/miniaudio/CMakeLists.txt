#
# CMakeLists.txt for miniaudio. Provides a static/shared library target for
# cmake projects. The library target is also installable.
#

cmake_minimum_required(VERSION 3.9...3.21)

project(miniaudio VERSION "0.10.42" LANGUAGES C)

#
# Build a static library by default
#
option(BUILD_SHARED_LIBS OFF)

# The library target uses the split version. The header-only version would
# require an INTERFACE library target. Perhaps a HEADER_ONLY option could specify
# which to use.

add_library(miniaudio STATIC "miniaudio.c")
target_include_directories(miniaudio PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/>")

macro(compileOption opt desc default)
    option(${opt} "${desc}" ${default})
    if (${opt})
        target_compile_definitions(miniaudio PUBLIC -D${opt})
    endif ()
endmacro()

compileOption( MA_NO_WASAPI         "Disables WASAPI backend"           OFF )
compileOption( MA_NO_DSOUND         "Disables DirectSound backend"      OFF )
compileOption( MA_NO_WINMM          "Disables the WinMM backend"        OFF )
compileOption( MA_NO_ALSA           "Disables the ALSA backend"         OFF )
compileOption( MA_NO_PULSEAUDIO     "Disables the PulseAudio backend"   OFF )
compileOption( MA_NO_JACK           "Disables the JACK backend"         OFF )
compileOption( MA_NO_COREAUDIO      "Disables the Core Audio backend"   OFF )
compileOption( MA_NO_SNDIO          "Disables the sndio backend"        OFF )
compileOption( MA_NO_AUDIO4         "Disables the audio(4) backend"     OFF )
compileOption( MA_NO_OSS            "Disables the OSS backend"          OFF )
compileOption( MA_NO_AAUDIO         "Disables the AAudio backend"       OFF )
compileOption( MA_NO_OPENSL         "Disables the OpenSL backend"       OFF )
compileOption( MA_NO_WEBAUDIO       "Disables the Web Audio backend"    OFF )
compileOption( MA_NO_NULL           "Disables the null backend"         OFF )
compileOption( MA_NO_CUSTOM         "Disables custom backend support"   OFF )
# ignoring the MA_ENABLE_* options as having them alongside MA_NO_* in the
# cmake gui may confuse the user
compileOption( MA_NO_DECODING       "Disables decoding APIs"                        OFF )
compileOption( MA_NO_ENCODING       "Disables encoding APIs"                        OFF )
compileOption( MA_NO_WAV            "Disables the built-in WAV decoder and encoder" OFF )
compileOption( MA_NO_FLAC           "Disables the built-in FLAC decoder"            OFF )
compileOption( MA_NO_MP3            "Disables the built-in MP3 decoder"             OFF )
compileOption( MA_NO_DEVICE_IO      "Disables playback and recording"               OFF )
compileOption( MA_NO_THREADING      "Disables threading API"                        OFF )
compileOption( MA_NO_GENERATION     "Disables generation APIs"                      OFF )
compileOption( MA_NO_SSE2           "Disables SSE2 optimizations"                   OFF )
compileOption( MA_NO_AVX2           "Disables AVX2 optimizations"                   OFF )
compileOption( MA_NO_AVX512         "Disables AVX-512 optimizations"                OFF )
compileOption( MA_NO_NEON           "Disables NEON optimizations"                   OFF )
compileOption( MA_NO_RUNTIME_LINKING "Disables runtime linking" OFF )
compileOption( MA_DEBUG_OUTPUT "Enable processing of MA_LOG_LEVEL_DEBUG messages" OFF)
compileOption( MA_DLL "Configures MA_API to either import or export APIs" OFF )


set(MA_API "" CACHE STRING "Controls how public API functions should be decorated. If unset extern is used")
if (MA_API)
    target_compile_definitions(miniaudio PUBLIC "-DMA_API=${MA_API}")
endif ()

set(MA_COINIT_VALUE "" CACHE STRING "Windows only. The value to pass to internal calls to CoInitializeEx(). If unsed, defaults to COINIT_MULTITHREADED.")
if (MA_COINIT_VALUE)
    target_compile_definitions(miniaudio PUBLIC "-DMA_COINIT_VALUE=${MA_COINIT_VALUE}")
endif ()

mark_as_advanced(MA_NO_SSE2)
mark_as_advanced(MA_NO_AVX2)
mark_as_advanced(MA_NO_AVX512)
mark_as_advanced(MA_NO_NEON)
mark_as_advanced(MA_NO_RUNTIME_LINKING)
mark_as_advanced(MA_DLL)
mark_as_advanced(MA_API)
mark_as_advanced(MA_COINIT_VALUE)

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


if (${CMAKE_PROJECT_NAME} STREQUAL ${PROJECT_NAME})
    
    install(
        TARGETS miniaudio
        EXPORT miniaudioTargets
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
    )
    install(FILES "extras/miniaudio_split/miniaudio.h" DESTINATION include)
    target_include_directories(miniaudio PUBLIC "$<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include>")

    include(CMakePackageConfigHelpers)
    write_basic_package_version_file(
        miniaudioConfigVersion.cmake
        VERSION ${CMAKE_PROJECT_VERSION}
        COMPATIBILITY AnyNewerVersion
    )

    install(
        EXPORT miniaudioTargets
        FILE miniaudioTargets.cmake
        NAMESPACE miniaudio::
        DESTINATION lib/cmake/miniaudio
    )

    configure_file(cmake/miniaudioConfig.cmake miniaudioConfig.cmake COPYONLY)
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/miniaudioConfig.cmake"
                  "${CMAKE_CURRENT_BINARY_DIR}/miniaudioConfigVersion.cmake"
            DESTINATION lib/cmake/miniaudio
            )    
endif ()
