
project("libtrackerboy")

set(SRC_FILES
    "data/DataItem.cpp"
    "data/Instrument.cpp"
    "data/Module.cpp"
    "data/Order.cpp"
    "data/Pattern.cpp"
    "data/PatternMaster.cpp"
    "data/Sequence.cpp"
    "data/Song.cpp"
    "data/SongList.cpp"
    "data/Table.cpp"
    "data/Track.cpp"
    "data/TrackRow.cpp"
    "data/Waveform.cpp"

    "engine/ChannelControl.cpp"
    "engine/Engine.cpp"
    "engine/FrequencyControl.cpp"
    "engine/IApu.cpp"
    "engine/InstrumentRuntime.cpp"
    "engine/MusicRuntime.cpp"
    "engine/Operation.cpp"
    "engine/RuntimeContext.cpp"
    "engine/Timer.cpp"
    "engine/TrackControl.cpp"

    "export/Player.cpp"
    
    "internal/endian.cpp"

    "fileformat.cpp"
    "InstrumentPreview.cpp"
    "note.cpp"
    "Synth.cpp"
    "version.cpp"
)

# give version information to version.cpp via preproccessor macros
set_source_files_properties("version.cpp" PROPERTIES COMPILE_DEFINITIONS "VERSION_MAJOR=${trackerboy_VERSION_MAJOR};VERSION_MINOR=${trackerboy_VERSION_MINOR};VERSION_PATCH=${trackerboy_VERSION_PATCH}")
add_library("trackerboy" STATIC ${SRC_FILES})

unityBuildable(trackerboy)

target_include_directories(trackerboy PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(trackerboy PRIVATE trackerboyWarnings)
target_link_libraries(trackerboy PUBLIC gbapu)

if (IS_BIG_ENDIAN)
    target_compile_definitions(trackerboy PRIVATE -DTRACKERBOY_BIG_ENDIAN)
endif ()

if (ENABLE_TESTS)

    add_executable(test_trackerboy
        "test/data/test_Table.cpp"
        "test/data/test_Module.cpp"
        "test/data/test_PatternMaster.cpp"
        
        "test/engine/test_InstrumentRuntime.cpp"
        "test/engine/test_Timer.cpp"

        "test/internal/test_endian.cpp"
    )
    target_link_libraries(test_trackerboy PRIVATE trackerboy Catch2Main)
    target_include_directories(test_trackerboy PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    if (IS_BIG_ENDIAN)
        target_compile_definitions(test_trackerboy PRIVATE -DTRACKERBOY_BIG_ENDIAN)
    endif ()

    catch_discover_tests(test_trackerboy)

endif ()
