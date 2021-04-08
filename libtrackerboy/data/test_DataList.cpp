
#include "catch.hpp"
#include "trackerboy/data/DataList.hpp"

using namespace trackerboy;
using Catch::Contains;

// https://github.com/catchorg/Catch2/issues/1134
static constexpr auto MAX_SIZE = DataListBase::MAX_SIZE;

TEMPLATE_TEST_CASE("list is empty", "[DataList]", InstrumentList, WaveformList) {

    TestType list;

    REQUIRE(list.size() == 0);

    for (size_t i = 0; i != TestType::MAX_SIZE; ++i) {
        uint8_t id = static_cast<uint8_t>(i);
        CHECK(list[id] == nullptr);
        CHECK_THROWS_AS(list.remove(id), std::runtime_error);
    }

    //CHECK(table.begin() == table.end());

}

TEMPLATE_TEST_CASE("list is full", "[DataList]", Instrument, Waveform) {

    DataList<TestType> list;
    // populate the table
    for (size_t i = 0; i != DataList<TestType>::MAX_SIZE; ++i) {
        CHECK_NOTHROW(list.insert());
    }

    CHECK(list.size() == MAX_SIZE);

    SECTION("inserting into a full table throws exception") {
        CHECK_THROWS_AS(list.insert(), std::runtime_error);
        CHECK_THROWS_AS(list.insert(2), std::runtime_error);
        CHECK_THROWS_AS(list.insert(56), std::runtime_error);
        CHECK_THROWS_AS(list.insert(255), std::runtime_error);

    }
}

TEMPLATE_TEST_CASE("list duplicates item", "[DataItem]", Instrument, Waveform) {
    DataList<TestType> list;
    list.insert();

    REQUIRE(list[0] != nullptr);

    auto nextId = list.nextAvailableId();

    std::shared_ptr<TestType> item;
    REQUIRE_NOTHROW(item = list.duplicate(0));
    REQUIRE(item != nullptr);
    REQUIRE(nextId == item->id());
    REQUIRE(item == list[nextId]);

}

TEMPLATE_TEST_CASE("list keeps track of the next available index", "[DataList]", InstrumentList, WaveformList) {

    TestType list;

    REQUIRE(list.nextAvailableId() == 0);
    REQUIRE_NOTHROW(list.insert());
    REQUIRE(list.nextAvailableId() == 1);
    REQUIRE_NOTHROW(list.insert());
    REQUIRE(list.nextAvailableId() == 2);
    REQUIRE_NOTHROW(list.insert());
    REQUIRE(list.nextAvailableId() == 3);
    REQUIRE_NOTHROW(list.insert());
    REQUIRE(list.nextAvailableId() == 4);
    
    REQUIRE_NOTHROW(list.remove(0));
    REQUIRE(list.nextAvailableId() == 0); // next available is 0 since 0 < 4
    REQUIRE_NOTHROW(list.remove(1));
    REQUIRE(list.nextAvailableId() == 0); // still 0, since 0 < 1

    REQUIRE_NOTHROW(list.insert());
    REQUIRE(list.nextAvailableId() == 1);
    REQUIRE_NOTHROW(list.insert());
    REQUIRE(list.nextAvailableId() == 4);
}


