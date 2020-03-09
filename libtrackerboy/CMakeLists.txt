
project("libtrackerboy")

set(SRC_FILES
    "instrument/Instrument.cpp"
    "instrument/InstrumentRuntime.cpp"
    "pattern/Pattern.cpp"
    "pattern/PatternRuntime.cpp"
    "pattern/TrackRuntime.cpp"
    "pattern/Track.cpp"
    "song/Order.cpp"
    "song/Song.cpp"
    "song/SongRuntime.cpp"
    "synth/Envelope.cpp"
    "synth/gbsynth.cpp"
    "synth/NoiseGen.cpp"
    "synth/Osc.cpp"
    "synth/PulseOsc.cpp"
    "synth/Sweep.cpp"
    "synth/Synth.cpp"
    "synth/Waveform.cpp"
    "synth/WaveOsc.cpp"
    "File.cpp"
    "fileformat.cpp"
    "Module.cpp"
    "note.cpp"
    "Table.cpp"
    "version.cpp"
)

# give version information to version.cpp via preproccessor macros
set_source_files_properties("version.cpp" PROPERTIES COMPILE_DEFINITIONS "VERSION_MAJOR=${CMAKE_PROJECT_VERSION_MAJOR};VERSION_MINOR=${CMAKE_PROJECT_VERSION_MINOR};VERSION_PATCH=${CMAKE_PROJECT_VERSION_PATCH}")
add_library("trackerboy" STATIC ${SRC_FILES})

if (MSVC)
target_compile_options(
    trackerboy
    PRIVATE
    /W4
)
endif ()

target_link_libraries(trackerboy PUBLIC trackerboyCoverage)

if (IS_BIG_ENDIAN)
    target_compile_definitions(trackerboy PRIVATE -DTRACKERBOY_BIG_ENDIAN)
endif ()

if (TRACKERBOY_ENABLE_TESTS)

    set(TEST_SRC_FILES
        "pattern/test_Pattern.cpp"
        "song/test_Order.cpp"
        "test_File.cpp"
        "test_fileformat.cpp"
        "test_Table.cpp"
    )

    add_catch_test(TEST_EXE ${TEST_SRC_FILES})

    target_link_libraries(${TEST_EXE} PRIVATE trackerboy)

    if (IS_BIG_ENDIAN)
        target_compile_definitions(${TEST_EXE} PRIVATE -DTRACKERBOY_BIG_ENDIAN)
    endif ()

    add_test(NAME Pattern COMMAND ${TEST_EXE} [pattern])
    add_test(NAME Order COMMAND ${TEST_EXE} [order])
    add_test(NAME File COMMAND ${TEST_EXE} [File])
    add_test(NAME fileformat COMMAND ${TEST_EXE} [fileformat])
    add_test(NAME Table COMMAND ${TEST_EXE} [Table])

endif (TRACKERBOY_ENABLE_TESTS)
