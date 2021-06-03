project(ui CXX)

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

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

set(UI_SRC
    "core/audio/Renderer.cpp"
    "core/audio/Ringbuffer.cpp"
    "core/clipboard/PatternClip.cpp"
    "core/model/BaseTableModel.cpp"
    "core/model/InstrumentListModel.cpp"
    "core/model/ModuleDocument.cpp"
    "core/model/ModuleModel.cpp"
    "core/model/OrderModel.cpp"
    "core/model/PatternModel.cpp"
    "core/model/SongModel.cpp"
    "core/model/WaveListModel.cpp"
    "core/Config.cpp"
    "core/Miniaudio.cpp"
    "core/PatternSelection.cpp"
    "core/PianoInput.cpp"
    "core/samplerates.cpp"
    "core/SyncWorker.cpp"

    "forms/AudioDiagDialog.cpp"
    "forms/ConfigDialog.cpp"
    "forms/MainWindow.cpp"
    
    "misc/IconManager.cpp"
    "misc/utils.cpp"

    "resources/fonts.qrc"
    "resources/icons.qrc"
    "resources/images.qrc"

    "widgets/config/ConfigTab.cpp"
    "widgets/config/SoundConfigTab.cpp"
    "widgets/config/SoundQualityPreview.cpp"
    "widgets/docks/ModuleSettingsWidget.cpp"
    "widgets/editors/BaseEditor.cpp"
    "widgets/editors/InstrumentEditor.cpp"
    "widgets/editors/WaveEditor.cpp"
    "widgets/grid/layout.cpp"
    "widgets/grid/PatternGrid.cpp"
    "widgets/grid/PatternGridHeader.cpp"
    "widgets/grid/PatternPainter.cpp"
    "widgets/visualizers/AudioScope.cpp"
    "widgets/visualizers/PeakMeter.cpp"
    "widgets/visualizers/VolumeMeterAnimation.cpp"
    "widgets/CustomSpinBox.cpp"
    "widgets/EnvelopeForm.cpp"
    "widgets/OrderEditor.cpp"
    "widgets/PatternEditor.cpp"
    "widgets/PianoWidget.cpp"
    #"widgets/SequenceEditor.cpp"
    #"widgets/WaveGraph.cpp"
)

if (MSVC)
    # prevent warnings from external libraries
    add_compile_options(/W4 /experimental:external /external:W0 /external:anglebrackets)
endif ()

#
# ui library
#
add_library(ui STATIC ${UI_SRC})
target_link_libraries(ui PUBLIC 
    trackerboy
    miniaudio
    Qt5::Widgets
)

#
# Target for the main ui, trackerboy_ui. We cannot use trackerboy as that name is used
# for libtrackerboy. However, the resulting executable is named trackerboy, not trackerboy_ui
#
add_executable(trackerboy_ui ${GUI_TYPE} "main.cpp" ${WINDOWS_APPICON_RC})
target_link_libraries(trackerboy_ui PRIVATE ui)
# output executable is "trackerboy" and not "trackerboy_ui"
set_target_properties(trackerboy_ui PROPERTIES OUTPUT_NAME "trackerboy")

# no info or debug output via qDebug for release builds
if (NOT DEBUG_BUILD)
    target_compile_definitions(trackerboy_ui PRIVATE QT_NO_INFO_OUTPUT QT_NO_DEBUG_OUTPUT)
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

#add_executable(test_pattern_painter ${GUI_TYPE} EXCLUDE_FROM_ALL "test/test_pattern_painter.cpp" )
#target_link_libraries(test_pattern_painter
#    trackerboy
#    Qt5::Widgets
#)
