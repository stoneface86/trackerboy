#include "catch.hpp"
#include "trackerboy/data/Instrument.hpp"

#include <sstream>

using namespace trackerboy;


TEST_CASE("serialization", "[Instrument]") {
    Instrument sample;
    std::istringstream in(std::ios::in | std::ios::binary);
    std::ostringstream out(std::ios::out | std::ios::binary);

    sample.data().envelope = 0xF1;
    sample.data().timbre = 0x02;

    // serialize the sample instrument
    REQUIRE(sample.serialize(out) == FormatError::none);

    // read it back in
    in.str(out.str());

    Instrument sampleReadIn;
    REQUIRE(sampleReadIn.deserialize(in) == FormatError::none);

    // check that both instruments are equal
    REQUIRE(memcmp(&sample.data(), &sampleReadIn.data(), sizeof(Instrument::Data)) == 0);
}

static const uint8_t SAMPLE_DATA[] = {
    5, // ID = 5
    'C', 'H', '1', ' ', 'L', 'e', 'a', 'd', 0, // name = "CH1 Lead"
    // payload (Instrument::Data)
    0x03, // timbre = 0x03 (Duty 75%)
    0xF2, // envelope = 0xF2
    0x11, // pan middle
    0x00, // no delay
    0x00, // infinite duration
    0x81, // offset pitch by 1
    0x44, // vibrato, speed = 4, extent = 4
    0x12, // vibrato delay = 18 frames
};



TEST_CASE("serialization matches sample data", "[Instrument]") {
    Instrument sample;
    std::istringstream in(std::ios::in | std::ios::binary);
    std::ostringstream out(std::ios::out | std::ios::binary);

    sample.setId(5);
    sample.setName("CH1 Lead");

    auto &data = sample.data();
    data.timbre = 0x03;
    data.envelope = 0xF2;
    data.panning = 0x11;
    data.delay = 0;
    data.duration = 0;
    data.tune = 0x81;
    data.vibrato = 0x44;
    data.vibratoDelay = 0x12;

    REQUIRE(sample.serialize(out) == FormatError::none);
    REQUIRE(memcmp(SAMPLE_DATA, out.str().c_str(), sizeof(SAMPLE_DATA)) == 0);
}
