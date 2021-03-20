
// Tag-length-value parser

#pragma once

#include <cstdint>
#include <istream>
#include <ostream>

namespace trackerboy {



//
// namespace contains utility code for a tlv (tag-length-value) parser.
// Tag-length-value format is a binary data format that is made up of a
// tag, a length, and a variable sequence of bytes representing the value. 
// The tag specifies the type of data value is and the length is the number
// of bytes the value occupies.
//
// TLV used by trackerboy has 1-byte tags and 4-byte lengths in little-endian
//
// For example I/4/234 is a TLV with tag 5, a length of 4, and a value of 234 (as an integer)
// the resulting byte stream for this data is
// 5, 4, 0, 0, 0, 234, 0, 0, 0
// totaling 9 bytes (4 bytes for value, 4 bytes for length and 1 byte for tag)
// little-endian was used for the value but is not required or enforced by this parser
//
// TLV is used by the library for the module format. Tags are essentially commands
// and values are command arguments. 
//
namespace tlvparser {


bool readTag(std::istream &stream, uint8_t &tag, uint32_t &length);
bool readValue(std::istream &stream, uint32_t const length, char *value);

template <typename T>
inline bool readValue(std::istream &stream, uint32_t const length, T &value) {
    if (length != sizeof(T)) {
        return false; // length mismatch!
    }
    return readValue(stream, sizeof(T), reinterpret_cast<char*>(&value));
}

// generic write function
bool write(std::ostream &stream, uint8_t const tag, uint32_t const length, const char *value);

// write void value (or no data, just the tag and length = 0)
bool write(std::ostream &stream, uint8_t const tag);


template <typename T>
inline bool write(std::ostream &stream, uint8_t const tag, T const& value) {
    return write(stream, tag, sizeof(T), reinterpret_cast<const char*>(&value));
}

}


}
