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

set(UI_SRC
    "core/audio/AudioProber.cpp"
    "core/audio/AudioStream.cpp"
    #"core/audio/Renderer.cpp"
    "core/audio/Ringbuffer.cpp"
    "core/audio/VisualizerBuffer.cpp"
    "core/clipboard/PatternClip.cpp"
    "core/clipboard/PatternClipboard.cpp"
    "core/midi/Midi.cpp"
    "core/midi/MidiProber.cpp"
    "core/model/graph/GraphModel.cpp"
    "core/model/graph/SequenceModel.cpp"
    "core/model/graph/WaveModel.cpp"
    "core/model/BaseTableModel.cpp"
    "core/model/InstrumentChoiceModel.cpp"
    "core/model/InstrumentListModel.cpp"
    #"core/model/ModuleModel.cpp"
    #"core/model/OrderModel.cpp"
    "core/model/PatternModel.cpp"
    "core/model/SongListModel.cpp"
    "core/model/SongModel.cpp"
    "core/model/WaveListModel.cpp"
    "core/Config.cpp"
    "core/FastTimer.cpp"
    "core/Module.cpp"
    "core/ModuleFile.cpp"
    "core/PatternSelection.cpp"
    "core/PianoInput.cpp"
    "core/samplerates.cpp"
    #"core/WavExporter.cpp"

    "forms/MainWindow/actions.cpp"
    "forms/MainWindow/slots.cpp"
    "forms/AboutDialog.cpp"
    #"forms/AudioDiagDialog.cpp"
    "forms/ConfigDialog.cpp"
    #"forms/ExportWavDialog.cpp"
    "forms/MainWindow.cpp"
    
    "misc/IconManager.cpp"
    "misc/utils.cpp"

    "resources/fonts.qrc"
    "resources/icons.qrc"
    "resources/images.qrc"
    "resources/stylesheets.qrc"

    "widgets/config/ConfigTab.cpp"
    "widgets/config/MidiConfigTab.cpp"
    "widgets/config/SoundConfigTab.cpp"
    "widgets/config/SoundQualityPreview.cpp"
    "widgets/docks/TableDock.cpp"
    #"widgets/editors/BaseEditor.cpp"
    #"widgets/editors/InstrumentEditor.cpp"
    #"widgets/editors/WaveEditor.cpp"
    "widgets/grid/layout.cpp"
    "widgets/grid/PatternGrid.cpp"
    "widgets/grid/PatternGridHeader.cpp"
    "widgets/grid/PatternPainter.cpp"
    "widgets/sidebar/AudioScope.cpp"
    "widgets/sidebar/OrderEditor.cpp"
    "widgets/sidebar/SongEditor.cpp"
    #"widgets/visualizers/PeakMeter.cpp"
    #"widgets/visualizers/VolumeMeterAnimation.cpp"
    "widgets/CustomSpinBox.cpp"
    "widgets/EnvelopeForm.cpp"
    #"widgets/GraphEdit.cpp"
    "widgets/PatternEditor.cpp"
    "widgets/PianoWidget.cpp"
    #"widgets/SequenceEditor.cpp"
    "widgets/Sidebar.cpp"
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
    if (${BUILD_IS_32BIT})
        string(APPEND DEPLOY_DIR_NAME "-x86")
    else ()
        string(APPEND DEPLOY_DIR_NAME "-x64")
    endif ()
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
