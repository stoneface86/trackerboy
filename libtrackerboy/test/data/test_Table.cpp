
#include "catch.hpp"
#include "trackerboy/data/Table.hpp"

using namespace trackerboy;
using Catch::Contains;

// https://github.com/catchorg/Catch2/issues/1134
static constexpr auto MAX_SIZE = BaseTable::MAX_SIZE;

TEMPLATE_TEST_CASE("table is empty", "[Table]", InstrumentTable, WaveformTable) {

    TestType table;

    REQUIRE(table.size() == 0);

    for (size_t i = 0; i != TestType::MAX_SIZE; ++i) {
        uint8_t id = static_cast<uint8_t>(i);
        CHECK(table[id] == nullptr);
        CHECK_THROWS_AS(table.remove(id), std::runtime_error);
    }

    //CHECK(table.begin() == table.end());

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
        CHECK_THROWS_AS(table.insert(2), std::runtime_error);
        CHECK_THROWS_AS(table.insert(56), std::runtime_error);
        CHECK_THROWS_AS(table.insert(255), std::runtime_error);

    }
}

TEMPLATE_TEST_CASE("table duplicates item", "[Table]", Instrument, Waveform) {
    Table<TestType> table;
    table.insert();

    REQUIRE(table[0] != nullptr);

    auto nextId = table.nextAvailableId();

    REQUIRE_NOTHROW(table.duplicate(0));

    //auto item = table[nextId];
    //REQUIRE(item != nullptr);
    //REQUIRE(nextId == item->id());
    //REQUIRE(item == table[nextId]);

}

TEMPLATE_TEST_CASE("table keeps track of the next available index", "[Table]", InstrumentTable, WaveformTable) {

    TestType table;

    REQUIRE(table.nextAvailableId() == 0);
    REQUIRE_NOTHROW(table.insert());
    REQUIRE(table.nextAvailableId() == 1);
    REQUIRE_NOTHROW(table.insert());
    REQUIRE(table.nextAvailableId() == 2);
    REQUIRE_NOTHROW(table.insert());
    REQUIRE(table.nextAvailableId() == 3);
    REQUIRE_NOTHROW(table.insert());
    REQUIRE(table.nextAvailableId() == 4);
    
    REQUIRE_NOTHROW(table.remove(0));
    REQUIRE(table.nextAvailableId() == 0); // next available is 0 since 0 < 4
    REQUIRE_NOTHROW(table.remove(1));
    REQUIRE(table.nextAvailableId() == 0); // still 0, since 0 < 1

    REQUIRE_NOTHROW(table.insert());
    REQUIRE(table.nextAvailableId() == 1);
    REQUIRE_NOTHROW(table.insert());
    REQUIRE(table.nextAvailableId() == 4);
}


