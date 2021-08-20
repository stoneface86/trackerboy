
#pragma once

#include "internal/fileformat/payload/PayloadHandler.hpp"
#include "trackerboy/data/Module.hpp"

namespace trackerboy {

class InstHandler : public PayloadHandler<BLOCK_ID_INSTRUMENT> {

public:
    using PayloadHandler::PayloadHandler;

    FormatError processIn(Module &mod, InputBlock &block, size_t index);

    void processOut(Module const& mod, OutputBlock &block, size_t index);


};

}

