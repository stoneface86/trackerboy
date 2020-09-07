
#include "catch.hpp"
#include "trackerboy/data/DataItem.hpp"

#include <sstream>

using namespace trackerboy;


class TestItem : public DataItem {

public:

    static constexpr int DATA_INIT = 0;
    static constexpr int DATA_TO_SERIALIZE = 4;

    TestItem() : 
        mNum(DATA_INIT),
        DataItem() 
    {
    }

    ~TestItem() {
    
    }

    int num() {
        return mNum; 
    }

protected:

    virtual FormatError serializeData(std::ostream &stream) noexcept override {
        int num = DATA_TO_SERIALIZE;
        stream.write(reinterpret_cast<const char *>(&num), sizeof(num));
        return FormatError::none;
    }

    virtual FormatError deserializeData(std::istream &stream) noexcept override {
        stream.read(reinterpret_cast<char *>(&mNum), sizeof(mNum));
        return FormatError::none;
    }

private:
    int mNum;
};


TEST_CASE("save/load equivalence", "[DataItem]") {

    TestItem item;
    std::istringstream in(std::ios::in | std::ios::binary);
    std::ostringstream out(std::ios::out | std::ios::binary);

    item.setId(3);
    item.setName("test name");

    REQUIRE(item.serialize(out) == FormatError::none);

    in.str(out.str());

    TestItem itemReadIn;
    REQUIRE(itemReadIn.deserialize(in) == FormatError::none);

    REQUIRE(item.id() == itemReadIn.id());
    REQUIRE(item.name() == itemReadIn.name());
    REQUIRE(item.num() == TestItem::DATA_INIT);
    REQUIRE(itemReadIn.num() == TestItem::DATA_TO_SERIALIZE);


}

static uint8_t SAMPLE_DATA[] = {
    0x2, // ID = 2
    'T', 'E', 'S', 'T', 0, // name = "TEST"
    TestItem::DATA_TO_SERIALIZE, 0, 0, 0 // mNum = 4
};


TEST_CASE("serialization format matches sample", "[DataItem]") {
    TestItem item;
    std::ostringstream out(std::ios::out | std::ios::binary);

    item.setId(2);
    item.setName("TEST");
    REQUIRE(item.serialize(out) == FormatError::none);

    REQUIRE(memcmp(SAMPLE_DATA, out.str().c_str(), sizeof(SAMPLE_DATA)) == 0);
}

