
#include "catch.hpp"
#include "trackerboy/File.hpp"
#include "version.hpp"

#include <strstream>

using namespace trackerboy;


TEST_CASE("header", "[File]") {
    std::unique_ptr<char[]> headerBuf(new char[sizeof(Header)]);

    File file;
    file.setArtist("foo");
    file.setCopyright("Copyright 2020 foo");
    file.setTitle("bar");
    
    std::ostrstream out(headerBuf.get(), sizeof(Header), std::ios::binary | std::ios::out);
    FormatError errorOnSave = file.saveHeader(out);
    std::istrstream in(headerBuf.get(), sizeof(Header));
    Header *header = reinterpret_cast<Header*>(headerBuf.get());
    File f;


    SECTION("load == save") {
        REQUIRE(errorOnSave == FormatError::none);
        REQUIRE(f.loadHeader(in) == FormatError::none);

        CHECK(f.artist() == file.artist());
        CHECK(f.title() == file.title());
        CHECK(f.copyright() == file.copyright());
        CHECK(f.fileType() == file.fileType());
        CHECK(f.revision() == file.revision());
    }

        

    SECTION("incorrect signature in header") {
        // make the signature incorrect
        header->signature[0] = ~FILE_SIGNATURE[0];
        REQUIRE(f.loadHeader(in) == FormatError::invalidSignature);
    }

    SECTION("future revision") {
        header->revision++;
        REQUIRE(f.loadHeader(in) == FormatError::invalidRevision);
    }

    SECTION("invalid type") {
        header->type = static_cast<uint8_t>(FileType::last) + 1;
        REQUIRE(f.loadHeader(in) == FormatError::invalidType);
    }

    

}