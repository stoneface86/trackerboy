project(ui CXX)


use_qt(Widgets Core)
#set(CMAKE_AUTOUIC_SEARCH_PATHS "designer")

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
    "model/SongListModel.cpp"
    "model/WaveListModel.cpp"
    
    "resources/icons.qrc"
    "resources/keys.qrc"

    "widgets/EnvelopeForm.cpp"
    "widgets/MainWidget.cpp"
    "widgets/OrderEditor.cpp"
    "widgets/PianoWidget.cpp"
    "widgets/TableForm.cpp"
    "widgets/WaveGraph.cpp"
    
    "Config.cpp"
    "main.cpp"
    "Renderer.cpp"
    "Tileset.cpp"
)

qt5_wrap_ui(UI_DESIGNER
    "designer/ConfigDialog.ui"
    "designer/EnvelopeForm.ui"
    "designer/MainWindow.ui"
    "designer/WaveEditor.ui"
    "designer/InstrumentEditor.ui"
)

if (MSVC)
    # prevent warnings from external libraries
    add_compile_options(/W4 /experimental:external /external:W0 /external:anglebrackets)
endif ()


add_executable(trackerboy_ui ${GUI_TYPE} ${UI_SRC} ${UI_DESIGNER})
target_link_libraries(trackerboy_ui
    trackerboy
    portaudio_static
    audio
    Qt5::Widgets
    Qt5::Core
)

