
#include "internal/fileformat/payload/payload.hpp"

#include "internal/fileformat/payload/PayloadHandler.hpp"
#include "internal/fileformat/payload/handlers/CommHandler.hpp"
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


#pragma pack(push, 1)

struct SongFormat {
    uint8_t rowsPerBeat;
    uint8_t rowsPerMeasure;
    uint8_t speed;
    uint8_t patternCount;
    uint8_t rowsPerTrack;
    uint16_t numberOfTracks;
    // order data...Order[patternCount]
    // pattern data TrackFormat[numberOfTracks]
};

struct TrackFormat {
    uint8_t channel;
    uint8_t trackId;
    uint8_t rows;
    // row data: TrackRow[rows]
};

struct RowFormat {
    uint8_t rowno;
    TrackRow rowdata;
};

struct InstrumentFormat {
    uint8_t channel;
    bool envelopeEnabled;
    uint8_t envelope;
    // sequence data follows
};

struct SequenceFormat {
    uint16_t length;
    bool loopEnabled;
    uint8_t loopIndex;
    // sequence data follows uint8_t[length]
};

#pragma pack(pop)

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
        item.setName(deserializeString(block));
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

        mod.songs().get(0)->setName(deserializeString(block));

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

class SongHandler : public PayloadHandler<BLOCK_ID_SONG> {

public:
    SongHandler() { }

    FormatError processIn(Module &mod, InputBlock &block, size_t index) {
        (void)index;

        auto song = mod.songs().get(0);

        // read in song settings
        SongFormat songFormat;
        block.read(songFormat);
        song->setRowsPerBeat(songFormat.rowsPerBeat);
        song->setRowsPerMeasure(songFormat.rowsPerMeasure);
        song->setSpeed(songFormat.speed);

        auto &pm = song->patterns();
        pm.setRowSize(unbias<uint16_t>(songFormat.rowsPerTrack));

        // read in the order
        {
            std::vector<OrderRow> orderData;
            orderData.resize(unbias<size_t>(songFormat.patternCount));
            for (auto &row : orderData) {
                block.read(row);
            }
            // the order takes ownership of orderData
            song->order().setData(std::move(orderData));
        }


        // read in track data
        size_t const tracks = (size_t)correctEndian(songFormat.numberOfTracks);
        for (size_t i = 0; i != tracks; ++i) {
            TrackFormat trackFormat;
            block.read(trackFormat);
            if (trackFormat.channel > static_cast<uint8_t>(ChType::ch4)) {
                return FormatError::unknownChannel;
            }
            auto &track = pm.getTrack(static_cast<ChType>(trackFormat.channel), trackFormat.trackId);
            for (size_t r = (size_t)trackFormat.rows + 1; r--; ) {
                RowFormat rowFormat;
                block.read(rowFormat);
                track.replace(rowFormat.rowno, rowFormat.rowdata);
            }
        }

        return FormatError::none;
    }

};

class InstHandler : public PayloadHandler<BLOCK_ID_INSTRUMENT> {

public:
    InstHandler() {}

    FormatError processIn(Module &mod, InputBlock &block, size_t index) {
        (void)index;

        auto &itable = mod.instrumentTable();
        for (uint8_t id = 0; id != BaseTable::MAX_SIZE; ++id) {
            auto inst = itable[id];
            if (inst) {
                InstrumentFormat format;
                block.read(format);
                if (format.channel > +ChType::ch4) {
                    return FormatError::unknownChannel;
                }
                inst->setChannel(static_cast<ChType>(format.channel));
                inst->setEnvelopeEnable(format.envelopeEnabled);
                inst->setEnvelope(format.envelope);

                for (size_t i = 0; i < Instrument::SEQUENCE_COUNT; ++i) {
                    auto &sequence = inst->sequence(i);

                    SequenceFormat sequenceFmt;
                    block.read(sequenceFmt);
                    sequenceFmt.length = correctEndian(sequenceFmt.length);
                    if (sequenceFmt.length > Sequence::MAX_SIZE) {
                        return FormatError::invalid;
                    }

                    sequence.resize(sequenceFmt.length);
                    if (sequenceFmt.loopEnabled) {
                        sequence.setLoop(sequenceFmt.loopIndex);
                    }
                    auto &seqdata = sequence.data();
                    block.read(sequenceFmt.length, seqdata.data());
                }
            }
        }

        return FormatError::none;
    }

};

class WaveHandler : public PayloadHandler<BLOCK_ID_WAVE> {

public:
    WaveHandler() {}

    FormatError processIn(Module &mod, InputBlock &block, size_t index) {
        (void)index;

        auto &wtable = mod.waveformTable();
        for (uint8_t id = 0; id != BaseTable::MAX_SIZE; ++id) {
            auto wave = wtable[id];
            if (wave) {
                block.read(wave->data());
            }
        }

        return FormatError::none;

    }

};

}


FormatError deserializePayload0(Module &mod, Header &header, std::istream &stream) noexcept {

    TU::IndxHandler indx(header.current.icount, header.current.wcount);
    CommHandler comm; // the COMM block did not change so keep using the current handler
    TU::SongHandler song;
    TU::InstHandler inst;
    TU::WaveHandler wave;
    return readPayload(mod, stream, indx, comm, song, inst, wave);

}


}

#undef TU
