project(audio CXX C)


set(AUDIO_SRC
#    "BackendTable.cpp"
#    "DeviceTable.cpp"
    "PlaybackQueue.cpp"
#    "SoundIoError.cpp"
)


add_library(audio STATIC ${AUDIO_SRC})

target_link_libraries(audio PUBLIC miniaudio)

#target_link_libraries(audio PUBLIC ${Soundio_LIBRARY})
#target_compile_definitions(audio PUBLIC SOUNDIO_STATIC_LIBRARY)
