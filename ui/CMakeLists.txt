project(ui CXX)


set(UI_SRC
    "main.cpp"
    "MainWindow.cpp"
)

add_executable(trackerboy_ui ${GUI_TYPE} ${UI_SRC})
target_link_libraries(trackerboy_ui 
    ${wxWidgets_LIBRARIES}
    trackerboy
    portaudio_static
    audio
)

