
#pragma once

#include "internal/fileformat/payload/PayloadHandler.hpp"
#include "trackerboy/data/Module.hpp"

namespace trackerboy {

//
// Handler class for the COMM block. Same format used in all major revisions.
//
class CommHandler : public PayloadHandler<BLOCK_ID_COMMENT> {

public:
    CommHandler();

    FormatError processIn(Module &mod, InputBlock &block, size_t index);

    void processOut(Module const& mod, OutputBlock &block, size_t index);

};

}

