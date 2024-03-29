
project(src CXX)

# Dependencies
# =============================================================================


# interface library containing all of our dependencies
add_library(deps INTERFACE)
target_link_libraries(deps INTERFACE vendoredIncludes)

include(CPM)

# libtrackerboy
CPMAddPackage(
    NAME libtrackerboy
    GITHUB_REPOSITORY "stoneface86/libtrackerboy"
    VERSION "cpp-last"
    GIT_TAG "cpp-last"
    OPTIONS
        "LIBTRACKERBOY_ENABLE_UNITY ${ENABLE_UNITY}"
    EXCLUDE_FROM_ALL ON
)
target_link_libraries(deps INTERFACE trackerboy)

# RtMidi
CPMAddPackage(
    NAME RtMidi
    GITHUB_REPOSITORY "thestk/rtmidi"
    VERSION "5.0.0"
    GIT_TAG "5.0.0"
    OPTIONS
        "BUILD_SHARED_LIBS OFF"
        "RTMIDI_API_JACK OFF"
        "RTMIDI_BUILD_TESTING OFF"
    EXCLUDE_FROM_ALL ON
)
target_link_libraries(deps INTERFACE rtmidi)

# Miniaudio
target_link_libraries(deps INTERFACE miniaudio)

# Qt
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC OFF)
find_package(Qt6 COMPONENTS Widgets REQUIRED)

if (WIN32)
    set(GUI_TYPE WIN32)
    set(WINDOWS_APPICON_RC "resources/icons/app/appicon.rc")
elseif (APPLE)
    set(GUI_TYPE MACOSX_BUNDLE)
    set(MACOSX_APPICON_ICNS "resources/icons/app/appicon.icns")
else ()
    set(GUI_TYPE "")
endif ()

# Build targets
# =============================================================================

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

    "clipboard/PatternClip"
    "clipboard/PatternClipboard"

    "config/data/FontTable"
    "config/data/GeneralConfig"
    "config/data/keys"
    "config/data/MidiConfig"
    "config/data/Palette"
    "config/data/PianoInput"
    "config/data/ShortcutTable"
    "config/data/SoundConfig"
    "config/tabs/AppearanceConfigTab"
    "config/tabs/ConfigTab"
    "config/tabs/GeneralConfigTab"
    "config/tabs/KeyboardConfigTab"
    "config/tabs/SoundConfigTab"
    "config/Config"
    "config/ConfigDialog"

    FILE "core/ChannelOutput.hpp"
    "core/Module"
    "core/ModuleFile"
    "core/NoteStrings"
    FILE "core/PatternCursor.hpp"
    "core/PatternSelection"
    "core/StandardRates"

    "export/ExportWavDialog"
    "export/WavExporter"

    "forms/editors/BaseEditor"
    "forms/editors/InstrumentEditor"
    "forms/editors/WaveEditor"
    FILE "forms/MainWindow/actions.cpp"
    FILE "forms/MainWindow/slots.cpp"
    "forms/AudioDiagDialog"
    "forms/CommentsDialog"
    "forms/EffectsListDialog"
    "forms/MainWindow"
    "forms/ModulePropertiesDialog"
    "forms/PersistantDialog"
    "forms/TempoCalculator"

    "graphics/CachedPen"
    "graphics/CellPainter"
    "graphics/PatternLayout"
    "graphics/PatternPainter"

    FILE "midi/IMidiReceiver.hpp"
    "midi/Midi"
    "midi/MidiEnumerator"

    "model/commands/order"
    "model/commands/pattern"
    "model/graph/GraphModel"
    "model/graph/SequenceModel"
    "model/graph/WaveModel"
    "model/BaseTableModel"
    "model/PatternModel"
    "model/SongModel"
    "model/SongListModel"
    "model/TableModel"

    FILE "resources/icons.qrc"
    FILE "resources/images.qrc"

    "utils/actions"
    "utils/FastTimer"
    FILE "utils/Guarded.hpp"
    "utils/IconLocator"
    FILE "utils/Locked.hpp"
    "utils/string"
    FILE "utils/TableActions.hpp"
    FILE "utils/connectutils.hpp"
    "utils/utils"

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
    "widgets/TableView"
)

# https://stackoverflow.com/questions/1435953/how-can-i-pass-git-sha1-to-compiler-as-definition-using-cmake/4318642#4318642
include(GetGitRevisionDescription)
get_git_head_revision(GIT_REFSPEC GIT_SHA1)

configure_file("version.cpp.in" "${CMAKE_CURRENT_BINARY_DIR}/version.cpp" @ONLY)
list(APPEND UI_SRC "${CMAKE_CURRENT_BINARY_DIR}/version.cpp" "version.hpp")

configure_file("forms/MainWindow/aboutstr.cpp.in" "${CMAKE_CURRENT_BINARY_DIR}/aboutstr.cpp" @ONLY)
list(APPEND UI_SRC "${CMAKE_CURRENT_BINARY_DIR}/aboutstr.cpp" "forms/MainWindow/aboutstr.hpp")

set(CMAKE_INCLUDE_CURRENT_DIR ON)

