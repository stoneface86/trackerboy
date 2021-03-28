#include "catch.hpp"
#include "trackerboy/data/Module.hpp"

#include <algorithm>
#include <sstream>
#include <cstring>

using namespace trackerboy;


TEST_CASE("Module::clear resets the module to its initialized state", "[Module]") {
    Module init;

    Module mod;
    mod.instrumentList().insert();
    mod.waveformList().insert();
    mod.setArtist("foobar");
    mod.setCopyright("2020");
    mod.setTitle("music");

    mod.clear();

    REQUIRE(mod.artist() == init.artist());
    REQUIRE(mod.title() == init.title());
    REQUIRE(mod.copyright() == init.copyright());
    REQUIRE(mod.instrumentList().size() == 0);
    REQUIRE(mod.waveformList().size() == 0);

}


TEST_CASE("Module::serialize writes a valid header", "[Module]") {
    Header sample = { 0 };
    std::copy(FILE_SIGNATURE, FILE_SIGNATURE + Header::SIGNATURE_SIZE, sample.signature);
    sample.versionMajor = VERSION.major;
    sample.versionMinor = VERSION.minor;
    sample.versionPatch = VERSION.patch;
    sample.revision = FILE_REVISION;


    std::string sampleData(sizeof(sample) + 10, '\0');
    memcpy(sampleData.data(), &sample, sizeof(sample));

    Module mod;
    std::ostringstream out(std::ios::out | std::ios::binary);

    REQUIRE(mod.serialize(out) == FormatError::none);
    REQUIRE(memcmp(out.str().c_str(), sampleData.c_str(), sizeof(sample)) == 0);

}

TEST_CASE("Module::deserialize returns error", "[Module]") {
    Module mod;
    std::istringstream in(std::ios::in | std::ios::binary);
    std::ostringstream out(std::ios::out | std::ios::binary);

    REQUIRE(mod.serialize(out) == FormatError::none);

    std::string dataStr = out.str();
    Header *header = reinterpret_cast<Header*>(dataStr.data());

    FormatError expected;

    SECTION("when the signature is invalid") {
        header->signature[0] = ~FILE_SIGNATURE[0];
        expected = FormatError::invalidSignature;
    }

    SECTION("when the file has a future revision") {
        header->revision++;
        expected = FormatError::invalidRevision;
    }

    in.str(dataStr);
    REQUIRE(mod.deserialize(in) == expected);
}

TEST_CASE("Module save/load equivalence", "[Module]") {
    Module mod;
    std::istringstream in(std::ios::in | std::ios::binary);
    std::ostringstream out(std::ios::out | std::ios::binary);

    mod.setArtist("foobar");
    mod.setCopyright("2020");
    mod.setTitle("stuff");


    REQUIRE(mod.serialize(out) == FormatError::none);

    in.str(out.str());

    Module modReadIn;
    REQUIRE(modReadIn.deserialize(in) == FormatError::none);

    REQUIRE(modReadIn.title() == mod.title());
    REQUIRE(modReadIn.artist() == mod.artist());
    REQUIRE(modReadIn.copyright() == mod.copyright());
    REQUIRE(modReadIn.version() == mod.version());
    REQUIRE(modReadIn.revision() == mod.revision());
    
}
