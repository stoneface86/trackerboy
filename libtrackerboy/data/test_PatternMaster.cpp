#include "catch.hpp"
#include "trackerboy/data/PatternMaster.hpp"

using namespace trackerboy;


TEST_CASE("getTrack", "[PatternMaster]") {

    PatternMaster pm(64);

    SECTION("creates a new track when the requested one does not exist") {

        for (int i = 0; i != 4; ++i) {
            ChType ch = static_cast<ChType>(i);
            REQUIRE(pm.tracks(ch) == 0);
            REQUIRE_NOTHROW(pm.getTrack(ch, 0));
            REQUIRE(pm.tracks(ch) == 1);
        }
    }

    SECTION("does not create a new track when the requested one exists") {
        for (int i = 0; i != 4; ++i) {
            ChType ch = static_cast<ChType>(i);
            REQUIRE_NOTHROW(pm.getTrack(ch, 0));
            REQUIRE(pm.tracks(ch) == 1);
            REQUIRE_NOTHROW(pm.getTrack(ch, 0));
            REQUIRE(pm.tracks(ch) == 1);
        }
    }
}

TEST_CASE("remove", "[PatternMaster]") {
    PatternMaster pm(64);
    pm.getTrack(ChType::ch1, 0);
    pm.getTrack(ChType::ch1, 1);
    pm.getTrack(ChType::ch1, 4);

    pm.getTrack(ChType::ch2, 0);

    REQUIRE(pm.tracks(ChType::ch1) == 3);
    REQUIRE(pm.tracks(ChType::ch2) == 1);

    SECTION("removes a track for the specified channel") {
        REQUIRE_NOTHROW(pm.remove(ChType::ch1, 0));
        REQUIRE(pm.tracks(ChType::ch1) == 2);
        REQUIRE(pm.tracks(ChType::ch2) == 1);
    }

    SECTION("removing a non-existing track has no effect") {
        REQUIRE_NOTHROW(pm.remove(ChType::ch1, 2));
        REQUIRE(pm.tracks(ChType::ch1) == 3);
        REQUIRE(pm.tracks(ChType::ch2) == 1);
    }

}

TEST_CASE("setRowSize resizes all Tracks", "[PatternMaster]") {
    constexpr uint16_t OLD_SIZE = 64;
    constexpr uint16_t NEW_SIZE = 32;
    
    PatternMaster pm(OLD_SIZE);
    pm.getTrack(ChType::ch1, 0);
    pm.getTrack(ChType::ch2, 1);
    pm.getTrack(ChType::ch3, 4);
    pm.getTrack(ChType::ch4, 0);

    for (uint8_t i = 0; i != 4; ++i) {
        auto begin = pm.tracksBegin(static_cast<ChType>(i));
        auto end = pm.tracksBegin(static_cast<ChType>(i));
        for (auto iter = begin; iter != end; ++iter) {
            REQUIRE(iter->second.end() - iter->second.begin() == OLD_SIZE);
        }
    }

    REQUIRE(pm.rowSize() == OLD_SIZE);
    REQUIRE_NOTHROW(pm.setRowSize(NEW_SIZE));
    REQUIRE(pm.rowSize() == NEW_SIZE);

    for (uint8_t i = 0; i != 4; ++i) {
        auto begin = pm.tracksBegin(static_cast<ChType>(i));
        auto end = pm.tracksBegin(static_cast<ChType>(i));
        for (auto iter = begin; iter != end; ++iter) {
            REQUIRE(iter->second.end() - iter->second.begin() == NEW_SIZE);
        }
    }

}
