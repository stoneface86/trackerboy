project(ui CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
find_package(Qt5 COMPONENTS Widgets REQUIRED)

option(FIND_RTMIDI OFF)

if (FIND_RTMIDI)
    find_package(RtMidi MODULE REQUIRED)
else ()
    find_package(RtMidi CONFIG REQUIRED)
endif ()

if (WIN32)
    set(GUI_TYPE WIN32)
    set(WINDOWS_APPICON_RC "src/resources/icons/app/appicon.rc")
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
    "src/core/audio/AudioProber"
    "src/core/audio/AudioStream"
    "src/core/audio/Renderer"
    "src/core/audio/Ringbuffer"
    "src/core/audio/VisualizerBuffer"
    "src/core/clipboard/PatternClip"
    "src/core/clipboard/PatternClipboard"
    "src/core/commands/order/OrderDuplicateCmd"
    "src/core/commands/order/OrderEditCmd"
    "src/core/commands/order/OrderInsertCmd"
    "src/core/commands/order/OrderRemoveCmd"
    "src/core/commands/order/OrderSwapCmd"
    "src/core/commands/pattern/EraseCmd"
    "src/core/commands/pattern/PasteCmd"
    "src/core/commands/pattern/ReverseCmd"
    "src/core/commands/pattern/SelectionCmd"
    "src/core/commands/pattern/TrackEditCmd"
    "src/core/commands/pattern/TransposeCmd"
    "src/core/config/AppearanceConfig"
    "src/core/config/GeneralConfig"
    #"src/core/config/KeyboardConfig"
    "src/core/config/keys"
    "src/core/config/MidiConfig"
    "src/core/config/SoundConfig"
    "src/core/graphics/CachedPen"
    "src/core/graphics/CellPainter"
    "src/core/graphics/PatternLayout"
    "src/core/graphics/PatternPainter"
    FILE "src/core/midi/IMidiReceiver.hpp"
    "src/core/midi/Midi"
    "src/core/midi/MidiProber"
    FILE "src/core/misc/connectutils.hpp"
    FILE "src/core/misc/TableActions.hpp"
    "src/core/misc/utils"
    "src/core/model/graph/GraphModel"
    "src/core/model/graph/SequenceModel"
    "src/core/model/graph/WaveModel"
    "src/core/model/BaseTableModel"
    "src/core/model/InstrumentListModel"
    "src/core/model/PatternModel"
    "src/core/model/SongModel"
    "src/core/model/SongListModel"
    "src/core/model/WaveListModel"
    FILE "src/core/ChannelOutput.hpp"
    "src/core/Config"
    "src/core/FastTimer"
    FILE "src/core/Guarded.hpp"
    "src/core/IconManager"
    FILE "src/core/Locked.hpp"
    "src/core/Module"
    "src/core/ModuleFile"
    "src/core/Palette"
    FILE "src/core/PatternCursor.hpp"
    "src/core/PatternSelection"
    "src/core/PianoInput"
    "src/core/samplerates"
    "src/core/WavExporter"

    "src/forms/editors/BaseEditor"
    "src/forms/editors/InstrumentEditor"
    "src/forms/editors/WaveEditor"
    FILE "src/forms/MainWindow/actions.cpp"
    FILE "src/forms/MainWindow/slots.cpp"
    "src/forms/AboutDialog"
    "src/forms/AudioDiagDialog"
    "src/forms/CommentsDialog"
    "src/forms/ConfigDialog"
    "src/forms/ExportWavDialog"
    "src/forms/MainWindow"
    "src/forms/ModulePropertiesDialog"
    "src/forms/TempoCalculator"

    FILE "src/resources/fonts.qrc"
    FILE "src/resources/icons.qrc"
    FILE "src/resources/images.qrc"

    "src/widgets/config/ConfigTab"
    "src/widgets/config/MidiConfigTab"
    "src/widgets/config/SoundConfigTab"
    "src/widgets/config/SoundQualityPreview"
    "src/widgets/docks/TableDock"
    "src/widgets/grid/PatternGrid"
    "src/widgets/grid/PatternGridHeader"
    "src/widgets/sidebar/AudioScope"
    "src/widgets/sidebar/OrderEditor"
    "src/widgets/sidebar/OrderGrid"
    "src/widgets/sidebar/SongEditor"
    #"src/widgets/visualizers/PeakMeter"
    #"src/widgets/visualizers/VolumeMeterAnimation"
    "src/widgets/CustomSpinBox"
    "src/widgets/EnvelopeForm"
    "src/widgets/GraphEdit"
    "src/widgets/PatternEditor"
    "src/widgets/PianoWidget"
    "src/widgets/SequenceEditor"
    "src/widgets/Sidebar"
    "src/widgets/SpeedLabel"
    "src/widgets/TempoLabel"
)

