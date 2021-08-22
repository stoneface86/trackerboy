
#include "internal/fileformat/payload/payload.hpp"
#include "internal/endian.hpp"

namespace trackerboy {


std::string deserializeString(InputBlock &block) {
    uint16_t size;
    block.read(size);
    size = correctEndian(size);

    std::string str;
    if (size) {
        str.resize(size);
        block.read(size, str.data());
    }

    return str;
}

void serializeString(OutputBlock &block, std::string const& str) {
    uint16_t size = correctEndian((uint16_t)str.size());
    block.write(size);
    if (size) {
        block.write(size, str.data());
    }
}

void serializeItem(OutputBlock &block, DataItem const& item) {
    block.write(item.id());
    serializeString(block, item.name());
}

}
