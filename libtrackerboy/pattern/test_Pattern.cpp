
#include "catch.hpp"
#include "./Track.cpp"
#include "./Pattern.cpp"

namespace trackerboy {

static bool rowIsZero(TrackRow row) {
    return row.flags == 0 &&
        row.note == 0 &&
        row.instrumentId == 0 &&
        row.effect1 == 0;
}



TEST_CASE("Pattern data should be cleared on construction", "[pattern]") {

    constexpr size_t PATTERN_SIZE = 64;
    Pattern pat(PATTERN_SIZE);
    REQUIRE(pat.size() == PATTERN_SIZE);

    for (auto iter = pat.begin(); iter != pat.end(); ++iter) {
        REQUIRE(rowIsZero(*iter));
    }

}

TEST_CASE("setting a new size effects rows", "[pattern]") {

    constexpr size_t INIT_SIZE = 5, SHRINK_SIZE = 2, GROW_SIZE = 10;
    Pattern pat(INIT_SIZE);

    SECTION("newsize < oldsize, data gets truncated") {
        pat.setSize(SHRINK_SIZE);
        REQUIRE(pat.size() == SHRINK_SIZE);
        //REQUIRE(pat.copy() != copy);
    }

    SECTION("newsize > oldsize, new empty rows get cleared") {
        pat.setSize(GROW_SIZE);
        REQUIRE(pat.size() == GROW_SIZE);
        //REQUIRE(pat.copy() != copy);
        for (auto iter = pat.begin() + ((GROW_SIZE - INIT_SIZE) * 4); iter != pat.end(); ++iter) {
            REQUIRE(rowIsZero(*iter));
        }
    }

    SECTION("newsize = oldsize, pattern remains the same") {
        pat.setSize(INIT_SIZE);
        REQUIRE(pat.size() == INIT_SIZE);
        // REQUIRE(pat.copy() == copy);
    }
}

//TEST_CASE("set methods", "[Pattern]") {
//
//    constexpr uint8_t ROWS = 10;
//    Pattern pat(ROWS);
//    auto copy = pat.copy();
//
//
//    SECTION("setEffect on all tracks")
//}


//TEST_CASE("track selection", "[pattern]") {
//    Pattern pat(1);
//
//    SECTION("track 0") {
//        pat.setEffect(ChType::ch1, 0, EffectType::arp, 50);
//    }
//
//    pat.clear(0, 0);
//
//
//
//}

}