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
    "forms/ConfigDialog.cpp"
    "forms/InstrumentEditor.cpp"
    "forms/MainWindow.cpp"
    "forms/WaveEditor.cpp"
    
    "model/BaseModel.cpp"
    "model/BaseTableModel.cpp"
    "model/ModuleDocument.cpp"
    "model/InstrumentListModel.cpp"
    "model/OrderModel.cpp"
    "model/SongListModel.cpp"
    "model/WaveListModel.cpp"
    
    "resources/fonts.qrc"
    "resources/icons.qrc"
    "resources/keys.qrc"

    "widgets/docks/ModulePropertiesWidget.cpp"
    "widgets/docks/OrderWidget.cpp"
    "widgets/docks/SongPropertiesWidget.cpp"
    "widgets/docks/SongWidget.cpp"
    "widgets/docks/TableForm.cpp"
    "widgets/grid/PatternGrid.cpp"
    "widgets/EnvelopeForm.cpp"
    "widgets/PatternEditor.cpp"
    "widgets/PianoWidget.cpp"
    "widgets/WaveGraph.cpp"
    
    "Config.cpp"
    "main.cpp"
    "Miniaudio.cpp"
    "Renderer.cpp"
    "RendererThread.cpp"
    "samplerates.cpp"
    "Tileset.cpp"
    "Trackerboy.cpp"
)

qt5_wrap_ui(UI_DESIGNER
    "designer/ConfigDialog.ui"
    "designer/EnvelopeForm.ui"
    "designer/MainWindow.ui"
    "designer/ModulePropertiesWidget.ui"
    "designer/OrderWidget.ui"
    "designer/SongPropertiesWidget.ui"
    "designer/SongWidget.ui"
    "designer/WaveEditor.ui"
    "designer/InstrumentEditor.ui"
)

if (MSVC)
    # prevent warnings from external libraries
    add_compile_options(/W4 /experimental:external /external:W0 /external:anglebrackets)
endif ()


add_executable(trackerboy_ui ${GUI_TYPE} ${UI_SRC} ${UI_DESIGNER} ${WINDOWS_APPICON_RC})
target_link_libraries(trackerboy_ui
    trackerboy
    miniaudio
    Qt5::Widgets
    Qt5::Core
)
set_target_properties(trackerboy_ui PROPERTIES OUTPUT_NAME "trackerboy")
