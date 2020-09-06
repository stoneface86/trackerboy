
#include "trackerboy/data/Song.hpp"
#include "trackerboy/fileformat.hpp"

#include <cmath>
#include <stdexcept>


namespace trackerboy {


Song::Song() :
    mMaster(64),
    mOrder(),
    mRowsPerBeat(DEFAULT_RPB),
    mTempo(DEFAULT_TEMPO),
    mMode(DEFAULT_MODE),
    mSpeed(DEFAULT_SPEED),
    DataItem()
{
    mOrder.push_back({ 0 });
}

Song::~Song() {

}

uint8_t Song::rowsPerBeat() {
    return mRowsPerBeat;
}

uint16_t Song::tempo() {
    return mTempo;
}

Speed Song::speed() {
    return mSpeed;
}

Song::Mode Song::mode() {
    return mMode;
}

std::vector<Order>& Song::orders() {
    return mOrder;
}

PatternMaster& Song::patterns() {
    return mMaster;
}

Pattern Song::getPattern(uint8_t orderNo) {
    if (orderNo >= mOrder.size()) {
        throw std::invalid_argument("order does not exist");
    }

    Order &order = mOrder[orderNo];
    return mMaster.getPattern(
        order.track1Id,
        order.track2Id,
        order.track3Id,
        order.track4Id
    );
}

void Song::setRowsPerBeat(uint8_t rowsPerBeat) {
    if (rowsPerBeat == 0) {
        throw std::invalid_argument("Cannot have 0 rows per beat");
    }
    mRowsPerBeat = rowsPerBeat;
}

void Song::setTempo(uint16_t tempo) {
    mTempo = tempo;
}

void Song::setSpeed(Speed speed) {
    if (speed < SPEED_MIN || speed > SPEED_MAX) {
        throw std::invalid_argument("speed out of range");
    }
    mSpeed = speed;
}

void Song::setMode(Mode mode) {
    mMode = mode;
}

void Song::apply(float framerate) {
    if (mMode == Mode::speed) {
        // adjust tempo setting to match the current speed

        float speed = static_cast<float>(mSpeed >> 3);
        speed += static_cast<float>(mSpeed & 0x3) / 8.0f;
        float tempo = (60.0f * framerate) / (mRowsPerBeat * speed);
        mTempo = static_cast<uint16_t>(std::roundf(tempo));
    } else {
        // adjust speed setting to match the current tempo

        float speed = (60.0f * framerate) / (mRowsPerBeat * mTempo);
        // F5.3 so round to nearest 1/8th
        speed = std::roundf(speed * 8) / 8;

        // now convert floating point -> fixed point

        // calculate the integral part
        mSpeed = static_cast<uint8_t>(speed) << 3;
        // calculate the fractional part
        float junk; // we only want the fractional part
        uint8_t fract = static_cast<uint8_t>(std::modf(speed, &junk) * 8);
        mSpeed |= fract;
    }
}

// SERIALIZATION ----

bool Song::deserializeData(std::istream &stream) noexcept {
    // TODO
    return true;
}

bool Song::serializeData(std::ostream &stream) noexcept {
    // TODO
    return true;
}

}
