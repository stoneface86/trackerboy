
#pragma once

#include "internal/fileformat/payload/handlers/TableHandler.hpp"
#include "trackerboy/data/Module.hpp"

namespace trackerboy {

class InstHandler : public TableHandler<Instrument, BLOCK_ID_INSTRUMENT> {

public:
    using TableHandler::TableHandler;

    FormatError processIn(Module &mod, InputBlock &block, size_t index);

    void processOut(Module const& mod, OutputBlock &block, size_t index);

    //
    // same code as major 0
    //
    static FormatError deserializeInstrument(InputBlock &block, Instrument &inst);


};

}

