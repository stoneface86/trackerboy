
#include "internal/fileformat/payload/handlers/SongHandler.hpp"
#include "internal/fileformat/payload/payload.hpp"
#include "internal/endian.hpp"
#include "internal/enumutils.hpp"

namespace trackerboy {

#define TU SongHandlerTU
namespace TU {

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

#pragma pack(pop)

}


SongHandler::SongHandler(size_t count, FormatMajor major) :
    PayloadHandler(count),
    mMajor(major)
{
}

FormatError SongHandler::processIn(Module &mod, InputBlock &block, size_t index) {

    auto song = mod.songs().get(index);

    if (mMajor > 0) {
        // starting in major 1, SONG blocks begin with the song's name
        song->setName(deserializeString(block));
    }

    // read in song settings
    TU::SongFormat songFormat;
    block.read(songFormat);
    song->setRowsPerBeat(songFormat.rowsPerBeat);
    song->setRowsPerMeasure(songFormat.rowsPerMeasure);
    song->setSpeed(songFormat.speed);

    auto &pm = song->patterns();
    auto const rowsPerTrack = unbias<uint16_t>(songFormat.rowsPerTrack);
    pm.setRowSize(rowsPerTrack);

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
        TU::TrackFormat trackFormat;
        block.read(trackFormat);
        if (trackFormat.channel > static_cast<uint8_t>(ChType::ch4)) {
            return FormatError::unknownChannel;
        }
        auto &track = pm.getTrack(static_cast<ChType>(trackFormat.channel), trackFormat.trackId);
        
        auto rowCount = unbias<size_t>(trackFormat.rows);
        if (rowCount > rowsPerTrack) {
            return FormatError::invalid;
        }

        for (size_t r = rowCount; r--; ) {
            TU::RowFormat rowFormat;
            block.read(rowFormat);
            track.replace(rowFormat.rowno, rowFormat.rowdata);
        }
    }

    return FormatError::none;

}

void SongHandler::processOut(Module const& mod, OutputBlock &block, size_t index) {

    // get the song
    auto song = mod.songs().get(index);

    // write the name
    if (mMajor > 0) {
        serializeString(block, song->name());
    }

    auto &order = song->order();
    auto &pm = song->patterns();

    TU::SongFormat songHeader;
    songHeader.rowsPerBeat = (uint8_t)song->rowsPerBeat();
    songHeader.rowsPerMeasure = (uint8_t)song->rowsPerMeasure();
    songHeader.speed = song->speed();
    songHeader.patternCount = bias(order.size());
    songHeader.rowsPerTrack = bias(pm.rowSize());
    songHeader.numberOfTracks = correctEndian((uint16_t)pm.tracks());
    block.write(songHeader);

    // write out song order
    for (auto &orderRow : order.data()) {
        block.write(orderRow);
    }

    // write out all tracks
    // iterate all channels
    for (uint8_t ch = 0; ch <= +ChType::ch4; ++ch) {
        // track iterators for the current channel
        auto begin = pm.tracksBegin(static_cast<ChType>(ch));
        auto end = pm.tracksEnd(static_cast<ChType>(ch));

        // iterate all tracks for this channel
        for (auto pair = begin; pair != end; ++pair) {
            // make sure track is non-empty, we only save non-empty tracks
            if (pair->second.rowCount() > 0) {

                TU::TrackFormat trackFormat;
                trackFormat.channel = ch;
                trackFormat.trackId = pair->first;
                trackFormat.rows = bias(pair->second.rowCount());
                block.write(trackFormat);

                // iterate all rows in this track
                uint8_t rowno = 0;
                for (auto &row : pair->second) {
                    if (!row.isEmpty()) {
                        TU::RowFormat rowFormat;
                        rowFormat.rowno = rowno;
                        rowFormat.rowdata = row;
                        block.write(rowFormat);
                    }

                    ++rowno;
                }
            }
        }
    }
}

}

#undef TU
