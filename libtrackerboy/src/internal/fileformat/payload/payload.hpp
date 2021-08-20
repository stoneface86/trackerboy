
#pragma once

#include "trackerboy/trackerboy.hpp"
#include "trackerboy/data/Module.hpp"
#include "internal/fileformat/Block.hpp"

namespace trackerboy {

//
// Deserializes the module payload using major version 0 format.
//
FormatError deserializePayload0(Module &mod, Header &header, std::istream &stream) noexcept;

//
// Deserializes the module payload using major version 1 format.
//
FormatError deserializePayload1(Module &mod, Header &header, std::istream &stream) noexcept;


// note that there is only one serializePayload version, as we always
// serialize using the current format.

//
// Serializes the payload of the given module.
//
bool serializePayload(Module const& mod, std::ostream &stream) noexcept;


template <class T>
FormatError readPayloadImpl(Module &mod, InputBlock &block, T& handler) {
    auto const count = handler.count();

    // TODO: optimize this for single block handlers (ie COMM block)
    // if we only need to read 1 block, we don't need the for loop
    for (size_t i = 0; i < count; ++i) {
        if (block.begin() != handler.id()) {
            return FormatError::invalid;
        }

        auto error = handler.processIn(mod, block, i);
        if (error != FormatError::none) {
            return error;
        }

        if (!block.finished()) {
            return FormatError::invalid;
        }
    }

    return FormatError::none;
}


template <class T, class... Ts>
FormatError readPayloadImpl(Module &mod, InputBlock &block, T& handler, Ts&... handlers) {
    auto error = readPayloadImpl(mod, block, handler);
    if (error != FormatError::none) {
        return error;
    } else {
        return readPayloadImpl(mod, block, handlers...);
    }
}

template <class... Ts>
FormatError readPayload(Module &mod, std::istream &stream, Ts&... handlers) {
    auto const exceptions = stream.exceptions();
    stream.exceptions(std::ios_base::failbit | std::ios_base::badbit | std::ios_base::eofbit);

    FormatError error;
    InputBlock block(stream);
    try {
        error = readPayloadImpl(mod, block, handlers...);
    } catch (std::ios_base::failure const&) {
        error = FormatError::readError;
    } catch (BoundsError const&) {
        error = FormatError::invalid;
    }

    stream.exceptions(exceptions);
    return error;
}


template <class T>
void writePayloadImpl(Module const& mod, OutputBlock &block, T& handler) {
    auto const count = handler.count();
    for (size_t i = 0; i < count; ++i) {
        block.begin(handler.id());
        handler.processOut(mod, block, i);
        block.finish();
    }
}

template <class T, class... Ts>
void writePayloadImpl(Module const& mod, OutputBlock &block, T& handler, Ts&... handlers) {
    writePayloadImpl(mod, block, handler);
    writePayloadImpl(mod, block, handlers...);
}

template <class... Ts>
bool writePayload(Module const& mod, std::ostream &stream, Ts&... handlers) {
    auto const exceptions = stream.exceptions();
    stream.exceptions(std::ios_base::failbit | std::ios_base::badbit | std::ios_base::eofbit);
    
    bool success = true;
    OutputBlock block(stream);
    try {
        writePayloadImpl(mod, block, handlers...);
    } catch (std::ios_base::failure const&) {
        success = false;
    }

    stream.exceptions(exceptions); // restore the stream's exception mask
    return success;
}



}

