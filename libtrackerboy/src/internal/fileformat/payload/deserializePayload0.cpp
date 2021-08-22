
#include "internal/fileformat/payload/payload.hpp"

#include "internal/fileformat/payload/PayloadHandler.hpp"
#include "internal/fileformat/payload/handlers/CommHandler.hpp"
#include "internal/fileformat/payload/handlers/SongHandler.hpp"
#include "internal/fileformat/payload/handlers/InstHandler.hpp"
#include "internal/fileformat/payload/handlers/WaveHandler.hpp"
#include "internal/endian.hpp"
#include "internal/enumutils.hpp"

//
// Payload deserializer for major revision 0 modules (legacy modules)
// This version of the file format was in use for all versions before v0.5.0
// We'll support this format even though a friend and I are probably the only
// ones with these legacy modules.
//

namespace trackerboy {


#define TU deserializePayload0TU
namespace TU {

// INDX block was removed so keep this constant in here
constexpr BlockId BLOCK_ID_INDEX       = 0x58444E49; // "INDX"




//
// legacy string deserializer, newer version defined in payload.hpp uses a
// 2-byte length prefix instead of a 1-byte one.
//
std::string deserializeString(InputBlock &block) {
    uint8_t size;
    block.read(size);

    std::string str;
    str.resize(size);
    block.read(size, str.data());

    return str;
}


template <class T>
void readListIndex(InputBlock &block, size_t count, Table<T> &list) {
    while (count--) {
        uint8_t id;
        block.read(id);
        auto &item = list.insert(id);
        item.setName(TU::deserializeString(block));
    }
}

class IndxHandler : public PayloadHandler<BLOCK_ID_INDEX> {

public:
    IndxHandler(int instruments, int waveforms) :
        PayloadHandler(1),
        mInstruments(instruments),
        mWaveforms(waveforms)
    {
    }

    FormatError processIn(Module &mod, InputBlock &block, size_t index) {
        (void)index;

        mod.songs().get(0)->setName(TU::deserializeString(block));

        readListIndex(block, mInstruments, mod.instrumentTable());
        readListIndex(block, mWaveforms, mod.waveformTable());
        return FormatError::none;
    }


private:
    int mInstruments;
    int mWaveforms;

};

// reimplement the handler classes here. note that we only implement processIn
// since we only need to be able to read from this version, not write.

class LegacyInstHandler : public PayloadHandler<BLOCK_ID_INSTRUMENT> {

public:
    LegacyInstHandler() {}

    FormatError processIn(Module &mod, InputBlock &block, size_t index) {
        (void)index;

        auto &itable = mod.instrumentTable();
        for (uint8_t id = 0; id != BaseTable::MAX_SIZE; ++id) {
            auto inst = itable[id];
            if (inst) {
                // same format as major 1
                InstHandler::deserializeInstrument(block, *inst);
            }
        }

        return FormatError::none;
    }

};

class LegacyWaveHandler : public PayloadHandler<BLOCK_ID_WAVE> {

public:
    LegacyWaveHandler() {}

    FormatError processIn(Module &mod, InputBlock &block, size_t index) {
        (void)index;

        auto &wtable = mod.waveformTable();
        for (uint8_t id = 0; id != BaseTable::MAX_SIZE; ++id) {
            auto wave = wtable[id];
            if (wave) {
                // same format as major 1
                WaveHandler::deserializeWaveform(block, *wave);
            }
        }

        return FormatError::none;

    }

};

}


FormatError deserializePayload0(Module &mod, Header &header, std::istream &stream) noexcept {

    TU::IndxHandler indx(header.current.icount, header.current.wcount);
    CommHandler comm; // the COMM block did not change so keep using the current handler
    SongHandler song;
    TU::LegacyInstHandler inst;
    TU::LegacyWaveHandler wave;
    return readPayload(mod, stream, indx, comm, song, inst, wave);

}


}

#undef TU
