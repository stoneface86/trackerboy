
#include "catch.hpp"
#include "trackerboy/File.hpp"

#include <algorithm>
#include <sstream>
#include <cstring>

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

        CHECK(sampleReadIn[0] != nullptr);
        CHECK(sampleReadIn[5] != nullptr);
        CHECK(sampleReadIn[2] != nullptr);

        CHECK(sample.name(0) == sampleReadIn.name(0));
        CHECK(sample.name(2) == sampleReadIn.name(2));
        CHECK(sample.name(5) == sampleReadIn.name(5));

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

        // sample song has 3 patterns, order is 0, 1, 1, 2 and loops at 1
        auto &order = sample.order();
        auto &indexVec = order.indexVec();
        indexVec.push_back(0);
        indexVec.push_back(1);
        indexVec.push_back(1);
        indexVec.push_back(2);
        order.setLoop(1);

        auto &patterns = sample.patterns();
        patterns.resize(3);

        sample.setTempo(165.0f);
        sample.setRowsPerBeat(8);
        sample.setSpeed();
        
        REQUIRE(file.serialize(out, sample) == FormatError::none);

        in.str(out.str());

        Song sampleReadIn;
        REQUIRE(file.deserialize(in, sampleReadIn) == FormatError::none);

        // check song settings
        CHECK(sampleReadIn.tempo() == sample.tempo());
        CHECK(sampleReadIn.rowsPerBeat() == sample.rowsPerBeat());
        CHECK(sampleReadIn.speed() == sample.speed());

        // check the order
        auto &orderReadIn = sampleReadIn.order();
        CHECK(orderReadIn.loops() == order.loops());
        CHECK(orderReadIn.loopIndex() == order.loopIndex());
        auto &indexVecReadIn = orderReadIn.indexVec();
        CHECK(std::equal(indexVecReadIn.begin(), indexVecReadIn.end(), order.indexVec().begin()));

        CHECK(sampleReadIn.patterns().size() == sample.patterns().size());


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
