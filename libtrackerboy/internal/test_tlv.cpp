
#include "internal/tlv.hpp"

#include "catch.hpp"

#include <algorithm>
#include <array>
#include <cstring>
#include <sstream>

using namespace trackerboy;


TEST_CASE("read/write equivalence", "[tlv]") {

    // this test writes some data, which we then read back
    // test passes if what we read is the same as what we wrote

    // note that this test does not verify if the data format is correct
    // only that we can read what we write

    std::istringstream in(std::ios::in | std::ios::binary);
    std::ostringstream out(std::ios::out | std::ios::binary);


    // write out some sample tlv data
    REQUIRE_NOTHROW(tlvparser::write(out, 'I', 4));
    const char str[] = "I'm a string or something";
    REQUIRE_NOTHROW(tlvparser::write(out, 'S', sizeof(str), str));


    // read it back
    in.str(out.str());

    uint8_t tag;
    uint32_t length;
    int value1;
    REQUIRE_NOTHROW(tlvparser::readTag(in, tag, length));
    REQUIRE(tag == 'I');
    REQUIRE(length == sizeof(int));
    REQUIRE_NOTHROW(tlvparser::readValue(in, length, value1));
    REQUIRE(value1 == 4);

    REQUIRE_NOTHROW(tlvparser::readTag(in, tag, length));
    REQUIRE(tag == 'S');
    REQUIRE(length == sizeof(str));

    char strReadIn[sizeof(str)];
    REQUIRE_NOTHROW(tlvparser::readValue(in, length, strReadIn));
    REQUIRE(strcmp(str, strReadIn) == 0);



}

static std::array SAMPLE_DATA = {
    
    // I/4/0x11223344
    'I',
    (char)4,
    (char)0,
    (char)0,
    (char)0,
    (char)0x44,
    (char)0x33,
    (char)0x22,
    (char)0x11,
    // S/3/"Hi"
    'S',
    (char)3,
    (char)0,
    (char)0,
    (char)0,
    'H',
    'i',
    '\0',
    // A/0/
    'A',
    (char)0,
    (char)0,
    (char)0,
    (char)0
};

TEST_CASE("format is correct", "[tlv]") {
    std::ostringstream out(std::ios::out | std::ios::binary);

    REQUIRE_NOTHROW(tlvparser::write(out, 'I', 0x11223344));
    const char str[] = "Hi";
    REQUIRE_NOTHROW(tlvparser::write(out, 'S', sizeof(str), str));
    REQUIRE_NOTHROW(tlvparser::write(out, 'A'));

    auto outputStr = out.str();
    REQUIRE(outputStr.length() == SAMPLE_DATA.size());
    REQUIRE(std::equal(SAMPLE_DATA.begin(), SAMPLE_DATA.end(), outputStr.begin()));
}

TEST_CASE("can read and write 0-length", "[tlv]") {
    std::istringstream in(std::ios::in | std::ios::binary);
    std::ostringstream out(std::ios::out | std::ios::binary);

    REQUIRE_NOTHROW(tlvparser::write(out, 'T'));
    REQUIRE_NOTHROW(tlvparser::write(out, 'A'));
    REQUIRE_NOTHROW(tlvparser::write(out, 'G'));

    // read it back
    in.str(out.str());

    uint8_t tag;
    uint32_t length;
    REQUIRE_NOTHROW(tlvparser::readTag(in, tag, length));
    REQUIRE(tag == 'T');
    REQUIRE(length == 0);

    REQUIRE_NOTHROW(tlvparser::readTag(in, tag, length));
    REQUIRE(tag == 'A');
    REQUIRE(length == 0);

    REQUIRE_NOTHROW(tlvparser::readTag(in, tag, length));
    REQUIRE(tag == 'G');
    REQUIRE(length == 0);

}

