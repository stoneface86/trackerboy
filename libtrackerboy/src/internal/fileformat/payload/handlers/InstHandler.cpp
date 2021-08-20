
#include "internal/fileformat/payload/handlers/InstHandler.hpp"

namespace trackerboy {

FormatError InstHandler::processIn(Module &mod, InputBlock &block, size_t index) {
    (void)mod;
    (void)block;
    (void)index;

    return FormatError::invalid;
}

void InstHandler::processOut(Module const& mod, OutputBlock &block, size_t index) {
    (void)mod;
    (void)block;
    (void)index;
}

}
