project(audio CXX C)


set(AUDIO_SRC
    "DeviceTable.cpp"
#    "PlaybackQueue.cpp"
)


add_library(audio STATIC ${AUDIO_SRC})

target_link_libraries(audio PUBLIC ${Soundio_LIBRARY})
