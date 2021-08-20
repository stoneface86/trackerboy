
#include "internal/fileformat/payload/payload.hpp"
#include "internal/fileformat/payload/handlers/CommHandler.hpp"
#include "internal/fileformat/payload/handlers/InstHandler.hpp"
#include "internal/fileformat/payload/handlers/SongHandler.hpp"
#include "internal/fileformat/payload/handlers/WaveHandler.hpp"


namespace trackerboy {

bool serializePayload(Module const& mod, std::ostream &stream) noexcept {

    CommHandler comm;
    SongHandler song(mod.songs().size());
    InstHandler inst(mod.instrumentTable().size());
    WaveHandler wave(mod.waveformTable().size());
    return writePayload(mod, stream, comm, song, inst, wave);



}

}
