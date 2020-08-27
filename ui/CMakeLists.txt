project(ui CXX)


use_qt(Widgets Core)

set(INSTEDIT_SRC
    "instedit/instedit.cpp"
#    "instedit/InstEditor.cpp"
#    "instedit/SynthWorker.cpp"
    "instedit/PianoWidget.cpp"
    "instedit/keys.qrc"
)

set(UI_SRC
    "mainwindow.ui"
    "waveeditor.ui"
    "main.cpp"
    "MainWindow.cpp"
    "OrderEditor.cpp"
    "MainWidget.cpp"
    "PianoWidget.cpp"
    "WaveEditor.cpp"
    "WaveGraph.cpp"
    "instedit/keys.qrc"
)


add_executable(instedit ${GUI_TYPE} ${INSTEDIT_SRC})
target_link_libraries(instedit
    trackerboy
    audio
    Qt5::Widgets
    Qt5::Core
)


add_executable(trackerboy_ui ${GUI_TYPE} ${UI_SRC})
target_link_libraries(trackerboy_ui
    trackerboy
    Qt5::Widgets
    Qt5::Core
)

