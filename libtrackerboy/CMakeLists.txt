
project("libtrackerboy")

set(PUB_HEADERS
    "include/trackerboy/data/DataItem.hpp"
    "include/trackerboy/data/InfoStr.hpp"
    "include/trackerboy/data/Instrument.hpp"
    "include/trackerboy/data/Module.hpp"
    "include/trackerboy/data/Order.hpp"
    "include/trackerboy/data/OrderRow.hpp"
    "include/trackerboy/data/Pattern.hpp"
    "include/trackerboy/data/PatternMaster.hpp"
    "include/trackerboy/data/PatternRow.hpp"
    "include/trackerboy/data/Sequence.hpp"
    "include/trackerboy/data/Song.hpp"
    "include/trackerboy/data/SongList.hpp"
    "include/trackerboy/data/Table.hpp"
    "include/trackerboy/data/Track.hpp"
    "include/trackerboy/data/TrackRow.hpp"
    "include/trackerboy/data/Waveform.hpp"
    "include/trackerboy/engine/ChannelControl.hpp"
    "include/trackerboy/engine/ChannelState.hpp"
    "include/trackerboy/engine/Engine.hpp"
    "include/trackerboy/engine/Frame.hpp"
    "include/trackerboy/engine/FrequencyControl.hpp"
    "include/trackerboy/engine/GlobalState.hpp"
    "include/trackerboy/engine/IApu.hpp"
    "include/trackerboy/engine/InstrumentRuntime.hpp"
    "include/trackerboy/engine/MusicRuntime.hpp"
    "include/trackerboy/engine/Operation.hpp"
    "include/trackerboy/engine/RuntimeContext.hpp"
    "include/trackerboy/engine/Timer.hpp"
    "include/trackerboy/engine/TrackControl.hpp"
    "include/trackerboy/export/Player.hpp"
    "include/trackerboy/InstrumentPreview.hpp"
    "include/trackerboy/note.hpp"
    "include/trackerboy/Synth.hpp"
    "include/trackerboy/trackerboy.hpp"
    "include/trackerboy/version.hpp"
)

set(SRC_FILES
    "src/data/DataItem.cpp"
    "src/data/InfoStr.cpp"
    "src/data/Instrument.cpp"
    "src/data/Module.cpp"
    "src/data/Order.cpp"
    "src/data/Pattern.cpp"
    "src/data/PatternMaster.cpp"
    "src/data/Sequence.cpp"
    "src/data/Song.cpp"
    "src/data/SongList.cpp"
    "src/data/Table.cpp"
    "src/data/Track.cpp"
    "src/data/TrackRow.cpp"
    "src/data/Waveform.cpp"

    "src/engine/ChannelControl.cpp"
    "src/engine/Engine.cpp"
    "src/engine/FrequencyControl.cpp"
    "src/engine/IApu.cpp"
    "src/engine/InstrumentRuntime.cpp"
    "src/engine/MusicRuntime.cpp"
    "src/engine/Operation.cpp"
    "src/engine/RuntimeContext.cpp"
    "src/engine/Timer.cpp"
    "src/engine/TrackControl.cpp"

    "src/export/Player.cpp"
    
    "src/internal/fileformat/payload/deserializePayload0.cpp"
    "src/internal/fileformat/payload/deserializePayload1.cpp"
    "src/internal/fileformat/payload/payload.cpp"
    "src/internal/fileformat/payload/payload.hpp"
    "src/internal/fileformat/payload/PayloadHandler.hpp"
    "src/internal/fileformat/payload/serializePayload.cpp"
    "src/internal/fileformat/payload/handlers/CommHandler.cpp"
    "src/internal/fileformat/payload/handlers/CommHandler.hpp"
    "src/internal/fileformat/payload/handlers/InstHandler.cpp"
    "src/internal/fileformat/payload/handlers/InstHandler.hpp"
    "src/internal/fileformat/payload/handlers/SongHandler.cpp"
    "src/internal/fileformat/payload/handlers/SongHandler.hpp"
    "src/internal/fileformat/payload/handlers/TableHandler.hpp"
    "src/internal/fileformat/payload/handlers/WaveHandler.cpp"
    "src/internal/fileformat/payload/handlers/WaveHandler.hpp"
    "src/internal/fileformat/Block.hpp"
    "src/internal/fileformat/Block.cpp"
    "src/internal/fileformat/fileformat.hpp"
    "src/internal/fileformat/fileformat.cpp"
    "src/internal/endian.hpp"
    "src/internal/enumutils.hpp"
    
    "src/InstrumentPreview.cpp"
    "src/note.cpp"
    "src/Synth.cpp"
    "src/version.cpp"
)

include(TestBigEndian)
TEST_BIG_ENDIAN(IS_BIG_ENDIAN)

if (IS_BIG_ENDIAN)
    list(APPEND SRC_FILES "src/internal/endian.cpp")
endif ()


# give version information to version.cpp via preproccessor macros
set_source_files_properties("src/version.cpp" PROPERTIES COMPILE_DEFINITIONS "VERSION_MAJOR=${trackerboy_VERSION_MAJOR};VERSION_MINOR=${trackerboy_VERSION_MINOR};VERSION_PATCH=${trackerboy_VERSION_PATCH}")
add_library("trackerboy" STATIC "${SRC_FILES}" "${PUB_HEADERS}")

unityBuildable(trackerboy)

target_include_directories(trackerboy PUBLIC "include")
target_include_directories(trackerboy PRIVATE "src")

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
        "test/internal/fileformat/test_Block.cpp"
    )
    target_link_libraries(test_trackerboy PRIVATE trackerboy Catch2Main)
    target_include_directories(test_trackerboy PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
    if (IS_BIG_ENDIAN)
        target_compile_definitions(test_trackerboy PRIVATE -DTRACKERBOY_BIG_ENDIAN)
    endif ()

    catch_discover_tests(test_trackerboy)

endif ()
