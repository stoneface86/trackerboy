
#include "trackerboy/data/Song.hpp"

#include "internal/endian.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>


namespace trackerboy {


Song::Song() :
    DataItem(),
    mMap(DEFAULT_ROWS),
    mOrder(),
    mRowsPerBeat(DEFAULT_RPB),
    mRowsPerMeasure(DEFAULT_RPM),
    mSpeed(DEFAULT_SPEED),
    mEffectCounts(DEFAULT_EFFECT_COUNTS)
{
}

Song::Song(const Song &song) :
    DataItem(song),
    mMap(song.mMap),
    mOrder(song.mOrder),
    mRowsPerBeat(song.mRowsPerBeat),
    mRowsPerMeasure(song.mRowsPerMeasure),
    mSpeed(song.mSpeed),
    mEffectCounts(song.mEffectCounts)
{
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

EffectCounts Song::effectCounts() const noexcept {
    return mEffectCounts;
}

Order& Song::order() noexcept {
    return mOrder;
}

Order const& Song::order() const noexcept {
    return mOrder;
}

PatternMap& Song::patterns() noexcept {
    return mMap;
}

PatternMap const& Song::patterns() const noexcept {
    return mMap;
}

Pattern Song::getPattern(int orderNo) {
    if (orderNo < 0 || orderNo >= mOrder.size()) {
        throw std::invalid_argument("order does not exist");
    }

    return mMap.getPattern(mOrder[orderNo]);
}

TrackRow& Song::getRow(ChType ch, int order, int row) {
    auto &track = mMap.getTrack(ch, mOrder[order][static_cast<int>(ch)]);
    return track[row];
}

TrackRow Song::getRow(ChType ch, int order, int row) const {
    auto track = mMap.getTrack(ch, mOrder[order][static_cast<int>(ch)]);
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

void Song::setEffectCounts(EffectCounts counts) {
    mEffectCounts = counts;
}

Speed Song::estimateSpeed(float tempo, float framerate) const noexcept {
    auto speedFloat = speedToTempo(tempo, mRowsPerBeat, framerate);
    auto speed = (Speed)roundf(speedFloat * (1 << SPEED_FRACTION_BITS));
    return std::clamp(speed, SPEED_MIN, SPEED_MAX);
}

float Song::tempo(float framerate) const noexcept {
    return speedToTempo(speedToFloat(mSpeed), mRowsPerBeat, framerate);
}

}


