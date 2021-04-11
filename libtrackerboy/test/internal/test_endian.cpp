
#include "catch.hpp"

#include <cstring>

#ifndef TRACKERBOY_BIG_ENDIAN
namespace otherEndian {

// testing on little endian, recompile in this namespace as big endian

#define TRACKERBOY_BIG_ENDIAN
#include "internal/endian.cpp"
#undef TRACKERBOY_BIG_ENDIAN

#define correctEndianTest(val) otherEndian::trackerboy::correctEndian(val)

}

#else

#include "internal/endian.hpp"
// testing on big endian, test what was compiled
#define correctEndianTest(val) correctEndian(val)

#endif



static const uint32_t TEST_CASES[] = { 
    0x00000000, 
    0x12345678, 
    0xFF0000FF, 
    0x00000001 
};

static const uint32_t TEST_CASES_SWAPPED[] = {
    0x00000000,
    0x78563412,
    0xFF0000FF,
    0x01000000
};

static constexpr size_t N_TEST_CASES = sizeof(TEST_CASES) / sizeof(uint32_t);



TEST_CASE("correctEndian", "[fileformat]") {

    SECTION("check involution") {
        // test that correctEndian is an involutory function (ie byte-swapping a 
        // byte-swapped value results in the original value)

        for (auto const &test : TEST_CASES) {
            CHECK(correctEndianTest(correctEndianTest(test)) == test);
            // same thing but with a 16-bit word
            uint16_t i16 = static_cast<uint16_t>(test);
            CHECK(correctEndianTest(correctEndianTest(i16)) == i16);
            // same thing but treat the test case as a float
            float f;
            memcpy(&f, &test, sizeof(test));
            CHECK(correctEndianTest(correctEndianTest(f)) == f);
        }
    }

    SECTION("check that result is byte swapped") {
        for (size_t i = 0; i != N_TEST_CASES; ++i) {
            
            CHECK(correctEndianTest(TEST_CASES[i]) == TEST_CASES_SWAPPED[i]);
        }
        
    }
}
