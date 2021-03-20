
#include "internal/tlv.hpp"
#include "internal/endian.hpp"

namespace trackerboy::tlvparser {

IOError::IOError(std::ios &stream) :
    mStream(stream)
{
}

std::ios& IOError::stream() {
    return mStream;
}

const char* IOError::what() const {
    return "IO error occurred during tlv parsing";
}

LengthMismatchError::LengthMismatchError(uint32_t expected, uint32_t got) :
    mExpected(expected),
    mGot(got)
{
}

const char* LengthMismatchError::what() const {
    return "length mismatch when reading value";
}



void readTag(std::istream &stream, uint8_t &tag, uint32_t &length) {

    // read the tag
    stream.read(reinterpret_cast<char*>(&tag), 1);
    if (!stream.good()) {
        throw IOError(stream);
    }

    // read the length
    uint32_t lengthRead;
    stream.read(reinterpret_cast<char*>(&lengthRead), sizeof(lengthRead));
    if (!stream.good()) {
        throw IOError(stream);
    }
    // set the length parameter
    length = correctEndian(lengthRead);
}

void readValue(std::istream &stream, uint32_t const length, char *value) {
    stream.read(value, length);
    if (!stream.good()) {
        throw IOError(stream);
    }
}

static void writeTagAndLength(std::ostream &stream, uint8_t const tag, uint32_t const length) {
    // write the tag
    stream.write(reinterpret_cast<char const*>(&tag), 1);
    if (!stream.good()) {
        throw IOError(stream);
    }

    // write the length (in little endian)
    uint32_t const lengthOut = correctEndian(length);
    stream.write(reinterpret_cast<char const*>(&lengthOut), sizeof(lengthOut));
    if (!stream.good()) {
        throw IOError(stream);
    }
}

void write(std::ostream &stream, uint8_t const tag, uint32_t const length, const char *value) {
    writeTagAndLength(stream, tag, length);

    // write the value
    stream.write(value, length);
    if (!stream.good()) {
        throw IOError(stream);
    }

}

void write(std::ostream &stream, uint8_t const tag) {
    writeTagAndLength(stream, tag, 0);
}


}
