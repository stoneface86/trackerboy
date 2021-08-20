
#include "internal/fileformat/payload/handlers/WaveHandler.hpp"

namespace trackerboy {

FormatError WaveHandler::processIn(Module &mod, InputBlock &block, size_t index) {
    (void)mod;
    (void)block;
    (void)index;

    return FormatError::invalid;
}

void WaveHandler::processOut(Module const& mod, OutputBlock &block, size_t index) {
    (void)mod;
    (void)block;
    (void)index;
}

}
