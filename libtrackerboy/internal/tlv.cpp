
#include "internal/tlv.hpp"
#include "internal/endian.hpp"

namespace trackerboy::tlvparser {


bool readTag(std::istream &stream, uint8_t &tag, uint32_t &length) {

    // read the tag
    stream.read(reinterpret_cast<char*>(&tag), 1);
    if (!stream.good()) {
        return false;
    }

    // read the length
    uint32_t lengthRead;
    stream.read(reinterpret_cast<char*>(&lengthRead), sizeof(lengthRead));
    if (!stream.good()) {
        return false;
    }
    // set the length parameter
    length = correctEndian(lengthRead);
    return true;
}

bool readValue(std::istream &stream, uint32_t const length, char *value) {
    stream.read(value, length);
    return stream.good();
}

static bool writeTagAndLength(std::ostream &stream, uint8_t const tag, uint32_t const length) {
    // write the tag
    stream.write(reinterpret_cast<char const*>(&tag), 1);
    if (!stream.good()) {
        return false;
    }

    // write the length (in little endian)
    uint32_t const lengthOut = correctEndian(length);
    stream.write(reinterpret_cast<char const*>(&lengthOut), sizeof(lengthOut));
    return stream.good();
}

bool write(std::ostream &stream, uint8_t const tag, uint32_t const length, const char *value) {
    if (!writeTagAndLength(stream, tag, length)) {
        return false;
    }

    // write the value
    stream.write(value, length);
    return stream.good();

}

bool write(std::ostream &stream, uint8_t const tag) {
    return writeTagAndLength(stream, tag, 0);
}


}
