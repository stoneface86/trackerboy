
#include "internal/fileformat/payload/handlers/WaveHandler.hpp"

namespace trackerboy {

FormatError WaveHandler::processIn(Module &mod, InputBlock &block, size_t index) {
    (void)index;

    auto wave = initItem(block, mod.waveformTable());
    if (wave == nullptr) {
        return FormatError::duplicateId;
    }
    return deserializeWaveform(block, *wave);
}

void WaveHandler::processOut(Module const& mod, OutputBlock &block, size_t index) {
    (void)index;

    auto wave = nextItem(mod.waveformTable());
    serializeItem(block, *wave);
    block.write(wave->data());
}

FormatError WaveHandler::deserializeWaveform(InputBlock &block, Waveform &wave) {
    block.read(wave.data());
    return FormatError::none;
}

}
