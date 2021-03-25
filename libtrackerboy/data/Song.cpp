
#include "trackerboy/data/Song.hpp"
#include "trackerboy/fileformat.hpp"

#include "./checkedstream.hpp"
#include "internal/endian.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>


namespace trackerboy {


Song::Song() :
    DataItem(),
    mMaster(64),
    mOrder(),
    mRowsPerBeat(DEFAULT_RPB),
    mRowsPerMeasure(DEFAULT_RPM),
    mSpeed(DEFAULT_SPEED)
{
}

Song::Song(const Song &song) :
    DataItem(song),
    mMaster(song.mMaster),
    mOrder(song.mOrder),
    mRowsPerBeat(song.mRowsPerBeat),
    mRowsPerMeasure(song.mRowsPerMeasure),
    mSpeed(song.mSpeed)
{
}

Song::~Song() {

}

uint8_t Song::rowsPerBeat() const noexcept {
    return mRowsPerBeat;
}

uint8_t Song::rowsPerMeasure() const noexcept {
    return mRowsPerMeasure;
}


Speed Song::speed() const noexcept {
    return mSpeed;
}


std::vector<Order>& Song::orders() noexcept {
    return mOrder;
}

PatternMaster& Song::patterns() noexcept {
    return mMaster;
}

Pattern Song::getPattern(uint8_t orderNo) {
    if (orderNo >= mOrder.size()) {
        throw std::invalid_argument("order does not exist");
    }

    Order &order = mOrder[orderNo];
    return mMaster.getPattern(
        order.tracks[0],
        order.tracks[1],
        order.tracks[2],
        order.tracks[3]
    );
}

TrackRow Song::getRow(ChType ch, uint8_t order, uint16_t row) {
    return mMaster.getTrack(ch, mOrder[order].tracks[static_cast<int>(ch)])[row];
}

void Song::setRowsPerBeat(uint8_t rowsPerBeat) {
    if (rowsPerBeat == 0) {
        throw std::invalid_argument("Cannot have 0 rows per beat");
    }
    mRowsPerBeat = rowsPerBeat;
}

void Song::setRowsPerMeasure(uint8_t rowsPerMeasure) {
    if (rowsPerMeasure < mRowsPerBeat) {
        throw std::invalid_argument("Rows per measure must be >= rows per beat");
    }

    mRowsPerMeasure = rowsPerMeasure;
}

void Song::setSpeed(Speed speed) {
    if (speed < SPEED_MIN || speed > SPEED_MAX) {
        throw std::invalid_argument("speed out of range");
    }
    mSpeed = speed;
}

float Song::tempo(float framerate) const noexcept {
    float speed = static_cast<float>(mSpeed >> 4);
    speed += static_cast<float>(mSpeed & 0xF) / 16.0f;
    return (60.0f * framerate) / (mRowsPerBeat * speed);
}

}

// ---- SERIALIZATION ----

namespace {

#pragma pack(push, 1)

struct SongFormat {
    uint8_t rowsPerBeat;
    uint8_t rowsPerMeasure;
    uint8_t speed;
    uint8_t orderCount;         // 1-256
    uint8_t rowsPerTrack;       // 1-256
    uint16_t tracks;            // 0-1024
    // order data...
    // pattern data...

};

struct TrackFormat {
    uint8_t channel;
    uint8_t trackId;
    uint8_t rows;      // counter is less one
    // row data follows
};


struct RowFormat {
    uint8_t row;
    trackerboy::TrackRow data;
};

#pragma pack(pop)


}

