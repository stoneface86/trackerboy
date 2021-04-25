
#include "trackerboy/data/Song.hpp"
#include "trackerboy/fileformat.hpp"

#include "internal/endian.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>


namespace trackerboy {


Song::Song() :
    DataItem(),
    mMaster(DEFAULT_ROWS),
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

void Song::reset() noexcept {

    mRowsPerBeat = DEFAULT_RPB;
    mRowsPerMeasure = DEFAULT_RPM;
    mSpeed = DEFAULT_SPEED;
    mOrder.resize(1);
    mOrder[0] = { 0 };
    mMaster.clear();
    mMaster.setRowSize(DEFAULT_ROWS);
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


Order& Song::order() noexcept {
    return mOrder;
}

Order const& Song::order() const noexcept {
    return mOrder;
}

PatternMaster& Song::patterns() noexcept {
    return mMaster;
}

PatternMaster const& Song::patterns() const noexcept {
    return mMaster;
}

Pattern Song::getPattern(uint8_t orderNo) {
    if (orderNo >= mOrder.size()) {
        throw std::invalid_argument("order does not exist");
    }

    auto &order = mOrder[orderNo];
    return mMaster.getPattern(
        order[0],
        order[1],
        order[2],
        order[3]
    );
}

TrackRow Song::getRow(ChType ch, uint8_t order, uint16_t row) const {
    auto track = mMaster.getTrack(ch, mOrder[order][static_cast<int>(ch)]);
    if (track) {
        return (*track)[row];
    } else {
        // the track does not exist, return an empty row
        return TrackRow();
    }
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