#
# ui library
#
add_library(ui OBJECT ${UI_SRC})
target_link_libraries(ui PUBLIC deps Qt6::Widgets)


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

    # remove any unused code/data when building
    if (NOT ${DEBUG_BUILD})
        add_compile_options(
            -ffunction-sections -fdata-sections -Wl,--gc-sections
            )
    endif ()
endif ()

if (NOT ${DEBUG_BUILD})
    target_compile_definitions(ui PUBLIC QT_NO_INFO_OUTPUT QT_NO_DEBUG_OUTPUT)
endif ()

if (ENABLE_UNITY AND ${CMAKE_VERSION} VERSION_GREATER "3.15")
    set_target_properties(ui PROPERTIES UNITY_BUILD ON)
endif ()

if (APPLE)
    set_source_files_properties(${MACOSX_APPICON_ICNS} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
endif ()
#
# Target for the main ui, trackerboy_ui. We cannot use trackerboy as that name is used
# for libtrackerboy. However, the resulting executable is named trackerboy, not trackerboy_ui
#
add_executable(trackerboy_ui ${GUI_TYPE} "main.cpp" $<TARGET_OBJECTS:ui> "${WINDOWS_APPICON_RC}" "${MACOSX_APPICON_ICNS}")
target_link_libraries(trackerboy_ui PRIVATE ui)
# output executable is "trackerboy" and not "trackerboy_ui"
set_target_properties(trackerboy_ui PROPERTIES OUTPUT_NAME "trackerboy")


# Deployment
# =============================================================================

option(ENABLE_DEPLOYMENT "enables deployment of the trackerboy application" OFF)

if (ENABLE_DEPLOYMENT)

    set(DEPLOY_DIR "${CMAKE_CURRENT_BINARY_DIR}/deploy")
    set(HAS_DEPLOYMENT FALSE)

    get_target_property(_qmake_executable Qt6::qmake IMPORTED_LOCATION)
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
                COMMAND ${WINDEPLOYQT} --dir ${DEPLOY_DIR} --no-translations --no-svg --no-system-d3d-compiler --no-opengl-sw "$<TARGET_FILE:trackerboy_ui>"
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
        find_program(LINUXDEPLOY linuxdeploy HINTS "${CMAKE_SOURCE_DIR}/tools")
        find_program(LINUXDEPLOY_PLUGIN_QT linuxdeploy-plugin-qt HINTS "${CMAKE_SOURCE_DIR}/tools")
        if (LINUXDEPLOY AND LINUXDEPLOY_PLUGIN_QT)
            set(HAS_DEPLOYMENT TRUE)
            set(DEPLOY_TARGET "${DEPLOY_DIR}/trackerboy.AppImage")
            add_custom_command(
                OUTPUT ${DEPLOY_TARGET}
                COMMAND 
                    ${CMAKE_COMMAND} -E env DESTDIR="${CMAKE_CURRENT_BINARY_DIR}/AppDir"
                        ${CMAKE_COMMAND} --install ${CMAKE_BINARY_DIR} --prefix /usr
                COMMAND 
                    ${CMAKE_COMMAND} -E chdir "${DEPLOY_DIR}" 
                        ${CMAKE_COMMAND} -E env OUTPUT="trackerboy.AppImage"
                        ${CMAKE_COMMAND} -E env QMAKE="${_qmake_executable}"
                            ${LINUXDEPLOY}
                                --appdir="${CMAKE_CURRENT_BINARY_DIR}/AppDir"
                                --desktop-file="${CMAKE_CURRENT_SOURCE_DIR}/trackerboy.desktop"
                                --icon-file="${CMAKE_CURRENT_SOURCE_DIR}/resources/icons/app/appicon-16.png"
                                --icon-file="${CMAKE_CURRENT_SOURCE_DIR}/resources/icons/app/appicon-24.png"
                                --icon-file="${CMAKE_CURRENT_SOURCE_DIR}/resources/icons/app/appicon-32.png"
                                --icon-file="${CMAKE_CURRENT_SOURCE_DIR}/resources/icons/app/appicon-48.png"
                                --icon-file="${CMAKE_CURRENT_SOURCE_DIR}/resources/icons/app/appicon-256.png"
                                --icon-filename="trackerboy"
                                --plugin qt
                                --output appimage
                DEPENDS trackerboy_ui
            )
        endif ()

    endif ()

    if (HAS_DEPLOYMENT)
        # common files to be included for each deployment platform
        # The following from the repo gets included:
        # - LICENSE
        # - CHANGELOG.md
        # - examples/ (example modules)
        add_custom_target(
            deployCommon
            COMMAND ${CMAKE_COMMAND} -E make_directory "${DEPLOY_DIR}"
            COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/LICENSE" "${DEPLOY_DIR}/LICENSE"
            COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/CHANGELOG.md" "${DEPLOY_DIR}/CHANGELOG.md"
            COMMAND ${CMAKE_COMMAND} -E make_directory "${DEPLOY_DIR}/examples"
            COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/examples" "${DEPLOY_DIR}/examples"
        )
        add_custom_target(deploy DEPENDS deployCommon ${DEPLOY_TARGET})
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
    BUNDLE DESTINATION .
)