# https://stackoverflow.com/questions/1435953/how-can-i-pass-git-sha1-to-compiler-as-definition-using-cmake/4318642#4318642
include(GetGitRevisionDescription)
get_git_head_revision(GIT_REFSPEC GIT_SHA1)

configure_file("src/GitSHA1.cpp.in" "${CMAKE_CURRENT_BINARY_DIR}/GitSHA1.cpp" @ONLY)
list(APPEND UI_SRC "${CMAKE_CURRENT_BINARY_DIR}/GitSHA1.cpp" "src/GitSHA1.hpp")

#
# ui library
#
add_library(ui STATIC ${UI_SRC})
target_link_libraries(ui PUBLIC 
    trackerboy
    miniaudio
    RtMidi::rtmidi
    Qt5::Widgets
    trackerboyWarnings
)
target_include_directories(ui PUBLIC "src")
if (VCPKG_TOOLCHAIN)
    # hack
    # the toolchain should do this but it does not for some reason ?
    # might just be a problem with the rtmidi port
    target_include_directories(ui PUBLIC "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include")
endif ()

unityBuildable(ui)

if (NOT ${DEBUG_BUILD})
    target_compile_definitions(ui PUBLIC QT_NO_INFO_OUTPUT QT_NO_DEBUG_OUTPUT)
endif ()


#
# Target for the main ui, trackerboy_ui. We cannot use trackerboy as that name is used
# for libtrackerboy. However, the resulting executable is named trackerboy, not trackerboy_ui
#
add_executable(trackerboy_ui ${GUI_TYPE} "src/main.cpp" ${WINDOWS_APPICON_RC})
target_link_libraries(trackerboy_ui PRIVATE ui)
# output executable is "trackerboy" and not "trackerboy_ui"
set_target_properties(trackerboy_ui PROPERTIES OUTPUT_NAME "trackerboy")

#
# Deployment
#

set(DEPLOY_DIR "${CMAKE_CURRENT_BINARY_DIR}/deploy")
set(DEPLOY_TARGET "deploy_timestamp")
set(HAS_DEPLOYMENT FALSE)

get_target_property(_qmake_executable Qt5::qmake IMPORTED_LOCATION)
get_filename_component(_qt_bin_dir "${_qmake_executable}" DIRECTORY)

if (WIN32)
    find_program(WINDEPLOYQT windeployqt HINTS "${_qt_bin_dir}")
    if (WINDEPLOYQT)
        set(HAS_DEPLOYMENT TRUE)

        add_custom_command(
            OUTPUT ${DEPLOY_TARGET}
            COMMAND ${WINDEPLOYQT} --dir ${DEPLOY_DIR} --no-translations --no-svg --no-angle --no-system-d3d-compiler --no-opengl-sw "$<TARGET_FILE:trackerboy_ui>"
            COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:trackerboy_ui>" ${DEPLOY_DIR}
            COMMAND ${CMAKE_COMMAND} -E touch ${DEPLOY_TARGET}
            DEPENDS trackerboy_ui
        )
    endif ()

