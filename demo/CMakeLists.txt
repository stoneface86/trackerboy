project(demo CXX)


# interactive demos
# these program play sound and read input from the user

# code used by all interactive demos
add_library(interactive_demo STATIC "interactive/interactive.cpp")
target_link_libraries(interactive_demo PUBLIC trackerboy miniaudio)

add_executable(demo_apu "interactive/apu_demo.cpp")
target_link_libraries(demo_apu interactive_demo)

add_executable(demo_frequency "interactive/frequency_demo.cpp")
target_link_libraries(demo_frequency interactive_demo)

# WAV demos
# these demos produce WAV files

add_executable(demo_song "song_demo.cpp")
target_link_libraries(demo_song trackerboy miniaudio)

add_executable(demo_instrument "instrument_demo.cpp")
target_link_libraries(demo_instrument trackerboy miniaudio)
