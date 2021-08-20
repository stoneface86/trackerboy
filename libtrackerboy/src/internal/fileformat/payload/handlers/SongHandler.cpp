
#include "internal/fileformat/payload/handlers/SongHandler.hpp"

namespace trackerboy {

FormatError SongHandler::processIn(Module &mod, InputBlock &block, size_t index) {
    (void)mod;
    (void)block;
    (void)index;

    return FormatError::invalid;
}

void SongHandler::processOut(Module const& mod, OutputBlock &block, size_t index) {
    (void)mod;
    (void)block;
    (void)index;
}

}
