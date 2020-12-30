project(ui CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
find_package(Qt5 COMPONENTS Widgets Core REQUIRED)
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
    "core/audio/RenderBuffer.cpp"
    "core/audio/Renderer.cpp"
    "core/model/BaseModel.cpp"
    "core/model/BaseTableModel.cpp"
    "core/model/ModuleDocument.cpp"
    "core/model/InstrumentListModel.cpp"
    "core/model/OrderModel.cpp"
    "core/model/SongListModel.cpp"
    "core/model/WaveListModel.cpp"
    "core/Config.cpp"
    "core/Miniaudio.cpp"
    "core/samplerates.cpp"
    "core/Trackerboy.cpp"

    "forms/ConfigDialog.cpp"
    "forms/InstrumentEditor.cpp"
    "forms/MainWindow.cpp"
    "forms/WaveEditor.cpp"
    
    "misc/IconManager.cpp"
    "misc/SongActions.cpp"
    "misc/utils.cpp"

    "resources/fonts.qrc"
    "resources/icons.qrc"
    "resources/images.qrc"

    "widgets/config/ConfigTab.cpp"
    "widgets/config/SoundConfigTab.cpp"
    "widgets/docks/ModulePropertiesWidget.cpp"
    "widgets/docks/OrderWidget.cpp"
    "widgets/docks/SongPropertiesWidget.cpp"
    "widgets/docks/SongWidget.cpp"
    "widgets/docks/TableForm.cpp"
    "widgets/grid/layout.cpp"
    "widgets/grid/PatternGrid.cpp"
    "widgets/grid/PatternGridHeader.cpp"
    "widgets/grid/PatternPainter.cpp"
    "widgets/CustomSpinBox.cpp"
    "widgets/EnvelopeForm.cpp"
    "widgets/PatternEditor.cpp"
    "widgets/PianoWidget.cpp"
    "widgets/WaveGraph.cpp"
    
    "main.cpp"
    
)

if (MSVC)
    # prevent warnings from external libraries
    add_compile_options(/W4 /experimental:external /external:W0 /external:anglebrackets)
endif ()


add_executable(trackerboy_ui ${GUI_TYPE} ${UI_SRC} ${WINDOWS_APPICON_RC})
target_link_libraries(trackerboy_ui
    trackerboy
    miniaudio
    Qt5::Widgets
    Qt5::Core
)
set_target_properties(trackerboy_ui PROPERTIES OUTPUT_NAME "trackerboy")


add_executable(test_pattern_painter ${GUI_TYPE} EXCLUDE_FROM_ALL "test/test_pattern_painter.cpp")
target_link_libraries(test_pattern_painter
    trackerboy
    Qt5::Widgets
    Qt5::Core
)
