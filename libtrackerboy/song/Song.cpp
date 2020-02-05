
#include "trackerboy/song/Song.hpp"
#include "trackerboy/fileformat.hpp"

#include <cmath>
#include <stdexcept>


namespace trackerboy {


Song::Song() :
    mRowsPerBeat(DEFAULT_RPB),
    mTempo(DEFAULT_TEMPO),
    mSpeed(DEFAULT_SPEED)
{
}

uint8_t Song::rowsPerBeat() {
    return mRowsPerBeat;
}

float Song::tempo() {
    return mTempo;
}

float Song::actualTempo() {
    // fixed point -> floating point
    float speed = static_cast<float>(mSpeed >> 3);
    speed += static_cast<float>(mSpeed & 0x3) / 8.0f;
    return 3600.0f / (speed * mRowsPerBeat);
}

Q53 Song::speed() {
    return mSpeed;
}

Order& Song::order() {
    return mOrder;
}

std::vector<Pattern>& Song::patterns() {
    return mPatterns;
}

void Song::setRowsPerBeat(uint8_t rowsPerBeat) {
    if (rowsPerBeat == 0) {
        throw std::invalid_argument("Cannot have 0 rows per beat");
    }
    mRowsPerBeat = rowsPerBeat;
   // calcSpeed();
}

void Song::setTempo(float tempo) {
    if (tempo <= 0.0f) {
        throw std::invalid_argument("tempo most be positive and nonzero");
    }
    mTempo = tempo;
    //calcSpeed();
}

void Song::setSpeed() {
    float speed = 3600.0f / (mRowsPerBeat * mTempo);
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

void Song::setSpeed(Q53 speed) {
    if (speed == 0) {
        throw std::invalid_argument("speed must be nonzero");
    }
    mSpeed = speed;
}


//void Song::calcSpeed() {
//    float speed = 3600.0f / (mRowsPerBeat * mTempo);
//    // F5.3 so round to nearest 1/8th
//    speed = std::roundf(speed * 8) / 8;
//
//    // now convert floating point -> fixed point
//
//    // calculate the integral part
//    mSpeed = static_cast<uint8_t>(speed) << 3;
//    // calculate the fractional part
//    float junk; // we only want the fractional part
//    uint8_t fract = static_cast<uint8_t>(std::modf(speed, &junk) * 8);
//    mSpeed |= fract;
//}


}
