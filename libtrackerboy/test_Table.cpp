
#include "catch.hpp"
#include "trackerboy/data/Table.hpp"

using namespace trackerboy;
using Catch::Contains;

// https://github.com/catchorg/Catch2/issues/1134
static constexpr auto MAX_SIZE = BaseTable::MAX_SIZE;

TEMPLATE_TEST_CASE("table is empty", "[Table]", InstrumentTable, WaveTable) {

    TestType table;

    REQUIRE(table.size() == 0);

    for (size_t i = 0; i != TestType::MAX_SIZE; ++i) {
        uint8_t id = static_cast<uint8_t>(i);
        CHECK(table[id] == nullptr);
        CHECK_THROWS_AS(table.remove(id), std::runtime_error);
    }

    CHECK(table.begin() == table.end());

}

TEMPLATE_TEST_CASE("table is full", "[Table]", Instrument, Waveform) {

    Table<TestType> table;
    // populate the table
    for (size_t i = 0; i != Table<TestType>::MAX_SIZE; ++i) {
        CHECK_NOTHROW(table.insert());
    }

    CHECK(table.size() == MAX_SIZE);

    SECTION("inserting into a full table throws exception") {
        CHECK_THROWS_AS(table.insert(), std::runtime_error);
        CHECK_THROWS_AS(table.insert(2, "test"), std::runtime_error);
        CHECK_THROWS_AS(table.insert(56, "test"), std::runtime_error);
        CHECK_THROWS_AS(table.insert(255, "test"), std::runtime_error);

    }
}
