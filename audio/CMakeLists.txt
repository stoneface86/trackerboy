project(audio CXX C)


set(AUDIO_SRC
    "DeviceManager.cpp"
    "PlaybackQueue.cpp"
    "PaException.cpp"
)


add_library(audio STATIC ${AUDIO_SRC})

target_link_libraries(audio PUBLIC portaudio_static portaudio_ringbuffer)