namespace trackerboy {

FormatError Song::deserializeData(std::istream &stream) noexcept {

    // read in the song settings
    SongFormat songHeader;
    checkedRead(stream, &songHeader, sizeof(songHeader));

    // correct endian if needed
    songHeader.tracks = correctEndian(songHeader.tracks);

    mRowsPerBeat = songHeader.rowsPerBeat;
    mRowsPerMeasure = songHeader.rowsPerMeasure;
    if (mRowsPerMeasure < mRowsPerBeat) {
        mRowsPerMeasure = mRowsPerBeat;
    }
    mSpeed = songHeader.speed;
    
    size_t orderCount = static_cast<size_t>(songHeader.orderCount) + 1;
    mOrder.resize(orderCount);
    for (size_t i = 0; i != orderCount; ++i) {
        Order ord;
        checkedRead(stream, &ord, sizeof(ord));
        mOrder[i] = ord;
    }

    mMaster.setRowSize(static_cast<uint16_t>(songHeader.rowsPerTrack) + 1);
    for (uint16_t i = 0; i != songHeader.tracks; ++i) {
        TrackFormat trackFormat;
        checkedRead(stream, &trackFormat, sizeof(TrackFormat));
        if (trackFormat.channel > static_cast<uint8_t>(ChType::ch4)) {
            return FormatError::unknownChannel;
        }

        Track &track = mMaster.getTrack(static_cast<ChType>(trackFormat.channel), trackFormat.trackId);
        size_t rowCount = static_cast<size_t>(trackFormat.rows) + 1;
        for (uint16_t r = 0; r != rowCount; ++r) {
            RowFormat rowFormat;
            checkedRead(stream, &rowFormat, sizeof(rowFormat));
            // only byte fields in RowFormat and TrackRow so endian correction isn't needed
            track.replace(rowFormat.row, rowFormat.data);
        }
    }
    return FormatError::none;
}

FormatError Song::serializeData(std::ostream &stream) noexcept {

    auto startpos = stream.tellp();

    SongFormat songHeader;
    songHeader.rowsPerBeat = mRowsPerBeat;
    songHeader.rowsPerMeasure = mRowsPerMeasure;
    songHeader.speed = mSpeed;

    // orderCount is a byte so the count has a bias of -1
    // the size of an order can never be 0
    // [0...255] == [1...256]
    songHeader.orderCount = static_cast<uint8_t>(mOrder.size() - 1);
    songHeader.rowsPerTrack = static_cast<uint8_t>(mMaster.rowSize() - 1);
    songHeader.tracks = 0;

    checkedWrite(stream, &songHeader, sizeof(songHeader));

    // order data

    for (auto &order : mOrder) {
        // TODO: Order struct must be packed for this to work on all machines
        checkedWrite(stream, &order, sizeof(order));
    }

    // track data
    // we go through every track in the song's pattern master
    // only non-empty tracks get saved (tracks with at least 1 row set)

    uint16_t trackCounter = 0;

    // iterate all channels
    for (uint8_t ch = 0; ch <= static_cast<uint8_t>(ChType::ch4); ++ch) {
        // track iterators for the current channel
        auto begin = mMaster.tracksBegin(static_cast<ChType>(ch));
        auto end = mMaster.tracksEnd(static_cast<ChType>(ch));

        // iterate all tracks for this channel
        for (auto pair = begin; pair != end; ++pair) {
            // make sure the track isn't empty (we only serialize non-empty tracks)
            if (pair->second.rowCount() > 0) {
                // write out the track data
                TrackFormat trackFormat;
                trackFormat.channel = ch;
                trackFormat.trackId = pair->first;
                trackFormat.rows = static_cast<uint8_t>(pair->second.rowCount() - 1);
                checkedWrite(stream, &trackFormat, sizeof(trackFormat));
                uint8_t rowno = 0;
                for (auto &row : pair->second) {
                    if (row.flags) {
                        // row is non-empty, write out the row
                        RowFormat rowFormat;
                        rowFormat.row = rowno;
                        rowFormat.data = row;
                        checkedWrite(stream, &rowFormat, sizeof(rowFormat));
                    }

                    ++rowno;
                }

                ++trackCounter;
            }
        }

    }

    // go back to the song format, we need to update the track count
    auto curpos = stream.tellp();
    stream.seekp(static_cast<size_t>(startpos) + offsetof(SongFormat, tracks));
    trackCounter = correctEndian(trackCounter);
    checkedWrite(stream, &trackCounter, sizeof(trackCounter));

    stream.seekp(curpos);

    return FormatError::none;
}

}
