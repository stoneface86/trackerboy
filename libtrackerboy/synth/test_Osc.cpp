
#include "catch.hpp"

#include "./Osc.cpp"


TEST_CASE("CircularIterator testing", "[CircularIterator]") {

    int TEST_BUF[6] = { 1, 2, 3, 4, 5, 6 };
    CircularIterator<int> ci(TEST_BUF, 6);

    SECTION("random access with positive indices") {
        CHECK(ci[5] == TEST_BUF[5]);
        CHECK(ci[2] == TEST_BUF[2]);
        CHECK(ci[10] == TEST_BUF[10 % 6]);
    }

    SECTION("random access with negative indices") {
        CHECK(ci[-1] == TEST_BUF[6 - 1]);
        CHECK(ci[-2] == TEST_BUF[6 - 2]);
        CHECK(ci[-10] == TEST_BUF[6 - (10 % 6)]);
    }

    SECTION("increment operators") {
        REQUIRE(*(ci++) == buf[0]);
        REQUIRE(*(ci++) == buf[1]);
        REQUIRE(*(++ci) == buf[3]);
        REQUIRE(*(++ci) == buf[4]);
        REQUIRE(*(++ci) == buf[5]);
        // wraps back to begin
        REQUIRE(*(++ci) == buf[0]);
        ci.setOffset(5);
        REQUIRE(*(ci++) == buf[5]);
        REQUIRE(*ci == buf[0]);
    }

    SECTION("decrement operators") {
        REQUIRE(*(ci--) == buf[0]); // post-decrement, should return start of buffer
        REQUIRE(*(ci--) == buf[5]);
        REQUIRE(*(--ci) == buf[3]);
        REQUIRE(*(--ci) == buf[2]);
    }



}
