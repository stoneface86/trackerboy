
#include "trackerboy/fileformat.hpp"
#include "catch.hpp"

#include <cstring>

using namespace trackerboy;

namespace otherEndian {

// recompile correctEndian functions with the "other" endian (if we are a big
// endian system then recompile for little endian and vice versa).

#ifdef TRACKERBOY_BIG_ENDIAN
#define TEMP TRACKERBOY_BIG_ENDIAN
#undef TRACKERBOY_BIG_ENDIAN
#else
#define TRACKERBOY_BIG_ENDIAN
#endif

#include "fileformat.cpp"

#ifdef TEMP
#define TRACKERBOY_BIG_ENDIAN TEMP
#undef TEMP
#else
#undef TRACKERBOY_BIG_ENDIAN
#endif

}

// macro for correctEndian of the other endian
#define correctEndian_other otherEndian::trackerboy::correctEndian

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
            CHECK(correctEndian(correctEndian(test)) == test);
            CHECK(correctEndian_other(correctEndian_other(test)) == test);
            // same thing but with a 16-bit word
            uint16_t i16 = static_cast<uint16_t>(test);
            CHECK(correctEndian(correctEndian(i16)) == i16);
            CHECK(correctEndian_other(correctEndian_other(i16)) == i16);
            // same thing but treat the test case as a float
            float f;
            memcpy(&f, &test, sizeof(test));
            CHECK(correctEndian(correctEndian(f)) == f);
            CHECK(correctEndian_other(correctEndian_other(f)) == f);
        }
    }

    SECTION("check that result is in little endian") {
        for (size_t i = 0; i != N_TEST_CASES; ++i) {
            uint32_t test = TEST_CASES[i];
            uint32_t expected;
            uint32_t expectedOther;
            #ifdef TRACKERBOY_BIG_ENDIAN
            // big endian system, result should be byte swapped
            expected = TEST_CASES_SWAPPED[i];
            expectedOther = test;
            #else
            // little endian system, result should be the same as the input
            expected = test;
            expectedOther = TEST_CASES_SWAPPED[i];
            #endif
                
            CHECK(correctEndian(test) == expected);
            CHECK(correctEndian_other(test) == expectedOther);
        }
        
    }
}
