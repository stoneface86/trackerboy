project(ui CXX)


set(UI_SRC
    "TrackerboyApp.cpp"
    "MainWindow.cpp"
    "docview/ModuleDocument.cpp"
    "docview/ModuleView.cpp"
)

add_executable(trackerboy_ui ${GUI_TYPE} ${UI_SRC})
target_include_directories(trackerboy_ui PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(trackerboy_ui 
    ${wxWidgets_LIBRARIES}
    trackerboy
    portaudio_static
    audio
)

