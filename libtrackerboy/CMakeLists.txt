
project("libtrackerboy")

set(INCLUDE_DIR "${CMAKE_SOURCE_DIR}/include")

# paths are relative to ${INCLUDE_DIR}/
set(PUB_HEADERS
    "trackerboy/data/DataItem.hpp"
    "trackerboy/data/Instrument.hpp"
    "trackerboy/data/Module.hpp"
    "trackerboy/data/Order.hpp"
    "trackerboy/data/OrderRow.hpp"
    "trackerboy/data/Pattern.hpp"
    "trackerboy/data/PatternMaster.hpp"
    "trackerboy/data/PatternRow.hpp"
    "trackerboy/data/Sequence.hpp"
    "trackerboy/data/Song.hpp"
    "trackerboy/data/SongList.hpp"
    "trackerboy/data/Table.hpp"
    "trackerboy/data/Track.hpp"
    "trackerboy/data/TrackRow.hpp"
    "trackerboy/data/Waveform.hpp"
    "trackerboy/engine/ChannelControl.hpp"
    "trackerboy/engine/ChannelState.hpp"
    "trackerboy/engine/Engine.hpp"
    "trackerboy/engine/Frame.hpp"
    "trackerboy/engine/FrequencyControl.hpp"
    "trackerboy/engine/GlobalState.hpp"
    "trackerboy/engine/IApu.hpp"
    "trackerboy/engine/InstrumentRuntime.hpp"
    "trackerboy/engine/MusicRuntime.hpp"
    "trackerboy/engine/Operation.hpp"
    "trackerboy/engine/RuntimeContext.hpp"
    "trackerboy/engine/Timer.hpp"
    "trackerboy/engine/TrackControl.hpp"
    "trackerboy/export/Player.hpp"
    "trackerboy/fileformat.hpp"
    "trackerboy/InstrumentPreview.hpp"
    "trackerboy/note.hpp"
    "trackerboy/Synth.hpp"
    "trackerboy/trackerboy.hpp"
    "trackerboy/version.hpp"
)

foreach (arg IN ITEMS ${PUB_HEADERS})
    list(APPEND _lst "${INCLUDE_DIR}/${arg}")
endforeach ()

set(PUB_HEADERS "${_lst}")
unset(_lst)

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

    "internal/endian.hpp"
    "internal/enumutils.hpp"

    "fileformat.cpp"
    "InstrumentPreview.cpp"
    "note.cpp"
    "Synth.cpp"
    "version.cpp"
)

if (IS_BIG_ENDIAN)
    list(APPEND SRC_FILES "internal/endian.cpp")
endif ()


# give version information to version.cpp via preproccessor macros
set_source_files_properties("version.cpp" PROPERTIES COMPILE_DEFINITIONS "VERSION_MAJOR=${trackerboy_VERSION_MAJOR};VERSION_MINOR=${trackerboy_VERSION_MINOR};VERSION_PATCH=${trackerboy_VERSION_PATCH}")
add_library("trackerboy" STATIC "${SRC_FILES}" "${PUB_HEADERS}")

unityBuildable(trackerboy)

target_include_directories(trackerboy PUBLIC "${INCLUDE_DIR}")
target_include_directories(trackerboy PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

target_link_libraries(trackerboy PRIVATE trackerboyWarnings)
# gbapu is the default APU implementation
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
