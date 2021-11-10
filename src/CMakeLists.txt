project(src CXX)

#
# Dependencies
#
# interface library containing all of our dependencies
add_library(deps INTERFACE)

option(FETCH_DEPS "Automatically fetch required dependencies" ON)
if (FETCH_DEPS)

    include(FetchContent)
    include("${CMAKE_CURRENT_LIST_DIR}/dependencies.cmake")

    # FetchContent_MakeAvailable is not used for compatibility reasons

    FetchContent_GetProperties(libtrackerboy)
    if (NOT libtrackerboy_POPULATED)
        message(STATUS "Fetching libtrackerboy")
        FetchContent_Populate(libtrackerboy)
        set(LIBTRACKERBOY_ENABLE_UNITY ${ENABLE_UNITY})
        add_subdirectory("${libtrackerboy_SOURCE_DIR}" "${libtrackerboy_BINARY_DIR}")
    endif ()

    FetchContent_GetProperties(miniaudio)
    if (NOT miniaudio_POPULATED)
        message(STATUS "Fetching miniaudio")
        FetchContent_Populate(miniaudio)

        set(MA_NO_DECODING ON)
        set(MA_NO_ENCODING ON)
        set(MA_NO_GENERATION ON)
        set(MA_NO_CUSTOM ON)
        add_subdirectory("${miniaudio_SOURCE_DIR}" "${miniaudio_BINARY_DIR}")
    endif ()

    FetchContent_GetProperties(rtmidi)
    if (NOT rtmidi_POPULATED)
        message(STATUS "Fetching RtMidi")
        FetchContent_Populate(rtmidi)

        set(RTMIDI_BUILD_STATIC_LIBS ON)
        set(RTMIDI_BUILD_TESTING OFF)
        add_subdirectory("${rtmidi_SOURCE_DIR}" "${rtmidi_BINARY_DIR}")
    endif ()

    target_link_libraries(deps INTERFACE trackerboy miniaudio rtmidi)
    # hack: RtMidi CMakeList doesn't add an include dir for the rtmidi library
    target_include_directories(deps INTERFACE "${rtmidi_SOURCE_DIR}")
else ()

    # do not fetch dependencies, find them on the system instead
    # this is typically only used by CI, as CI will build and install the
    # deps so that they can be cached

    find_package(libtrackerboy CONFIG REQUIRED)
    find_package(miniaudio CONFIG REQUIRED)
    find_package(RtMidi CONFIG REQUIRED)

    target_link_libraries(deps INTERFACE trackerboy::trackerboy miniaudio::miniaudio RtMidi::rtmidi)
endif ()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
find_package(Qt5 COMPONENTS Widgets REQUIRED)

if (WIN32)
    set(GUI_TYPE WIN32)
    set(WINDOWS_APPICON_RC "resources/icons/app/appicon.rc")
elseif (APPLE)
    set(GUI_TYPE MACOSX_BUNDLE)
else ()
    set(GUI_TYPE "")
endif ()

#
# Creates a source list containing the header and source files for each argument
# given. For regular files, pass the FILE option before the desired argument
#
# Example:
# makeSourceList(srcList foo FILE main.cpp)
# ${srcList} => "foo.cpp;foo.hpp;main.cpp"
#
function(makeSourceList srcVar)
    set(_LIST "")
    set(_FILE FALSE)
    foreach(arg IN ITEMS ${ARGN})
        if ("${arg}" STREQUAL "FILE")
            set(_FILE TRUE)
        else ()
            if (_FILE)
                list(APPEND _LIST "${arg}")
                set(_FILE FALSE)
            else ()
                list(APPEND _LIST "${arg}.cpp" "${arg}.hpp")
            endif ()
        endif ()
    endforeach()
    set(${srcVar} "${_LIST}" PARENT_SCOPE)
endfunction()


# When adding a source+header to the list, just add the filename without the
# extension. For regular files, or source files with no complementing header,
# use FILE <filename>

