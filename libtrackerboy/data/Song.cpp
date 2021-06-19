
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

int Song::rowsPerBeat() const noexcept {
    return mRowsPerBeat;
}

int Song::rowsPerMeasure() const noexcept {
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

Pattern Song::getPattern(int orderNo) {
    if (orderNo < 0 || orderNo >= mOrder.size()) {
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

TrackRow& Song::getRow(ChType ch, int order, int row) {
    auto &track = mMaster.getTrack(ch, mOrder[order][static_cast<int>(ch)]);
    return track[row];
}

TrackRow Song::getRow(ChType ch, int order, int row) const {
    auto track = mMaster.getTrack(ch, mOrder[order][static_cast<int>(ch)]);
    if (track) {
        return (*track)[row];
    } else {
        // the track does not exist, return an empty row
        return TrackRow();
    }
}

void Song::setRowsPerBeat(int rowsPerBeat) {
    if (rowsPerBeat <= 0 || rowsPerBeat >= 256) {
        throw std::invalid_argument("invalid rows per beat argument");
    }
    mRowsPerBeat = rowsPerBeat;
}

void Song::setRowsPerMeasure(int rowsPerMeasure) {
    if (rowsPerMeasure < mRowsPerBeat) {
        throw std::invalid_argument("Rows per measure must be >= rows per beat");
    } else if (rowsPerMeasure >= 256) {
        throw std::invalid_argument("Rows per measure must be < 256");
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


