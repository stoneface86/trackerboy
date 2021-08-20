
//
// Utility code for reading/writing blocks in the module's payload
//

#pragma once

#include <istream>
#include <ostream>
#include <stdexcept>

#include "internal/fileformat/fileformat.hpp"

namespace trackerboy {

class IOError : public std::exception {

};

class BoundsError : public std::exception {

};


//
// Class for reading data from a "block"
// attempting to read past the block results in a BoundsError being thrown
//
class InputBlock {

public:


    InputBlock(std::istream &stream);

    size_t size() const;

    BlockId begin();

    template <typename T>
    void read(T& ref) {
        read(sizeof(T), reinterpret_cast<char*>(&ref));
    }

    template <typename T>
    void read(size_t count, T *data) {
        read(count * sizeof(T), reinterpret_cast<char*>(data));
    }

    //
    // Returns true if the entire block has been read
    //
    bool finished();


private:
    std::istream &mStream;
    size_t mSize;
    size_t mPosition;

};

//
// Class for writing data and encapsulating it in a "block"
// A block contains a 4-byte id and a 4-byte size.
//
class OutputBlock {

public:

    OutputBlock(std::ostream &stream);

    void begin(BlockId id);

    //
    // Finish writing the block. The block's length field is updated with
    // this block's length.
    //
    void finish();

    template <typename T>
    void write(T const& ref) {
        write(sizeof(T), reinterpret_cast<const char*>(&ref));
    }

    template <typename T>
    void write(size_t count, T const *data) {
        write(sizeof(T) * count, reinterpret_cast<const char*>(data));
    }

private:
    std::ostream &mStream;
    std::streampos mLengthPos;
    size_t mSize;

};





}
