
project("libtrackerboy")

set(SRC_FILES
    "data/DataItem.cpp"
    "data/DataList.cpp"
    "data/Instrument.cpp"
    "data/Module.cpp"
    "data/Order.cpp"
    "data/Pattern.cpp"
    "data/PatternMaster.cpp"
    "data/Sequence.cpp"
    "data/Song.cpp"
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
    
    "internal/endian.cpp"

    "fileformat.cpp"
    "note.cpp"
    "Synth.cpp"
    "version.cpp"
)

# give version information to version.cpp via preproccessor macros
set_source_files_properties("version.cpp" PROPERTIES COMPILE_DEFINITIONS "VERSION_MAJOR=${trackerboy_VERSION_MAJOR};VERSION_MINOR=${trackerboy_VERSION_MINOR};VERSION_PATCH=${trackerboy_VERSION_PATCH}")
add_library("trackerboy" STATIC ${SRC_FILES})

target_include_directories(trackerboy PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

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

target_link_libraries(trackerboy PUBLIC gbapu)

if (IS_BIG_ENDIAN)
    target_compile_definitions(trackerboy PRIVATE -DTRACKERBOY_BIG_ENDIAN)
endif ()

if (TRACKERBOY_ENABLE_TESTS)

    set(TEST_SRC_FILES
        "test/data/test_DataList.cpp"
        "test/data/test_Module.cpp"
        "test/data/test_PatternMaster.cpp"
        
        "test/engine/test_InstrumentRuntime.cpp"
        "test/engine/test_Timer.cpp"

        "test/internal/test_endian.cpp"
    )

    add_catch_test(TEST_EXE ${TEST_SRC_FILES})

    target_link_libraries(${TEST_EXE} PRIVATE trackerboy)

    if (IS_BIG_ENDIAN)
        target_compile_definitions(${TEST_EXE} PRIVATE -DTRACKERBOY_BIG_ENDIAN)
    endif ()

    macro(unit_test unit)
        add_test(NAME ${unit} COMMAND ${TEST_EXE} "[${unit}]")
    endmacro(unit_test)

    unit_test(DataList)
    unit_test(Module)
    unit_test(InstrumentRuntime)
    unit_test(PatternMaster)
    unit_test(NoteControl)
    unit_test(Timer)
    unit_test(endian)
    unit_test(Table)
    
    

endif (TRACKERBOY_ENABLE_TESTS)