makeSourceList(UI_SRC
    "audio/AudioEnumerator"
    "audio/AudioStream"
    "audio/Renderer"
    "audio/Ringbuffer"
    "audio/VisualizerBuffer"
    "audio/Wav"
    "core/clipboard/PatternClip"
    "core/clipboard/PatternClipboard"
    "core/commands/order"
    "core/commands/pattern"
    "core/config/AppearanceConfig"
    "core/config/GeneralConfig"
    #"core/config/KeyboardConfig"
    "core/config/keys"
    "core/config/MidiConfig"
    "core/config/SoundConfig"
    "core/graphics/CachedPen"
    "core/graphics/CellPainter"
    "core/graphics/PatternLayout"
    "core/graphics/PatternPainter"
    "core/input/PianoInput"
    FILE "core/midi/IMidiReceiver.hpp"
    "core/midi/Midi"
    "core/midi/MidiProber"
    FILE "core/misc/connectutils.hpp"
    "core/misc/NoteStrings"
    FILE "core/misc/TableActions.hpp"
    "core/misc/utils"
    "core/model/graph/GraphModel"
    "core/model/graph/SequenceModel"
    "core/model/graph/WaveModel"
    "core/model/BaseTableModel"
    "core/model/PaletteModel"
    "core/model/PatternModel"
    "core/model/SongModel"
    "core/model/SongListModel"
    "core/model/TableModel"
    FILE "core/ChannelOutput.hpp"
    "core/Config"
    "core/FastTimer"
    FILE "core/Guarded.hpp"
    "core/IconManager"
    FILE "core/Locked.hpp"
    "core/Module"
    "core/ModuleFile"
    "core/Palette"
    FILE "core/PatternCursor.hpp"
    "core/PatternSelection"
    "core/StandardRates"
    "core/WavExporter"

    "forms/editors/BaseEditor"
    "forms/editors/InstrumentEditor"
    "forms/editors/WaveEditor"
    FILE "forms/MainWindow/actions.cpp"
    FILE "forms/MainWindow/slots.cpp"
    "forms/AboutDialog"
    "forms/AudioDiagDialog"
    "forms/CommentsDialog"
    "forms/ConfigDialog"
    "forms/ExportWavDialog"
    "forms/MainWindow"
    "forms/ModulePropertiesDialog"
    "forms/TempoCalculator"

    FILE "resources/fonts.qrc"
    FILE "resources/icons.qrc"
    FILE "resources/images.qrc"

    "widgets/config/AppearanceConfigTab"
    "widgets/config/ConfigTab"
    "widgets/config/KeyboardConfigTab"
    "widgets/config/MidiConfigTab"
    "widgets/config/SoundConfigTab"
    "widgets/docks/TableDock"
    "widgets/grid/PatternGrid"
    "widgets/grid/PatternGridHeader"
    "widgets/sidebar/AudioScope"
    "widgets/sidebar/OrderEditor"
    "widgets/sidebar/OrderGrid"
    "widgets/sidebar/SongEditor"
    #"widgets/visualizers/PeakMeter"
    #"widgets/visualizers/VolumeMeterAnimation"
    "widgets/CustomSpinBox"
    "widgets/EnvelopeForm"
    "widgets/GraphEdit"
    "widgets/PatternEditor"
    "widgets/PianoWidget"
    "widgets/SequenceEditor"
    "widgets/Sidebar"
    "widgets/SpeedLabel"
    "widgets/TempoLabel"
)

# https://stackoverflow.com/questions/1435953/how-can-i-pass-git-sha1-to-compiler-as-definition-using-cmake/4318642#4318642
include(GetGitRevisionDescription)
get_git_head_revision(GIT_REFSPEC GIT_SHA1)

configure_file("version.cpp.in" "${CMAKE_CURRENT_BINARY_DIR}/version.cpp" @ONLY)
list(APPEND UI_SRC "${CMAKE_CURRENT_BINARY_DIR}/version.cpp" "version.hpp")


set(CMAKE_INCLUDE_CURRENT_DIR ON)

#
# ui library
#
add_library(ui OBJECT ${UI_SRC})
target_link_libraries(ui PUBLIC deps Qt5::Widgets)


target_compile_features(ui PUBLIC cxx_std_17)
if (MSVC)
    target_compile_options(ui PUBLIC
        /W4
        /experimental:external /external:W0 /external:anglebrackets
    )