elseif (APPLE)

    # needs some testing
    find_program(MACDEPLOYQT macdeployqt HINTS "${_qt_bin_dir}")
    if (MACDEPLOYQT)
        set(HAS_DEPLOYMENT TRUE)

        add_custom_command(
            OUTPUT ${DEPLOY_TARGET}
            COMMAND ${CMAKE_COMMAND} -E chdir "${DEPLOY_DIR}" "${MACDEPLOYQT}" "$<TARGET_FILE:trackerboy_ui>" -dmg
            COMMAND ${CMAKE_COMMAND} -E touch ${DEPLOY_TARGET}
            DEPENDS trackerboy_ui
        )
    endif ()


elseif (CMAKE_SYSTEM_NAME STREQUAL "Linux")

    find_program(LINUXDEPLOYQT linuxdeployqt)
    if (LINUXDEPLOYQT)
        set(HAS_DEPLOYMENT TRUE)

        add_custom_command(
            OUTPUT ${DEPLOY_TARGET}
            COMMAND ${CMAKE_COMMAND} -E make_directory "AppDir/usr/bin"
            COMMAND ${CMAKE_COMMAND} -E make_directory "AppDir/usr/lib"
            COMMAND ${CMAKE_COMMAND} -E make_directory "AppDir/usr/share/applications"
            COMMAND ${CMAKE_COMMAND} -E make_directory "AppDir/usr/share/icons/hicolor/256x256"
            COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:trackerboy_ui>" "AppDir/usr/bin"
            COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/src/resources/icons/app/appicon-256.png" "AppDir/usr/share/icons/hicolor/256x256/trackerboy.png"
            COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/trackerboy.desktop" "AppDir/usr/share/applications"
            COMMAND ${CMAKE_COMMAND} -E chdir "${DEPLOY_DIR}" "${LINUXDEPLOYQT}" "${CMAKE_CURRENT_BINARY_DIR}/AppDir/usr/share/applications/trackerboy.desktop" -appimage -qmake="${_qmake_executable}"
            COMMAND ${CMAKE_COMMAND} -E touch ${DEPLOY_TARGET}
            DEPENDS trackerboy_ui
        )
    endif ()

endif ()

if (HAS_DEPLOYMENT)
    configure_file("${CMAKE_SOURCE_DIR}/LICENSE" "${DEPLOY_DIR}/LICENSE" COPYONLY)
    configure_file("${CMAKE_SOURCE_DIR}/CHANGELOG.md" "${DEPLOY_DIR}/CHANGELOG.md" COPYONLY)
    add_custom_target(deploy DEPENDS ${DEPLOY_TARGET})
else ()
    # cannot deploy to this system
    # provide a deploy target that returns an error code
    add_custom_target(
        deploy
        ${CMAKE_COMMAND} -E echo "no deployment possible"
        COMMAND ${CMAKE_COMMAND} -E false
    )
endif ()


#
# test programs
# These small programs are used to test specific parts of the UI instead of building the entire thing
# Essentially unit tests, but without the automation
#

add_executable(test_BaseEditor ${GUI_TYPE} EXCLUDE_FROM_ALL "test/test_BaseEditor.cpp")
target_link_libraries(test_BaseEditor PRIVATE ui)

add_executable(test_OrderWidget ${GUI_TYPE} EXCLUDE_FROM_ALL "test/test_OrderWidget.cpp")
target_link_libraries(test_OrderWidget PRIVATE ui)

add_executable(test_PianoWidget ${GUI_TYPE} EXCLUDE_FROM_ALL "test/test_PianoWidget.cpp")
target_link_libraries(test_PianoWidget PRIVATE ui)

add_executable(test_AudioStream ${GUI_TYPE} EXCLUDE_FROM_ALL "test/test_AudioStream.cpp")
target_link_libraries(test_AudioStream PRIVATE ui)

#add_executable(test_pattern_painter ${GUI_TYPE} EXCLUDE_FROM_ALL "test/test_pattern_painter.cpp" )
#target_link_libraries(test_pattern_painter
#    trackerboy
#    Qt5::Widgets
#)

install(
    TARGETS trackerboy_ui
    RUNTIME DESTINATION bin
    BUNDLE DESTINATION bin
)
