
#pragma once

#include "internal/fileformat/Block.hpp"

namespace trackerboy {

template <BlockId tId>
class PayloadHandler {

public:
    constexpr PayloadHandler(size_t count = 1) :
        mCount(count)
    {
    }

    constexpr size_t count() const noexcept {
        return mCount;
    }

    constexpr BlockId id() const noexcept {
        return tId;
    }

    // subclasses should implement these methods for reading/writing

    // required by readPayload
    //FormatError processIn(Module &mod, InputBlock &block, size_t index);

    // required by writePayload
    //void processOut(Module const& mod, OutputBlock &block, size_t index);

private:
    size_t const mCount;

};

}
