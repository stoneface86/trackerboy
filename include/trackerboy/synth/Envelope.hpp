
#pragma once

#include "trackerboy/gbs.hpp"

#include <cstdint>


namespace trackerboy {


class Envelope {

public:

    Envelope() noexcept;

    uint8_t readRegister() const noexcept;

    void reset() noexcept;

    void restart() noexcept;

    void writeRegister(uint8_t reg) noexcept;

    void trigger() noexcept;

    uint8_t value() const noexcept;

private:

    uint8_t mEnvelope;
    Gbs::EnvMode mEnvMode;
    uint8_t mEnvLength;

    uint8_t mEnvCounter;
    uint8_t mRegister;

};




}
