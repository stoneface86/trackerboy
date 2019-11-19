
#pragma once

#include "trackerboy/pattern/Pattern.hpp"
#include "trackerboy/song/Order.hpp"


namespace trackerboy {


class Song {

public:

    static constexpr uint8_t DEFAULT_RPB = 4;
    static constexpr float DEFAULT_TEMPO = 150.0f;

    Song();

    uint8_t rowsPerBeat();

    float tempo();

    float actualTempo();

    uint8_t speed();

    Order& order();

    std::vector<Pattern>& patterns();

    void setRowsPerBeat(uint8_t rowsPerBeat);

    void setTempo(float tempo);

    void setSpeed(uint8_t speed);

private:

    void calcSpeed();

    std::vector<Pattern> mPatterns;
    Order mOrder;

    uint8_t mRowsPerBeat;
    float mTempo;

    // Speed - fixed point F5.3
    // frame timing for each row
    // 4.0:   4, 4, 4, 4, ...
    // 4.125: 4, 4, 4, 4, 4, 4, 4, 5, ...
    // 4.25:  4, 4, 4, 5, 4, 4, 4, 5, ...
    // 4.375: 4, 4, 5, 4, 4, 5, 4, 4, 5, ...
    // 4.5:   4, 5, 4, 5, ...
    // 4.675: 4, 5, 5, 4, 5, 5, 4, 5, 5, 4, ...
    // 4.75:  4, 5, 5, 5, 4, 5, 5, 5, 4, ...
    // 4.875: 4, 5, 5, 5, 5, 5, 5, 4, ...

    uint8_t mSpeed; // frames per row

};


}