elseif (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    # -Wno-missing-field-initializers prevents warnings when zero-initializing a struct
    target_compile_options(ui PUBLIC
        -Wall
        -Wextra
        -pedantic
        -Wno-missing-field-initializers
    )

    # flags to help minimize the binary size
#    if (NOT ${DEBUG_BUILD})
#        target_compile_options(ui PRIVATE
#            -fno-rtti
#            -ffunction-sections -fdata-sections -Wl,--gc-sections
#            )
#    endif ()
endif ()

if (NOT ${DEBUG_BUILD})
    target_compile_definitions(ui PUBLIC QT_NO_INFO_OUTPUT QT_NO_DEBUG_OUTPUT)
endif ()

if (ENABLE_UNITY AND ${CMAKE_VERSION} VERSION_GREATER "3.15")
    set_target_properties(ui PROPERTIES UNITY_BUILD ON)
endif ()

#
# Target for the main ui, trackerboy_ui. We cannot use trackerboy as that name is used
# for libtrackerboy. However, the resulting executable is named trackerboy, not trackerboy_ui
#
add_executable(trackerboy_ui ${GUI_TYPE} "main.cpp" $<TARGET_OBJECTS:ui> "${WINDOWS_APPICON_RC}")
target_link_libraries(trackerboy_ui PRIVATE ui)
# output executable is "trackerboy" and not "trackerboy_ui"
set_target_properties(trackerboy_ui PROPERTIES OUTPUT_NAME "trackerboy")


#
# Deployment
#

option(ENABLE_DEPLOYMENT "enables deployment of the trackerboy application" OFF)

if (ENABLE_DEPLOYMENT)

    set(DEPLOY_DIR "${CMAKE_CURRENT_BINARY_DIR}/deploy")
    set(HAS_DEPLOYMENT FALSE)

    get_target_property(_qmake_executable Qt5::qmake IMPORTED_LOCATION)
    get_filename_component(_qt_bin_dir "${_qmake_executable}" DIRECTORY)

    if (WIN32)
        find_program(WINDEPLOYQT windeployqt HINTS "${_qt_bin_dir}")
        if (WINDEPLOYQT)
            set(HAS_DEPLOYMENT TRUE)
            # windeployqt doesn't create anything, just dumps dlls into the directory
            # use a timestamp file as a target
            set(DEPLOY_TARGET "deploy_timestamp")
            add_custom_command(
                OUTPUT ${DEPLOY_TARGET}
                COMMAND ${WINDEPLOYQT} --dir ${DEPLOY_DIR} --no-translations --no-svg --no-angle --no-system-d3d-compiler --no-opengl-sw "$<TARGET_FILE:trackerboy_ui>"
                COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:trackerboy_ui>" ${DEPLOY_DIR}
                COMMAND ${CMAKE_COMMAND} -E touch ${DEPLOY_TARGET}
                DEPENDS trackerboy_ui
            )
        endif ()

    elseif (APPLE)

        find_program(MACDEPLOYQT macdeployqt HINTS "${_qt_bin_dir}")
        if (MACDEPLOYQT)
            set(HAS_DEPLOYMENT TRUE)

            # macdeployqt creates a dmg in the same directory as the bundle
            set(DEPLOY_TARGET "${DEPLOY_DIR}/trackerboy.dmg")
            add_custom_command(
                OUTPUT ${DEPLOY_TARGET}
                COMMAND "${MACDEPLOYQT}" "$<TARGET_BUNDLE_DIR:trackerboy_ui>" -dmg
                COMMAND ${CMAKE_COMMAND} -E rename "trackerboy.dmg" "${DEPLOY_TARGET}"
                DEPENDS trackerboy_ui
            )
        endif ()


    elseif (CMAKE_SYSTEM_NAME STREQUAL "Linux")

        # CI will download the tool into the tools folder in the source tree
        # you can do the same if deploying manually, or by just having it in your PATH
        find_program(LINUXDEPLOYQT linuxdeployqt HINTS "${CMAKE_SOURCE_DIR}/tools")
        if (LINUXDEPLOYQT)
            set(HAS_DEPLOYMENT TRUE)

            # linuxdeployqt creates an AppImage file with the following naming scheme:
            #  <Appname>-<version>-<arch>.AppImage
            # version is omitted if environment variable VERSION is unset and we are not in the repo folder
            # otherwise it gets the value of $VERSION or the result of `git rev-parse --short HEAD` if unset
            #
            # linuxdeployqt offers no way to override this, and the dev seems adamant to keeping it this way so...
            #
            # this will be the filename of the generated AppImage
            set(_linuxdeployqtOut "Trackerboy-v${CMAKE_PROJECT_VERSION}-x86_64.AppImage")

            set(DEPLOY_TARGET "${DEPLOY_DIR}/${_linuxdeployqtOut}")
            add_custom_command(
                OUTPUT ${DEPLOY_TARGET}
                COMMAND ${CMAKE_COMMAND} -E make_directory "AppDir/usr/bin"
                COMMAND ${CMAKE_COMMAND} -E make_directory "AppDir/usr/lib"
                COMMAND ${CMAKE_COMMAND} -E make_directory "AppDir/usr/share/applications"
                COMMAND ${CMAKE_COMMAND} -E make_directory "AppDir/usr/share/icons/hicolor/256x256"
                COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:trackerboy_ui>" "AppDir/usr/bin"
                COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/resources/icons/app/appicon-256.png" "AppDir/usr/share/icons/hicolor/256x256/trackerboy.png"
                COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/trackerboy.desktop" "AppDir/usr/share/applications"
                COMMAND ${CMAKE_COMMAND} -E chdir "${DEPLOY_DIR}" ${CMAKE_COMMAND} -E env VERSION="v${CMAKE_PROJECT_VERSION}" "${LINUXDEPLOYQT}" "${CMAKE_CURRENT_BINARY_DIR}/AppDir/usr/share/applications/trackerboy.desktop" -appimage -qmake="${_qmake_executable}"
                DEPENDS trackerboy_ui
            )
        endif ()

    endif ()

    if (HAS_DEPLOYMENT)
        # documentation, example modules, etc can be copied over here as well
        # for now just include the CHANGELOG and LICENSE
        configure_file("${CMAKE_SOURCE_DIR}/LICENSE" "${DEPLOY_DIR}/LICENSE" COPYONLY)
        configure_file("${CMAKE_SOURCE_DIR}/CHANGELOG.md" "${DEPLOY_DIR}/CHANGELOG.md" COPYONLY)
        add_custom_target(deploy DEPENDS ${DEPLOY_TARGET})
    else ()
        # cannot deploy to this system
        # provide a deploy target that returns an error code
        # a bit more informative than error for a missing target
        add_custom_target(
            deploy
            ${CMAKE_COMMAND} -E echo "no deployment possible"
            COMMAND ${CMAKE_COMMAND} -E false
        )
    endif ()

endif ()

install(
    TARGETS trackerboy_ui
    RUNTIME DESTINATION bin
    BUNDLE DESTINATION bin
)
