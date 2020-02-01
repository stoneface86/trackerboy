
#include "catch.hpp"
#include "trackerboy/File.hpp"
#include "version.hpp"

#include <algorithm>
#include <sstream>

using namespace trackerboy;


TEST_CASE("header", "[File]") {

    File file;

    file.setArtist("foo");
    file.setCopyright("Copyright 2020 foo");
    file.setTitle("bar");

    std::ostringstream out(std::ios::out | std::ios::binary);
    REQUIRE(file.saveHeader(out) == FormatError::none);

    
    Header header;
    out.str().copy(reinterpret_cast<char*>(&header), sizeof(Header));
    
    FormatError expectedError;
    

    SECTION("incorrect signature in header") {    
        header.signature[0] = ~FILE_SIGNATURE[0];
        expectedError = FormatError::invalidSignature;
    }

    SECTION("future revision") {
        header.revision++;
        expectedError = FormatError::invalidRevision;
    }

    SECTION("invalid type") {
        header.type = static_cast<uint8_t>(FileType::last) + 1;
        expectedError = FormatError::invalidType;
    }

    // attempt to read and assert we get the expected error
    std::string headerAsString(reinterpret_cast<char*>(&header), sizeof(Header));
    std::istringstream in(headerAsString, std::ios::in | std::ios::binary);

    REQUIRE(file.loadHeader(in) == expectedError);


}

TEST_CASE("save/load equivalence", "[File]") {
    File file;
    std::istringstream in(std::ios::in | std::ios::binary);
    std::ostringstream out(std::ios::out | std::ios::binary);

    SECTION("header") {
        file.setArtist("foo");
        file.setCopyright("Copyright 2020 foo");
        file.setTitle("bar");

        REQUIRE(file.saveHeader(out) == FormatError::none);
        
        File f;
        in.str(out.str());

        REQUIRE(f.loadHeader(in) == FormatError::none);

        CHECK(f.artist() == file.artist());
        CHECK(f.title() == file.title());
        CHECK(f.copyright() == file.copyright());
        CHECK(f.fileType() == file.fileType());
        CHECK(f.revision() == file.revision());
    }

    SECTION("Instrument") {
        Instrument sample;
        auto &program = sample.getProgram();
        program.push_back({ 1, 33, 22, 44, 50 });
        program.push_back({ 0 });

        // serialize the sample instrument
        REQUIRE(file.serialize(out, sample) == FormatError::none);
        
        in.str(out.str());
        
        Instrument sampleReadIn;
        REQUIRE(file.deserialize(in, sampleReadIn) == FormatError::none);

        // check that both instruments are equal
        auto &samplePrgm = sample.getProgram();
        auto &sampleReadInPrgm = sampleReadIn.getProgram();
        REQUIRE(samplePrgm.size() == sampleReadInPrgm.size());
        REQUIRE(memcmp(samplePrgm.data(), sampleReadInPrgm.data(), samplePrgm.size() * sizeof(Instruction)) == 0);
        
    }

    SECTION("Song") {
        Song sample;
    }

    SECTION("Waveform") {
        // sample data, triangle wave
        Waveform sample;
        uint8_t* data = sample.data();
        const uint8_t TRIANGLE[] = { 0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF, 0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10 };
        std::copy_n(TRIANGLE, Gbs::WAVE_RAMSIZE, data);

        // serialize the sample wave
        REQUIRE(file.serialize(out, sample) == FormatError::none);

        in.str(out.str());

        Waveform sampleReadIn;
        REQUIRE(file.deserialize(in, sampleReadIn) == FormatError::none);

        CHECK(std::equal(sample.data(), sample.data() + Gbs::WAVE_RAMSIZE, sampleReadIn.data()));
    }


}
