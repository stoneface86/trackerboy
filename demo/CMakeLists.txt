project(demo CXX)


# interactive demos
# these demos play sound and read input from the user

# code used by all interactive demos
add_library(interactive_demo STATIC "src/interactive/interactive.cpp")
target_link_libraries(interactive_demo PUBLIC trackerboy miniaudio)

add_executable(demo_apu "src/interactive/apu_demo.cpp")
target_link_libraries(demo_apu interactive_demo)

add_executable(demo_frequency "src/interactive/frequency_demo.cpp")
target_link_libraries(demo_frequency interactive_demo)

# WAV demos
# these demos produce WAV files

add_executable(demo_song "src/song_demo.cpp")
target_link_libraries(demo_song trackerboy miniaudio)

add_executable(demo_instrument "src/instrument_demo.cpp")
target_link_libraries(demo_instrument trackerboy miniaudio)
