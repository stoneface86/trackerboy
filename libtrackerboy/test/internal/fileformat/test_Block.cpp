
#include "catch.hpp"

#include "internal/fileformat/Block.hpp"

#include <algorithm>
#include <sstream>

using namespace trackerboy;

constexpr BlockId TEST_ID = 0x54534554; // "TEST"
constexpr int TEST_DATA = 23;

//
// writes a block to the stream with ID = TEST and a payload of a single int,
// TEST_DATA.
//
void writeTestBlock(std::ostream &stream) {
    OutputBlock block(stream);
    block.begin(TEST_ID);
    block.write(TEST_DATA);
    block.finish();
}


TEST_CASE("Block save/load equivalence", "[Block]") {

    // this test checks if we can write out the data and get the same data when
    // reading it back in. This does not check if the underlying format is
    // correct.

    std::stringstream stream(std::ios_base::binary | std::ios_base::in | std::ios_base::out);

    int firstVal = 23;
    struct Custom {
        int a;
        float b;
        char c[10];
    };

    Custom secondVal;
    secondVal.b = 23.5f;
    secondVal.c[2] = '2';

    const char thirdVal[] = "just some string data...";

    {
        // write out a sample block
        OutputBlock block(stream);
        REQUIRE_NOTHROW(block.begin(TEST_ID));
        // primitive
        REQUIRE_NOTHROW(block.write(firstVal));
        // using a struct
        REQUIRE_NOTHROW(block.write(secondVal));
        // using the char array specialization
        REQUIRE_NOTHROW(block.write(sizeof(thirdVal), thirdVal));
        REQUIRE_NOTHROW(block.finish());
    }

    {
        // read the block in, checking that the data read in matches what was
        // written out
        InputBlock block(stream);
        REQUIRE(block.begin() == TEST_ID);

        int firstValIn;
        REQUIRE_NOTHROW(block.read(firstValIn));
        REQUIRE(firstValIn == firstVal);

        Custom secondValIn;
        REQUIRE_NOTHROW(block.read(secondValIn));
        REQUIRE(std::equal(reinterpret_cast<const char*>(&secondValIn),
                           reinterpret_cast<const char*>(&secondValIn) + sizeof(secondValIn),
                           reinterpret_cast<const char*>(&secondVal)
                ));
        
        char thirdValIn[sizeof(thirdVal)];
        REQUIRE_NOTHROW(block.read(sizeof(thirdVal), thirdValIn));
        REQUIRE(std::equal(thirdValIn, thirdValIn + sizeof(thirdValIn), thirdVal));


        REQUIRE(block.finished());
    }

}

TEST_CASE("InputBlock throws BoundsError when reading past the block", "[Block]") {
    std::stringstream stream(std::ios_base::binary | std::ios_base::in | std::ios_base::out);

    // create a block with just an integer in it
    REQUIRE_NOTHROW(writeTestBlock(stream));

    {
        InputBlock block(stream);
        REQUIRE(block.begin() == TEST_ID);
        int num;
        REQUIRE_NOTHROW(block.read(num));
        REQUIRE(block.finished());
        REQUIRE_THROWS_AS(block.read(num), BoundsError);
    }


}

TEST_CASE("OutputBlock format check", "[Block]") {

    std::stringstream stream(std::ios_base::binary | std::ios_base::out);

    REQUIRE_NOTHROW(writeTestBlock(stream));

    // get the data that was written to the stream
    auto str = stream.str();
    auto data = str.data();

    // check that the size of the data written by OutputBlock matches the
    // expected size of the format
    REQUIRE(str.size() == sizeof(BlockId) + sizeof(BlockSize) + sizeof(int));

    // now check the data format

    CHECK(*reinterpret_cast<const BlockId*>(data) == TEST_ID);
    data += sizeof(BlockId);

    CHECK(*reinterpret_cast<const BlockSize*>(data) == sizeof(int));
    data += sizeof(BlockSize);

    CHECK(*reinterpret_cast<const int*>(data) == TEST_DATA);

}

