# declare dependencies here, so that CI can use this file's contents as a cache key


FetchContent_Declare(
    miniaudio
    GIT_REPOSITORY https://github.com/stoneface86/miniaudio-cmake.git
    GIT_TAG dadfa05c8e189e8336f6441206c3f08d6a52f750 # version 0.10.42
)

FetchContent_Declare(
    libtrackerboy
    GIT_REPOSITORY https://github.com/stoneface86/libtrackerboy.git
    GIT_TAG e942550c9985c93741e7580a697b65c9639109d3
)

FetchContent_Declare(
    rtmidi
    GIT_REPOSITORY https://github.com/thestk/rtmidi.git
    GIT_TAG cc887191c3b4cb6697aeba5536d9f4eb423090aa # 4.0.0
)
