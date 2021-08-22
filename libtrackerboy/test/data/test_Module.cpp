#include "catch.hpp"
#include "trackerboy/data/Module.hpp"
#include "internal/fileformat/fileformat.hpp"

#include <algorithm>
#include <sstream>
#include <cstring>

using namespace trackerboy;

// these tests suck and should be rewritten

TEST_CASE("Module::clear resets the module to its initialized state", "[Module]") {
    Module init;

    Module mod;
    mod.instrumentTable().insert();
    mod.waveformTable().insert();
    mod.setArtist("foobar");
    mod.setCopyright("2020");
    mod.setTitle("music");

    mod.clear();

    REQUIRE(mod.artist() == init.artist());
    REQUIRE(mod.title() == init.title());
    REQUIRE(mod.copyright() == init.copyright());
    REQUIRE(mod.instrumentTable().size() == 0);
    REQUIRE(mod.waveformTable().size() == 0);

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
        header->current.signature[0] = ~FILE_SIGNATURE[0];
        expected = FormatError::invalidSignature;
    }

    SECTION("when the file has a future revision") {
        header->current.revMajor++;
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
    REQUIRE(modReadIn.revisionMajor() == mod.revisionMajor());
    REQUIRE(modReadIn.revisionMinor() == mod.revisionMinor());

    
}
