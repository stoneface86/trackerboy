project(demo CXX)

set(DEMO "demo")
set(DEMO_SRC
    "main.cpp"
    "Demo.cpp"
    "demos/DutyDemo.cpp"
    "demos/SweepDemo.cpp"
    "demos/WaveDemo.cpp"
    "demos/WaveVolDemo.cpp"
)

add_executable(${DEMO} ${DEMO_SRC})
target_include_directories(${DEMO} PUBLIC ${INC_DIR})
target_link_libraries(${DEMO} trackerboy portaudio_static)

add_executable(demo_song "song_demo.cpp")
target_link_libraries(demo_song trackerboy portaudio_static audio)

add_executable(demo_device_table "device_table.cpp")
target_link_libraries(demo_device_table audio)
