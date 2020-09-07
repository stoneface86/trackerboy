
project("libtrackerboy")

set(SRC_FILES
    "export/Wav.cpp"
    "engine/Engine.cpp"
    "engine/FrequencyControl.cpp"
    "engine/MusicRuntime.cpp"
    "engine/NoteControl.cpp"
    "engine/PatternCursor.cpp"
    "engine/RuntimeContext.cpp"
    "engine/Timer.cpp"
    "data/DataItem.cpp"
    "data/Instrument.cpp"
    "data/Module.cpp"
    "data/PatternMaster.cpp"
    "data/Song.cpp"
    "data/Table.cpp"
    "data/Track.cpp"
    "data/Waveform.cpp"
    "synth/Envelope.cpp"
    "synth/Generator.cpp"
    "synth/Mixer.cpp"
    "synth/NoiseGen.cpp"
    "synth/PulseGen.cpp"
    "synth/Sequencer.cpp"
    "synth/Sweep.cpp"
    "synth/Synth.cpp"
    "synth/WaveGen.cpp"
    "fileformat.cpp"
    "note.cpp"
    "version.cpp"
)

# give version information to version.cpp via preproccessor macros
set_source_files_properties("version.cpp" PROPERTIES COMPILE_DEFINITIONS "VERSION_MAJOR=${trackerboy_VERSION_MAJOR};VERSION_MINOR=${trackerboy_VERSION_MINOR};VERSION_PATCH=${trackerboy_VERSION_PATCH}")
add_library("trackerboy" STATIC ${SRC_FILES})

if (MSVC)
set(WARNFLAGS /W4)
elseif (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
# -Wno-missing-field-initializers prevents warnings when zero-initializing a struct
set(WARNFLAGS -Wall -Wextra -pedantic -Wno-missing-field-initializers)
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
        "data/test_DataItem.cpp"
        "data/test_Instrument.cpp"
        "data/test_PatternMaster.cpp"
        "engine/test_NoteControl.cpp"
        "engine/test_Timer.cpp"
        "test_fileformat.cpp"
        "test_Table.cpp"
    )

    add_catch_test(TEST_EXE ${TEST_SRC_FILES})

    target_link_libraries(${TEST_EXE} PRIVATE trackerboy)

    if (IS_BIG_ENDIAN)
        target_compile_definitions(${TEST_EXE} PRIVATE -DTRACKERBOY_BIG_ENDIAN)
    endif ()

    add_test(NAME DataItem COMMAND ${TEST_EXE} [DataItem])
    add_test(NAME Instrument COMMAND ${TEST_EXE} [Instrument])
    add_test(NAME PatternMaster COMMAND ${TEST_EXE} [PatternMaster])
    add_test(NAME NoteControl COMMAND ${TEST_EXE} [NoteControl])
    add_test(NAME Timer COMMAND ${TEST_EXE} [Timer])
    add_test(NAME fileformat COMMAND ${TEST_EXE} [fileformat])
    add_test(NAME Table COMMAND ${TEST_EXE} [Table])
    
    

endif (TRACKERBOY_ENABLE_TESTS)
