project(ui CXX)


use_qt(Widgets Core)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(UI_SRC
    "designer/mainwindow.ui"
    "designer/waveeditor.ui"
    #"model/ModuleDocument.cpp"
    #"model/ModuleModel.cpp"
    "model/WaveListModel.cpp"
    "resources/keys.qrc"
    "InstrumentEditor.cpp"
    "main.cpp"
    "MainWidget.cpp"
    "MainWindow.cpp"
    "OrderEditor.cpp"
    "PianoWidget.cpp"
    "WaveEditor.cpp"
    "WaveGraph.cpp"
)

if (MSVC)
    # prevent warnings from external libraries
    add_compile_options(/W4 /experimental:external /external:W0 /external:anglebrackets)
endif ()


add_executable(trackerboy_ui ${GUI_TYPE} ${UI_SRC})
target_link_libraries(trackerboy_ui
    trackerboy
    Qt5::Widgets
    Qt5::Core
)

# Test UIs

# Wave editor widget
add_executable(ui_test_WaveEditor ${GUI_TYPE}
    "designer/waveeditor.ui"
    "model/WaveListModel.cpp"
    "resources/keys.qrc"
    "ui_test_WaveEditor.cpp"
    "PianoWidget.cpp"
    "WaveEditor.cpp"
    "WaveGraph.cpp"
)
target_link_libraries(ui_test_WaveEditor
    trackerboy
    Qt5::Widgets
    Qt5::Core
)

