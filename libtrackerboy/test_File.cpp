
#include "catch.hpp"
#include "trackerboy/File.hpp"

#include <algorithm>
#include <sstream>
#include <cstring>

#ifdef _MSC_VER
#pragma warning(disable : 6319 6237)
#endif

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

TEST_CASE("invalid table size", "[File]") {

    std::stringstream ss;
    InstrumentTable itable;
    File file;
    REQUIRE(file.saveTable(ss, itable) == FormatError::none);

    ss.seekp(0);
    uint16_t size = InstrumentTable::MAX_SIZE + 1;
    ss << correctEndian(size); // overwrite the table size with an illegal one
    ss.seekg(0);

    InstrumentTable itable2;
    REQUIRE(file.loadTable(ss, itable2) == FormatError::tableSizeBounds);


}

TEST_CASE("save/load equivalence", "[File]") {
    File file;
    std::istringstream in(std::ios::in | std::ios::binary);
    std::ostringstream out(std::ios::out | std::ios::binary);

    SECTION("header") {
        file.setArtist("foo");
        file.setCopyright("Copyright 2020 foo");
        file.setTitle("bar");
        file.setFileType(FileType::mod);

        REQUIRE(file.saveHeader(out) == FormatError::none);
        CHECK(out.tellp() == sizeof(Header));
        
        File f;
        in.str(out.str());

        REQUIRE(f.loadHeader(in) == FormatError::none);
        CHECK(in.tellg() == sizeof(Header));

        CHECK(f.artist() == file.artist());
        CHECK(f.title() == file.title());
        CHECK(f.copyright() == file.copyright());
        CHECK(f.fileType() == file.fileType());
        CHECK(f.revision() == file.revision());

        
    }

    SECTION("table") {
        InstrumentTable sample;
        sample.insert();
        sample.insert(5, "foo");
        sample.insert(2, "bar");

        REQUIRE(file.saveTable(out, sample) == FormatError::none);

        in.str(out.str());

        InstrumentTable sampleReadIn;
        REQUIRE(file.loadTable(in, sampleReadIn) == FormatError::none);

        Instrument *inst, *instIn;

        inst = sample[0];
        instIn = sampleReadIn[0];
        CHECK(instIn != nullptr);
        CHECK(inst->name() == instIn->name());

        inst = sample[5];
        instIn = sampleReadIn[5];
        CHECK(instIn != nullptr);
        CHECK(inst->name() == instIn->name());

        inst = sample[2];
        instIn = sampleReadIn[2];
        CHECK(instIn != nullptr);
        CHECK(inst->name() == instIn->name());

    }

    SECTION("Instrument") {
        Instrument sample;
        sample.data().envelope = 0xF1;
        sample.data().timbre = 0x02;
        
        // serialize the sample instrument
        REQUIRE(file.serialize(out, sample) == FormatError::none);
        
        in.str(out.str());
        
        Instrument sampleReadIn;
        REQUIRE(file.deserialize(in, sampleReadIn) == FormatError::none);

        // check that both instruments are equal
        REQUIRE(memcmp(&sample.data(), &sampleReadIn.data(), sizeof(Instrument::Data)) == 0);
    }

    SECTION("Song") {
        Song sample;

        // sample song has 4 tracks, but only 1 has note data
        auto &tr = sample.patterns().getTrack(ChType::ch1, 0);
        tr.setNote(10, 22);
        sample.patterns().getPattern(0, 0, 0, 0);

        sample.setTempo(165.0f);
        sample.setRowsPerBeat(8);
        sample.apply();

        auto &order = sample.orders();
        order[0].track2Id = 2;
        
        REQUIRE(file.serialize(out, sample) == FormatError::none);

        in.str(out.str());

        Song sampleReadIn;
        REQUIRE(file.deserialize(in, sampleReadIn) == FormatError::none);

        // check song settings
        CHECK(sampleReadIn.tempo() == sample.tempo());
        CHECK(sampleReadIn.rowsPerBeat() == sample.rowsPerBeat());
        CHECK(sampleReadIn.speed() == sample.speed());

        // check the order
        auto &orderReadIn = sampleReadIn.orders();
        CHECK(memcmp(orderReadIn.data(), order.data(), orderReadIn.size() * sizeof(Order)));

        //auto &trackReadIn = sampleReadIn.patterns().getTrack(ChType::ch1, 0);
        //CHECK(std::equal(trackReadIn.begin(), trackReadIn.end(), tr.begin()));


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
