#pragma once

#include "trackerboy/Speed.hpp"


namespace trackerboy {

class Timer {

public:

    static constexpr Speed DEFAULT_PERIOD = 8;

    Timer() noexcept;

    bool active() const noexcept;

    void reset() noexcept;

    void setPeriod(Speed period) noexcept;

    bool step() noexcept;

private:

    Speed mPeriod;
    Speed mCounter;

};

}
