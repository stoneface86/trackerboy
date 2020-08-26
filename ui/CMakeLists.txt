project(ui CXX)


use_qt(Widgets Core)

set(INSTEDIT_SRC
    "instedit/instedit.cpp"
#    "instedit/InstEditor.cpp"
#    "instedit/SynthWorker.cpp"
    "instedit/PianoWidget.cpp"
    "instedit/keys.qrc"
)


add_executable(instedit ${GUI_TYPE} ${INSTEDIT_SRC})
target_link_libraries(instedit
    trackerboy
    audio
    Qt5::Widgets
    Qt5::Core
)

