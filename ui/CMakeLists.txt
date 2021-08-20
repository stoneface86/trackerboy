project(ui CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
find_package(Qt5 COMPONENTS Widgets REQUIRED)


get_target_property(_qmake_executable Qt5::qmake IMPORTED_LOCATION)
get_filename_component(_qt_bin_dir "${_qmake_executable}" DIRECTORY)

if (WIN32)
    set(GUI_TYPE WIN32)
    set(WINDOWS_APPICON_RC "src/resources/icons/app/appicon.rc")
    find_program(WINDEPLOYQT windeployqt HINTS "${_qt_bin_dir}")
elseif (APPLE)
    set(GUI_TYPE MACOSX_BUNDLE)
    find_program(MACDEPLOYQT macdeployqt HINTS "${_qt_bin_dir}")
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
    "src/core/config/AppearanceConfig"
    "src/core/config/GeneralConfig"
    #"src/core/config/KeyboardConfig"
    "src/core/config/keys"
    "src/core/config/MidiConfig"
    "src/core/config/SoundConfig"
    "src/core/graphics/CellPainter"
    "src/core/graphics/PatternLayout"
    "src/core/graphics/PatternPainter"
    FILE "src/core/midi/IMidiReceiver.hpp"
    "src/core/midi/Midi"
    "src/core/midi/MidiProber"
    "src/core/model/graph/GraphModel"
    "src/core/model/graph/SequenceModel"
    "src/core/model/graph/WaveModel"
    "src/core/model/BaseTableModel"
    "src/core/model/InstrumentListModel"
    "src/core/model/PatternModel"
    "src/core/model/SongListModel"
    "src/core/model/SongModel"
    "src/core/model/WaveListModel"
    FILE "src/core/ChannelOutput.hpp"
    "src/core/Config"
    "src/core/FastTimer"
    FILE "src/core/Guarded.hpp"
    FILE "src/core/Locked.hpp"
    "src/core/Module"
    "src/core/ModuleFile"
    "src/core/Palette"
    FILE "src/core/PatternCursor.hpp"
    "src/core/PatternSelection"
    "src/core/PianoInput"
    "src/core/samplerates"
    "src/core/WavExporter"

    FILE "src/forms/MainWindow/actions.cpp"
    FILE "src/forms/MainWindow/slots.cpp"
    "src/forms/AboutDialog"
    "src/forms/AudioDiagDialog"
    "src/forms/ConfigDialog"
    "src/forms/ExportWavDialog"
    "src/forms/MainWindow"
    "src/forms/TempoCalculator"

    FILE "src/misc/connectutils.hpp"
    "src/misc/IconManager"
    FILE "src/misc/TableActions.hpp"
    "src/misc/utils"

    FILE "src/resources/fonts.qrc"
    FILE "src/resources/icons.qrc"
    FILE "src/resources/images.qrc"
    FILE "src/resources/stylesheets.qrc"

    "src/widgets/config/ConfigTab"
    "src/widgets/config/MidiConfigTab"
    "src/widgets/config/SoundConfigTab"
    "src/widgets/config/SoundQualityPreview"
    "src/widgets/docks/BaseEditor"
    "src/widgets/docks/InstrumentEditor"
    "src/widgets/docks/TableDock"
    "src/widgets/docks/WaveEditor"
    "src/widgets/grid/PatternGrid"
    "src/widgets/grid/PatternGridHeader"
    "src/widgets/sidebar/AudioScope"
    "src/widgets/sidebar/OrderEditor"
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
if (NOT DEFINED WINDEPLOYQT-NOTFOUND)
    set(DEPLOY_DIR_NAME "${PROJECT_AND_VERSION_NAME}-win")
    string(APPEND DEPLOY_DIR_NAME "-${BUILD_ARCH}")
    # directory for deployment
    set(DEPLOY_DIR "${CMAKE_CURRENT_BINARY_DIR}/${DEPLOY_DIR_NAME}")
    # deploy target is a zip of the directory contents created by windeployqt
    set(DEPLOY_TARGET "${DEPLOY_DIR_NAME}.zip")
    add_custom_command(
        OUTPUT ${DEPLOY_TARGET}
        COMMAND ${WINDEPLOYQT} --dir ${DEPLOY_DIR} --no-translations --no-svg --no-angle --no-system-d3d-compiler --no-opengl-sw "$<TARGET_FILE:trackerboy_ui>"
        COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:trackerboy_ui>" ${DEPLOY_DIR}
        COMMAND ${CMAKE_COMMAND} -E tar "cfv" ${DEPLOY_TARGET} --format=zip ${DEPLOY_DIR}
        COMMAND ${CMAKE_COMMAND} -E rm -r ${DEPLOY_DIR}
        DEPENDS trackerboy_ui
    )
    add_custom_target(deploy DEPENDS ${DEPLOY_TARGET})

elseif (NOT DEFINED MACDEPLOYQT-NOTFOUND)

endif()

# also need to handle deployment for linux, qt doesn't provide a tool for linux unfortunately

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
