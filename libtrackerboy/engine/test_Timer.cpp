
#include "catch.hpp"

#include "trackerboy/engine/Timer.hpp"


namespace trackerboy {

namespace {

struct TimerExpect {
    bool active;
    bool overflow;
    
    TimerExpect(bool active, bool overflow) :
        active(active),
        overflow(overflow)
    {
    }
};

}


TEST_CASE("timer is active on default state", "[Timer]") {

    Timer timer;

    SECTION("after construction") {
        REQUIRE(timer.active());
    }

    SECTION("after reset") {
        timer.reset();
        REQUIRE(timer.active());
        timer.setPeriod(0x10); // 2 frames
        timer.step();
        timer.reset();
        REQUIRE(timer.active());

    }


}

TEST_CASE("timer behavior", "[Timer]") {
    Timer timer;
    uint8_t period = 0;
    std::vector<TimerExpect> expected;

    SECTION("when period = 2.5") {
        
        // Frame:    0 1 2   3 4
        // State:  | A - O | A O |

        period = 0x14; // 2.5
        // 3, 2, ...
        expected.emplace_back(true, false);
        expected.emplace_back(false, false);
        expected.emplace_back(false, true);
        expected.emplace_back(true, false);
        expected.emplace_back(false, true);


    }

    SECTION("when period = 1.0") {
        period = 0x08;
        // 1, ...
        expected.emplace_back(true, true);
    }

    SECTION("when period = 1.25") {
        period = 0x0A;
        // 2, 1, 1, 1, ...
        
        expected.emplace_back(true, false);
        expected.emplace_back(false, true);
        expected.emplace_back(true, true);
        expected.emplace_back(true, true);
        expected.emplace_back(true, true);


    }

    timer.setPeriod(period);

    // multiple periods

    for (size_t i = 0; i != 3; ++i) {
        for (auto &expect : expected) {
            bool active = timer.active();
            bool overflow = timer.step();
            REQUIRE(overflow == expect.overflow);
            REQUIRE(active == expect.active);
        }
    }
}


}
