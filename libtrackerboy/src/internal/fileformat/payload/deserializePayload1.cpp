
#include "internal/fileformat/payload/payload.hpp"
#include "internal/fileformat/payload/handlers/CommHandler.hpp"
#include "internal/fileformat/payload/handlers/InstHandler.hpp"
#include "internal/fileformat/payload/handlers/SongHandler.hpp"
#include "internal/fileformat/payload/handlers/WaveHandler.hpp"

#include <array>

namespace trackerboy {

FormatError deserializePayload1(Module &mod, Header &header, std::istream &stream) noexcept {

    CommHandler comm;
    SongHandler song(unbias<size_t>(header.current.scount));
    InstHandler inst(header.current.icount);
    WaveHandler wave(header.current.wcount);
    return readPayload(mod, stream, comm, song, inst, wave);

}

}

#undef TU
