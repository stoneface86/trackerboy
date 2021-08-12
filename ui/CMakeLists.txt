project(ui CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
find_package(Qt5 COMPONENTS Widgets REQUIRED)


get_target_property(_qmake_executable Qt5::qmake IMPORTED_LOCATION)
get_filename_component(_qt_bin_dir "${_qmake_executable}" DIRECTORY)

if (WIN32)
    set(GUI_TYPE WIN32)
    set(WINDOWS_APPICON_RC "resources/icons/app/appicon.rc")
    find_program(WINDEPLOYQT windeployqt HINTS "${_qt_bin_dir}")
elseif (APPLE)
    set(GUI_TYPE MACOSX_BUNDLE)
    find_program(MACDEPLOYQT macdeployqt HINTS "${_qt_bin_dir}")
else ()
    set(GUI_TYPE "")
endif ()

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

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
    "core/audio/AudioProber"
    "core/audio/AudioStream"
    "core/audio/Renderer"
    "core/audio/Ringbuffer"
    "core/audio/VisualizerBuffer"
    "core/config/AppearanceConfig"
    "core/config/GeneralConfig"
    #"core/config/KeyboardConfig"
    "core/config/keys"
    "core/config/MidiConfig"
    "core/config/SoundConfig"
    "core/clipboard/PatternClip"
    "core/clipboard/PatternClipboard"
    "core/graphics/CellPainter"
    "core/graphics/PatternLayout"
    "core/graphics/PatternPainter"
    "core/midi/Midi"
    "core/midi/MidiProber"
    "core/model/graph/GraphModel"
    "core/model/graph/SequenceModel"
    "core/model/graph/WaveModel"
    "core/model/BaseTableModel"
    "core/model/InstrumentListModel"
    "core/model/PatternModel"
    "core/model/SongListModel"
    "core/model/SongModel"
    "core/model/WaveListModel"
    "core/Config"
    "core/FastTimer"
    "core/Module"
    "core/ModuleFile"
    "core/Palette"
    "core/PatternSelection"
    "core/PianoInput"
    "core/samplerates"
    "core/WavExporter"

    FILE "forms/MainWindow/actions.cpp"
    FILE "forms/MainWindow/slots.cpp"
    "forms/AboutDialog"
    "forms/AudioDiagDialog"
    "forms/ConfigDialog"
    "forms/ExportWavDialog"
    "forms/MainWindow"
    "forms/TempoCalculator"

    "misc/IconManager"
    "misc/utils"

    FILE "resources/fonts.qrc"
    FILE "resources/icons.qrc"
    FILE "resources/images.qrc"
    FILE "resources/stylesheets.qrc"

    "widgets/config/ConfigTab"
    "widgets/config/MidiConfigTab"
    "widgets/config/SoundConfigTab"
    "widgets/config/SoundQualityPreview"
    "widgets/docks/BaseEditor"
    "widgets/docks/InstrumentEditor"
    "widgets/docks/TableDock"
    "widgets/docks/WaveEditor"
    "widgets/grid/PatternGrid"
    "widgets/grid/PatternGridHeader"
    "widgets/sidebar/AudioScope"
    "widgets/sidebar/OrderEditor"
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

unityBuildable(ui)

if (NOT ${DEBUG_BUILD})
    target_compile_definitions(ui PUBLIC QT_NO_INFO_OUTPUT QT_NO_DEBUG_OUTPUT)
endif ()


#
# Target for the main ui, trackerboy_ui. We cannot use trackerboy as that name is used
# for libtrackerboy. However, the resulting executable is named trackerboy, not trackerboy_ui
#
add_executable(trackerboy_ui ${GUI_TYPE} "main.cpp" ${WINDOWS_APPICON_RC})
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
