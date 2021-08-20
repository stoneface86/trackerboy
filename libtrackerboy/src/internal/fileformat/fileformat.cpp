
#include "internal/fileformat/fileformat.hpp"

#include <limits>


namespace trackerboy {

Signature const FILE_SIGNATURE = {
  '\0', 'T', 'R', 'A', 'C', 'K', 'E', 'R', 'B', 'O', 'Y', '\0'
};


bool upgradeHeader(Header &header) noexcept {

    // upgrade incrementally to the current major
    switch (header.current.revMajor) {
        case 0: {
            // 0 -> 1 changes in header:
            // system, customFramerate moved to offset 127 (after wcount)
            // revision -> revMajor
            // system -> revMinor
            // customFramerate -> reserved
            auto const system = header.rev0.system;
            auto const framerate = header.rev0.customFramerate;
            auto const instruments = header.rev0.numberOfInstruments;
            auto const waveforms = header.rev0.numberOfWaveforms;

            constexpr auto maxbyte = std::numeric_limits<uint8_t>::max();
            if (instruments > maxbyte || waveforms > maxbyte) {
                return false;
            }

            header.rev1.revMajor = 1;
            header.rev1.revMinor = 0;
            header.rev1.reserved = 0;
            header.rev1.icount = (uint8_t)instruments;
            header.rev1.scount = bias(1);
            header.rev1.wcount = (uint8_t)waveforms;
            header.rev1.system = system;
            header.rev1.customFramerate = framerate;
        }
            [[fallthrough]];
        default:
            return true;
    }
}


}
