
#pragma once

#include "internal/fileformat/payload/PayloadHandler.hpp"
#include "trackerboy/data/Module.hpp"

namespace trackerboy {

class SongHandler : public PayloadHandler<BLOCK_ID_SONG> {

public:
    using PayloadHandler::PayloadHandler;

    FormatError processIn(Module &mod, InputBlock &block, size_t index);

    void processOut(Module const& mod, OutputBlock &block, size_t index);

};

}

