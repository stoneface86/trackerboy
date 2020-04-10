
project("libtrackerboy")

set(SRC_FILES
    "export/Wav.cpp"
    "instrument/Instrument.cpp"
    "instrument/InstrumentRuntime.cpp"
    "pattern/PatternMaster.cpp"
    "pattern/PatternRuntime.cpp"
    "pattern/TrackRuntime.cpp"
    "pattern/Track.cpp"
    "song/Song.cpp"
    "song/SongRuntime.cpp"
    "synth/Envelope.cpp"
    "synth/Generator.cpp"
    "synth/Mixer.cpp"
    "synth/NoiseGen.cpp"
    "synth/PulseGen.cpp"
    "synth/Sequencer.cpp"
    "synth/Sweep.cpp"
    "synth/Synth.cpp"
    "synth/Waveform.cpp"
    "synth/WaveGen.cpp"
    "File.cpp"
    "fileformat.cpp"
    "Module.cpp"
    "note.cpp"
    "Table.cpp"
    "version.cpp"
)

# give version information to version.cpp via preproccessor macros
set_source_files_properties("version.cpp" PROPERTIES COMPILE_DEFINITIONS "VERSION_MAJOR=${trackerboy_VERSION_MAJOR};VERSION_MINOR=${trackerboy_VERSION_MINOR};VERSION_PATCH=${trackerboy_VERSION_PATCH}")
add_library("trackerboy" STATIC ${SRC_FILES})

if (MSVC)
set(WARNFLAGS /W4)
elseif (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
set(WARNFLAGS -Wall -Wextra -pedantic)
endif ()

target_compile_options(
    trackerboy
    PRIVATE
    ${WARNFLAGS}
)

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
